#include "ssdp_msearch_sender.h"


ssdp_msearch_sender_t * create_ssdp_msearch_sender(void) {
	ssdp_msearch_sender_t * sender = (ssdp_msearch_sender_t*)malloc(sizeof(ssdp_msearch_sender_t));
	memset(sender, 0, sizeof(ssdp_msearch_sender_t));
	sender->sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert(sender->sock >= 0);
	FD_ZERO(&(sender->read_fds));
	FD_SET(sender->sock, &(sender->read_fds));
	return sender;
}

void ssdp_free_msearch_sender(ssdp_msearch_sender_t * sender) {
	close(sender->sock);
	free(sender);
}

void ssdp_send_msearch(ssdp_msearch_sender_t * sender, const char * type, int mx)
{
	const char * SSDP_FMT =
		"M-SEARCH * HTTP/1.1\r\n"
		"Host: %s:%d\r\n"
		"ST: %s\r\n"
		"MX: %d\r\n"
		"MAN: %s\r\n"
		"User-Agent: UPnP/1.0 Sample/1.0 Linux/1.0\r\n"
		"\r\n";

	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	char buffer[4096] = {0,};
	snprintf(buffer, sizeof(buffer), SSDP_FMT,
			 SSDP_HOST, SSDP_PORT, type, mx, "\"ssdp:discover\"");

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(SSDP_HOST);
	addr.sin_port = htons(SSDP_PORT);

	sendto(sender->sock, buffer, strlen(buffer), 0, (struct sockaddr *)&addr, addr_len);
}

int ssdp_pending_msearch_sender(ssdp_msearch_sender_t * sender, unsigned long wait_milli) {
	fd_set fds = sender->read_fds;
	struct timeval timeout;
	timeout.tv_sec = wait_milli / 1000;
	timeout.tv_usec = (wait_milli % 1000) * 1000;
	return select(sender->sock + 1, &fds, NULL, NULL, &timeout);
}

void ssdp_receive_ssdp_response(ssdp_msearch_sender_t * sender) {
	char buffer[4096] = {0,};
	struct sockaddr_in addr = {0,};
	socklen_t addr_len = sizeof(addr);
	int len = recvfrom(sender->sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addr_len);
	assert(len > 0);
	if (sender->response_handler_cb) {
		sender->response_handler_cb((struct sockaddr *)&addr, buffer, sender->user_data);
	}
}
