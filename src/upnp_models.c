#include "upnp_models.h"
#include "upnp_usn.h"
#include "clock.h"


/* device */

upnp_device_t * upnp_create_device(void)
{
	upnp_device_t * device = (upnp_device_t*)malloc(sizeof(upnp_device_t));
	memset(device, 0, sizeof(upnp_device_t));
	upnp_device_update_tick(device);
	return device;
}

void upnp_free_device(upnp_device_t * device)
{
	if (device == NULL) {
		return;
	}
	free(device->base_url);
	list_clear(device->properties, (_free_cb)free_property);
	list_clear(device->services, (_free_cb)upnp_free_service);
	list_clear(device->embedded_devices, (_free_cb)upnp_free_device);
	free(device);
}

void upnp_device_update_tick(upnp_device_t * device) {
	device->tick = tick_milli();
}

void upnp_device_set_timeout(upnp_device_t * device, unsigned long timeout)
{
	device->timeout = timeout;
}

int upnp_device_expired(upnp_device_t * device)
{
	return ((tick_milli() - device->tick) >= device->timeout);
}

char * upnp_device_get_base_url(upnp_device_t * device)
{
	return device->base_url;
}

void upnp_device_set_base_url(upnp_device_t * device, const char * base_url)
{
	free(device->base_url);
	device->base_url = strdup(base_url);
}

int upnp_device_cmp_udn(upnp_device_t * device, const char * udn)
{
	return strcmp(upnp_device_get_udn(device), udn);
}

const char * upnp_device_get_udn(upnp_device_t * device)
{
	property_t * prop = property_get(device->properties, "UDN");
	if (prop) {
		return property_get_value(prop);
	}
	return NULL;
}

const char * upnp_device_get_friendlyname(upnp_device_t * device)
{
	property_t * prop = property_get(device->properties, "friendlyName");
	if (prop) {
		return property_get_value(prop);
	}
	return NULL;
}

const char * upnp_device_get_device_type(upnp_device_t * device)
{
	property_t * prop = property_get(device->properties, "deviceType");
	if (prop) {
		return property_get_value(prop);
	}
	return NULL;
}

void upnp_device_add_child_device(upnp_device_t * device, upnp_device_t * child_device)
{
	device->embedded_devices = list_add(device->embedded_devices, child_device);
}

void upnp_device_set_udn(upnp_device_t * device, const char * udn)
{
	device->properties = property_put(device->properties, "UDN", udn);
	list_iter(device->embedded_devices, (void*)udn, (_iter_cb)upnp_device_set_udn);
}

void upnp_device_set_scpd_url(upnp_device_t * device, const char * url)
{
	list_iter(device->services, (void*)url, (_iter_cb)upnp_service_set_scpd_url);
	list_iter(device->embedded_devices, (void*)url, (_iter_cb)upnp_device_set_scpd_url);
}

void upnp_device_set_control_url(upnp_device_t * device, const char * url)
{
	list_iter(device->services, (void*)url, (_iter_cb)upnp_service_set_control_url);
	list_iter(device->embedded_devices, (void*)url, (_iter_cb)upnp_device_set_control_url);
}

void upnp_device_set_subscribe_url(upnp_device_t * device, const char * url)
{
	list_iter(device->services, (void*)url, (_iter_cb)upnp_service_set_subscribe_url);
	list_iter(device->embedded_devices, (void*)url, (_iter_cb)upnp_device_set_subscribe_url);
}

upnp_service_t * upnp_device_get_service(upnp_device_t * device, const char * type)
{
	list_t * find;
	find = list_find(device->services, (void*)type, (_cmp_cb)upnp_service_cmp_type);
	if (find) {
		return (upnp_service_t*)find->data;
	}
	find = list_find(device->embedded_devices, (void*)type, (_cmp_cb)upnp_device_get_service);
	if (find) {
		return (upnp_service_t*)find->data;
	}
	return NULL;
}

