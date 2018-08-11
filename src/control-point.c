#include "public.h"
#include "clock.h"
#include "listutil.h"
#include "urlutil.h"
#include "networkutil.h"
#include "ssdp_header.h"
#include "ssdp_receiver.h"
#include "ssdp_msearch_sender.h"
#include "http_client.h"
#include "http_server.h"
#include "upnp_models.h"
#include "upnp_serialize.h"
#include "upnp_usn.h"
#include "upnp_action_invoke.h"
#include "upnp_control_point.h"

#define STRNCMP(A, B) strncmp((A), (B), strlen((B)))


typedef struct _session_t
{
	char * device;
	char * service;
	char * action;
} session_t;


static void on_device_added(upnp_device_t * device)
{
	printf("[upnp] device added : %s\n", upnp_device_get_friendlyname(device));
}

static void on_device_removed(upnp_device_t * device)
{
	printf("[upnp] device removed : %s\n", upnp_device_get_friendlyname(device));
}

static void on_event(const char * sid, list_t * properties)
{
	printf("[event] sid: %s\n", sid);
	for (; properties; properties = properties->next)
	{
		name_value_t * nv = (name_value_t*)properties->data;
		printf("%s : %s\n", nv->name, nv->value);
	}
}


int main(int argc, char *argv[])
{
	upnp_control_point_t * cp = upnp_create_control_point(9999);

	session_t session = {0,};
	
	http_client_global_init();

	upnp_control_point_set_on_device_added(cp, on_device_added);
	upnp_control_point_set_on_device_removed(cp, on_device_removed);
	upnp_control_point_set_on_event(cp, on_event);
	upnp_control_point_start(cp);

	printf("(help|h)\n");
	while (1)
	{
		char line[1024] = {0,};
		assert(fgets(line, sizeof(line), stdin) != NULL);
		line[strlen(line) - 1] = '\0';
		if (strlen(line) == 0) {
			char * ipv4 = get_ipv4();
			printf("[Info]\n");
			printf(" * ipv4: %s\n", ipv4);
			printf("[selection]\n");
			printf(" * device: %s\n", session.device);
			printf(" * service: %s\n", session.service);
			printf(" * action: %s\n", session.action);
			printf("\n");
			free(ipv4);
		} else if (strcmp(line, "help") == 0 || strcmp(line, "h") == 0) {
			printf("[Help]\n");
			printf("Available commands:\n");
			printf(" * help | h\n");
			printf(" * quit | q\n");
			printf(" * search <type>\n");
			printf(" * ls \n");
			printf(" * device <udn>\n");
			printf(" * service <service type>\n");
			printf(" * action <action name>\n");
			printf(" * invoke\n");
			printf(" * subscriptions\n");
			printf(" * subscribe\n");
			printf(" * unsubscribe\n");
			printf("\n");
		} else if (strcmp(line, "quit") == 0 || strcmp(line, "q") == 0) {
			printf("[quit]\n");
			break;
		} else if (strcmp(line, "ls") == 0) {
			list_t * lst;
			printf("[Device List]\n");
			lst = upnp_control_point_get_devices(cp);
			for (; lst; lst = lst->next) {
				upnp_device_t * device = (upnp_device_t*)lst->data;
				const char * friendlyname = upnp_device_get_friendlyname(device);
				printf("%s / %s\n", friendlyname, upnp_device_get_udn(device));
				list_t * lst_service = device->services;
				for (; lst_service; lst_service = lst_service->next) {
					upnp_service_t * service = (upnp_service_t*)lst_service->data;
					printf(" * %s\n", upnp_service_get_type(service));
					if (service->scpd == NULL) {
						continue;
					}
					list_t * lst_action = service->scpd->actions;
					for (; lst_action; lst_action = lst_action->next) {
						upnp_action_t * action = (upnp_action_t*)lst_action->data;
						printf("  - %s\n", action->name);
					}
				}
			}
		} else if (STRNCMP(line, "search ") == 0) {
			char * st = line + strlen("search ");
			printf("[Search] '%s'\n", st);
			upnp_control_point_send_msearch(cp, st, 3);
			printf("[Search] done\n");
		} else if (STRNCMP(line, "device ") == 0) {
			char * udn = line + strlen("device ");
			printf("You selected devcie '%s'\n", udn);
			free(session.device);
			session.device = strdup(udn);
		} else if (STRNCMP(line, "service ") == 0) {
			char * service = line + strlen("service ");
			printf("You selected service '%s'\n", service);
			free(session.service);
			session.service = strdup(service);
		} else if (STRNCMP(line, "action ") == 0) {
			char * action = line + strlen("action ");
			printf("You selected action '%s'\n", action);
			free(session.action);
			session.action = strdup(action);
		} else if (strcmp(line, "invoke") == 0) {
			upnp_device_t * device;
			printf("[Invoke]\n");
			device = upnp_control_point_get_device(cp, session.device);
			if (device) {
				upnp_service_t * service = upnp_device_get_service(device, session.service);
				const char * base_url = upnp_device_get_base_url(device);
				const char * control_url = upnp_service_get_control_url(service);
				char * url = url_relative(base_url, control_url);
				printf("* Device: %s\n", upnp_device_get_friendlyname(device));
				if (service) {
					upnp_action_t * action = upnp_service_get_action(service, session.action);
					list_t * lst_argument = action->arguments;
					upnp_action_request_t * request = upnp_create_action_request();
					upnp_action_response_t * response;
					printf(" * action name: %s\n", action->name);
					upnp_action_request_set_service_type(request, upnp_service_get_type(service));
					upnp_action_request_set_action_name(request, action->name);
					for (; lst_argument; lst_argument = lst_argument->next) {
						upnp_argument_t * argument = (upnp_argument_t*)lst_argument->data;
						if (argument->direction == DIR_IN) {
							char input[1024] = {0,};
							printf("[in] %s: ", argument->name);
							assert(fgets(input, sizeof(input), stdin) != NULL);
							input[strlen(input) - 1] = '\0';
							upnp_action_request_put(request, argument->name, input);
						}
					}
					response = upnp_action_invoke(url, request);
					printf("[action] response action name: %s\n", response->action_name);
					{
						list_t * params = response->params;
						for (; params; params = params->next) {
							name_value_t * nv = (name_value_t*)params->data;
							printf("[out] %s : %s\n", nv->name, nv->value);
						}
					}
					upnp_free_action_request(request);
					upnp_free_action_response(response);
				}
				free(url);
			}
		} else if (strcmp(line, "subscriptions") == 0) {
			list_t * lst = cp->subscriptions;
			printf("[Subscriptions]\n");
			for (; lst; lst = lst->next) {
				upnp_subscription_t * subscription = (upnp_subscription_t*)lst->data;
				printf(" * %s\n", subscription->sid);
			}
		} else if (strcmp(line, "subscribe") == 0) {
			upnp_device_t * device = upnp_control_point_get_device(cp, session.device);
			upnp_service_t * service = upnp_device_get_service(device, session.service);
			char * base_url = upnp_device_get_base_url(device);
			char * url = url_relative(base_url, upnp_service_get_subscribe_url(service));
			upnp_subscription_t * subscription = upnp_control_point_subscribe(cp, url);
			printf("[subscribe] sid: %s\n", subscription->sid);
			free(url);
		} else if (STRNCMP(line, "unsubscribe ") == 0) {
			char * sid = line + strlen("unsubscribe ");
			upnp_control_point_unsubscribe(cp, sid);
		} else {
			printf("[Error] Unknown command - '%s'\n", line);
		}
	}

	upnp_control_point_stop(cp);
	upnp_free_control_point(cp);

	free(session.device);
	free(session.service);
	free(session.action);

	printf("Bye\n");
    return 0;
}

