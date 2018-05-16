#include "upnp_action_invoke.h"
#include "http_client.h"
#include "upnp_serialize.h"


upnp_action_response_t * upnp_action_invoke(const char * url, upnp_action_request_t * request)
{
	http_response_t * res;
	upnp_action_response_t * response;
	char * xml = upnp_write_action_request(request);
	list_t * headers = NULL;
	char soapaction[1024] = {0,};
	snprintf(soapaction, sizeof(soapaction), "\"%s#%s\"",
			 upnp_action_request_get_service_type(request),
			 upnp_action_request_get_action_name(request));
	headers = list_add(headers, create_name_value_with_namevalue("SOAPACTION", soapaction));
	res = http_client_post(url, headers, "text/xml", xml);
	response = upnp_read_action_response(res->data);
	list_clear(headers, (_free_cb)free_name_value);
	free_http_response(res);
	free(xml);
	
	return response;
}
