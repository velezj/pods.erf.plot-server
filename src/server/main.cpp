
#include <mongoose/mongoose.h>
#include <plot-server/api/plot.hpp>
#include <plot-server/plotter/plotter.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

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
    oss << "<li>" << id << "  <a href=\"plot?" << id << "\"/>" << "</li>" << std::endl;
  }
  oss << "</ol>";
  oss << "</body></html>" << std::endl;
  
  mg_write( conn, oss.str().c_str(), oss.str().size() );
  return 1;
}


static int handle_plot_serve( const std::string& plot_id,
			      struct mg_connection* conn )
{
  std::ostringstream oss;
  oss << "HTTP/1.0 200 OK\r\n\r\n";
  oss << "<html><body>" << std::endl;
  plot_server::plotter::plot( plot_id, oss );
  oss << "</body></html>" << std::endl;
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

  // ok, we have a uri beyond plot
  // so actually return a plot to the user
  return handle_plot_serve( query_string, conn );
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
