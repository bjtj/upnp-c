#include "http_client.h"
#include <curl/curl.h>


typedef struct _data_t {
    char * buffer;
	int buffer_size;
	int pos;
} data_t;


static size_t cb_write(void * contents, size_t size, size_t nmemb, void * userp) {
	data_t * data = (data_t*)userp;
	size_t total = size * nmemb;
	size_t read_size = total;
	if (total + data->pos > data->buffer_size) {
		read_size = data->buffer_size - data->pos;
	}
	if (read_size > 0) {
		memcpy(data->buffer + data->pos, contents, read_size);
	}
	data->pos += read_size;
	return total;
}

static size_t cb_header(char * buffer, size_t size, size_t nitems, void * userdata) {
	size_t total = nitems * size;
	http_response_t * response = (http_response_t*)userdata;
	http_header_t * header = http_response_get_header(response);
	str_t line = strutil_trim(strutil_str(buffer, buffer + total));
	if (strutil_len(line) == 0) {
		return total;
	}
	if (http_header_get_firstline(header) == NULL) {
		char * firstline = strutil_dup_cstr(&line);
		http_header_set_firstline_nocopy(header, firstline);
	} else {
		name_value_t * nv = http_header_read_parameter(&line);
		http_header_set_parameter(header, nv->name, nv->value);
		free_name_value(nv);
	}
	return total;
}


void http_client_global_init(void) {
	curl_global_init(CURL_GLOBAL_ALL);
}

void http_client_global_release(void) {
	curl_global_cleanup();
}

http_response_t * http_client_get_dump(const char * url, list_t * parameters) {
	struct curl_slist * headers = NULL;
	CURLcode res;
	CURL * curl;
	data_t data = {0,};
	http_response_t * response = NULL;
	char parameter[512] = {0,};
	list_t * lst = NULL;
	
	curl = curl_easy_init();
	if (!curl) {
		fprintf(stderr, "curl_easy_init() failed\n");
		return response;
	}

	data.buffer_size = 12 * 1024;
	data.buffer = (char*)malloc(data.buffer_size);
	memset(data.buffer, 0, data.buffer_size);

	response = create_http_response();

	lst = parameters;
	for (; lst; lst = lst->next) {
		name_value_t * nv =(name_value_t*)lst->data;
		snprintf(parameter, sizeof(parameter), "%s: %s", nv->name, nv->value);
		headers = curl_slist_append(headers, parameter);
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb_write);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, cb_header);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)response);
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed\n");
		goto done;
	}

	response->data = strndup(data.buffer, data.pos);
	response->data_size = data.pos;

done:
	curl_slist_free_all(headers);
	free(data.buffer);
	curl_easy_cleanup(curl);
	
	return response;
}


http_response_t * http_client_post(const char * url, list_t * parameters,
								   const char * type, const char * text) {
	struct curl_slist * headers = NULL;
	CURLcode res;
	CURL * curl;
	data_t data = {0,};
	http_response_t * response = NULL;
	char parameter[512] = {0,};
	list_t * lst = NULL;

	curl = curl_easy_init();
	if (!curl) {
		fprintf(stderr, "curl_easy_init() failed\n");
		return response;
	}

	data.buffer_size = 12 * 1024;
	data.buffer = (char*)malloc(data.buffer_size);
	memset(data.buffer, 0, data.buffer_size);

	response = create_http_response();

	snprintf(parameter, sizeof(parameter), "Content-Type: %s", type);
	headers = curl_slist_append(headers, parameter);
	lst = parameters;
	for (; lst; lst = lst->next) {
		name_value_t * nv =(name_value_t*)lst->data;
		snprintf(parameter, sizeof(parameter), "%s: %s", nv->name, nv->value);
		headers = curl_slist_append(headers, parameter);
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, text);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(text));
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb_write);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, cb_header);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)response);
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		goto done;
	}

	response->data = strndup(data.buffer, data.pos);
	response->data_size = data.pos;

done:
	curl_slist_free_all(headers);
	free(data.buffer);
	curl_easy_cleanup(curl);
	return response;
}

http_response_t * http_client_custom(const char * url, list_t * parameters, const char * method) {
	CURLcode res;
	CURL * curl;
	struct curl_slist * headers = NULL;
	data_t data = {0,};
	http_response_t * response = NULL;
	char parameter[512] = {0,};
	list_t * lst = NULL;
	
	curl = curl_easy_init();
	if (!curl) {
		fprintf(stderr, "curl_easy_init() failed\n");
		return response;
	}

	data.buffer_size = 12 * 1024;
	data.buffer = (char*)malloc(data.buffer_size);
	memset(data.buffer, 0, data.buffer_size);

	response = create_http_response();

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);

	lst = parameters;
	for (; lst; lst = lst->next) {
		name_value_t * nv =(name_value_t*)lst->data;
		snprintf(parameter, sizeof(parameter), "%s: %s", nv->name, nv->value);
		headers = curl_slist_append(headers, parameter);
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb_write);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, cb_header);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)response);
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed\n");
		goto done;
	}

	response->data = strndup(data.buffer, data.pos);
	response->data_size = data.pos;

done:
	curl_slist_free_all(headers);
	free(data.buffer);
	curl_easy_cleanup(curl);
	
	return response;
}
