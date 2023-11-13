#ifndef __SSDP_RECEIVER_H__
#define __SSDP_RECEIVER_H__

#include "public.h"
#include "ssdp.h"
#include "ssdp_header.h"

typedef struct _ssdp_receiver_t
{
  int sock;
	fd_set read_fds;
} ssdp_receiver_t;

extern ssdp_receiver_t * create_ssdp_receiver(void);
extern void free_ssdp_receiver(ssdp_receiver_t * receiver);
extern int pending_ssdp_receiver(ssdp_receiver_t * receiver, unsigned long wait_milli);
extern ssdp_header_t * receive_ssdp_header(ssdp_receiver_t * receiver);

#endif