list_t * upnp_device_get_all_usns(upnp_device_t * device)
{
	list_t * usn_list = NULL;
	const char * udn = upnp_device_get_udn(device);
	const char * device_type = upnp_device_get_device_type(device);
	usn_list = list_add(usn_list, (void*)upnp_create_usn_with_init(udn, device_type));
	list_t * service_node = device->services;
	while (service_node) {
		upnp_service_t * service = (upnp_service_t*)service_node->data;
		const char * service_type = upnp_service_get_type(service);
		usn_list = list_add(usn_list, upnp_create_usn_with_init(udn, service_type));
		service_node = service_node->next;
	}

	list_t * device_node = device->embedded_devices;
	while (device_node) {
		upnp_device_t * embedded_device = (upnp_device_t*)device_node->data;
		list_t * usns = upnp_device_get_all_usns(embedded_device);
		list_t * usn = usns;
		for (; usn; usn = usn->next) {
			upnp_usn_set_udn(((upnp_usn_t*)usn->data), udn);
		}
		usn_list = list_append_list(usn_list, usns);
		device_node = device_node->next;
	}
	
	return usn_list;
}

upnp_device_t * upnp_device_get_device_with_type(upnp_device_t * device, const char * type)
{
	const char * device_type = upnp_device_get_device_type(device);
	if (strcmp(device_type, type) == 0) {
		return device;
	}

	list_t * embedded_device_node = device->embedded_devices;
	for (; embedded_device_node; embedded_device_node = embedded_device_node->next) {
		upnp_device_t * embedded_device = (upnp_device_t*)embedded_device_node->data;
		upnp_device_t * found = upnp_device_get_device_with_type(embedded_device, type);
		if (found) {
			return found;
		}
	}

	return NULL;
}

upnp_service_t * upnp_device_get_service_with_type(upnp_device_t * device, const char * type)
{
	list_t * service = device->services;
	for (; service; service = service->next) {
		const char * service_type = upnp_service_get_type((upnp_service_t*)service->data);
		if (strcmp(service_type, type) == 0) {
			return (upnp_service_t*)service->data;
		}
	}

	list_t * embedded_device_node = device->embedded_devices;
	for (; embedded_device_node; embedded_device_node = embedded_device_node->next) {
		upnp_device_t * embedded_device = (upnp_device_t*)embedded_device_node->data;
		upnp_service_t * service = upnp_device_get_service_with_type(embedded_device, type);
		if (service) {
			return service;
		}
	}
	return NULL;
}

upnp_service_t * upnp_device_get_service_with_scpdurl(upnp_device_t * device, const char * scpdurl)
{
	list_t * service = device->services;
	for (; service; service = service->next) {
		const char * service_type = upnp_service_get_scpd_url((upnp_service_t*)service->data);
		if (strcmp(service_type, scpdurl) == 0) {
			return (upnp_service_t*)service->data;
		}
	}

	list_t * embedded_device_node = device->embedded_devices;
	for (; embedded_device_node; embedded_device_node = embedded_device_node->next) {
		upnp_device_t * embedded_device = (upnp_device_t*)embedded_device_node->data;
		upnp_service_t * service = upnp_device_get_service_with_scpdurl(embedded_device, scpdurl);
		if (service) {
			return service;
		}
	}
	return NULL;
}

upnp_service_t * upnp_device_get_service_with_controlurl(upnp_device_t * device, const char * controlurl)
{
	list_t * service = device->services;
	for (; service; service = service->next) {
		const char * service_type = upnp_service_get_scpd_url((upnp_service_t*)service->data);
		if (strcmp(service_type, controlurl) == 0) {
			return (upnp_service_t*)service->data;
		}
	}

	list_t * embedded_device_node = device->embedded_devices;
	for (; embedded_device_node; embedded_device_node = embedded_device_node->next) {
		upnp_device_t * embedded_device = (upnp_device_t*)embedded_device_node->data;
		upnp_service_t * service = upnp_device_get_service_with_controlurl(embedded_device, controlurl);
		if (service) {
			return service;
		}
	}
	return NULL;
}

