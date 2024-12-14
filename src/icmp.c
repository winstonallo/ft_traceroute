#include "traceroute.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

__attribute__((always_inline)) static inline uint8_t
checksum(const void *const buffer, int len) {
    const unsigned short *buf = buffer;
    unsigned int sum = 0;

    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++;
    }
    if (len == 1) {
        sum += *(unsigned char *)buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    return ~sum;
}

// Initializes the ip/icmp header into `packet`.
//
// Other than in ping, `seq` is used as the time-to-live, which will be incrementally
// altered in order to get the IP addresses of all the hops.
int
icmp_init_header(uint8_t *packet, const uint8_t seq, const char *const target_addr) {
    struct iphdr *ip_hdr = (struct iphdr *)packet;
    struct icmp *icmp_hdr = (struct icmp *)(packet + sizeof(struct iphdr));

    pid_t pid = getpid();

    ip_hdr->ihl = 5;
    ip_hdr->version = 4;
    ip_hdr->tos = 0;
    ip_hdr->tot_len = htons(ICMP_PAYLOAD_SIZE);
    ip_hdr->id = htons(pid);
    ip_hdr->frag_off = 0;
    ip_hdr->ttl = seq;
    ip_hdr->protocol = IPPROTO_ICMP;

    struct in_addr saddr, daddr;

    if (inet_pton(AF_INET, "0.0.0.0", &saddr) != 1) {
        fprintf(stderr, "ft_traceroute: inet_pton (saddr): %s\n", strerror(errno));
        return -1;
    }
    ip_hdr->saddr = saddr.s_addr;

    if (inet_pton(AF_INET, target_addr, &daddr) != 1) {
        fprintf(stderr, "ft_traceroute: inet_pton (daddr): %s\n", strerror(errno));
        return -1;
    }
    ip_hdr->daddr = daddr.s_addr;

    ip_hdr->check = 0;
    ip_hdr->check = checksum(ip_hdr, sizeof(struct iphdr));

    icmp_hdr->icmp_type = ICMP_ECHO;
    icmp_hdr->icmp_id = pid;
    icmp_hdr->icmp_seq = seq;
    icmp_hdr->icmp_cksum = 0;
    icmp_hdr->icmp_cksum = checksum(packet, ICMP_PAYLOAD_SIZE);

    return 0;
}

// int
// icmp_send_packet(uint8_t *packet, const int sockfd, struct sockaddr_in *saddr) {



//     return 0;
// }

int
icmp_recv_packet(const int sockfd, const uint8_t seq, struct sockaddr_in *saddr) {

    while (true) {

        char buf[RECV_BUFFER_SIZE];

        socklen_t saddr_len = sizeof(*saddr);

        ssize_t recv_len = recvfrom(sockfd, buf, RECV_BUFFER_SIZE, 0, (struct sockaddr *)saddr, &saddr_len);

        struct iphdr *ip_hdr = (struct iphdr *)buf;
        size_t ip_hdr_len = ip_hdr->ihl << 2;
        struct icmp *icmp_hdr = (struct icmp *)(buf + ip_hdr_len);
        if (recv_len <= 0) {
            fprintf(stderr, "ft_traceroute: recvfrom: %s\n", strerror(errno));
            return -1;
        }

        char raddr_str[INET_ADDRSTRLEN];
        struct in_addr raddr;
        raddr.s_addr = ip_hdr->saddr;
        if (!inet_ntop(AF_INET, &raddr, raddr_str, INET_ADDRSTRLEN)) {
            fprintf(stderr, "ft_traceroute: inet_ntop: %s\n", strerror(errno));
            return -1;
        }

        if (icmp_hdr->icmp_type == ICMP_ECHO) {

            printf("icmp_hdr->icmp_type=%d\n", icmp_hdr->icmp_type);
        }

        if (icmp_hdr->icmp_type == ICMP_TIME_EXCEEDED) {
            printf(" %d  %s (%s)\n", seq, raddr_str, raddr_str);
            return 1;
        } else if (icmp_hdr->icmp_type == ICMP_ECHOREPLY) {
            printf(" %d  %s (%s)\n", seq, raddr_str, raddr_str);
            return 0;
        }
    }

    return 0;
}
