#ifndef __HTTP_HEADER_H__
#define __HTTP_HEADER_H__

#include "public.h"
#include "listutil.h"
#include "strutil.h"
#include "namevalue.h"

typedef struct _http_header_t
{
  char * firstline;
	list_t * parameters;
} http_header_t;


extern http_header_t * create_http_header(void);
extern void free_http_header(http_header_t * header);
extern name_value_t * http_header_read_parameter(str_t * str);
extern void http_header_set_firstline(http_header_t * header, const char * firstline);
extern void http_header_set_firstline_nocopy(http_header_t * header, char * firstline);
extern void http_header_set_parameter(http_header_t * header, const char * name, const char * value);
extern void http_header_set_parameter_nocopy(http_header_t * header, char * name, char * value);
extern char * http_header_get_firstline(http_header_t * header);
extern char * http_header_get_parameter(http_header_t * header, const char * name);

#endif
