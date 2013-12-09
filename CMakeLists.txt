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


add_library( plot-server-api SHARED
  src/plot.cpp
)
pods_install_headers(
  src/plot.hpp
  DESTINATION
  plot-server/api
)
pods_use_pkg_config_packages(plot-server-api boost-1.54.0 erf-couchdb)
pods_install_libraries( plot-server-api )
pods_install_pkg_config_file(plot-server-api
    CFLAGS
    LIBS -lplot-server-api
    REQUIRES boost-1.54.0 erf-couchdb
    VERSION 0.0.1)


add_subdirectory( test )