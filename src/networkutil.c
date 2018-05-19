#include "networkutil.h"


char * get_ipaddr_str(struct sockaddr * addr)
{
	char ipstr[INET6_ADDRSTRLEN] = {0,};
	inet_ntop(addr->sa_family,
			  (addr->sa_family == AF_INET ?
			   (void*)&((struct sockaddr_in*)addr)->sin_addr :
			   (void*)&((struct sockaddr_in6 *)addr)->sin6_addr),
			  ipstr, sizeof(ipstr));
	return strdup(ipstr);
}

char * get_ipv4(void)
{
	char * ret = NULL;
	struct ifaddrs * ifaddrs, * tmp;
	getifaddrs(&ifaddrs);
	tmp = ifaddrs;
	for (; tmp; tmp = tmp->ifa_next) {
		if (tmp->ifa_addr->sa_family == AF_INET &&
			(tmp->ifa_flags & IFF_LOOPBACK) == 0) {
			ret = get_ipaddr_str(tmp->ifa_addr);
			break;
		}
	}
	freeifaddrs(ifaddrs);
	return ret;
}
