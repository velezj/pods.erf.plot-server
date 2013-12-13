
#include <mongoose/mongoose.h>
#include <plot-server/api/plot.hpp>
#include <plot-server/api/internal/internal.hpp>
#include <plot-server/plotter/plotter.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <boost/chrono.hpp>
#include <ctime>
#include <cstring>

static int handle_plot_uri_test( struct mg_connection *conn )
{
  std::ostringstream oss;
  oss << "HTTP/1.0 200 OK\r\n\r\nTESTING PLOT/URI handler\n";
  mg_write( conn, oss.str().c_str(), oss.str().size() );
  return 1;
}


static int handle_plot_listing( struct mg_connection* conn )
{
  std::vector<std::string> plot_ids =
    plot_server::api::fetch_known_plots();
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
    plot_server::api::internal::globaldb().try_update( plot_id, add_int );
  } else {
    add_int.push_back( std::make_pair( "config.interactive", "false" ) );
    plot_server::api::internal::globaldb().try_update( plot_id, add_int );
  }
  plot_server::plotter::plot( plot_id, oss );
  if( interactive ) {
    oss.str("");
    oss << "HTTP/1.0 302 Found\r\nLocation: localhost:8888/plot\r\n";
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
    return handle_plot_listing( conn );
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


int main( int argc, char** argv ) {


  // create a mongoose server
  struct mg_server *server = mg_create_server(NULL);
  mg_set_option(server, "document_root", ".");
  mg_set_option(server, "listening_port", "8888");
  mg_add_uri_handler(server, "/plot", &handle_plot_uri);
  for (;;) mg_poll_server(server, 1000);  // Infinite loop, Ctrl-C to stop
  mg_destroy_server(&server);
  return 0;

}
