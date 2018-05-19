#include "upnp_usn.h"
#include "strutil.h"

upnp_usn_t * upnp_create_usn(void)
{
	upnp_usn_t * usn = (upnp_usn_t*)malloc(sizeof(upnp_usn_t));
	memset(usn, 0, sizeof(upnp_usn_t));
	return usn;
}

upnp_usn_t * upnp_create_usn_with_init(const char * udn, const char * rest)
{
	upnp_usn_t * usn = upnp_create_usn();
	usn->udn = strdup_quiet(udn);
	usn->rest = strdup_quiet(rest);
	return usn;
}

void upnp_free_usn(upnp_usn_t * usn)
{
	free(usn->udn);
	free(usn->rest);
	free(usn);
}

upnp_usn_t * upnp_read_usn(const char * str)
{
	char * sep = NULL;
	if (str == NULL) {
		return NULL;
	}
	sep = strstr(str, "::");
	if (sep) {
		*sep = '\0';
		return upnp_create_usn_with_init(str, sep + 2);
	}
	return upnp_create_usn_with_init(str, NULL);
}
