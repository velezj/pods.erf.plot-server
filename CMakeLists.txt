cmake_minimum_required(VERSION 2.6.0)

# pull in the pods macros. See cmake/pods.cmake for documentation
set(POD_NAME plot-server)
include(cmake/pods.cmake)

add_definitions( -std=c++0x -Wall -fdiagnostics-show-option )
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")

option ( USE_PEDANTIC "Turn on -pendantic mode in gcc. This will spit out *lots* of warnings from lcm :-(, but hopefully none from the rest of the code" OFF)
if( USE_PEDANTIC )
    add_definitions( -pedantic )
endif (USE_PEDANTIC)


add_library( plot-server-api-internal SHARED
  src/api/internal.cpp
)
pods_install_headers(
  src/api/internal.hpp
  DESTINATION
  plot-server/api/internal
)
pods_use_pkg_config_packages(plot-server-api-internal boost-1.54.0 erf-couchdb )
pods_install_libraries( plot-server-api-internal )
pods_install_pkg_config_file(plot-server-api-internal
    CFLAGS
    LIBS -lplot-server-api-internal
    REQUIRES boost-1.54.0 erf-couchdb
    VERSION 0.0.1)


add_library( plot-server-api SHARED
  src/api/plot.cpp
  src/api/common_plots.cpp
)
pods_install_headers(
  src/api/plot.hpp
  src/api/common_plots.hpp
  src/api/ranker.hpp
  DESTINATION
  plot-server/api
)
pods_use_pkg_config_packages(plot-server-api plot-server-api-internal)
pods_install_libraries( plot-server-api )
pods_install_pkg_config_file(plot-server-api
    CFLAGS
    LIBS -lplot-server-api
    REQUIRES plot-server-api-internal
    VERSION 0.0.1)


add_library( plot-server-plotter SHARED
  src/plotter/plotter.cpp
)
pods_install_headers(
  src/plotter/plotter.hpp
  DESTINATION
  plot-server/plotter
)
pods_use_pkg_config_packages(plot-server-plotter plot-server-api)
pods_install_libraries( plot-server-plotter )
pods_install_pkg_config_file(plot-server-plotter
    CFLAGS
    LIBS -lplot-server-plotter
    REQUIRES plot-server-api
    VERSION 0.0.1)
INSTALL(FILES src/server/js/SVGPan.js 
  DESTINATION bin )


add_executable( plot-server-server
  src/server/main.cpp
)
pods_use_pkg_config_packages(plot-server-server mongoose-4.1 plot-server-api plot-server-plotter)
pods_install_executables( plot-server-server )


add_subdirectory( test )