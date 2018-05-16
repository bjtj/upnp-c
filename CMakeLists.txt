cmake_minimum_required(VERSION 3.5)

# project
project (upnp-c)

# dependencies
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules (DEP REQUIRED libmicrohttpd libcurl libxml-2.0)

# library
add_library (upnp-c
  src/clock.c
  src/clock.h
  src/http_client.c
  src/http_client.h
  src/http_header.c
  src/http_header.h
  src/http_response.c
  src/http_response.h
  src/http_server.c
  src/http_server.h
  src/listutil.c
  src/listutil.h
  src/namevalue.c
  src/namevalue.h
  src/networkutil.c
  src/networkutil.h
  src/property.c
  src/property.h
  src/public.h
  src/ssdp.h
  src/ssdp_header.c
  src/ssdp_header.h
  src/ssdp_msearch_sender.c
  src/ssdp_msearch_sender.h
  src/ssdp_receiver.c
  src/ssdp_receiver.h
  src/strutil.c
  src/strutil.h
  src/upnp_action_invoke.c
  src/upnp_action_invoke.h
  src/upnp_control_point.c
  src/upnp_control_point.h
  src/upnp_models.c
  src/upnp_models.h
  src/upnp_serialize.c
  src/upnp_serialize.h
  src/upnp_usn.c
  src/upnp_usn.h
  src/urlutil.c
  src/urlutil.h)


# link libraries
target_link_libraries (upnp-c ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (upnp-c ${DEP_LIBRARIES})
target_include_directories (upnp-c PUBLIC ${DEP_INCLUDE_DIRS})
target_compile_options (upnp-c PUBLIC ${DEP_CFLAGS_OTHER})


# executable
add_executable (control-point src/control-point.c)
target_link_libraries(control-point upnp-c)
