#include "upnp_control_point.h"
#include "clock.h"
#include "ssdp_header.h"
#include "ssdp_receiver.h"
#include "ssdp_msearch_sender.h"
#include "upnp_usn.h"
#include "upnp_serialize.h"
#include "http_client.h"
#include "http_response.h"
#include "http_server.h"
#include "urlutil.h"
#include "networkutil.h"


static void _on_device_added(upnp_control_point_t * cp, upnp_device_t * device);
static void _on_device_removed(upnp_control_point_t * cp, upnp_device_t * device);
static void _on_event(upnp_control_point_t * cp, const char * sid, list_t * props);
static upnp_device_t * _get_device_from_ssdp(ssdp_header_t * ssdp);
static int _contains_usn(upnp_control_point_t * cp, const char * usn_str);
static void _add_device(upnp_control_point_t * cp, upnp_device_t * device);
static void _remove_device(upnp_control_point_t * cp, upnp_device_t * device);
static void * _ssdp_receiver(void * arg);
static void _ssdp_response_handler(struct sockaddr * addr, ssdp_header_t * ssdp, void * userdata);
static http_response_t * _http_server_handler(http_server_t * server, http_server_request_t * req);


upnp_control_point_t * upnp_create_control_point(int port)
{
	upnp_control_point_t * cp = (upnp_control_point_t*)malloc(sizeof(upnp_control_point_t));
	memset(cp, 0, sizeof(upnp_control_point_t));
	cp->port = port;
	cp->http_server = create_http_server(port, _http_server_handler);
	cp->http_server->cls = cp;
	return cp;
}

void upnp_free_control_point(upnp_control_point_t * cp)
{
	free_http_server(cp->http_server);
	list_clear(cp->subscriptions, (_free_cb)upnp_free_subscription);
	free(cp);
}

void upnp_control_point_start(upnp_control_point_t * cp)
{
	assert(pthread_create(&cp->ssdp_receiver_thread, NULL, &_ssdp_receiver, cp) == 0);
	start_http_server(cp->http_server);
}

void upnp_control_point_stop(upnp_control_point_t * cp)
{
	cp->done = 1;
	pthread_join(cp->ssdp_receiver_thread, NULL);
	stop_http_server(cp->http_server);
	list_clear(cp->devices, (_free_cb)upnp_free_device);
}

list_t * upnp_control_point_get_devices(upnp_control_point_t * cp)
{
	return cp->devices;
}

void upnp_control_point_resolve_expired(upnp_control_point_t * cp)
{
	list_t * lst = cp->devices;
	for (; lst; ) {
		upnp_device_t * device = (upnp_device_t*)lst->data;
		if (upnp_device_expired(device)) {
			list_t * next = lst->next;
			_remove_device(cp, device);
			lst = next;
		} else {
			lst = lst->next;
		}
	}
}

void upnp_control_point_set_on_device_added(upnp_control_point_t * cp, cb_on_device_added on_device_added)
{
	cp->on_device_added = on_device_added;
}

void upnp_control_point_set_on_device_removed(upnp_control_point_t * cp, cb_on_device_removed on_device_removed)
{
	cp->on_device_removed = on_device_removed;
}

void _on_event(upnp_control_point_t * cp, const char * sid, list_t * props)
{
	if (cp->on_event) {
		cp->on_event(sid, props);
	}
}

void upnp_control_point_set_on_event(upnp_control_point_t * cp, cb_on_event on_event)
{
	cp->on_event = on_event;
}

void upnp_control_point_send_msearch(upnp_control_point_t * cp, const char * type, int mx)
{
	unsigned long tick;
	ssdp_msearch_sender_t * sender = create_ssdp_msearch_sender();
	sender->response_handler_cb = _ssdp_response_handler;
	sender->user_data = cp;
	ssdp_send_msearch(sender, "upnp:rootdevice", mx);
	tick = tick_milli();
	while ((tick_milli() - tick) < (mx * 1000)) {
		if (ssdp_pending_msearch_sender(sender, 10) > 0) {
			ssdp_receive_ssdp_response(sender);
		}
	}
	ssdp_free_msearch_sender(sender);
}

upnp_device_t * upnp_control_point_get_device(upnp_control_point_t * cp, const char * udn)
{
	list_t * find = list_find(upnp_control_point_get_devices(cp),
                            (void*)udn,
                            (_cmp_cb)upnp_device_cmp_udn);
	if (find) {
		return (upnp_device_t*)find->data;
	}
	return NULL;
}

