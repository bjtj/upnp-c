#ifndef __UPNP_USN_H__
#define __UPNP_USN_H__

#include "public.h"

typedef struct _upnp_usn_t
{
	char * udn;
	char * rest;
} upnp_usn_t;

extern upnp_usn_t * upnp_create_usn(void);
extern upnp_usn_t * upnp_create_usn_with_init(const char * udn, const char * rest);
extern void upnp_free_usn(upnp_usn_t * usn);
extern upnp_usn_t * upnp_read_usn(const char * str);
extern char * upnp_usn_to_string(upnp_usn_t * usn);
extern void upnp_usn_set_udn(upnp_usn_t * usn, const char * udn);
extern void upnp_usn_set_rest(upnp_usn_t * usn, const char * rest);

#endif
