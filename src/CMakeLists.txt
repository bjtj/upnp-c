add_library (libupnp-c SHARED
  clock.c
  clock.h
  http_client.c
  http_client.h
  http_header.c
  http_header.h
  http_response.c
  http_response.h
  http_server.c
  http_server.h
  listutil.c
  listutil.h
  namevalue.c
  namevalue.h
  networkutil.c
  networkutil.h
  property.c
  property.h
  public.h
  ssdp.h
  ssdp_header.c
  ssdp_header.h
  ssdp_msearch_sender.c
  ssdp_msearch_sender.h
  ssdp_receiver.c
  ssdp_receiver.h
  strutil.c
  strutil.h
  upnp_action_invoke.c
  upnp_action_invoke.h
  upnp_control_point.c
  upnp_control_point.h
  upnp_models.c
  upnp_models.h
  upnp_serialize.c
  upnp_serialize.h
  upnp_server.c
  upnp_server.h
  upnp_usn.c
  upnp_usn.h
  urlutil.c
  urlutil.h)

target_include_directories(libupnp-c PRIVATE .)
install(TARGETS libupnp-c DESTINATION lib)
install(FILES clock.h
  http_client.h
  http_header.h
  http_response.h
  http_server.h
  listutil.h
  namevalue.h
  networkutil.h
  property.h
  public.h
  ssdp.h
  ssdp_header.h
  ssdp_msearch_sender.h
  ssdp_receiver.h
  strutil.h
  upnp_action_invoke.h
  upnp_control_point.h
  upnp_models.h
  upnp_serialize.h
  upnp_server.h
  upnp_usn.h
  urlutil.h
  DESTINATION include/upnp_c)


# dependencies
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules (DEP REQUIRED libmicrohttpd libcurl libxml-2.0)

# link libraries
target_link_libraries (libupnp-c ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (libupnp-c ${DEP_LIBRARIES})
target_include_directories (libupnp-c PUBLIC ${DEP_INCLUDE_DIRS})
target_compile_options (libupnp-c PUBLIC ${DEP_CFLAGS_OTHER})
