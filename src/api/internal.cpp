
#include "internal.hpp"


using namespace boost::property_tree;
using namespace couchdb;
using namespace std;


namespace plot_server {
  namespace api {
    namespace internal {

      //=============================================================

      static std::string _current_namespace = "default";
      static std::string _base_url = "http://localhost:5984/";

      //=============================================================

      static Couchdb _namespaces_couchdb( _base_url + "plots-namespaces/");
      static Couchdb _current_couchdb( _base_url + "plots_" + _current_namespace + "/");
      
      //=============================================================

      string
      set_base_url( const string& url )
      {
	string temp = _base_url;
	_base_url = url;
	_namespaces_couchdb
	  = couchdb::Couchdb( _base_url + "plots-namespaces/");
	_current_couchdb
	  = couchdb::Couchdb( _base_url + "plots_" + _current_namespace + "/");
	return temp;
      }
      
      //=============================================================

      string
      set_namespace( const string& ns )
      {
	string temp = _current_namespace;
	_current_namespace = ns;
	ptree ns_doc;
	ns_doc.put( "available", true );
	namespacesdb().try_ensure_substructure( _current_namespace, ns_doc );
	_current_couchdb
	  = couchdb::Couchdb( _base_url + "plots_" + _current_namespace + "/");
	return temp;
      }

      //=============================================================
      
      void open_currentdb( const std::string& url )
      {
	_current_couchdb = Couchdb( url );
      }

      //=============================================================

      Couchdb& currentdb()
      {
	return _current_couchdb;
      }

      //=============================================================

      Couchdb& namespacesdb()
      {
	return _namespaces_couchdb;
      }

      //=============================================================


      ptree
      fetch_plot( const string& id ) 
      {
	return currentdb().fetch( id );
      }
      
      //=============================================================

      ptree
      fetch_plot_sequence( const string& id )
      {
	return currentdb().fetch( id );
      }
      
      //=============================================================

      ptree
      fetch_data_series( const string& id )
      {
	return currentdb().fetch( id );
      }
      
      //=============================================================
      

    }

  }
}
