#include "strutil.h"


int is_space(char ch)
{
	switch (ch) {
	case ' ':
	case '\t':
	case '\r':
	case '\n':
		return 1;
	default:
		break;
	}
	return 0;
}

str_t strutil_str(const char * begin, const char * end)
{
	str_t ret = {.begin = begin, .end = end};
	return ret;
}

int strutil_empty(str_t * str)
{
	return ((str == NULL) || (str->begin == str->end));
}

int strutil_len(str_t str)
{
	return (int)(str.end - str.begin);
}

char * strutil_strstr(str_t * str, const char * find)
{
	const char * ptr = str->begin;
	int len = strlen(find);
	while (str->end - ptr >= len) {
		if (strncmp(ptr, find, len) == 0) {
			return (char*)ptr;
		}
		ptr++;
	}
	return NULL;
}

char * strutil_dup_cstr(str_t * str)
{
	return strndup(str->begin, (int)(str->end - str->begin));
}

str_t strutil_trim(str_t str)
{
	const char * ptr;
	if (str.begin == str.end) {
		return str;
	}
	for (ptr = str.begin; ptr != str.end; ptr++) {
		if (!is_space(*ptr)) {
			str.begin = ptr;
			break;
		}
	}
	if (ptr == str.end) {
		str.begin = str.end;
		return str;
	}
	for (ptr = str.end-1; ptr != str.begin; ptr--) {
		if (!is_space(*ptr)) {
			str.end = ptr+1;
			break;
		}
	}
	return str;
}

int strcmp_ignorecase(char const * a, char const * b)
{
	return strcasecmp(a, b);
}

char * strstr_last(const char * str, const char * pat)
{
	const char * ptr = str + strlen(str) - 1;
	while (ptr != str) {
		if (strcmp(ptr, pat) == 0) {
			return (char*)ptr;
		}
		ptr--;
	}
	return NULL;
}

static int _contains(char ch, const char * t)
{
	while (*t) {
		if (ch == *t) {
			return 1;
		}
		t++;
	}
	return 0;
}

char * find_first(const char * str, const char * t)
{
	while (*str) {
		if (_contains(*str, t) == 1) {
			return (char*)str;
		}
		str++;
	}
	return NULL;
}

char * find_first_not(const char * str, const char * t)
{
	while (*str) {
		if (_contains(*str, t) == 0) {
			return (char*)str;
		}
		str++;
	}
	return NULL;
}

char * strdup_silent(const char * str)
{
	if (str) {
		return strdup(str);
	}
	return NULL;
}
