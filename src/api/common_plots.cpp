
#include "common_plots.hpp"
#include "internal.hpp"
#include "ranker.hpp"
#include <algorithm>
#include <boost/property_tree/xml_parser.hpp>
#include <sstream>

using namespace boost::property_tree;

namespace plot_server {
  namespace api {

    struct slice_1d {
      double x;
      slice_1d( const data_point_t& data,
		const std::string& dim = "x",
		const double& default_value = 0.0)
      {
	x = data.get( dim, default_value );
      }
    };

    bool operator< ( const slice_1d& a,
		     const slice_1d& b )
    {
      return a.x < b.x;
    }

    //=================================================================

    std::string
    create_histogram_data_series
    ( const std::vector<std::string>& source_series_ids,
      const std::string& dimension,
      const size_t& num_bins,
      boost::optional<std::string> wanted_id )
    {
      
      // first, we need to grab the wanted series data
      // and get their x/y values
      std::vector<slice_1d> data;
      for( std::string sid : source_series_ids ) {
	ptree series = internal::fetch_data_series( sid );
	for( auto pc : series.get_child( "data_series.data" ) ) {
	  data.push_back( slice_1d( pc.second, dimension ) );
	}
      }
      
      // ok, now we want to get the min and max x;
      double min_x, max_x;
      if( data.size() < 1 ) {
	min_x = max_x = 0;
      } else {
	min_x = max_x = data[0].x;
      }
      for( auto s : data ) {
	if( s.x < min_x ) {
	  min_x = s.x;
	}
	if( s.x > max_x ) {
	  max_x = s.x;
	}
      }

      // now compute the bin widths
      double bin_width = (max_x - min_x) / num_bins;
      std::vector<size_t> bin_counts = std::vector<size_t>( num_bins, 0 );
      
      // collect data into bins
      for( auto s : data ) {
	size_t bin = (size_t)floor( ( s.x - min_x ) / bin_width );
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
	config.push_back( ptree::value_type("sources", ptree(id) ) );
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
      config.put( "gnuplot.style" , "boxes" );
      
      // create a new plot wit hthe given config and series
      return create_plot( config,
			  { data_series },
			  wanted_id );
      
    }
    
    //=================================================================
    
    std::string
    create_quantile_data_series
    ( const std::vector<std::string>& source_series_ids,
      const std::string& dimension,
      const std::vector<double>& wanted_quantiles,
      boost::optional<std::string> wanted_id )
    {

      // sort the given qualtile vector
      std::vector<double> quantiles( wanted_quantiles );
      std::sort( quantiles.begin(), quantiles.end() );
      
      // first, we need to grab the wanted series data
      // and get their x values
      std::vector<double> data;
      for( std::string sid : source_series_ids ) {
	ptree series = internal::fetch_data_series( sid );
	for( auto pc : series.get_child( "data_series.data" ) ) {
	  data.push_back( slice_1d( pc.second, dimension ).x );
	}
      }

      // now calculate the wanted quantiles
      std::vector<double> res_q;
      for( double q : quantiles ) {
	res_q.push_back( quantile( data, q ) );
      }
      
      // create a config for a data series
      // and set it up as a qualtile dat aseries
      ptree config;
      config.put( "type", "quantile" );

      // add the sources for this quantiles
      for( auto id : source_series_ids ) {
	config.push_back( ptree::value_type( "sources", ptree(id) ) );
      }
      
      // create teh dat aseries data (really only a single data_point!
      std::vector<data_point_t> data_s;
      data_point_t the_data = data_point_t( ptree() );
      for( size_t i = 0; i < res_q.size(); ++i ) {
	double q = res_q[i];
	double wq = quantiles[i];
	//the_data.attributes.add( "all_q.", q );
	std::ostringstream oss;
	/* TODO */
	/* may have to set precision for sized width output */
	oss << "q-" << wq;
	std::string key = oss.str();
	// we cannot have dots in ptree since they will be treated
	// as substructure access, so replace . with ,
	std::replace( key.begin(), key.end(), '.', ',' );
	the_data.put( key, q );
      }
      data_s.push_back( the_data );

      // create the deries
      std::string sid = add_data_series( data_s, config, wanted_id );
      
      return sid;
    }
    

    //=================================================================
    
    std::string
    create_box_plot
    ( const std::vector<std::string>& source_series_ids,
      const std::string& coordinate,
      const std::string& dimension,
      const std::string& factor,
      const std::string& width,
      boost::optional<std::string> wanted_id )
    {
      // create the config to make this a box plot
      ptree config;
      config.put( "backend", "gnuplot" );
      config.put( "plot_prefix", "plot" );
      config.put( "gnuplot.style" , "boxplot" );

      // make sure we get all hte compute quantials
      std::vector<std::string> atts = { coordinate, dimension, width, factor };
      for( std::string at : atts ) {
	config.push_back( ptree::value_type( "wanted_attributes", ptree(at) ) );
      }
      
      
      // create a new plot wit hthe given config and series
      return create_plot( config,
			  source_series_ids,
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



  }
}
