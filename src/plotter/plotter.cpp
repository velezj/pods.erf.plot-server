
#include "plotter.hpp"
#include <plot-server/api/plot.hpp>
#include <plot-server/api/internal/internal.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
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
      double minx, miny, minz;
      double maxx, maxy, maxz;
      for( ptree series_doc : series ) {
	
	// for each data point
	for( ptree::value_type data_doc : series_doc.get_child( "data_series.data" ) ) {
	
	  // if we are the first, set bounds
	  data_point_t d = data_point_t( data_doc.second );
	  if( first_point ) {
	    minx = maxx = d.get("x",0.0);
	    miny = maxy = d.get("y",0.0);
	    minz = maxz = d.get("z",0.0);
	    
	    first_point = false;
	  }

	  double x = d.get("x", 0.0);
	  double y = d.get("y", 0.0);
	  double z = d.get("z", 0.0);
	  if( x < minx )
	    minx = x;
	  if( x > maxx )
	    maxx = x;
	  if( y < miny )
	    miny = y;
	  if( y > maxy )
	    maxy = y;
	  if( z < minz )
	    minz = z;
	  if( z > maxz )
	    maxz = z;
	  
	}
      }

      min_point = data_point_t( minx, miny, minz );
      max_point = data_point_t( maxx, maxy, maxz );
      
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

	std::string
	build_gnuplot_range_options( const ptree& plot_doc ) 
	{
	  std::ostringstream oss;
	  oss << "[" 
	      << plot_doc.get( "config.range.x.min",
			       "*" )
	      << ":"
	      << plot_doc.get( "config.range.x.max",
			       "*" )
	      << "] ";
	  oss << "["
	      << plot_doc.get( "config.range.y.min",
			       "*" )
	      << ":"
	      << plot_doc.get( "config.range.y.max",
			       "*" )
	      << "] ";
	  return oss.str();
	}


	std::string
	build_gnuplot_plot_options( const ptree& plot_doc ) 
	{
	  std::ostringstream oss;
	  
	  
	  // get the plot style
	  oss << " with " << plot_doc.get( "config.gnuplot.style", "linespoints" ) << " ";

	  // get plot style options
	  if( plot_doc.get_optional<std::string>( "config.gnuplot.linestyle" ) ) {
	    oss << " linestyle " << plot_doc.get( "config.gnuplot.linestyle", "" ) << " ";
	  }
	  if( plot_doc.get_optional<std::string>( "config.gnuplot.linetype" ) ) {
	    oss << " linetype " << plot_doc.get( "config.gnuplot.linetype", "" ) << " ";
	  }
	  if( plot_doc.get_optional<std::string>( "config.gnuplot.linewidth" ) ) {
	    oss << " linewidth " << plot_doc.get( "config.gnuplot.linewidth", "" ) << " ";
	  }
	  if( plot_doc.get_optional<std::string>( "config.gnuplot.linecolor" ) ) {
	    oss << " linecolor " << plot_doc.get( "config.gnuplot.linecolor", "" ) << " ";
	  }
	  if( plot_doc.get_optional<std::string>( "config.gnuplot.pointtype" ) ) {
	    oss << " pointtype " << plot_doc.get( "config.gnuplot.pointtype", "" ) << " ";
	  }
	  if( plot_doc.get_optional<std::string>( "config.gnuplot.pointsize" ) ) {
	    oss << " pointsize " << plot_doc.get( "config.gnuplot.pointsize", "" ) << " ";
	  }
	  if( plot_doc.get_optional<std::string>( "config.gnuplot.fill" ) ) {
	    oss << " fill " << plot_doc.get( "config.gnuplot.fill", "" ) << " ";
	  }
	  if( plot_doc.get_optional<std::string>( "config.gnuplot.nohidden3d" )) {
	    oss << " nohidden3d ";
	  }
	  if( plot_doc.get_optional<std::string>( "config.gnuplot.nocountours" ) ) {
	    oss << " nocountours ";
	  }
	  if( plot_doc.get_optional<std::string>( "config.gnuplot.nosurface" ) ) {
	    oss << " nosurface ";
	  }
	  if( plot_doc.get_optional<std::string>( "config.gnuplot.palette" ) ) {
	    oss << " palette ";
	  }

	  return oss.str();
	}

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

	  // create a composite name of the series in this plot
	  std::ostringstream s_title_oss;
	  
	  // ok, we will create a temporary data file with the
	  // series
	  std::string data_filename = tmpnam( NULL );
	  temp_filenames.push_back( data_filename );
	  std::ofstream ftemp( data_filename.c_str() );
	  for( ptree s : series ) {
	    for( ptree::value_type data_doc : s.get_child( "data_series.data", ptree() ) ) {
	      data_point_t d = data_point_t( data_doc.second );
	      if( !d.get( "skip", false ) ) {
		for( auto attr : wanted_attributes ) {
		  try {
		    double constant = boost::lexical_cast<double>( attr );
		    ftemp << constant << " ";
		  } catch ( boost::bad_lexical_cast& e ) {
		    ftemp << d.get(attr, 0.0) << " ";
		  }
		}
	      }
	      ftemp << std::endl;
	    }
	    if( plot_doc.get( "config.contiguous_series", false ) == false ) {
	      ftemp << std::endl; // add extra line between series
	    }
	    if( s_title_oss.str().size() > 0 ) {
	      s_title_oss << " & ";
	    }
	    s_title_oss << s.get( "config.title", 
				  s.get( "_id", "" ) );
	  }
	  ftemp.close();

	  // get the configuration for the gnuplot
	  std::string title 
	    = plot_doc.get( "config.title",
			    plot_doc.get( "_id", "plot" ) );
	  std::string series_title 
	    = plot_doc.get( "config.series_title",
			    s_title_oss.str() );
	  
	  size_t plot_width
	    = plot_doc.get( "config.width", 700 );
	  size_t plot_height
	    = plot_doc.get( "config.height", 700 );
	  std::ostringstream term_oss;
	  //term_oss << "svg size " << plot_width << "," << plot_height << " dynamic enhanced mouse standalone";
	  term_oss << "canvas standalone mousing enhanced jsdir '/gnuplot?path=' size " << plot_width << "," << plot_height;
	  
	  std::string terminal 
	    = plot_doc.get( "config.terminal",
			    term_oss.str() );

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

	  // ok, build the gnuplot plot command options given the
	  // config node of the plot_doc
	  std::string plot_options 
	    = build_gnuplot_plot_options( plot_doc );

	  // build any range information wanted
	  std::string range_options
	    = build_gnuplot_range_options( plot_doc );

	  // see if we want an "interactive" plot and if so reset some of
	  // the options and create a nice pause script
	  bool interactive = plot_doc.get( "config.interactive", false );
	  if( interactive ) {
	    terminal = "wxt persist";
	  }
	  

	  pre_out << "# start of plot " << plot_doc.get("_id","unk") << std::endl;
	  post_out << "# start of plot " << plot_doc.get("_id","unk") << std::endl;
	  // write out the gnuplot script
	  if( replot == false ) {
	    // if( plot_doc.get( "config.square", true ) ) {
	    //   pre_out << "set size square 1,1" << std::endl;
	    // }
	    pre_out << "set title \"" << title << "\"" << std::endl;
	    pre_out << "set terminal " << terminal << std::endl;
	    pre_out << "set output \"" << output_name << "\"" << std::endl;
	    
	    plot_out << plot_prefix << " " << range_options << " \"" << data_filename << "\" title \"" << series_title << "\" " << plot_options << " " << plot_postfix;
	  } else {
	    plot_out << ", \"" << data_filename << "\" " << " title \"" << series_title << "\" "  << plot_options << " " << plot_postfix;
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

	  // we want to add a special group to add teh SVGPan.js code
	  bool looking_for_end_svg = false;
	  bool looking_for_desc = false;
	  bool looking_for_viewbox = true;
	  while( fin ) {
	    std::string line;
	    std::getline( fin, line );
	    if( looking_for_viewbox &&
		line.substr( 0, 9 ) == " viewBox=" ) {
	      // remove this line from the svg
	      looking_for_viewbox = false;
	      looking_for_desc = true;
	    } else if( looking_for_desc && line.substr(0, 6 ) == "<desc>" ) {
	      // ok, add the line and then add our script and g tags
	      out << line << std::endl;
	      out << "<script xlink:href=\"SVGPan.js\"/>" << std::endl;
	      out << "<g id=\"viewport\" transform=\"translate(0,0)\">" << std::endl;
	      looking_for_desc = false;
	      looking_for_end_svg = true;
	    } else if( looking_for_end_svg &&
		       line.substr( 0, 6 ) == "</svg>" ) {
	      // close our new group
	      out << "</g>" << std::endl;
	      out << line << std::endl;
	      looking_for_end_svg = false;
	    } else {
	      out << line << std::endl;
	    }
	  }
	  
	  // std::copy( std::istreambuf_iterator<char>(fin),
	  // 	     std::istreambuf_iterator<char>(),
	  // 	     std::ostreambuf_iterator<char>(out) );
	  
	  // remove all temporary files used
	  // if( plot_doc.get( "config.interactive", false ) == false &&
	  //     plot_doc.get( "config.keep_gnuplot_files", false ) == false ) {
	  //   for( std::string fn : temp_filenames ) {
	  //     remove( fn.c_str() );
	  //   }
	  // }
	  
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
      
      namespace mathboxjs {
	
	void plot( const ptree& plot_doc,
		   const std::vector<ptree>& series,
		   const data_point_t& min_point,
		   const data_point_t& max_point,
		   std::ostream& out )
	{

	  data_point_t the_min_point = min_point;
	  data_point_t the_max_point = max_point;
	  the_min_point.put( "x" , min_point.get<double>("x") - 1.0 );
	  the_min_point.put( "y" , min_point.get<double>("y") - 1.0 );
	  the_min_point.put( "z" , min_point.get<double>("z") - 1.0 );
	  the_max_point.put( "x", max_point.get<double>("x") + 1.0 );
	  the_max_point.put( "y", max_point.get<double>("y") + 1.0 );
	  the_max_point.put( "z", max_point.get<double>("z") + 1.0 );
	  

	  std::ostringstream oss;

	  // craete the html header and dom ready func
	  oss <<
	    "<html><head><script type=\"text/javascript\" charset=\"utf-8\" src=\"MathBox-bundle.min.js\"></script>\
\
  <script type=\"text/javascript\">\
  /**\
   * Bootstrap\
   */\
  DomReady.ready(function() {\
    ThreeBox.preload([\
      'shaders/snippets.glsl.html',\
    ], function () {\
\
      var mathbox = window.mathbox = mathBox({\
        cameraControls: true,\
        cursor:         true,\
        controlClass:   ThreeBox.OrbitControls,\
        elementResize:  true,\
        fullscreen:     true,\
        screenshot:     true,\
        stats:          false,\
        scale:          1,\
      }).start();"
	    
	    // create viewport
	      <<
	    "mathbox.viewport({\
type: 'cartesian',\
range: [[" << the_min_point("x") << ", " << the_max_point("x") << "],["
	      << the_min_point("y") << ", " << the_max_point("y") << "],["
	      << the_min_point("z") << ", " << the_max_point("z") << "]],\
          scale: [1, 1, 1],\
        })\
        .camera({\
          orbit: 3.5,\
          phi: 2*3.14159/6,\
          theta: 0.3,\
        })\
        .transition(300)\
\
        .axis({\
          id: 'a',\
          axis: 0,\
          color: 0xa0a0a0,\
          ticks: 5,\
          lineWidth: 2,\
          size: .05,\
          labels: true,\
        })\
        .axis({\
          id: 'b',\
          axis: 1,\
          color: 0xa0a0a0,\
          ticks: 5,\
          lineWidth: 2,\
          size: .05,\
          zero: false,\
          labels: true,\
        })\
        .axis({\
          id: 'c',\
          axis: 2,\
          color: 0xa0a0a0,\
          ticks: 5,\
          lineWidth: 2,\
          size: .05,\
          zero: false,\
          labels: true,\
        })\
\
        .grid({\
          axis: [0, 2],\
          color: 0xc0c0c0,\
          lineWidth: 1,\
        });" << std::endl;

	  for( size_t si = 0; si < series.size(); ++si ) {
	    ptree ds = series[si];
	    ptree data = ds.get_child("data_series.data");
	    ptree::const_iterator iter;
	    std::vector<double> x,y,z;
	    for( iter = data.begin(); iter != data.end(); ++iter ) {
	      x.push_back( iter->second.get<double>("x", 0.0) );
	      y.push_back( iter->second.get<double>("y", 0.0) );
	      z.push_back( iter->second.get<double>("z", 0.0) );
	    }

	    size_t n = x.size();

	    // write out hte curve information
	    oss << "mathbox.curve({n: " << n
		<< ", domain: [0,1]"
		<< ", data: [";
	    for( size_t i = 0; i < n; ++i ) {
	      oss << "[" << x[i] << "," << y[i] << "," << z[i] << "],";
	    }
	    oss << "]"
		<< ", points: true"
		<< ", line: true"
		<< "});" << std::endl;
	  }

	  oss << "}); });" << std::endl;
	  oss << "</script> </head><body></body></html>" << std::endl;

	  out << oss.str();
	  
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
      std::string backend = plot_doc.get( "config.backend", "mathbox.js" );
      
      if( backend == "svg" ) {
	backends::svg::plot( plot_doc, series, min_point, max_point, out );
      } else if( backend == "gnuplot" ) {
	backends::gnuplot::plot( plot_doc, series, min_point, max_point, out );
      } else if( backend == "three.js" ) {
	backends::threejs::plot( plot_doc, series, min_point, max_point, out );
      } else if( backend == "mathbox.js" ) {
	backends::mathboxjs::plot( plot_doc, series, min_point, max_point, out );
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
