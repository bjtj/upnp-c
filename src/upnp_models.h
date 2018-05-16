#ifndef __UPNP_MODELS_H__
#define __UPNP_MODELS_H__

#include "namevalue.h"
#include "property.h"
#include "strutil.h"
#include "listutil.h"

typedef enum _upnp_direction_e
{
	DIR_IN, DIR_OUT
} upnp_direction_e;

typedef struct _upnp_range
{
    int minimum;
	int maximum;
	int step;
} upnp_range;

typedef struct _upnp_state_variable_t
{
	char * name;
	int send_events;
	int multicast;
	char * data_type;
	char * default_value;
	list_t * allowed_list;
	upnp_range range;
} upnp_state_variable_t;

typedef struct _upnp_argument_t
{
	char * name;
	char * related_state_variable;
	upnp_direction_e direction;
} upnp_argument_t;

typedef struct _upnp_action_t
{
	char * name;
	list_t * arguments;
} upnp_action_t;

typedef struct _upnp_scpd_t
{
	list_t * properties;
	list_t * actions;
	list_t * state_variables;
} upnp_scpd_t;

typedef struct _upnp_service_t
{
	list_t * properties;
	upnp_scpd_t * scpd;
} upnp_service_t;

typedef struct _upnp_device_t
{
	unsigned long tick;
	unsigned long timeout;
	char * base_url;
	list_t * properties;
	list_t * services;
	list_t * embedded_devices;
} upnp_device_t;


typedef struct _upnp_action_request_t
{
	char * service_type;
    char * action_name;
	list_t * params;
} upnp_action_request_t;

typedef struct _upnp_action_response_t
{
	char * service_type;
    char * action_name;
	list_t * params;
} upnp_action_response_t;


typedef struct _upnp_subscription_t
{
	unsigned long tick;
	unsigned long timeout;
	char * url;
    char * sid;
} upnp_subscription_t;


// device
extern upnp_device_t * upnp_create_device(void);
extern void upnp_free_device(upnp_device_t * device);
extern void upnp_device_update_tick(upnp_device_t * device);
extern void upnp_device_set_timeout(upnp_device_t * device, unsigned long timeout);
extern int upnp_device_expired(upnp_device_t * device);
extern char * upnp_device_get_base_url(upnp_device_t * device);
extern void upnp_device_set_base_url(upnp_device_t * device, const char * base_url);
extern int upnp_device_cmp_udn(upnp_device_t * device, const char * udn);
extern const char * upnp_device_get_udn(upnp_device_t * device);
extern const char * upnp_device_get_friendlyname(upnp_device_t * device);
extern void upnp_device_add_child_device(upnp_device_t * device, upnp_device_t * child_device);
extern void upnp_device_set_udn(upnp_device_t * device, const char * udn);
extern void upnp_device_set_scpd_url(upnp_device_t * device, const char * url);
extern void upnp_device_set_control_url(upnp_device_t * device, const char * url);
extern void upnp_device_set_subscribe_url(upnp_device_t * device, const char * url);
extern upnp_service_t * upnp_device_get_service(upnp_device_t * device, const char * type);

// sevice
extern upnp_service_t * upnp_create_service(void);
extern void upnp_free_service(upnp_service_t * service);
extern int upnp_service_cmp_type(upnp_service_t * service, const char * type);
extern int upnp_service_cmp_id(upnp_service_t * service, const char * id);
extern int upnp_service_cmp_scpd_url(upnp_service_t * service, const char * url);
extern int upnp_service_cmp_control_url(upnp_service_t * service, const char * url);
extern int upnp_service_cmp_subscribe_url(upnp_service_t * service, const char * url);
extern const char * upnp_service_get_type(upnp_service_t * service);
extern const char * upnp_service_get_id(upnp_service_t * service);
extern const char * upnp_service_get_scpd_url(upnp_service_t * service);
extern const char * upnp_service_get_control_url(upnp_service_t * service);
extern const char * upnp_service_get_subscribe_url(upnp_service_t * service);
extern upnp_action_t * upnp_service_get_action(upnp_service_t * service, const char * name);
extern upnp_state_variable_t * upnp_service_get_state_variable(upnp_service_t * service, const char * name);
extern void upnp_service_set_type(upnp_service_t * service, const char * type);
extern void upnp_service_set_id(upnp_service_t * service, const char * id);
extern void upnp_service_set_scpd_url(upnp_service_t * service, const char * url);
extern void upnp_service_set_control_url(upnp_service_t * service, const char * url);
extern void upnp_service_set_subscribe_url(upnp_service_t * service, const char * url);


