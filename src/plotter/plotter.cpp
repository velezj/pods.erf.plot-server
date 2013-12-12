
#include "plotter.hpp"
#include <plot-server/api/plot.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fstream>
#include <cstdlib>

using namespace plot_server::api;
using namespace boost::property_tree;

namespace plot_server {
  namespace plotter {


    //================================================================
    
    // Description:
    // Compares only the xyz of two data points and returns if 
    // the first is less than the second
    bool is_less_than_xyz( const data_point_t& a,
			   const data_point_t& b )
    {
      if( a.get( "x", 0.0 ) < b.get( "x", 0.0 ) ) {
	return true;
      }
      if( a.get( "x", 0.0 ) > b.get( "x", 0.0 ) ) {
	return false;
      }
      
      if( a.get( "y", 0.0 ) < b.get( "y", 0.0 ) ) {
	return true;
      }
      if( a.get( "y", 0.0 ) > b.get( "y", 0.0 ) ) {
	return false;
      }
      
      if( a.get( "z", 0.0 ) < b.get( "z", 0.0 ) ) {
	return true;
      }
      if( a.get( "z", 0.0 ) > b.get( "z", 0.0 ) ) {
	return false;
      }
      
      return false;
    }

    //================================================================

    // Description:
    // Find the bounds of all the data series (each given as JSON ptree)
    void find_bounds_of_all_series( const std::vector<ptree>& series,
				    data_point_t& min_point,
				    data_point_t& max_point )
    {
      bool first_point = true;
      for( ptree series_doc : series ) {
	
	// for each data point
	for( ptree::value_type data_doc : series_doc.get_child( "data_series.data" ) ) {
	
	  // if we are the first, set bounds
	  data_point_t d = data_point_t( data_doc.second );
	  if( first_point ) {
	    min_point = d;
	    max_point = d;
	    first_point = false;
	  }
	  
	  // check the point against hte bounds
	  if( is_less_than_xyz( d, min_point ) ) {
	    min_point = d;
	  }
	  if( is_less_than_xyz( max_point, d ) ) {
	    max_point = d;
	  }
	}
      }
    }


    //================================================================

    namespace backends {

      //================================================================
      
      namespace svg {
	
	void plot( const ptree& plot_doc,
		   const std::vector<ptree>& series,
		   const data_point_t& min_point,
		   const data_point_t& max_point,
		   std::ostream& out )
	{
	}
      }

      //================================================================

      namespace gnuplot {
	
	void plot( const ptree& plot_doc,
		   const std::vector<ptree>& series,
		   const data_point_t& min_point,
		   const data_point_t& max_point,
		   std::ostream& out )
	{

	  // ok, we will create a temporary data file with the
	  // series
	  std::ofstream ftemp( "temp.dat" );
	  for( ptree s : series ) {
	    for( ptree::value_type data_doc : s.get_child( "data_series.data" ) ) {
	      data_point_t d = data_point_t( data_doc.second );
	      ftemp << d.get("x", 0.0) << " "
		    << d.get("y", 0.0) << " "
		    << d.get("z", 0.0) << " ";
	      for( ptree::value_type att : d.attributes ) {
		if( att.first != "x" &&
		    att.first != "y" &&
		    att.first != "z" ) {
		  ftemp << att.second.data() << " ";
		}
	      }
	      ftemp << std::endl;
	    }
	  }
	  ftemp.close();

	  // create gnuplot command line to plot the given
	  system( "gnuplot -e 'set terminal svg size 400,400 mouse standalone enhanced' -e 'set output \"temp.svg\"' -e 'splot \"temp.dat\"'" );

	  // ok, now copy the resulting svg (temp.svg) into the output stream
	  std::ifstream fin( "temp.svg" );
	  std::copy( std::istreambuf_iterator<char>(fin),
		     std::istreambuf_iterator<char>(),
		     std::ostreambuf_iterator<char>(out) );

	}
      }


      //================================================================

      namespace threejs {
	
	void plot( const ptree& plot_doc,
		   const std::vector<ptree>& series,
		   const data_point_t& min_point,
		   const data_point_t& max_point,
		   std::ostream& out )
	{
	}
      }


      //================================================================
      //================================================================
      //================================================================
      //================================================================


    }

    //================================================================

    void plot( const std::string& plot_id,
	       std::ostream& out )
    {
      // get the plot JSON 
      ptree plot_doc = internal::fetch_plot( plot_id );

      // fetch each data series JSON document
      std::vector<ptree> series;
      for( auto node : plot_doc.get_child( "plot.data_series" ) ) {
	series.push_back( internal::fetch_data_series( node.second.data() ) );
      }

      // find hte bounds of the data series
      data_point_t min_point = data_point_t( ptree() );
      data_point_t max_point = data_point_t( ptree() );
      find_bounds_of_all_series( series, min_point, max_point );
      
      // Ok, now we have to plot each series using gnuplot
      // or the wanted backend
      std::string backend = plot_doc.get( "plot_backend", "gnuplot" );
      
      if( backend == "svg" ) {
	backends::svg::plot( plot_doc, series, min_point, max_point, out );
      } else if( backend == "gnuplot" ) {
	backends::gnuplot::plot( plot_doc, series, min_point, max_point, out );
      } else if( backend == "three.js" ) {
	backends::threejs::plot( plot_doc, series, min_point, max_point, out );
      }
      
      // // ACTULALLY: just print out the bounding box and the data points
      // out << "BOUNDS: ";
      // json_parser::write_json( out, min_point.attributes );
      // out << " , ";
      // json_parser::write_json( out, max_point.attributes );
      // out << std::endl;
      // for( auto node : series ) {
      // 	for( auto d : node.get_child( "data_series.data" ) ) {
      // 	  out << "dp: ";
      // 	  json_parser::write_json( out, d.second );
      // 	  out << std::endl;
      // 	}
      // }
    }

    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    


  }
}