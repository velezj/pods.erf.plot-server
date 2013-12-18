
#if !defined( __PLOT_SERVER_UTIL_color_HPP__ )
#define __PLOT_SERVER_UTIL_color_HPP__

#include <iostream>

namespace plot_server {
  namespace util {


    //==============================================================

    // Description:
    // A colorizer maps from a singe real-value to 
    // a color in a color space.
    class colorizer_t
    {
    public:
      
      // Description:
      // Create a colorizer with a range
      colorizer_t( const double& min_value,
		   const double& max_value )
	: _min_value( min_value ),
	  _max_value( max_value )
      {}

      // Description:
      // Returns the RGB triple, normalized to [0,1], for the
      // given value within the range
      void rgb( const double& arg_value,
		double& r,
		double& g,
		double& b ) const
      {
	
	// special case for a delta range => red
	if( _min_value == _max_value ) {
	  r = 1.0;
	  g = 0.0;
	  b = 0.0;
	  return;
	}
	
	// clamp value
	double value = arg_value;
	if( value < _min_value )
	  value = _min_value;
	if( value > _max_value )
	  value = _max_value;
	
	// walk through the rgb color cube blue->cyan,green,yellow->red
	r = 1.0;
	g = 1.0;
	b = 1.0;
	double range = _max_value - _min_value;
	if( value < ( _min_value + 0.25 * range ) ) {
	  r = 0;
	  g = 4.0 * ( value - _min_value ) / range;
	} else if( value < ( _min_value + 0.5 * range ) ) {
	  r = 0;
	  b = 1 + 4.0 * ( _min_value + 0.25 * range - value ) / range;
	} else if( value < ( _min_value + 0.75 * range ) ) {
	  r = 4.0 * ( value - _min_value - 0.5 * range ) / range;
	  b = 0;
	} else {
	  g = 1 + 4.0 * ( _min_value + 0.75 * range - value ) / range;
	  b = 0;
	}
      }

    protected:
      
      double _min_value, _max_value;
    };

    //==============================================================
    //==============================================================
    //==============================================================
    //==============================================================
    //==============================================================
    //==============================================================
    //==============================================================
    //==============================================================
    //==============================================================
    //==============================================================
    //==============================================================
    //==============================================================
    //==============================================================
    //==============================================================
    //==============================================================
    //==============================================================


  }
}

#endif

