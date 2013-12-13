
#include "common_plots.hpp"
#include "internal.hpp"

using namespace boost::property_tree;

namespace plot_server {
  namespace api {


    struct xypoint {
      double x;
      double y;
      xypoint( const ptree& data_point ) {
	x = data_point.get( "x", 0.0 );
	y = data_point.get( "y", 0.0 );
      }
    };

    //=================================================================

    std::string
    create_histogram_data_series
    ( const std::vector<std::string>& source_series_ids,
      const size_t& num_bins,
      boost::optional<std::string> wanted_id )
    {
      
      // first, we need to grab the wanted series data
      // and get their x/y values
      std::vector<xypoint> data;
      for( std::string sid : source_series_ids ) {
	ptree series = internal::fetch_data_series( sid );
	for( auto pc : series.get_child( "data_series.data" ) ) {
	  data.push_back( xypoint( pc.second ) );
	}
      }
      
      // ok, now we want to get the min and max x;
      double min_x, max_x;
      if( data.size() < 1 ) {
	min_x = max_x = 0;
      } else {
	min_x = max_x = data[0].x;
      }
      for( auto xy : data ) {
	if( xy.x < min_x ) {
	  min_x = xy.x;
	}
	if( xy.x > max_x ) {
	  max_x = xy.x;
	}
      }

      // now compute the bin widths
      double bin_width = (max_x - min_x) / num_bins;
      std::vector<size_t> bin_counts = std::vector<size_t>( num_bins, 0 );
      
      // collect data into bins
      for( auto xy : data ) {
	size_t bin = (size_t)floor( ( xy.x - min_x ) / bin_width );
	if( bin >= bin_counts.size() )
	  bin = bin_counts.size() - 1;
	bin_counts[bin] += 1;
      }

      // ok, create a new data series with the bin counts
      std::vector<data_point_t> data_points;
      for( size_t bin = 0; bin < bin_counts.size(); ++bin ) {
	double bin_x = min_x + bin * bin_width + (bin_width/2.0);
	size_t bin_y = bin_counts[ bin ];
	data_points.push_back( data_point_t( bin_x, bin_y, 0.0 ) );
      }

      // create a config for data series, reflect where the data came from
      ptree config;
      for( auto id : source_series_ids ) {
	config.add( "sources.", id );
      }
      
      // make this data series be a histogram data series
      config.put( "type", "histogram" );

      // add the series
      std::string new_id = add_data_series( data_points,
					    config,
					    wanted_id );
      
      return new_id;
    }

    //=================================================================

    std::string
    create_histogram_plot
    ( const std::string& data_series,
      boost::optional<std::string> wanted_id )
    {
      
      // create the config to make this a histogram plot
      ptree config;
      config.put( "backend", "gnuplot" );
      config.put( "plot_prefix", "plot" );
      config.put( "plot_postfix" , " with boxes" );
      
      // create a new plot wit hthe given config and series
      return create_plot( config,
			  { data_series },
			  wanted_id );
      
    }

    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================
    //=================================================================



  }
}
