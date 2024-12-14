#include <bits/types/struct_iovec.h>
#include <ifaddrs.h>
#include <mem.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <traceroute.h>

__attribute__((always_inline)) static inline int
get_target_addr(const char *target, struct sockaddr_in *const target_addr) {
    struct addrinfo hints = {0}, *res;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;

    int err = getaddrinfo(target, NULL, &hints, &res);
    if (err != 0) {
        fprintf(stderr, "ft_ping: %s: %s\n", target, gai_strerror(err));
        return -1;
    }

    // Create new temporary variable so we can copy the values of res into target_addr
    // and free it.
    // This allows us to only keep a stack-allocated copy of the destination address
    // and makes memory management easier.
    struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
    target_addr->sin_family = addr->sin_family;
    target_addr->sin_addr = addr->sin_addr;

    freeaddrinfo(res);
    return 0;
}

int
main(int ac, char **av) {
    if (ac != 2) {
        fprintf(stderr, "Usage:\n  ./ft_traceroute [HOST]\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in target_addr;
    if (get_target_addr(av[1], &target_addr) != 0) {
        return EXIT_FAILURE;
    }

    int sockfd = socket_init();
    if (sockfd == -1) {
        return EXIT_FAILURE;
    }

    uint8_t packet[ICMP_PAYLOAD_SIZE] = {0};
    ft_memset((void *)packet + sizeof(struct icmp), 0x42, ICMP_PAYLOAD_SIZE - sizeof(struct icmp));

    printf("traceroute to %s (%s), 30 hops max, %d byte packets\n", av[1], av[1], ICMP_PAYLOAD_SIZE);

    for (int seq = 1; seq <= 30; ++seq) {

        if (icmp_init_header(packet, seq, av[1]) == -1) {
            return EXIT_FAILURE;
        }

        if (icmp_send_packet(packet, sockfd, &target_addr) == -1) {
            return EXIT_FAILURE;
        }

        int recv_res = icmp_recv_packet(packet, sockfd, seq, &target_addr);
        if (recv_res == -1) {
            return EXIT_FAILURE;
        } else if (recv_res == 0) {
            break;
        }
    }

    return EXIT_SUCCESS;
}
