#ifndef __SSDP_HEADER_H__
#define __SSDP_HEADER_H__

#include "public.h"
#include "strutil.h"
#include "listutil.h"
#include "namevalue.h"

typedef struct _ssdp_header_t {
	char * firstline;
	list_t * parameters;
} ssdp_header_t;


extern ssdp_header_t * create_ssdp_header(void);
extern void free_ssdp_header(ssdp_header_t * ssdp);
extern char * ssdp_header_get_parameter(ssdp_header_t * header, const char * name);
extern void ssdp_header_set_parameter(ssdp_header_t * header, const char * name, const char * value);
extern name_value_t * read_ssdp_header_parameter(str_t line);
extern ssdp_header_t * read_ssdp_header(const char * str);

#endif
