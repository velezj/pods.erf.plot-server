
#if !defined( __PLOT_SERVER_API_plot_HPP__ )
#define __PLOT_SERVER_API_plot_HPP__


#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <erf-couchdb/couchdb.hpp>
#include <string>
#include <vector>
#include <map>


namespace plot_server {
  namespace api {


    //===============================================================

    // Descriotion:
    // Set the current plot namespace for the followign plot commands.
    // Returns the previous namespace
    std::string
    set_namespace( const std::string& ns );


    //===============================================================

    // Description:
    // A 'data' point for plots.
    // This is a 3D point plus a mapping of string attributes by name.
    struct data_point_t
    {
    public:
      boost::property_tree::ptree attributes;

      data_point_t( const double& x, const double& y, const double& z ) 
      {
	put( "x", x );
	put( "y", y );
	put( "z", z );
      }
      data_point_t( const boost::property_tree::ptree& data )
      {
	attributes = data;
      }

      std::string operator() ( const std::string& at ) const {
	return attributes.get<std::string>( at );
      }
      template< class T >
      data_point_t& put( const std::string& at, const T& val ) {
	attributes.put( at, val );
	return *this;
      }
      template<class T>
      T get( const std::string& at ) const
      {
	return attributes.get<T>( at );
      }
      template< class T >
      T get( const std::string& at, const T& default_val ) const
      {
	return attributes.get<T>( at, default_val );
      }
      
    };

    //===============================================================

    // Description:
    // adds a data series and returns it's id
    std::string
    add_data_series
    (const std::vector<data_point_t>& data ,
     const boost::property_tree::ptree& series_config,
     const boost::optional<std::string>& title = boost::optional<std::string>(),
     const boost::optional<std::string>& id = boost::optional<std::string>() );
    

    //===============================================================

    // Description:
    // Creates a new plot and returns it's ID
    // The plot will include the given data series
    std::string
    create_plot
    (const boost::property_tree::ptree& plot_config,
     const std::vector<std::string>& data_series,
     const boost::optional<std::string>& title = boost::optional<std::string>(),
     const boost::optional<std::string>& id = boost::optional<std::string>() );
    

    //===============================================================

    // Description:
    // add a dat aseries to a plot
    void add_data_series_to_plot( const std::string& data_series_id,
				  const std::string& plot_id );

    //===============================================================

    // Description:
    // add a plot to a plot, thereby creating composite plots
    void add_plot_to_plot( const std::string& source_plot_id,
			   const std::string& target_plot_id );



    //===============================================================

    // Description:
    // Creates a new plot sequence and return it's ID
    // Also adds the given plots to the sequence in order
    std::string
    create_plot_sequence
    (const boost::property_tree::ptree& sequence_config,
     const std::vector<std::string>& plots,
     const boost::optional<std::string>& title = boost::optional<std::string>(),
     const boost::optional<std::string>& id = boost::optional<std::string>() );
    
    //===============================================================

    // Description:
    // Add a plot to a plot sequence
    void add_plot_to_plot_sequence( const std::string& plot_sequence_id,
				    const std::string& plot_id );
    
    //===============================================================

    // Description:
    // Returns a list of all known series ids
    std::vector<std::string>
    fetch_known_data_series(const boost::optional<size_t>& max_returned = boost::optional<size_t>() );

    //===============================================================

    // Description:
    // Returns a list of all known plot ids
    // or up to the given number (if provided)
    std::vector<std::string>
    fetch_known_plots(const boost::optional<size_t>& max_returned = boost::optional<size_t>() );

    //===============================================================

    // Description:
    // Returns a list of all known plot sequence ids
    std::vector<std::string>
    fetch_known_plot_sequences(const boost::optional<size_t>& max_returned = boost::optional<size_t>() );

    //===============================================================

    // Description:
    // Returns a list of all known plot namespaces
    std::vector<std::string>
    fetch_known_namespaces(const boost::optional<size_t>& max_returned = boost::optional<size_t>() );
    
    //===============================================================
    //===============================================================
    //===============================================================
    //===============================================================
    //===============================================================
    //===============================================================
    //===============================================================
    //===============================================================
    //===============================================================
    //===============================================================

    

  }
}


#endif

