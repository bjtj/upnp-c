#include "http_response.h"
#include "namevalue.h"


http_header_t * http_response_get_header(http_response_t * res)
{
	return res->header;
}

http_response_t * create_http_response(void)
{
	http_response_t * res = (http_response_t*)malloc(sizeof(http_response_t));
	memset(res, 0, sizeof(http_response_t));
	res->header = create_http_header();
	return res;
}

void free_http_response(http_response_t * res)
{
	if (res == NULL) {
		return;
	}
	free_http_header(res->header);
	free(res->data);
}

int http_response_get_status_code(http_response_t * res)
{
	char * firstline = http_response_get_firstline(res);
	char * f = find_first(firstline, " \t\n");
	if (f == NULL) {
		return -1;
	}
	f = find_first_not(f, " \t\n");
	if (f == NULL) {
		return -1;
	}
	return atoi(f);
}

void http_response_set_firstline(http_response_t * res, const char * firstline)
{
	http_header_set_firstline(http_response_get_header(res), firstline);
}

char * http_response_get_firstline(http_response_t * res)
{
	return http_header_get_firstline(http_response_get_header(res));
}

void http_response_set_parameter(http_response_t * res, const char * key, const char * value)
{
	http_header_set_parameter(http_response_get_header(res), key, value);
}

char * http_response_get_parameter(http_response_t * res, const char * key)
{
	return http_header_get_parameter(http_response_get_header(res), key);
}
