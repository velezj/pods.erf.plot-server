
add_executable( plot-server-test-api
  test-api.cpp )
pods_use_pkg_config_packages( plot-server-test-api plot-server-api )
pods_install_executables( plot-server-test-api )
