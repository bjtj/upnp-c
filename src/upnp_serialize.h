#ifndef __UPNP_SERIALIZE_H__
#define __UPNP_SERIALIZE_H__

#include "upnp_models.h"

extern upnp_device_t * upnp_read_device_xml(const char * xml);
extern upnp_scpd_t * upnp_read_scpd_xml(const char * xml);
extern upnp_action_request_t * upnp_read_action_request(const char * xml);
extern upnp_action_response_t * upnp_read_action_response(const char * xml);
extern char * upnp_write_action_request(upnp_action_request_t * req);
extern char * upnp_write_action_response(upnp_action_response_t * res);
extern list_t * upnp_read_propertyset(const char * xml);
extern char * upnp_write_propertyset(list_t * props);

#endif