upnp_service_t * upnp_device_get_service_with_eventsuburl(upnp_device_t * device, const char * eventsuburl)
{
	list_t * service = device->services;
	for (; service; service = service->next) {
		const char * service_type = upnp_service_get_scpd_url((upnp_service_t*)service->data);
		if (strcmp(service_type, eventsuburl) == 0) {
			return (upnp_service_t*)service->data;
		}
	}

	list_t * embedded_device_node = device->embedded_devices;
	for (; embedded_device_node; embedded_device_node = embedded_device_node->next) {
		upnp_device_t * embedded_device = (upnp_device_t*)embedded_device_node->data;
		upnp_service_t * service = upnp_device_get_service_with_eventsuburl(embedded_device, eventsuburl);
		if (service) {
			return service;
		}
	}
	return NULL;
}

/* service */

upnp_service_t * upnp_create_service(void)
{
	upnp_service_t * service = (upnp_service_t*)malloc(sizeof(upnp_service_t));
	memset(service, 0, sizeof(upnp_service_t));
	return service;
}

void upnp_free_service(upnp_service_t * service)
{
	if (service == NULL) {
		return;
	}
	list_clear(service->properties, (_free_cb)free_property);
	upnp_free_scpd(service->scpd);
	free(service);
}


int upnp_service_cmp_type(upnp_service_t * service, const char * type)
{
	return strcmp(upnp_service_get_type(service), type);
}

int upnp_service_cmp_id(upnp_service_t * service, const char * id)
{
	return strcmp(upnp_service_get_id(service), id);
}

int upnp_service_cmp_scpd_url(upnp_service_t * service, const char * url)
{
	return strcmp(upnp_service_get_scpd_url(service), url);
}

int upnp_service_cmp_control_url(upnp_service_t * service, const char * url)
{
	return strcmp(upnp_service_get_control_url(service), url);
}

int upnp_service_cmp_subscribe_url(upnp_service_t * service, const char * url)
{
	return strcmp(upnp_service_get_subscribe_url(service), url);
}

const char * upnp_service_get_type(upnp_service_t * service)
{
	property_t * prop = property_get(service->properties, "serviceType");
	if (prop) {
		return property_get_value(prop);
	}
	return NULL;
}

const char * upnp_service_get_id(upnp_service_t * service)
{
	property_t * prop =  property_get(service->properties, "serviceId");
	if (prop) {
		return property_get_value(prop);
	}
	return NULL;
}

const char * upnp_service_get_scpd_url(upnp_service_t * service)
{
	property_t * prop = property_get(service->properties, "SCPDURL");
	if (prop) {
		return property_get_value(prop);
	}
	return NULL;
}

const char * upnp_service_get_control_url(upnp_service_t * service)
{
	property_t * prop = property_get(service->properties, "controlURL");
	if (prop) {
		return property_get_value(prop);
	}
	return NULL;
}

const char * upnp_service_get_subscribe_url(upnp_service_t * service)
{
	property_t * prop = property_get(service->properties, "eventSubURL");
	if (prop) {
		return property_get_value(prop);
	}
	return NULL;
}

upnp_action_t * upnp_service_get_action(upnp_service_t * service, const char * name)
{
	list_t * lst;
	if (service->scpd == NULL) {
		return NULL;
	}
	lst = service->scpd->actions;
	for (; lst; lst = lst->next) {
		upnp_action_t * action = (upnp_action_t*)lst->data;
		if (strcmp(action->name, name) == 0) {
			return action;
		}
	}
	return NULL;
}
upnp_state_variable_t * upnp_service_get_state_variable(upnp_service_t * service, const char * name)
{
	list_t * lst;
	if (service->scpd == NULL) {
		return NULL;
	}
	lst = service->scpd->state_variables;
	for (; lst; lst = lst->next) {
		upnp_state_variable_t * state_variable = (upnp_state_variable_t*)lst->data;
		if (strcmp(state_variable->name, name) == 0) {
			return state_variable;
		}
	}
	return NULL;
}

void upnp_service_set_type(upnp_service_t * service, const char * type)
{
	service->properties = property_put(service->properties, "serviceType", type);
}

void upnp_service_set_id(upnp_service_t * service, const char * id)
{
	service->properties = property_put(service->properties, "serviceId", id);
}

void upnp_service_set_scpd_url(upnp_service_t * service, const char * url)
{
	service->properties = property_put(service->properties, "SCPDURL", url);
}

void upnp_service_set_control_url(upnp_service_t * service, const char * url)
{
	service->properties = property_put(service->properties, "controlURL", url);
}

