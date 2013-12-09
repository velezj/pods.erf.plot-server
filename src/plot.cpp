
#include "plot.hpp"
#include <erf-couchdb/couchdb.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>


using namespace boost::property_tree;
using namespace couchdb;
using namespace std;


namespace plot_server {
  namespace api {


    namespace internal {

      //=============================================================

      static Couchdb _global_couchdb( boost::network::uri::uri("http://localhost:5984/plots-database/") );
      
      //=============================================================
      
      void open_globaldb( const boost::network::uri::uri& url )
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

    //=============================================================

    string
    add_data_series( const vector<data_point_t>& data ,
		     const ptree& series_config )
    {
      ptree doc;
      for( auto dpoint : data ) {
	doc.add_child( "data_series.data.", dpoint.attributes );
      }
      doc.put_child( "config", series_config );

      // debug write out ptree as xml
      xml_parser::write_xml( std::cout, doc );

      ptree res = internal::globaldb().save( doc );
      return res.get<string>( "id" );
    }


    //=============================================================

    void add_data_series_to_plot( const string& data_series_id,
				  const string& plot_id )
    {
      ptree plot_doc = internal::fetch_plot( plot_id );
      plot_doc.add( "plot.data_series.", data_series_id );
      ptree res = internal::globaldb().save( plot_doc, plot_id );
    }


    //=============================================================

    void add_plot_to_plot_sequence( const string& plot_id,
				    const string& plot_sequence_id )
    {
      ptree seq_doc = internal::fetch_plot_sequence( plot_sequence_id );
      seq_doc.add( "plot_sequence.plots.", plot_id );
      ptree res = internal::globaldb().save( seq_doc, plot_sequence_id );
    }

    //=============================================================

    string
    create_plot( const ptree& plot_config,
		 const vector<string>& data_series )
    {
      ptree plot_doc;
      plot_doc.put_child( "config", plot_config );
      ptree res = internal::globaldb().save( plot_doc );
      string id = res.get<string>("id");
      for( string series_id : data_series ) {
	add_data_series_to_plot( series_id, id );
      }
      return id;
    }


    //=============================================================

    string
    create_plot_sequence( const ptree& sequence_config,
			  const vector<string>& plots )
    {
      ptree seq_doc;
      seq_doc.put_child( "config", sequence_config );
      ptree res = internal::globaldb().save( seq_doc );
      string id = res.get<string>( "id" );
      for( string plot_id : plots ) {
	add_plot_to_plot_sequence( plot_id, id );
      }
      return id;
    }
    
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    //=============================================================
    



  }
}
