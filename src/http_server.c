#include <microhttpd.h>
#include "http_server.h"


static ssize_t _callback(void * cls, uint64_t pos, char * buf, size_t buf_size)
{
	size_t size_to_copy;
	http_response_t * const param = (http_response_t*)cls;

	if (pos >= param->data_size)
    {
		return MHD_CONTENT_READER_END_OF_STREAM;
    }

	if (buf_size < (param->data_size - pos)) {
		size_to_copy = buf_size;
	} else {
		size_to_copy = param->data_size - pos;
	}

	memcpy(buf, param->data + pos, size_to_copy);

	return size_to_copy;
}

static int _handler(void * cls,
					struct MHD_Connection * conn,
					const char * url,
					const char * method,
					const char * version,
					const char * upload_data,
					size_t * upload_data_size,
					void ** ptr)
{
	static int aptr;
	http_server_t * server = (http_server_t*)cls;
	if (&aptr != *ptr) {
		/* do never respond on first call */
		*ptr = &aptr;
		return MHD_YES;
    }
	*ptr = NULL;				/* reset when done */

	if (server && server->handler_cb) {
		http_server_request_t req = {.method = method,
									 .path = url,
									 .headers = NULL,
									 .data = upload_data,
									 .data_size = *upload_data_size};
		http_response_t * res;
		struct MHD_Response * _res;
		int ret;
		int status_code;
		list_t * parameters;

		res = server->handler_cb(server, &req);
		if (res == NULL) {
			return MHD_NO;
		}
		
		_res = MHD_create_response_from_callback(MHD_SIZE_UNKNOWN,
												 1024,
												 &_callback,
												 res,
												 (_free_cb)free_http_response);
		if (_res == NULL)
		{
			free_http_response(res);
			return MHD_NO;
		}

		parameters = http_response_get_header(res)->parameters;
		for (; parameters; parameters = parameters->next) {
			name_value_t * nv = (name_value_t*)parameters->data;
			MHD_add_response_header(_res, nv->name, nv->value);
		}
		
		status_code = http_response_get_status_code(res);
		ret = MHD_queue_response(conn, status_code, _res);
		MHD_destroy_response(_res);
		return ret;
	} else {
		const char * text = "Not Found\n";
		struct MHD_Response * res;
		res = MHD_create_response_from_buffer(strlen(text),
											  (void*)text,
											  MHD_RESPMEM_PERSISTENT);
		return MHD_queue_response(conn, MHD_HTTP_NOT_FOUND, res);
	}
}


http_server_t * create_http_server(int port, http_server_handler_cb handler_cb) {
	http_server_t * server = (http_server_t*)malloc(sizeof(http_server_t));
	memset(server, 0, sizeof(http_server_t));
	server->port = port;
	server->handler_cb = handler_cb;
	return server;
}

void free_http_server(http_server_t * server) {
	free(server);
}

void start_http_server(http_server_t * server) {
	server->ptr = (void*)MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
										  server->port,
										  NULL,
										  NULL,
										  &_handler,
										  server,
										  MHD_OPTION_END);
	assert(server->ptr != NULL);
}

void stop_http_server(http_server_t * server) {
	MHD_stop_daemon((struct MHD_Daemon *)server->ptr);
	server->ptr = NULL;
}

int http_server_is_running(http_server_t * server) {
	return server->ptr != NULL;
}

int http_server_get_port(http_server_t * server) {
	return server->port;
}
