#include "http_header.h"
#include "namevalue.h"


http_header_t * create_http_header(void)
{
	http_header_t * header = (http_header_t*)malloc(sizeof(http_header_t));
	memset(header, 0, sizeof(http_header_t));
	return header;
}

void free_http_header(http_header_t * header)
{
	if (header == NULL) {
		return;
	}
	free(header->firstline);
	list_clear(header->parameters, (_free_cb)free_name_value);
	free(header);
}

name_value_t * http_header_read_parameter(str_t * str)
{
	char * sep = strutil_strstr(str, ":");
	if (sep) {
		str_t name = strutil_trim(strutil_str(str->begin, sep));
		str_t value = strutil_trim(strutil_str(sep+1, str->end));
		return create_name_value_with_namevalue_nocopy(strutil_dup_cstr(&name),
													   strutil_dup_cstr(&value));
	}
	return NULL;
}

void http_header_set_firstline(http_header_t * header, const char * firstline)
{
	free(header->firstline);
	header->firstline = strdup(firstline);
}

void http_header_set_firstline_nocopy(http_header_t * header, char * firstline)
{
	free(header->firstline);
	header->firstline = firstline;
}

void http_header_set_parameter(http_header_t * header, const char * name, const char * value)
{
	list_t * find = list_find(header->parameters, (void*)name, (_cmp_cb)name_value_cmp_name);
	if (find) {
		name_value_t * nv = (name_value_t*)find->data;
		name_value_set_value(nv, value);
	}
	header->parameters = list_add(header->parameters, create_name_value_with_namevalue(name, value));
}

void http_header_set_parameter_nocopy(http_header_t * header, char * name, char * value)
{
	list_t * find = list_find(header->parameters, (void*)name, (_cmp_cb)name_value_cmp_name);
	if (find) {
		name_value_t * nv = (name_value_t*)find->data;
		name_value_set_value_nocopy(nv, value);
	}
	header->parameters = list_add(header->parameters,
								  create_name_value_with_namevalue_nocopy(name, value));
}

char * http_header_get_firstline(http_header_t * header)
{
	return header->firstline;
}

char * http_header_get_parameter(http_header_t * header, const char * name)
{
	list_t * find = list_find(header->parameters, (void*)name, (_cmp_cb)name_value_cmp_name);
	if (find) {
		name_value_t * nv = (name_value_t*)find->data;
		return name_value_get_value(nv);
	}
	return NULL;
}