upnp_service_t * upnp_control_point_get_service(upnp_control_point_t * cp, const char * udn, const char * service_type)
{
	upnp_device_t * device = upnp_control_point_get_device(cp, udn);
	if (device) {
		return upnp_device_get_service(device, service_type);
	}
	return NULL;
}

list_t * upnp_control_point_get_subscriptions(upnp_control_point_t * cp)
{
	return cp->subscriptions;
}

static void _get_callback_urls(upnp_control_point_t * cp, char * urls, size_t size)
{
	char * ipv4 = get_ipv4();
	int port = cp->http_server->port;
	snprintf(urls, size, "<http://%s:%d/event>", ipv4, port);
	free(ipv4);
}

upnp_subscription_t * upnp_control_point_get_subscription(upnp_control_point_t * cp, const char * sid)
{
	list_t * lst = list_find(cp->subscriptions, (void*)sid, (_cmp_cb)upnp_subscription_cmp_sid);
	if (lst) {
		return (upnp_subscription_t*)lst->data;
	}
	return NULL;
}

void upnp_control_point_add_subscription(upnp_control_point_t * cp, upnp_subscription_t * subscription)
{
	cp->subscriptions = list_add(cp->subscriptions, subscription);
}

void upnp_control_point_remove_subscription(upnp_control_point_t * cp, upnp_subscription_t * subscription)
{
	cp->subscriptions = list_remove(cp->subscriptions, subscription, (_free_cb)upnp_free_subscription);
}


upnp_subscription_t * upnp_control_point_subscribe(upnp_control_point_t * cp, const char * url)
{
	upnp_subscription_t * subscription;
	list_t * headers = NULL;
	char callback_urls[1024] = {0,};
	http_response_t * response;
	char * sid;
	_get_callback_urls(cp, callback_urls, sizeof(callback_urls));
	headers = list_add(headers, create_name_value_with_namevalue("CALLBACK", callback_urls));
	headers = list_add(headers, create_name_value_with_namevalue("NT", "upnp:event"));
	headers = list_add(headers, create_name_value_with_namevalue("TIMEOUT", "Second-1800"));
	response = http_client_custom(url, headers, "SUBSCRIBE");
	list_clear(headers, (_free_cb)free_name_value);
	sid = http_header_get_parameter(http_response_get_header(response), "SID");
	if (sid == NULL) {
		return NULL;
	}
	subscription = upnp_create_subscription();
	upnp_subscription_set_url(subscription, url);
	upnp_subscription_set_sid(subscription, sid);
	free_http_response(response);
	upnp_control_point_add_subscription(cp, subscription);
	return subscription;
}

int upnp_control_point_unsubscribe(upnp_control_point_t * cp, const char * sid)
{
	list_t * headers = NULL;
	http_response_t * response = NULL;
	upnp_subscription_t * subscription = upnp_control_point_get_subscription(cp, sid);
	if (subscription == NULL) {
		return -1;
	}
	headers = list_add(headers, create_name_value_with_namevalue("SID", subscription->sid));
	printf("url: %s\n", subscription->url);
	response = http_client_custom(subscription->url, headers, "UNSUBSCRIBE");
	list_clear(headers, (_free_cb)free_name_value);
	free_http_response(response);
	upnp_control_point_remove_subscription(cp, subscription);
	return 0;
}


void _on_device_added(upnp_control_point_t * cp, upnp_device_t * device)
{
	if (cp->on_device_added) {
		cp->on_device_added(device);
	}
}

void _on_device_removed(upnp_control_point_t * cp, upnp_device_t * device)
{
	if (cp->on_device_removed) {
		cp->on_device_removed(device);
	}
}

