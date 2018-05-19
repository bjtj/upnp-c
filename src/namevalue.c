#include "namevalue.h"
#include "strutil.h"


name_value_t * create_name_value(void)
{
	name_value_t * nv;
	nv = (name_value_t*)malloc(sizeof(name_value_t));
	memset(nv, 0, sizeof(name_value_t));
	return nv;
}

name_value_t * create_name_value_with_namevalue(const char * name, const char * value)
{
	name_value_t * nv = create_name_value();
	nv->name = strdup(name);
	nv->value = strdup(value);
	return nv;
}

name_value_t * create_name_value_with_namevalue_nocopy(char * name, char * value)
{
	name_value_t * nv = create_name_value();
	nv->name = name;
	nv->value = value;
	return nv;
}

void free_name_value(name_value_t * nv)
{
	free(nv->name);
	free(nv->value);
	free(nv);
}

int name_value_cmp_name(name_value_t * nv, const char * name)
{
	return strcmp(nv->name, name);
}

int name_value_cmp_name_ignorecase(name_value_t * nv, const char * name)
{
	return strcmp_ignorecase(nv->name, name);
}

char * name_value_get_name(name_value_t * nv)
{
	return nv->name;
}

void name_value_set_name(name_value_t * nv, const char * name)
{
	free(nv->name);
	nv->name = strdup(name);
}

void name_value_set_name_nocopy(name_value_t * nv, char * name)
{
	free(nv->name);
	nv->name = name;
}

char * name_value_get_value(name_value_t * nv)
{
	return nv->value;
}

void name_value_set_value(name_value_t * nv, const char * value)
{
	free(nv->value);
	nv->value = strdup(value);
}

void name_value_set_value_nocopy(name_value_t * nv, char * value)
{
	free(nv->value);
	nv->value = value;
}

