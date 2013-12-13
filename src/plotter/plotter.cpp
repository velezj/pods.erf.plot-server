
#include "plotter.hpp"
#include <plot-server/api/plot.hpp>
#include <plot-server/api/internal/internal.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fstream>
#include <cstdlib>
#include <cstdio>

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

	void write_plot_script( const bool& replot,
				const ptree& plot_doc,
				const std::vector<ptree>& series,
				const std::string& output_name,
				std::ostream& pre_out,
				std::ostream& plot_out,
				std::ostream& post_out,
				std::vector<std::string>& temp_filenames )
	{
	  // get the wanted columns of data_points for gnuplot
	  std::vector<std::string> wanted_attributes;
	  if( plot_doc.get_child_optional( "config.wanted_attributes" ) ) {
	    for( ptree::value_type atts : plot_doc.get_child( "config.wanted_attributes" ) ) {
	      wanted_attributes.push_back( atts.second.data() );
	    }
	  } else {
	    wanted_attributes = { "x", "y", "z" };
	  }
	  
	  // ok, we will create a temporary data file with the
	  // series
	  std::string data_filename = tmpnam( NULL );
	  temp_filenames.push_back( data_filename );
	  std::ofstream ftemp( data_filename.c_str() );
	  for( ptree s : series ) {
	    for( ptree::value_type data_doc : s.get_child( "data_series.data" ) ) {
	      data_point_t d = data_point_t( data_doc.second );
	      for( auto attr : wanted_attributes ) {
		ftemp << d.get(attr, 0.0) << " ";
	      }
	      ftemp << std::endl;
	    }
	  }
	  ftemp.close();

	  // get the configuration for the gnuplot
	  std::string title 
	    = plot_doc.get( "config.title",
			    plot_doc.get( "_id", "plot" ) );
	  std::string terminal 
	    = plot_doc.get( "config.terminal",
			    /*"svg size 400,400 mouse standalone enhanced"*/
			    "svg size 400,400 standalone" );

	  std::string plot_prefix
	    = plot_doc.get( "config.plot_prefix",
			    "splot" );
	  std::string plot_postfix
	    = plot_doc.get( "config.plot_postfix",
			    "" );
	  std::string pre_gnuplot_commands
	    = plot_doc.get( "config.pre_gnuplot_commands",
			    "" );
	  std::string extra_gnuplot_commands
	    = plot_doc.get( "config.post_gnuplot_commands",
			    "" );

	  // see if we want an "interactive" plot and if so reset some of
	  // the options and create a nice pause script
	  bool interactive = plot_doc.get( "config.interactive", false );
	  if( interactive ) {
	    terminal = "wxt persist";
	  }
	  

	  pre_out << "# start of plot " << plot_doc.get("id","unk") << std::endl;
	  post_out << "# start of plot " << plot_doc.get("id","unk") << std::endl;
	  // write out the gnuplot script
	  if( replot == false ) {
	    pre_out << "set terminal " << terminal << std::endl;
	    pre_out << "set output \"" << output_name << "\"" << std::endl;
	    plot_out << plot_prefix << " \"" << data_filename << "\" title \"" << title<< "\" "  << plot_postfix;
	  } else {
	    plot_out << ", \"" << data_filename << "\" " << " title \"" << title<< "\" "  << plot_postfix;
	  }
	  pre_out << pre_gnuplot_commands << std::endl;
	  post_out << extra_gnuplot_commands << std::endl;
	  pre_out << "# end of plot " << plot_doc.get("_id","unk") << std::endl;
	  post_out << "# end of plot " << plot_doc.get("_id","unk") << std::endl;
	  pre_out << std::endl;
	  post_out << std::endl;

	  
	  // now go through any composite plots
	  if( plot_doc.get_child_optional( "plot.composite_plots" ) ) {
	    for(ptree::value_type pc : plot_doc.get_child( "plot.composite_plots") ) {

	      // get the composite plot_doc and it's series
	      ptree composite_doc = internal::fetch_plot( pc.second.data() );
	      std::vector<ptree> composite_series;
	      for( ptree::value_type psc : composite_doc.get_child( "plot.data_series" ) ) {
		composite_series.push_back( internal::fetch_data_series( psc.second.data() ) );
	      }

	      // ok, now append it's script as a "replot"
	      write_plot_script( true,
				 composite_doc,
				 composite_series,
				 output_name,
				 pre_out,
				 plot_out,
				 post_out,
				 temp_filenames );
	      
	    }
	  }
	}

	
	void plot( const ptree& plot_doc,
		   const std::vector<ptree>& series,
		   const data_point_t& min_point,
		   const data_point_t& max_point,
		   std::ostream& out )
	{

	  // ok, create a temporary filename to creat the gnuplot script
	  std::vector<std::string> temp_filenames;
	  std::string script_filename = tmpnam( NULL );
	  temp_filenames.push_back( script_filename );
	  std::ofstream script_fout( script_filename.c_str() );

	  // create temporary files for pre and post scripts
	  std::string pre_script_fn = tmpnam( NULL );
	  std::string post_script_fn = tmpnam( NULL );
	  temp_filenames.push_back( pre_script_fn );
	  temp_filenames.push_back( post_script_fn );
	  std::ofstream pre_script_fout( pre_script_fn.c_str() );
	  std::ofstream post_script_fout( post_script_fn.c_str() );

	  // create a temporary file for the gnuplot output
	  std::string output_name = tmpnam( NULL );
	  temp_filenames.push_back( output_name );

	  // now, go through the first plot, and then every composite plot
	  // and write out their script to the script file
	  write_plot_script( false,
			     plot_doc,
			     series,
			     output_name,
			     pre_script_fout,
			     script_fout,
			     post_script_fout,
			     temp_filenames );

	  // terminate newline for plot file
	  script_fout << std::endl;

	  // append a last "set output" to script
	  post_script_fout << std::endl << "set output" << std::endl;

	  std::cout << "GNUPLOT script: " 
		    << pre_script_fn << " " 
		    << script_filename << " "
		    << post_script_fn << std::endl;

	  // now that we have the gnuplot script, run it
	  std::ostringstream oss;
	  oss << "gnuplot " << pre_script_fn << " " << script_filename << " " << post_script_fn;
	  system( oss.str().c_str() );

	  
	  // ok, now copy the resulting svg (temp.svg) into the output stream
	  std::ifstream fin( output_name.c_str() );
	  std::copy( std::istreambuf_iterator<char>(fin),
		     std::istreambuf_iterator<char>(),
		     std::ostreambuf_iterator<char>(out) );

	  // remove all temporary files used
	  if( plot_doc.get( "config.interactive", false ) == false ) {
	    for( std::string fn : temp_filenames ) {
	      remove( fn.c_str() );
	    }
	  }
	  
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
      std::string backend = plot_doc.get( "config.backend", "gnuplot" );
      
      if( backend == "svg" ) {
	backends::svg::plot( plot_doc, series, min_point, max_point, out );
      } else if( backend == "gnuplot" ) {
	backends::gnuplot::plot( plot_doc, series, min_point, max_point, out );
      } else if( backend == "three.js" ) {
	backends::threejs::plot( plot_doc, series, min_point, max_point, out );
      }
      
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