// scpd
extern upnp_scpd_t * upnp_create_scpd(void);
extern void upnp_free_scpd(upnp_scpd_t * scpd);


// action invoke
extern upnp_action_t * upnp_create_action(void);
extern void upnp_free_action(upnp_action_t * action);
extern upnp_action_request_t * upnp_create_action_request(void);
extern void upnp_free_action_request(upnp_action_request_t * request);
extern char * upnp_action_request_get_service_type(upnp_action_request_t * req);
extern void upnp_action_request_set_service_type(upnp_action_request_t * req, const char * type);
extern char * upnp_action_request_get_action_name(upnp_action_request_t * req);
extern void upnp_action_request_set_action_name(upnp_action_request_t * req, const char * name);
extern char * upnp_action_request_get(upnp_action_request_t * request, const char * name);
extern void upnp_action_request_put(upnp_action_request_t * request,
									const char * name,
									const char * value);

extern upnp_action_response_t * upnp_create_action_response(void);
extern void upnp_free_action_response(upnp_action_response_t * response);
extern char * upnp_action_response_get_service_type(upnp_action_response_t * res);
extern void upnp_action_response_set_service_type(upnp_action_response_t * res, const char * type);
extern char * upnp_action_response_get_action_name(upnp_action_response_t * res);
extern void upnp_action_response_set_action_name(upnp_action_response_t * res, const char * name);
extern char * upnp_action_response_get(upnp_action_response_t * response, const char * name);
extern void upnp_action_response_put(upnp_action_response_t * response,
									 const char * name,
									 const char * value);


// argument
extern upnp_argument_t * upnp_create_argument(void);
extern void upnp_free_argument(upnp_argument_t * argument);

// state variable
extern upnp_state_variable_t * upnp_create_state_variable(void);
extern void upnp_free_state_variable(upnp_state_variable_t * state_variable);
extern void upnp_state_variable_set_send_events(upnp_state_variable_t * state_variable, int yes);
extern int upnp_state_variable_get_send_events(upnp_state_variable_t * state_variable);
extern void upnp_state_variable_set_multicast(upnp_state_variable_t * state_variable, int yes);
extern int upnp_state_variable_get_multicast(upnp_state_variable_t * state_variable);
extern void upnp_state_variable_set_allowed_list(upnp_state_variable_t * state_variable, list_t * allowed_list);
extern list_t * upnp_state_variable_get_allowed_list(upnp_state_variable_t * state_variable);

// upnp subscription
extern upnp_subscription_t * upnp_create_subscription(void);
extern void upnp_free_subscription(upnp_subscription_t * subscription);
extern void upnp_subscription_update_tick(upnp_subscription_t * subscription);
extern void upnp_subscription_set_timeout(upnp_subscription_t * subscription, unsigned long timeout);
extern int upnp_subscription_expired(upnp_subscription_t * subscription);
extern int upnp_subscription_cmp_sid(upnp_subscription_t * subscription, const char * sid);
extern char * upnp_subscription_get_url(upnp_subscription_t * subscription);
extern void upnp_subscription_set_url(upnp_subscription_t * subscription, const char * url);
extern char * upnp_subscription_get_sid(upnp_subscription_t * subscription);
extern void upnp_subscription_set_sid(upnp_subscription_t * subscription, const char * sid);


#endif
