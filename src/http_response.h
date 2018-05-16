#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

#include "public.h"
#include "http_header.h"
#include "listutil.h"

typedef struct _http_response_t
{
	http_header_t * header;
	char * data;
	size_t data_size;
} http_response_t;


extern http_header_t * http_response_get_header(http_response_t * res);
extern http_response_t * create_http_response(void);
extern void free_http_response(http_response_t * res);
extern int http_response_get_status_code(http_response_t * res);
extern void http_response_set_firstline(http_response_t * res, const char * firstline);
extern char * http_response_get_firstline(http_response_t * res);
extern void http_response_set_parameter(http_response_t * res, const char * key, const char * value);
extern char * http_response_get_parameter(http_response_t * res, const char * key);

#endif