void upnp_service_set_subscribe_url(upnp_service_t * service, const char * url)
{
	service->properties = property_put(service->properties, "eventSubURL", url);
}


/* scpd */

upnp_scpd_t * upnp_create_scpd(void)
{
	upnp_scpd_t * scpd = (upnp_scpd_t*)malloc(sizeof(upnp_scpd_t));
	memset(scpd, 0, sizeof(upnp_scpd_t));
	return scpd;
}

void upnp_free_scpd(upnp_scpd_t * scpd)
{
	if (scpd == NULL) {
		return;
	}
	list_clear(scpd->properties, (_free_cb)free_property);
	list_clear(scpd->actions, (_free_cb)upnp_free_action);
	list_clear(scpd->state_variables, (_free_cb)upnp_free_state_variable);
	free(scpd);
}

/* action invoke */

upnp_action_t * upnp_create_action(void)
{
	upnp_action_t * action = (upnp_action_t*)malloc(sizeof(upnp_action_t));
	memset(action, 0, sizeof(upnp_action_t));
	return action;
}

void upnp_free_action(upnp_action_t * action)
{
	if (action == NULL) {
		return;
	}
	free(action->name);
	list_clear(action->arguments, (_free_cb)upnp_free_argument);
	free(action);
}

upnp_action_request_t * upnp_create_action_request(void)
{
	upnp_action_request_t * req = (upnp_action_request_t*)malloc(sizeof(upnp_action_request_t));
	memset(req, 0, sizeof(upnp_action_request_t));
	return req;
}

void upnp_free_action_request(upnp_action_request_t * request)
{
	if (request == NULL) {
		return;
	}
	free(request->service_type);
	free(request->action_name);
	list_clear(request->params, (_free_cb)free_name_value);
	free(request);
}

char * upnp_action_request_get_service_type(upnp_action_request_t * req)
{
	return req->service_type;
}

void upnp_action_request_set_service_type(upnp_action_request_t * req, const char * type)
{
	free(req->service_type);
	req->service_type = strdup(type);
}

char * upnp_action_request_get_action_name(upnp_action_request_t * req)
{
	return req->action_name;
}

void upnp_action_request_set_action_name(upnp_action_request_t * req, const char * name)
{
	free(req->action_name);
	req->action_name = strdup(name);
}

char * upnp_action_request_get(upnp_action_request_t * request, const char * name)
{
	list_t * lst = list_find(request->params, (void*)name, (_cmp_cb)name_value_cmp_name);
	if (lst) {
		name_value_t * nv = (name_value_t*)lst->data;
		return name_value_get_value(nv);
	}
	return NULL;
}

void upnp_action_request_put(upnp_action_request_t * request, const char * name, const char * value)
{
	list_t * lst = list_find(request->params, (void*)name, (_cmp_cb)name_value_cmp_name);
	if (lst) {
		name_value_t * nv = (name_value_t*)lst->data;
		name_value_set_value(nv, value);
	} else {
		request->params = list_add(request->params, create_name_value_with_namevalue(name, value));
	}
}

upnp_action_response_t * upnp_create_action_response(void)
{
	upnp_action_response_t * res = (upnp_action_response_t*)malloc(sizeof(upnp_action_response_t));
	memset(res, 0, sizeof(upnp_action_response_t));
	return res;
}

void upnp_free_action_response(upnp_action_response_t * response)
{
	if (response == NULL) {
		return;
	}
	free(response->service_type);
	free(response->action_name);
	list_clear(response->params, (_free_cb)free_name_value);
	free(response);
}

char * upnp_action_response_get_service_type(upnp_action_response_t * res)
{
	return res->service_type;
}

void upnp_action_response_set_service_type(upnp_action_response_t * res, const char * type)
{
	free(res->service_type);
	res->service_type = strdup(type);
}

char * upnp_action_response_get_action_name(upnp_action_response_t * res)
{
	return res->action_name;
}

void upnp_action_response_set_action_name(upnp_action_response_t * res, const char * name)
{
	free(res->action_name);
	res->action_name = strdup(name);
}

