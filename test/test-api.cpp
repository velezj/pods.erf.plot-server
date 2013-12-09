
#include <plot-server/api/plot.hpp>
#include <iostream>

using namespace plot_server;
using namespace plot_server::api;
using namespace boost::property_tree;

int main( int argc, char** argv )
{

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
  std::string series_1_id = add_data_series( series_1, ptree() );
  std::string series_2_id = add_data_series( series_2, ptree() );

  // add a plot with only series 1  
  std::vector< std::string > plot_1_series;
  plot_1_series.push_back( series_1_id );
  std::string plot_1 = create_plot( ptree(), plot_1_series );
  
  // add a plot with both series
  std::vector< std::string > plot_2_series;
  plot_2_series.push_back( series_1_id );
  plot_2_series.push_back( series_2_id );
  std::string plot_2 = create_plot( ptree(), plot_2_series );
  

  // add a sequence with both plots
  std::vector<std::string> seq_1_plots;
  seq_1_plots.push_back( plot_1 );
  seq_1_plots.push_back( plot_2 );
  std::string seq_1 = create_plot_sequence( ptree(), seq_1_plots );
  
  return 0;
}
