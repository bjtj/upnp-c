#ifndef __PROPERTY_H__
#define __PROPERTY_H__

#include "listutil.h"

typedef struct _property_t
{
	char * name;
	char * value;
	list_t * attributes;
} property_t;


extern property_t * create_property(void);
extern property_t * create_property_with_namevalue(const char * name, const char * value);
extern void free_property(property_t * prop);
extern int property_cmp_name(property_t * prop, const char * name);
extern char * property_get_name(property_t * prop);
extern void property_set_name(property_t * prop, const char * name);
extern char * property_get_value(property_t * prop);
extern void property_set_value(property_t * prop, const char * value);
extern list_t * property_put(list_t * properties, const char * name, const char * value);
extern property_t * property_get(list_t * properties, const char * name);
extern void property_add_attribute(property_t * prop, const char * name, const char * value);
extern void property_remove_attribute(property_t * prop, const char * name);
extern char * property_get_attribute(property_t * prop, const char * name);
extern void property_clear_attributes(property_t * prop);
extern void property_set_value(property_t * prop, const char * value);

#endif
