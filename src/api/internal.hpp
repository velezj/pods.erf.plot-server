
#if !defined( __PLOT_SERVER_API_INTERNAL_internal_HPP__ )
#define __PLOT_SERVER_API_INTERNAL_internal_HPP__


#include <erf-couchdb/couchdb.hpp>



namespace plot_server {
  namespace api {
    namespace internal {

      //===============================================================

      // Description:
      // Set the base url for ouchdb instances, returning the previously
      // set base url
      std::string set_base_url( const std::string& url );
      
      
      //===============================================================

      // Description:
      // Sets the curent namespace for plots, returning previous one.
      // THe namespace specifies the base databse where plots are added
      std::string set_namespace( const std::string& ns );

      //===============================================================

      // Description:
      // Returns teh plot namespaces db
      couchdb::Couchdb& namespacesdb();
      
      
      //===============================================================

      // Description:
      // returns the current databse isntance (couchdb)
      couchdb::Couchdb& currentdb();

      //===============================================================
      
      // Description:
      // Opens and assigns the current databse instance
      void open_currentdb( const std::string& url );

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
