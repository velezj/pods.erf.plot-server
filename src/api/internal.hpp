
#if !defined( __PLOT_SERVER_API_INTERNAL_internal_HPP__ )
#define __PLOT_SERVER_API_INTERNAL_internal_HPP__


#include <erf-couchdb/couchdb.hpp>



namespace plot_server {
  namespace api {
    namespace internal {


      //===============================================================

      // Description:
      // returns the global databse isntance (couchdb)
      couchdb::Couchdb& globaldb();

      //===============================================================
      
      // Description:
      // Opens and assigns the blobal databse instance
      void open_globaldb( const boost::network::uri::uri& url );

      //===============================================================
      
      // Description:
      // Returns the entire structure for a plot given it's id
      boost::property_tree::ptree
      fetch_plot( const std::string& id );
      
      //===============================================================
      
      // Description:
      // Returns the enture strucgure for a plot sequence given it's id
      boost::property_tree::ptree
      fetch_plot_sequence( const std::string& id );
      
      //===============================================================
      
      // Description:
      // Returns the entire structure for a data series given it's id
      boost::property_tree::ptree
      fetch_data_series( const std::string& id );


    }
  }
}


#endif
