#include "upnp_server.h"
#include "ssdp_header.h"
#include "ssdp_receiver.h"
#include "ssdp_msearch_sender.h"

static void * _ssdp_receiver(void * arg);
static http_response_t * _http_server_handler(http_server_t * server, http_server_request_t * req);

upnp_server_t * upnp_create_server(int port) {
	upnp_server_t * server = (upnp_server_t*)malloc(sizeof(upnp_server_t));
	memset(server, 0, sizeof(upnp_server_t));
	server->port = port;
	server->http_server = create_http_server(port, _http_server_handler);
	server->http_server->cls = server;
	return server;
}

void upnp_free_server(upnp_server_t * server) {
	free_http_server(server->http_server);
	list_clear(server->subscriptions, (_free_cb)upnp_free_subscription);
	free(server);
}

void upnp_server_start(upnp_server_t * server) {
	assert(pthread_create(&server->ssdp_receiver_thread, NULL, &_ssdp_receiver, server) == 0);
	start_http_server(server->http_server);
}

void upnp_server_stop(upnp_server_t * server) {
	server->done = 1;
	pthread_join(server->ssdp_receiver_thread, NULL);
	stop_http_server(server->http_server);
	list_clear(server->devices, (_free_cb)upnp_free_device);
}

void upnp_server_register_device(upnp_server_t * server, upnp_device_t * device) {
}

void upnp_server_unregister_device(upnp_server_t * server, upnp_device_t * device) {
}

void * _ssdp_receiver(void * arg)
{
	upnp_server_t * cp = (upnp_server_t*)arg;
	ssdp_receiver_t * ssdp_receiver = create_ssdp_receiver();
	while (!cp->done)
	{
		if (pending_ssdp_receiver(ssdp_receiver, 100)) {
			ssdp_type_e type;
			ssdp_header_t * ssdp = receive_ssdp_header(ssdp_receiver);
			if (ssdp == NULL) {
				continue;
			}
			type = ssdp_header_get_type(ssdp);
			switch (type) {
			case SSDP_MSEARCH:
				/* gather usn list && send datagram */
				break;
			default:
				break;
			}
			free_ssdp_header(ssdp);
		}
	}
	free_ssdp_receiver(ssdp_receiver);
	return NULL;
}

http_response_t * _http_server_handler(http_server_t * server, http_server_request_t * req)
{
	if (ends_with(req->path, "device.xml")) {
		char firstline[1024] = {0,};
		http_response_t * res;

		/* http response ok */		
		snprintf(firstline, sizeof(firstline), "HTTP/1.1 200 OK");
		res = create_http_response();
		http_response_set_firstline(res, firstline);
		return res;
	} else {
		
	}
	
	return NULL;
}
