#ifndef __UPNP_CONTROL_POINT_H__
#define __UPNP_CONTROL_POINT_H__

#include "public.h"
#include "upnp_models.h"
#include "http_server.h"

typedef void (*cb_on_device_added)(upnp_device_t *);
typedef void (*cb_on_device_removed)(upnp_device_t *);
typedef void (*cb_on_event)(const char *, list_t *);

typedef struct _upnp_control_point_t
{
    int done;
	int port;
	list_t * devices;
	pthread_t ssdp_receiver_thread;
	http_server_t * http_server;
	list_t * subscriptions;
	cb_on_device_added on_device_added;
	cb_on_device_removed on_device_removed;
	cb_on_event on_event;
} upnp_control_point_t;


extern upnp_control_point_t * upnp_create_control_point(int port);
extern void upnp_free_control_point(upnp_control_point_t * cp);
extern void upnp_control_point_start(upnp_control_point_t * cp);
extern void upnp_control_point_stop(upnp_control_point_t * cp);
extern list_t * upnp_control_point_get_devices(upnp_control_point_t * cp);
extern void upnp_control_point_send_msearch(upnp_control_point_t * cp, const char * type, int mx);
extern list_t * upnp_control_point_get_subscriptions(upnp_control_point_t * cp);
extern void upnp_control_point_set_on_device_added(upnp_control_point_t * cp, cb_on_device_added on_device_added);
extern void upnp_control_point_set_on_device_removed(upnp_control_point_t * cp, cb_on_device_removed on_device_removed);
extern void upnp_control_point_set_on_event(upnp_control_point_t * cp, cb_on_event on_event);
extern upnp_device_t * upnp_control_point_get_device(upnp_control_point_t * cp, const char * udn);
extern upnp_service_t * upnp_control_point_get_service(upnp_control_point_t * cp, const char * udn, const char * service_type);
extern upnp_subscription_t * upnp_control_point_get_subscription(upnp_control_point_t * cp, const char * sid);
extern void upnp_control_point_add_subscription(upnp_control_point_t * cp, upnp_subscription_t * subscription);
extern void upnp_control_point_remove_subscription(upnp_control_point_t * cp, upnp_subscription_t * subscription);
extern upnp_subscription_t * upnp_control_point_subscribe(upnp_control_point_t * cp, const char * url);
extern int upnp_control_point_unsubscribe(upnp_control_point_t * cp, upnp_subscription_t * subscription);

#endif
