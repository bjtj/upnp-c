#ifndef __NAME_VALUE_H__
#define __NAME_VALUE_H__

#include "public.h"

typedef struct _name_value_t {
	char * name;
	char * value;
} name_value_t;

extern name_value_t * create_name_value(void);
extern name_value_t * create_name_value_with_namevalue(const char * name, const char * value);
extern name_value_t * create_name_value_with_namevalue_nocopy(char * name, char * value);
extern void free_name_value(name_value_t * nv);
extern int name_value_cmp_name(name_value_t * nv, const char * name);
extern int name_value_cmp_name_ignorecase(name_value_t * nv, const char * name);
extern char * name_value_get_name(name_value_t * nv);
extern void name_value_set_name(name_value_t * nv, const char * name);
extern void name_value_set_name_nocopy(name_value_t * nv, char * name);
extern char * name_value_get_value(name_value_t * nv);
extern void name_value_set_value(name_value_t * nv, const char * value);
extern void name_value_set_value_nocopy(name_value_t * nv, char * value);


#endif
