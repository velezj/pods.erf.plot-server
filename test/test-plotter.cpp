

#include <plot-server/api/plot.hpp>
#include <plot-server/plotter/plotter.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace plot_server;
using namespace plot_server::api;
using namespace plot_server::plotter;
using namespace boost::property_tree;
namespace po = boost::program_options;
using std::string;


int main( int argn, char** argv )
{

  set_namespace( "test-plotter" );

  ptree ds1_config, ds2_config;
  ptree p1_config, p2_config;
  ptree seq_config;
  std::ostream out_stream( std::cout.rdbuf() );

  // setup the program options
  po::options_description po_desc( "test-plotter options" );
  po_desc.add_options()
    ( "help", "usage and help message")
    ( "output",
      po::value<std::string>(),
      "File to output plot to" )
    ( "ds1-config",
      po::value<std::string>(),
      "config JSON for data-series 1")
    ( "ds2-config",
      po::value<std::string>(),
      "config JSON for data-series 2")
    ( "p1-config",
      po::value<std::string>(),
      "config JSON for plot 1")
    ( "p2-config",
      po::value<std::string>(),
      "config JSON for plot 2")
    ( "seq-config",
      po::value<std::string>(),
      "config JSON for plot-sequence 1");
      

  // parse the program options
  po::variables_map po_vm;
  po::store( po::parse_command_line( argn, argv, po_desc ), po_vm );
  po::notify( po_vm );

  // handle help request
  if( po_vm.count( "help" )) {
    std::cout << po_desc << std::endl;
    return -1;
  }

  // get the configs
  std::istringstream arg_iss;
  if( po_vm.count("ds1-config") ) {
    arg_iss.str( po_vm["ds1-config"].as<std::string>() );
    json_parser::read_json( arg_iss, ds1_config );
  }
  if( po_vm.count("ds2-config") ) {
    arg_iss.str( po_vm["ds2-config"].as<std::string>() );
    json_parser::read_json( arg_iss, ds2_config );
  }
  if( po_vm.count("p1-config") ) {
    arg_iss.str( po_vm["p1-config"].as<std::string>() );
    json_parser::read_json( arg_iss, p1_config );
  }
  if( po_vm.count("p2-config") ) {
    arg_iss.str( po_vm["p2-config"].as<std::string>() );
    json_parser::read_json( arg_iss, p2_config );
  }
  if( po_vm.count("seq-config") ) {
    arg_iss.str( po_vm["seq-config"].as<std::string>() );
    json_parser::read_json( arg_iss, seq_config );
  }

  // switch stream to what we want
  std::filebuf *file_buf = NULL;
  if( po_vm.count( "output" ) ) {
    file_buf = new std::filebuf();
    file_buf->open( po_vm["output"].as<std::string>(), std::ios::out );
    out_stream.rdbuf( file_buf );
  }
  

  std::vector<data_point_t> series_1;
  for( double x = 1; x < 10.0; x *= 1.2 ) {
    series_1.push_back( data_point_t( x, x, 1.0 ) );
  }
  std::vector<data_point_t> series_2;
  for( double x = 1.1; x < 10.0; x *= 1.37 ) {
    data_point_t dp = data_point_t( -x, x, 1.0 );
    dp.put( "attributes.weight", 0.5 );
    series_2.push_back( dp );
  }
  
  // add teh data series
  std::string series_1_id = add_data_series( series_1,
					     ds1_config,
					     string("ds1"),
					     string("test-plotter.ds.1") );
  std::string series_2_id = add_data_series( series_2,
					     ds2_config,
					     string("ds2"),
					     string("test-plotter.ds.2") );

  // add a plot with only series 1  
  std::vector< std::string > plot_1_series;
  plot_1_series.push_back( series_1_id );
  std::string plot_1 = create_plot( p1_config,
				    plot_1_series,
				    string("p1"),
				    string("test-plotter.plot.1") );
  
  // add a plot with both series
  std::vector< std::string > plot_2_series;
  plot_2_series.push_back( series_1_id );
  plot_2_series.push_back( series_2_id );
  std::string plot_2 = create_plot( p2_config,
				    plot_2_series,
				    string("p2"),
				    string("test-plotter.plot.2") );
  

  // add a sequence with both plots
  std::vector<std::string> seq_1_plots;
  seq_1_plots.push_back( plot_1 );
  seq_1_plots.push_back( plot_2 );
  std::string seq_1 = create_plot_sequence( seq_config,
					    seq_1_plots,
					    string("seq1"),
					    string("test-plotter.plot-sequence.1") );

  // ok, now get the "plot" for plot_2
  plot( plot_2, out_stream );

  if( file_buf ) {
    file_buf->close();
    delete file_buf;
  }
  
  return 0;
}
