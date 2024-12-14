#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int
socket_init() {

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd == -1) {
        fprintf(stderr, "ft_traceroute: socket: %s: raw socket creation requires root privileges\n", strerror(errno));
        return -1;
    }

    // The kernel creates the IP header for new sockets by itself. We prevent it by activating the `IP_HDRINCL` option,
    // in order to make it ourselves.
    // We need it in order to be able to send new ICMP packets with incremented TTL without having to create
    // a new socket everytime.
    const bool option_on = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &option_on, sizeof(bool)) == -1) {
        close(sockfd);
        fprintf(stderr, "ft_traceroute: setsockopt: %s\n", strerror(errno));
        return -1;
    }

    return sockfd;
}