char * upnp_action_response_get(upnp_action_response_t * response, const char * name)
{
	list_t * lst = list_find(response->params, (void*)name, (_cmp_cb)name_value_cmp_name);
	if (lst) {
		name_value_t * nv = (name_value_t*)lst->data;
		return name_value_get_value(nv);
	}
	return NULL;
}

void upnp_action_response_put(upnp_action_response_t * response, const char * name, const char * value)
{
	list_t * lst = list_find(response->params, (void*)name, (_cmp_cb)name_value_cmp_name);
	if (lst) {
		name_value_t * nv = (name_value_t*)lst->data;
		name_value_set_value(nv, value);
	} else {
		response->params = list_add(response->params,
                                create_name_value_with_namevalue(name, value));
	}
}


/* argument */

upnp_argument_t * upnp_create_argument(void)
{
	upnp_argument_t * argument = (upnp_argument_t*)malloc(sizeof(upnp_argument_t));
	memset(argument, 0, sizeof(upnp_argument_t));
	return argument;
}

void upnp_free_argument(upnp_argument_t * argument)
{
	if (argument == NULL) {
		return;
	}
	free(argument->name);
	free(argument->related_state_variable);
	free(argument);
}


/* state variable */

upnp_state_variable_t * upnp_create_state_variable(void)
{
	upnp_state_variable_t * state_variable = (upnp_state_variable_t*)malloc(sizeof(upnp_state_variable_t));
	memset(state_variable, 0, sizeof(upnp_state_variable_t));
	return state_variable;
}

void upnp_free_state_variable(upnp_state_variable_t * state_variable)
{
	if (state_variable == NULL) {
		return;
	}
	free(state_variable->name);
	free(state_variable->data_type);
	free(state_variable->default_value);
	list_clear(state_variable->allowed_list, free);
	free(state_variable);
}

void upnp_state_variable_set_send_events(upnp_state_variable_t * state_variable, int yesno)
{
	state_variable->send_events = yesno;
}

int upnp_state_variable_get_send_events(upnp_state_variable_t * state_variable)
{
	return state_variable->send_events;
}

void upnp_state_variable_set_multicast(upnp_state_variable_t * state_variable, int yesno)
{
	state_variable->multicast = yesno;
}

int upnp_state_variable_get_multicast(upnp_state_variable_t * state_variable)
{
	return state_variable->multicast;
}

void upnp_state_variable_set_allowed_list(upnp_state_variable_t * state_variable, list_t * allowed_list)
{
	state_variable->allowed_list = allowed_list;
}

list_t * upnp_state_variable_get_allowed_list(upnp_state_variable_t * state_variable)
{
	return state_variable->allowed_list;
}


/* upnp subscription */

upnp_subscription_t * upnp_create_subscription(void)
{
	upnp_subscription_t * subscription = (upnp_subscription_t*)malloc(sizeof(upnp_subscription_t));
	memset(subscription, 0, sizeof(upnp_subscription_t));
	upnp_subscription_update_tick(subscription);
	return subscription;
}

void upnp_free_subscription(upnp_subscription_t * subscription)
{
	if (subscription == NULL) {
		return;
	}
	free(subscription->url);
	free(subscription->sid);
	free(subscription);
}

void upnp_subscription_update_tick(upnp_subscription_t * subscription)
{
	subscription->tick = tick_milli();
}

void upnp_subscription_set_timeout(upnp_subscription_t * subscription, unsigned long timeout)
{
	subscription->timeout = timeout;
}

int upnp_subscription_expired(upnp_subscription_t * subscription)
{
	return ((tick_milli() - subscription->tick) >= subscription->timeout);
}

int upnp_subscription_cmp_sid(upnp_subscription_t * subscription, const char * sid)
{
	return strcmp(subscription->sid, sid);
}

char * upnp_subscription_get_url(upnp_subscription_t * subscription)
{
	return subscription->url;
}

void upnp_subscription_set_url(upnp_subscription_t * subscription, const char * url)
{
	free(subscription->url);
	subscription->url = strdup(url);
}

char * upnp_subscription_get_sid(upnp_subscription_t * subscription)
{
	return subscription->sid;
}

void upnp_subscription_set_sid(upnp_subscription_t * subscription, const char * sid)
{
	free(subscription->sid);
	subscription->sid = strdup(sid);
}
