#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__

#include "public.h"
#include "namevalue.h"
#include "http_response.h"

typedef struct _http_server_request_t
{
    const char * method;
	const char * path;
	const name_value_t * headers;
	const char * data;
	size_t data_size;
} http_server_request_t;


struct _http_server_t;

typedef http_response_t * (*http_server_handler_cb)(struct _http_server_t *,
													http_server_request_t *);

typedef struct _http_server_t
{
	int port;
	void * ptr;
	http_server_handler_cb handler_cb;
	void * cls;
} http_server_t;


extern http_server_t * create_http_server(int port, http_server_handler_cb handler_cb);
extern void free_http_server(http_server_t * server);
extern void start_http_server(http_server_t * server);
extern void stop_http_server(http_server_t * server);
extern int http_server_is_running(http_server_t * server);
extern int http_server_get_port(http_server_t * server);

#endif
