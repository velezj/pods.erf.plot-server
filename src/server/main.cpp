
#include <mongoose/mongoose.h>
#include <plot-server/api/plot.hpp>
#include <plot-server/api/internal/internal.hpp>
#include <plot-server/plotter/plotter.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#define BOOST_CHRONO_HEADER_ONLY
#include <boost/chrono.hpp>
#include <ctime>
#include <cstring>

// we should be getting this via our build script :-)
#ifndef USER_SHARE_DIR
#define USER_SHARE_DIR ""
#endif

static int handle_plot_uri_test( struct mg_connection *conn )
{
  std::ostringstream oss;
  oss << "HTTP/1.0 200 OK\r\n\r\nTESTING PLOT/URI handler\n";
  mg_write( conn, oss.str().c_str(), oss.str().size() );
  return 1;
}


static int handle_plot_listing( struct mg_connection* conn,
				const size_t& limit )
{
  std::vector<std::string> plot_ids =
    plot_server::api::fetch_known_plots( limit );
  std::ostringstream oss;
  oss << "HTTP/1.0 200 ok\r\n\r\n";
  oss << "<html><body>";
  oss << "<ol>" << std::endl;
  for( std::string id : plot_ids ) {
    boost::property_tree::ptree plot_doc = plot_server::api::internal::fetch_plot( id );
    oss << "<li style=\"text-align : justify\">  <a href=\"plot?plot_id=" << id << "\">" << id << "</a>";
    oss << "  (<a href=\"plot?plot_id=" << id << "&interactive=true\">interactive</a>) ";
    
    if( plot_doc.get("created", "") != "" ) {
      boost::chrono::system_clock::time_point tp;
      std::istringstream iss( plot_doc.get<std::string>("created"));
      iss >> tp;
      boost::chrono::system_clock::time_point now = 
	boost::chrono::system_clock::now();
      boost::chrono::minutes diff_min = 
	boost::chrono::duration_cast<boost::chrono::minutes>( now - tp );
      
      oss << "<div style=\"text-align : right; font-size : x-small\">";
      if( diff_min.count() < 60 ) {
	oss << "<span style=\"color : red\">[created " << diff_min.count() << " minutes ago!]</span>";
      } else {
	 
	time_t created_time = boost::chrono::system_clock::to_time_t(tp);
	oss << "[created on: " << ctime(&created_time) << "]";
	
      }
      oss << "</div>";
    }   
    oss << "</li>";
    oss << std::endl;
  }
  oss << "</ol>";
  oss << "</body></html>" << std::endl;
  
  mg_write( conn, oss.str().c_str(), oss.str().size() );
  return 1;
}


static int handle_plot_serve( const std::string& plot_id,
			      const bool interactive,
			      struct mg_connection* conn )
{
  std::ostringstream oss;
  oss << "HTTP/1.0 200 OK\r\n\r\n";
  std::vector<std::pair<std::string,std::string> > add_int;
  if( interactive ) {
    add_int.push_back( std::make_pair( "config.interactive", "true" ) );
    plot_server::api::internal::currentdb().try_update( plot_id, add_int );
  } else {
    add_int.push_back( std::make_pair( "config.interactive", "false" ) );
    plot_server::api::internal::currentdb().try_update( plot_id, add_int );
  }
  plot_server::plotter::plot( plot_id, oss );
  if( interactive ) {
    oss.str("");
    oss << "HTTP/1.0 302 Found\r\nLocation: localhost:9999/plot\r\n";
  }
  mg_write( conn, oss.str().c_str(), oss.str().size() );
  return 1;
}


static int handle_plot_uri( struct mg_connection* conn )
{
  std::string uri = conn->uri;
  std::string query_string = "";
  if( conn->query_string ) {
    query_string = conn->query_string;
  }
  
  // ok, if we have no query and the uri is *just* /plot then
  // we really only list any known plots (from couchdb :-) )
  if( uri == "/plot" && query_string == "" ) {
    return handle_plot_listing( conn, 30 );
  } 

  char plot_id[256];
  char temp_buff[16];
  bool interactive = false;
  mg_get_var( conn, "plot_id", plot_id, 255 );
  mg_get_var( conn, "interactive", temp_buff, 15 );
  if( strncmp( temp_buff, "true", 15 ) == 0 ) {
    interactive = true;
  }

  // ok, we have a uri beyond plot
  // so actually return a plot to the user
  return handle_plot_serve( plot_id, interactive, conn );
}



