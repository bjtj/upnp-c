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
	usn->udn = strdup_silent(udn);
	usn->rest = strdup_silent(rest);
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

char * upnp_usn_to_string(upnp_usn_t * usn)
{
	if (usn->rest) {
		int size = strlen(usn->udn) + 2 + strlen(usn->rest) + 1;
		char * str = (char*)malloc(size);
		sprintf(str, "%s::%s", usn->udn, usn->rest);
		return str;
	}
	return strdup(usn->udn);
}

void upnp_usn_set_udn(upnp_usn_t * usn, const char * udn)
{
	if (usn->udn) {
		free(usn->udn);
	}
	usn->udn = strdup(udn);
}

void upnp_usn_set_rest(upnp_usn_t * usn, const char * rest)
{
	if (usn->rest) {
		free(usn->rest);
	}
	usn->rest = strdup(rest);
}
