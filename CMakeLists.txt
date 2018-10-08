cmake_minimum_required(VERSION 3.5)

# project
project (upnp-c)
add_subdirectory(src)
add_subdirectory(test)
enable_testing()
add_test(NAME upnp-c-test COMMAND upnp-c-test)

# executable
add_executable (control-point src/control-point.c)
add_executable (server src/server.c)
target_link_libraries(control-point upnp-c)