static int handle_namespace_listing( struct mg_connection* conn,
				     const size_t& limit )
{
  std::vector<std::string> ns_ids =
    plot_server::api::fetch_known_namespaces( limit );
  std::ostringstream oss;
  oss << "HTTP/1.0 200 ok\r\n\r\n";
  oss << "<html><body>";
  oss << "<ol>" << std::endl;
  for( std::string id : ns_ids ) {
    oss << "<li style=\"text-align : justify\">  <a href=\"namespace?namespace_id=" << id << "\">" << id << "</a>";
    oss << "</li>";
    oss << std::endl;
  }
  oss << "</ol>";
  oss << "</body></html>" << std::endl;
  
  mg_write( conn, oss.str().c_str(), oss.str().size() );
  return 1;
}


static int handle_namespace_uri( struct mg_connection* conn )
{
  std::string uri = conn->uri;
  std::string query_string = "";
  if( conn->query_string ) {
    query_string = conn->query_string;
  }
  
  // ok, if we have no query and the uri is *just* /namespace then
  // we really only list any known namespaces (from couchdb :-) )
  if( uri == "/namespace" && query_string == "" ) {
    return handle_namespace_listing( conn, 30 );
  } 

  char namespace_id[256];
  mg_get_var( conn, "namespace_id", namespace_id, 255 );

  // ok, we have a uri beyond namespace
  // so actually switch to this namespace
  std::string old_namespace
    = plot_server::api::set_namespace( namespace_id );
  
  // return a message to the user saying we switched!
  std::ostringstream oss;
  oss << "HTTP/1.0 200 OK\r\n\r\n";
  oss << "<html><body>Switched namespace to <b>" << namespace_id << "</b> from " << old_namespace << "</body></html>";
  mg_write( conn, oss.str().c_str(), oss.str().size() );
  return 1;
}


static int handle_gnuplot_uri( struct mg_connection* conn )
{
  std::string uri = conn->uri;
  std::string query_string = "";
  if( conn->query_string ) {
    query_string = conn->query_string;
  }

  // ok, find the relative file wanted
  std::string relative_filepath = query_string.substr( query_string.find( "path="), query_string.find( "&" ) ).substr( std::string("path=").size() );
  std::string full_path = std::string(USER_SHARE_DIR) + "gnuplot/5.0/js/" + relative_filepath;

  std::cout << "GNUPLOT file requested: " << relative_filepath << " (FULL= " << full_path << ")" << std::endl;

  // see if there is a file relative to the user share directory
  std::ifstream is( full_path );

  // create the response strream
  std::ostringstream oss;
  
  if( is ) {

    // found the file, serve it's content
    oss << "HTTP/1.0 200 OK\r\n\r\n";
    oss << is.rdbuf();
    
  } else {

    // no file, return a 404
    std::cout << " ** gnuplot file not found!" << std::endl;
    oss << "HTTP/1.0 404 Not Found\r\n\r\n";
  }
  
  mg_write( conn, oss.str().c_str(), oss.str().size() );  
  return 1;
}

int main( int argc, char** argv ) {

  std::cout << "Serving on port 9999, USER_SHARE_DIR=" << USER_SHARE_DIR << std::endl;

  // create a mongoose server
  struct mg_server *server = mg_create_server(NULL);
  mg_set_option(server, "listening_port", "9999");
  mg_add_uri_handler(server, "/plot", &handle_plot_uri);
  mg_add_uri_handler(server, "/namespace", &handle_namespace_uri);
  mg_add_uri_handler(server, "/gnuplot", &handle_gnuplot_uri);
  for (;;) mg_poll_server(server, 1000);  // Infinite loop, Ctrl-C to stop
  mg_destroy_server(&server);
  return 0;

}
