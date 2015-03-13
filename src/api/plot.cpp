
#include "plot.hpp"
#include "internal.hpp"
#include <boost/chrono.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>

using namespace boost::property_tree;
using namespace couchdb;
using namespace std;


namespace plot_server {
  namespace api {

    //=============================================================

    string
    set_namespace( const string& ns )
    {
      return internal::set_namespace( ns );
    }
    
    
    //=============================================================

    string
    add_data_series( const vector<data_point_t>& data ,
		     const ptree& series_config,
		     const boost::optional<std::string>& title,
		     const boost::optional<std::string>& wanted_id )
    {
      ptree doc;
      ptree data_doc;
      for( auto dpoint : data ) {
	data_doc.push_back( ptree::value_type("", dpoint.attributes ) );
	//doc.add_child( "data_series.data", dpoint.attributes );
      }
      doc.put_child( "data_series.data", data_doc );
      doc.put_child( "config", series_config );
      if( title ) {
	doc.put( "config.title", *title );
      }
      std::ostringstream oss;
      oss << boost::chrono::system_clock::now();
      doc.put( "created" , oss.str() );
      ptree res;
      if( wanted_id ) {
	res = internal::currentdb().try_ensure_substructure( wanted_id.get(),
							    doc );
      } else {
	res = internal::currentdb().save( doc );
      }
      return res.get<string>( "id" );
    }


    //=============================================================

    void add_data_series_to_plot_doc( ptree& plot_doc,
				      const std::string& data_series_id )
    {
      ptree ds_doc = plot_doc.get_child( "plot.data_series", ptree() );
      ds_doc.push_back( ptree::value_type("", ptree(data_series_id) ) );
      plot_doc.put_child( "plot.data_series", ds_doc );
    }


    void add_data_series_to_plot( const string& data_series_id,
				  const string& plot_id )
    {
      ptree plot_doc = internal::fetch_plot( plot_id );
      add_data_series_to_plot_doc( plot_doc,
				   data_series_id );
      ptree res = internal::currentdb().save( plot_doc, plot_id );
    }

    
    //=============================================================

    void add_plot_to_plot_doc( ptree& plot_doc,
			       const std::string& source_plot_id )
    {
      ptree cp_doc = plot_doc.get_child( "plot.composite_plots", ptree() );
      cp_doc.push_back( ptree::value_type( "", ptree(source_plot_id ) ) );
      plot_doc.put_child( "plot.composite_plots", cp_doc );
    }

    void add_plot_to_plot( const string& source_plot_id,
			   const string& target_plot_id )
    {
      ptree plot_doc = internal::fetch_plot( target_plot_id );
      add_plot_to_plot_doc( plot_doc, source_plot_id );
      ptree res = internal::currentdb().save( plot_doc, target_plot_id );
    }



    //=============================================================

    void add_plot_to_plot_sequence_doc( ptree& seq_doc,
				        const std::string& plot_id )
    {
      ptree p_doc = seq_doc.get_child( "plot_sequence.plots", ptree());
      p_doc.push_back( ptree::value_type( "", ptree( plot_id) ) );
      seq_doc.put_child( "plot_sequence.plots", p_doc );
    }

    void add_plot_to_plot_sequence( const string& plot_id,
				    const string& plot_sequence_id )
    {
      ptree seq_doc = internal::fetch_plot_sequence( plot_sequence_id );
      add_plot_to_plot_sequence_doc( seq_doc,
				     plot_id );
      ptree res = internal::currentdb().save( seq_doc, plot_sequence_id );
    }

    //=============================================================

