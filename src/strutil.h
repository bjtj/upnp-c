#ifndef __STRUTIL_H__
#define __STRUTIL_H__

#include "public.h"

typedef struct _str_t {
	const char * begin;
	const char * end;
} str_t;


extern str_t strutil_str(const char * begin, const char * end);
extern int is_space(char ch);
extern int strutil_empty(str_t * str);
extern int strutil_len(str_t str);
extern char * strutil_strstr(str_t * str, const char * find);
extern char * strutil_dup_cstr(str_t * str);
extern str_t strutil_trim(str_t str);

extern int strcmp_icase(char const * a, char const * b);
extern char * strstr_last(const char * str, const char * pat);
extern char * find_first(const char * str, const char * t);
extern char * find_first_not(const char * str, const char * t);

#endif
