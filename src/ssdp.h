#ifndef __SSDP_H__
#define __SSDP_H__

#define SSDP_HOST "239.255.255.250"
#define SSDP_PORT 1900
#define SSDP_PORTSTR "1900"
#define SSDP_PACKET_MAX 4096

typedef enum _notify_type_e
{
	NTS_UNKNOWN, NTS_ALIVE, NTS_UPDATE, NTS_BYEBYE
} notify_type_e;

typedef enum _ssdp_type_e {
	SSDP_UNKNOWN, SSDP_MSEARCH, SSDP_NOTIFY, SSDP_RESPONSE
} ssdp_type_e;

#endif