    string
    create_plot( const ptree& plot_config,
		 const vector<string>& data_series,
		 const boost::optional<std::string>& title,
		 const boost::optional<std::string>& wanted_id )
    {
      ptree plot_doc;
      plot_doc.put_child( "config", plot_config );
      if( title ) {
	plot_doc.put( "config.title", *title );
      }
      std::ostringstream oss;
      oss << boost::chrono::system_clock::now();
      plot_doc.put( "created" , oss.str() );
      for( string series_id : data_series ) {
	add_data_series_to_plot_doc( plot_doc, series_id );
      }
      ptree res;
      if( wanted_id ) {
	res = internal::currentdb().try_ensure_substructure( wanted_id.get(),
							    plot_doc );
      } else {
	res = internal::currentdb().save( plot_doc );
      }
      string id = res.get<string>("id");
      return id;
    }


    //=============================================================

    string
    create_plot_sequence( const ptree& sequence_config,
			  const vector<string>& plots,
			  const boost::optional<std::string>& title,
			  const boost::optional<std::string>& wanted_id )
    {
      ptree seq_doc;
      seq_doc.put_child( "config", sequence_config );
      if( title ) {
	seq_doc.put( "config.title", *title );
      }
      std::ostringstream oss;
      oss << boost::chrono::system_clock::now();
      seq_doc.put( "created" , oss.str() );
      for( string plot_id : plots ) {
	add_plot_to_plot_sequence_doc( seq_doc, plot_id );
      }
      ptree res;
      if( wanted_id ) {
	res = internal::currentdb().try_ensure_substructure( wanted_id.get(), 
							    seq_doc );
      } else {
	res = internal::currentdb().save( seq_doc );
      }
      string id = res.get<string>( "id" );
      return id;
    }
    
    //=============================================================

    std::vector<std::string>
    fetch_known_data_series(const boost::optional<size_t>& max_returned)
    {
      ptree view;
      if( !max_returned ) {
	view = internal::currentdb().fetch( "_design/docs_by_type/_view/all_data_series");
      } else {
	view = internal::currentdb().fetch( string("_design/docs_by_type/_view/all_data_series") + string("?limit=") + boost::lexical_cast<std::string>(*max_returned));
      }
      std::vector<std::string> ids;
      for( ptree::value_type c : view.get_child( "rows" ) ) {
	ids.push_back( c.second.get<std::string>("id") );
      }
      return ids;
    }

    //=============================================================

    std::vector<std::string>
    fetch_known_plots( const boost::optional<size_t>& max_returned )
    {
      ptree view;
      if( !max_returned ) {
	view = internal::currentdb().fetch("_design/docs_by_type/_view/all_plots?descending=true&include_docs=false");
      } else {
	view = internal::currentdb().fetch( string("_design/docs_by_type/_view/all_plots?descending=true&include_docs=false&limit=") + boost::lexical_cast<std::string>(*max_returned));
      }
      std::vector<std::string> ids;
      for( ptree::value_type c : view.get_child( "rows" ) ) {
	ids.push_back( c.second.get<std::string>("id") );
      }
      return ids;
    }

    //=============================================================

    std::vector<std::string>
    fetch_known_plot_sequences(const boost::optional<size_t>& max_returned)
    {
      ptree view;
      if( !max_returned ) {
	view = internal::currentdb().fetch("_design/docs_by_type/_view/all_plot_sequences");
      } else {
	view = internal::currentdb().fetch( string("_design/docs_by_type/_view/all_plot_sequences") + string("?limit=") + boost::lexical_cast<std::string>(*max_returned));
      }
      std::vector<std::string> ids;
      for( ptree::value_type c : view.get_child( "rows" ) ) {
	ids.push_back( c.second.get<std::string>("id") );
      }
      return ids;
    }

    //=============================================================

    vector<string>
    fetch_known_namespaces( const boost::optional<size_t>& max_returned )
    {
      ptree view;
      if( !max_returned ) {
	view = internal::namespacesdb().fetch( "_all_docs" );
      } else {
	view = internal::namespacesdb().fetch( string("_all_docs") + string("?limit=") + boost::lexical_cast<std::string>(*max_returned) );
      }
      vector<string> ids;
      for( ptree::value_type c : view.get_child( "rows" ) ) {
	ids.push_back( c.second.get<string>("id"));
      }
      return ids;
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
    



  }
}
