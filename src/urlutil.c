#include "urlutil.h"
#include "strutil.h"


char * url_relative(const char * url, const char * relative) {
	const char * first_sep;
	const char * last_sep;
	const char * beg = strstr(url, "//");
	const char * end = url + strlen(url);
	if (beg) {
		beg += 2;
	} else {
		beg = url;
	}

	first_sep = strstr(beg, "/");
	last_sep = strstr_last(beg, "/");
	if (*relative == '/') {
		int size;
		char * ret;
		if (first_sep == NULL) {
			first_sep = end;
		}
		size = (first_sep - url) + strlen(relative) + 1;
		ret = (char*)malloc(size);
		memset(ret, 0, sizeof(size));
		strncat(ret, url, first_sep - url);
		strcat(ret, relative);
		return ret;
	} else {
		int size;
		char * ret;
		if (last_sep == NULL) {
			last_sep = end;
		}
		size = (last_sep - url) + 1 + strlen(relative) + 1;
		ret = (char*)malloc(size);
		memset(ret, 0, sizeof(size));
		strncat(ret, url, last_sep - url);
		strcat(ret, "/");
		strcat(ret, relative);
		return ret;
	}
}
