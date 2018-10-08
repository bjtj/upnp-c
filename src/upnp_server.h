#ifndef __UPNP_SERVER_H__
#define __UPNP_SERVER_H__

#include "public.h"
#include "upnp_models.h"
#include "http_server.h"

typedef void (*cb_on_action_request)(upnp_action_request_t *, upnp_action_response_t *);

typedef struct _upnp_server_t
{
	int done;
    int port;
	list_t * devices;
	pthread_t ssdp_receiver_thread;
	http_server_t * http_server;
	list_t * subscriptions;
	cb_on_action_request on_action_request;
} upnp_server_t;


extern upnp_server_t * upnp_create_server(int port);
extern void upnp_free_server(upnp_server_t * server);
extern void upnp_server_start(upnp_server_t * server);
extern void upnp_server_stop(upnp_server_t * server);
extern void upnp_server_register_device(upnp_server_t * server, upnp_device_t * device);
extern void upnp_server_unregister_device(upnp_server_t * server, upnp_device_t * device);

#endif
