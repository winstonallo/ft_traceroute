#ifndef TRACEROUTE_H
#define TRACEROUTE_H

#include <arpa/inet.h>
#include <stdint.h>

#define ICMP_PAYLOAD_SIZE 60
#define RECV_BUFFER_SIZE 64

int socket_init();

__attribute__((warn_unused_result)) int icmp_init_header(uint8_t *packet, const uint8_t seq, const char *const target_addr);
__attribute__((warn_unused_result)) int icmp_send_packet(uint8_t *packet, const int sockfd, struct sockaddr_in *saddr);
__attribute__((warn_unused_result)) int icmp_recv_packet(const int sockfd, const uint8_t seq, struct sockaddr_in *saddr);

#endif
