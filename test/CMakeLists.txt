
add_executable( plot-server-test-api
  test-api.cpp )
pods_use_pkg_config_packages( plot-server-test-api plot-server-api )
pods_install_executables( plot-server-test-api )



add_executable( plot-server-test-common-plots
  test-common-plots.cpp )
pods_use_pkg_config_packages( plot-server-test-common-plots plot-server-api )
pods_install_executables( plot-server-test-common-plots )




add_executable( plot-server-test-plotter
  test-plotter.cpp )
pods_use_pkg_config_packages( plot-server-test-plotter plot-server-plotter )
pods_install_executables( plot-server-test-plotter )
