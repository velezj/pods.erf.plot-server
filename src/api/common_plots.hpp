
#if !defined( __PLOT_SERVER_API_common_plots_HPP__ )
#define __PLOT_SERVER_API_common_plots_HPP__


#include "plot.hpp"


namespace plot_server {
  namespace api {

    //================================================================

    // Description:
    // Given a set of data series by id, creates a new
    // data series with a histogram of the given series.
    // The new series id is returned
    std::string
    create_histogram_data_series
    ( const std::vector<std::string>& source_series_ids,
      const std::string& dimension = "x",
      const size_t& num_bins = 100,
      boost::optional<std::string> wanted_id = boost::optional<std::string>());

    //================================================================

    // Description:
    // Given a set of data series by id, creates a new
    // data series with the wanted quantiles of the series given.
    // The new series id is returned.
    // By default, the min, 25, 75, and max quantiles are computed
    // which works for a traditional "boxplot" style information plot.
    std::string
    create_quantile_data_series
    ( const std::vector<std::string>& source_series_ids,
      const std::string& dimension = "x",
      const std::vector<double>& quantiles = std::vector<double>({0.0,.25,.75, 1.0}),
      boost::optional<std::string> wanted_id = boost::optional<std::string>());
      
    //================================================================

    // Description:
    // Creates a new histogram plot with the given single data
    // series. The data series should be a histogram data series
    // as per create_histogram_data_series.
    std::string
    create_histogram_plot
    ( const std::string& data_series,
      boost::optional<std::string> wanted_id = boost::optional<std::string>());

    //================================================================
    
    // Description:
    // Creates a boxplot from a quantile data series.
    // Returns the id of the new plot.
    std::string
    create_box_plot
    ( const std::vector<std::string>& source_series_ids,
      const std::string& coordinate = "x",
      const std::string& dimension = "y",
      const std::string& factor = "x",
      const std::string& width = "width",
      boost::optional<std::string> wanted_id = boost::optional<std::string>());
    
    //================================================================
    //================================================================
    //================================================================
    //================================================================
    //================================================================


  }
}

#endif

