#include "ssdp_header.h"
#include "namevalue.h"


ssdp_header_t * create_ssdp_header(void)
{
	ssdp_header_t * ssdp = (ssdp_header_t*)malloc(sizeof(ssdp_header_t));
	memset(ssdp, 0, sizeof(ssdp_header_t));
	return ssdp;
}

void free_ssdp_header(ssdp_header_t * ssdp)
{
	if (ssdp->remote_addr) {
		free(ssdp->remote_addr);
	}
	free(ssdp->firstline);
	list_clear(ssdp->parameters, (_free_cb)free_name_value);
	free(ssdp);
}

char * ssdp_header_get_parameter(ssdp_header_t * header, const char * name)
{
	list_t * find = list_find(header->parameters,
                            (void*)name,
                            (_cmp_cb)name_value_cmp_name_ignorecase);
	if (find) {
		name_value_t * nv = (name_value_t*)find->data;
		return name_value_get_value(nv);
	}
	return NULL;
}

void ssdp_header_set_parameter(ssdp_header_t * header, const char * name, const char * value)
{
	list_t * find = list_find(header->parameters,
                            (void*)name,
                            (_cmp_cb)name_value_cmp_name_ignorecase);

	if (find) {
		name_value_t * nv = (name_value_t*)find->data;
		name_value_set_value(nv, value);
	} else {
		header->parameters = list_add(header->parameters,
                                  create_name_value_with_namevalue(name, value));
	}
}

name_value_t * read_ssdp_header_parameter(str_t line)
{
	name_value_t * nv = NULL;
	char * sep = strutil_strstr(&line, ":");
	if (sep == NULL) {
		return NULL;
	}

	nv = create_name_value();
	{
		str_t name = strutil_trim(strutil_str(line.begin, sep));
		str_t value = strutil_trim(strutil_str(sep + 1, line.end));
		name_value_set_name_nocopy(nv, strutil_dup_cstr(&name));
		name_value_set_value_nocopy(nv, strutil_dup_cstr(&value));
	}
	return nv;
}

ssdp_header_t * read_ssdp_header(const char * str)
{
	ssdp_header_t * ssdp = create_ssdp_header();
	int first = 1;
	const char * end = strstr(str, "\r\n");
	while (end) {
		str_t line = {str, end};
		if (line.end - line.begin == 0) {
			break;
		}
		if (first) {
			ssdp->firstline = strndup(line.begin, (int)(line.end - line.begin));
			first = 0;
		} else {
			name_value_t * nv = read_ssdp_header_parameter(line);
			ssdp_header_set_parameter(ssdp,
                                name_value_get_name(nv),
                                name_value_get_value(nv));
		}
		str = end + 2;
		end = strstr(str, "\r\n");
	}
	return ssdp;
}

notify_type_e ssdp_header_get_nts(ssdp_header_t * ssdp)
{
	char * nts = ssdp_header_get_parameter(ssdp, "NTS");
	if (nts == NULL) {
		return NTS_UNKNOWN;
	}
	if (strcmp(nts, "ssdp:alive") == 0) {
		return NTS_ALIVE;
	} else if (strcmp(nts, "ssdp:update") == 0) {
		return NTS_UPDATE;
	} else if (strcmp(nts, "ssdp:byebye") == 0) {
		return NTS_BYEBYE;
	}
	return NTS_UNKNOWN;
}

void ssdp_header_set_nts(ssdp_header_t * header, notify_type_e type)
{
	switch (type) {
	case NTS_ALIVE: {
		ssdp_header_set_parameter(header, "NTS", "ssdp:alive");
		break;
	}
	case NTS_UPDATE: {
		ssdp_header_set_parameter(header, "NTS", "ssdp:update");
		break;
	}
	case NTS_BYEBYE: {
		ssdp_header_set_parameter(header, "NTS", "ssdp:byebye");
		break;
	}
	default:
		break;
	}
}

ssdp_type_e ssdp_header_get_type(ssdp_header_t * header)
{
	if (header->firstline == NULL)
	{
		return SSDP_UNKNOWN;
	}

	if (strstr(header->firstline, "M-SEARCH") == header->firstline) {
		return SSDP_MSEARCH;
	}

	if (strstr(header->firstline, "NOTIFY") == header->firstline) {
		return SSDP_NOTIFY;
	}

	if (strstr(header->firstline, "HTTP/") == header->firstline) {
		return SSDP_RESPONSE;
	}

	return SSDP_UNKNOWN;
}
