
#include "internal.hpp"


using namespace boost::property_tree;
using namespace couchdb;
using namespace std;


namespace plot_server {
  namespace api {
    namespace internal {

      //=============================================================

      static Couchdb _global_couchdb( "http://localhost:5984/plots-database/");
      
      //=============================================================
      
      void open_globaldb( const std::string& url )
      {
	_global_couchdb = Couchdb( url );
      }

      //=============================================================

      Couchdb& globaldb()
      {
	return _global_couchdb;
      }

      //=============================================================

      ptree
      fetch_plot( const string& id ) 
      {
	return globaldb().fetch( id );
      }
      
      //=============================================================

      ptree
      fetch_plot_sequence( const string& id )
      {
	return globaldb().fetch( id );
      }
      
      //=============================================================

      ptree
      fetch_data_series( const string& id )
      {
	return globaldb().fetch( id );
      }
      
      //=============================================================
      

    }

  }
}
