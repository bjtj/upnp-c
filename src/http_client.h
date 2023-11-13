#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include "public.h"
#include "http_response.h"
#include "listutil.h"


extern void http_client_global_init(void);
extern void http_client_global_release(void);
extern http_response_t * http_client_get_dump(const char * url,
                                              list_t * parameters);
extern http_response_t * http_client_post(const char * url,
                                          list_t * parameters,
                                          const char * type,
                                          const char * text);
extern http_response_t * http_client_custom(const char * url,
                                            list_t * parameters,
                                            const char * method);

#endif
