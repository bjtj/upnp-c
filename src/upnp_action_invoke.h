#ifndef __UPNP_ACTION_INVOKE_H__
#define __UPNP_ACTION_INVOKE_H__

#include "public.h"
#include "listutil.h"
#include "upnp_models.h"


extern upnp_action_response_t * upnp_action_invoke(const char * url, upnp_action_request_t * request);

#endif
