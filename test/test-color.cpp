


#include <plot-server/util/color.hpp>
#include <iostream>

using namespace plot_server::util;

int main( int argc, char** argv )
{
  
  colorizer_t c( 0, 10 );
  for( int i = 0; i <= 10; ++i ) {

    double r,g,b;
    c.rgb( i, r, g, b );
    std::cout << "c(" << i << "): " << r << " " << g << " " << b << std::endl;
  }
  
}