void * _ssdp_receiver(void * arg)
{
	upnp_control_point_t * cp = (upnp_control_point_t*)arg;
	ssdp_receiver_t * ssdp_receiver = create_ssdp_receiver();
	while (!cp->done)
	{
		if (pending_ssdp_receiver(ssdp_receiver, 100)) {
			notify_type_e nts;
			ssdp_header_t * ssdp = receive_ssdp_header(ssdp_receiver);
			if (ssdp == NULL) {
				continue;
			}
			nts = ssdp_header_get_nts(ssdp);
			switch (nts) {
			case NTS_ALIVE: {
				if (_contains_usn(cp, ssdp_header_get_parameter(ssdp, "USN")) == 0) {
					upnp_device_t * device = _get_device_from_ssdp(ssdp);
					_add_device(cp, device);
				}
				break;
			}
			case NTS_UPDATE: {
				break;
			}
			case NTS_BYEBYE: {
				if (_contains_usn(cp, ssdp_header_get_parameter(ssdp, "USN")) == 1) {
					upnp_usn_t * usn = upnp_read_usn(ssdp_header_get_parameter(ssdp, "USN"));
					upnp_device_t * device = upnp_control_point_get_device(cp, usn->udn);
					_remove_device(cp, device);
					free(usn);
				}
				break;
			}
			default:
				break;
			}
			free_ssdp_header(ssdp);
		}
	}
	free_ssdp_receiver(ssdp_receiver);
	return NULL;
}

upnp_device_t * _get_device_from_ssdp(ssdp_header_t * ssdp)
{
	upnp_device_t * device = NULL;
	char * location = NULL;
	
	location = ssdp_header_get_parameter(ssdp, "LOCATION");
	if (location == NULL) {
		return NULL;
	}

	{
		http_response_t * response = http_client_get_dump(location, NULL);
		char * xml = response->data;
		if (xml == NULL) {
			fprintf(stderr, "get dump null\n");
			free_http_response(response);
			return NULL;
		}
		device = upnp_read_device_xml(xml);
		if (device == NULL) {
			fprintf(stderr, "read device xml null\n");
			free_http_response(response);
			return NULL;
		}
		upnp_device_set_base_url(device, location);
		free_http_response(response);
	}
	
	{
		list_t * lst = device->services;
		for (; lst; lst = lst->next) {
			upnp_service_t * service = (upnp_service_t*)lst->data;
			const char * scpd_url = upnp_service_get_scpd_url(service);
			if (scpd_url == NULL) {
				continue;
			}
			char * url = url_relative(location, scpd_url);
			http_response_t * response = http_client_get_dump(url, NULL);
			free(url);
			{
				char * xml = response->data;
				if (xml) {
					upnp_scpd_t * scpd = upnp_read_scpd_xml(xml);
					service->scpd = scpd;
				}
			}
			free_http_response(response);
		}
	}

	return device;
}

int _contains_usn(upnp_control_point_t * cp, const char * usn_str)
{
	int ret = 0;
	upnp_usn_t * usn = upnp_read_usn(usn_str);
	if (usn == NULL) {
		return 0;
	}
	
	if (list_find(cp->devices, (void*)usn->udn, (_cmp_cb)upnp_device_cmp_udn)) {
		ret = 1;
	}
	
	upnp_free_usn(usn);
	return ret;
}

void _add_device(upnp_control_point_t * cp, upnp_device_t * device)
{
	cp->devices = list_add(cp->devices, device);
	_on_device_added(cp, device);
}

void _remove_device(upnp_control_point_t * cp, upnp_device_t * device)
{
	_on_device_removed(cp, device);
	cp->devices = list_remove(cp->devices, device, (_free_cb)upnp_free_device);	
}

void _ssdp_response_handler(struct sockaddr * addr, ssdp_header_t * ssdp, void * userdata)
{
	upnp_control_point_t * cp = (upnp_control_point_t*)userdata;
	upnp_device_t * device = NULL;

	if (ssdp == NULL) {
		return;
	}

	if (_contains_usn(cp, ssdp_header_get_parameter(ssdp, "USN")) == 1) {
		return;
	}

	device = _get_device_from_ssdp(ssdp);
	if (device == NULL) {
		return;
	}
	
	cp->devices = list_add(cp->devices, device);
	_on_device_added(cp, device);
}

http_response_t * _http_server_handler(http_server_t * server, http_server_request_t * req)
{
	if (strcmp(req->path, "/event") == 0) {
		char firstline[1024] = {0,};
		http_response_t * res;
		list_t * properties;
		upnp_control_point_t * cp = server->cls;
		char * sid = http_header_get_parameter(req->header, "SID");
		if (sid == NULL) {
			return NULL;
		}
		properties = upnp_read_propertyset(req->data);
		_on_event(cp, sid, properties);
		list_clear(properties, (_free_cb)free_name_value);
		
		/* http response ok */		
		snprintf(firstline, sizeof(firstline), "HTTP/1.1 200 OK");
		res = create_http_response();
		http_response_set_firstline(res, firstline);
		return res;
	}
	
	return NULL;
}
