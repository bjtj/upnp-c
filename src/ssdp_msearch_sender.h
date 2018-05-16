#ifndef __SSDP_MSEARCH_SENDER_H__
#define __SSDP_MSEARCH_SENDER_H__

#include "public.h"
#include "ssdp.h"

typedef void (*__ssdp_response_handler_cb)(struct sockaddr *, const char *, void *);

typedef struct _ssdp_msearch_sender_t
{
	
    int sock;
	fd_set read_fds;
	void * user_data;
	__ssdp_response_handler_cb response_handler_cb;
} ssdp_msearch_sender_t;


extern ssdp_msearch_sender_t * create_ssdp_msearch_sender(void);
extern void ssdp_free_msearch_sender(ssdp_msearch_sender_t * sender);
extern void ssdp_send_msearch(ssdp_msearch_sender_t * sender, const char * type, int mx);
extern int ssdp_pending_msearch_sender(ssdp_msearch_sender_t * sender, unsigned long wait_milli);
extern void ssdp_receive_ssdp_response(ssdp_msearch_sender_t * sender);

#endif
