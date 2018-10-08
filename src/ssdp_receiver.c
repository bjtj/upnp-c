#include "ssdp_receiver.h"


ssdp_receiver_t * create_ssdp_receiver(void)
{
	int on = 1;
	struct addrinfo hints, * res;
	struct ip_mreq mreq;
	ssdp_receiver_t * receiver = (ssdp_receiver_t*)malloc(sizeof(ssdp_receiver_t));
	memset(receiver, 0, sizeof(ssdp_receiver_t));
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	assert(getaddrinfo(NULL, SSDP_PORTSTR, &hints, &res) == 0);
	receiver->sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	assert(receiver->sock >= 0);
	assert(setsockopt(receiver->sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) == 0);
	assert(bind(receiver->sock, res->ai_addr, res->ai_addrlen) == 0);
	freeaddrinfo(res);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_NUMERICHOST;
	assert(getaddrinfo(SSDP_HOST, NULL, &hints, &res) == 0);
	memcpy(&mreq.imr_multiaddr, &(((struct sockaddr_in*)res->ai_addr)->sin_addr), sizeof(mreq.imr_multiaddr));
	freeaddrinfo(res);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	assert(setsockopt(receiver->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == 0);
	FD_ZERO(&receiver->read_fds);
	FD_SET(receiver->sock, &(receiver->read_fds));
	return receiver;
}

void free_ssdp_receiver(ssdp_receiver_t * receiver)
{

	/*
	 * IP_DROP_MEMBERSHIP is not necessary
	 * ----
	 * [http://www.tldp.org/HOWTO/Multicast-HOWTO-6.html]
	 */
	close(receiver->sock);
	free(receiver);
}

int pending_ssdp_receiver(ssdp_receiver_t * receiver, unsigned long wait_milli)
{
	fd_set fds = receiver->read_fds;
	struct timeval timeout;
	timeout.tv_sec = wait_milli / 1000;
	timeout.tv_usec = (wait_milli % 1000) * 1000;
	return select(receiver->sock + 1, &fds, NULL, NULL, &timeout);
}

ssdp_header_t * receive_ssdp_header(ssdp_receiver_t * receiver)
{
	char buffer[SSDP_PACKET_MAX] = {0,};
	struct sockaddr_in addr = {0,};
	socklen_t addr_len = sizeof(addr);
	ssdp_header_t * ssdp = NULL;
	int len;
	len = recvfrom(receiver->sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addr_len);
	if (len > 0) {
		ssdp = read_ssdp_header(buffer);
		ssdp->remote_addr = (struct sockaddr*)malloc(addr_len);
		memcpy(ssdp->remote_addr, &addr, addr_len);
		ssdp->remote_addr_len = addr_len;
	}
	return ssdp;
}
