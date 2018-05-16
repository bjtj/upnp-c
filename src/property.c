#include "property.h"
#include "strutil.h"
#include "namevalue.h"


property_t * create_property(void) {
	property_t * prop = (property_t*)malloc(sizeof(property_t));
	memset(prop, 0, sizeof(property_t));
	return prop;
}

property_t * create_property_with_namevalue(const char * name, const char * value) {
	property_t * prop = create_property();
	prop->name = strdup(name);
	prop->value = strdup(value);
	return prop;
}

void free_property(property_t * prop) {
	free(prop->name);
	free(prop->value);
	property_clear_attributes(prop);
	free(prop);
}

int property_cmp_name(property_t * prop, const char * name) {
	return strcmp_icase(prop->name, name);
}

char * property_get_name(property_t * prop) {
	return prop->name;
}

void property_set_name(property_t * prop, const char * name) {
	if (prop == NULL) {
		return;
	}
	free(prop->name);
	prop->name = strdup(name);
}

char * property_get_value(property_t * prop) {
	return prop->value;
}

void property_set_value(property_t * prop, const char * value) {
	if (prop == NULL) {
		return;
	}
	free(prop->value);
	prop->value = strdup(value);
}

list_t * property_put(list_t * properties, const char * name, const char * value) {
	list_t * find = list_find(properties, (void*)name, (_cmp_cb)property_cmp_name);
	if (find) {
		property_set_value((property_t*)find->data, value);
		return properties;
	}
	return list_add(properties, create_property_with_namevalue(name, value));
}

property_t * property_get(list_t * properties, const char * name) {
	list_t * find = list_find(properties, (void*)name, (_cmp_cb)property_cmp_name);
	if (find) {
		return (property_t*)find->data;
	}
	return NULL;
}

void property_add_attribute(property_t * prop, const char * name, const char * value) {
	name_value_t * nv = create_name_value_with_namevalue(name, value);
	prop->attributes = list_add(prop->attributes, nv);
}

void property_remove_attribute(property_t * prop, const char * name) {
	list_t * lst = list_find(prop->attributes, (void*)name, (_cmp_cb)name_value_cmp_name);
	if (lst) {
		prop->attributes = list_remove(prop->attributes, lst, (_free_cb)free_name_value);
	}
}

char * property_get_attribute(property_t * prop, const char * name) {
	list_t * lst = list_find(prop->attributes, (void*)name, (_cmp_cb)name_value_cmp_name);
	if (lst) {
		name_value_t * nv = (name_value_t*)lst->data;
		return nv->value;
	}
	return NULL;
}

void property_clear_attributes(property_t * prop) {
	prop->attributes = list_clear(prop->attributes, (_free_cb)free_name_value);
}

