//
// Created by jude on 03/02/2026.
//
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <sqlite3.h>
#include "sodium.h"
#include "include/network/packet.h"
#include "include/auth/auth.h"
#define MAX_CLIENTS 10

typedef struct client_t {
    uint32_t id;
    int fd;
    char name[10];
} client_t;


int getListener() {
    struct addrinfo hints;
    struct addrinfo *service_info; // will point to the results
    int socket_fd;

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

    getaddrinfo(NULL, "5000", &hints, &service_info);
    socket_fd = socket(service_info->ai_family, service_info->ai_socktype, service_info->ai_protocol);
    bind(socket_fd, service_info->ai_addr, service_info->ai_addrlen);
    listen(socket_fd, 5);


    return socket_fd;
}


int handleConnection(struct pollfd poll_fds[], nfds_t *num_clients, client_t **clients) {


    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;
    int new_fd = accept(poll_fds[0].fd, (struct sockaddr *) &their_addr, &addr_size);
    struct pollfd new_fd_struct;
    new_fd_struct.fd = new_fd;
    new_fd_struct.events = POLLIN;
    new_fd_struct.revents = 0;
    poll_fds[*num_clients + 1] = new_fd_struct;
    *num_clients += 1;
    printf("Client connection fully handled\n");
    return new_fd;
}


void handleDisconnection(int remove_index, struct pollfd poll_fds[], nfds_t *num_clients) {
    printf("Removed client on fd %d \n\n\n", poll_fds[remove_index].fd);
    poll_fds[remove_index] = poll_fds[*num_clients];
    *num_clients -= 1;
}

struct pollfd packPollFd(int socket_fd) {
    struct pollfd p;
    p.fd = socket_fd;
    p.events = POLLIN;
    p.revents = 0;
    return p;
}


void send_message(int sender_fd, client_t **clients, nfds_t *num_clients, packet_t packet) {
    for (int i = 1; i <= *num_clients; ++i) {
        if (sender_fd != clients[i]->fd) {
            forward_packet(&packet, sender_fd);
        }
    }
}


void handle_packet_server(int sender_fd, client_t **clients, nfds_t *num_clients, sqlite3 *db) {
    packet_t packet = recieve_packet(sender_fd);
    switch (packet.header.type) {
        case MESSAGE: {
            send_message(sender_fd, clients, num_clients, packet);

            break;
        }
        case SYSTEM: {
            break;
        }
        case COMMAND: {
            break;
        }
        case AUTH_SEND: {
            printf("confirming auth\n");
                confirm_auth(packet,db);
            break;
        }
    }
}


int main() {
    if (sodium_init() < 0) {
        printf("Libsodium error");
        exit(-1);
    }

    sqlite3 *db;
    sqlite3_open("../users.db", &db);


    struct pollfd poll_fds[MAX_CLIENTS];
    client_t *clients[MAX_CLIENTS];


    int listener_fd = getListener();
    poll_fds[0] = packPollFd(listener_fd);

    nfds_t num_clients = 0;

    int ready;

    while (1) {
        ready = poll(poll_fds, num_clients + 1, -1);
        if (ready == -1) {
            printf("Error");
            return -1;
        }

        for (nfds_t i = 0; i < num_clients + 1; i++) {
            if ((i == 0) && (poll_fds[i].revents & POLLIN)) {
                int fd= handleConnection(poll_fds, &num_clients, clients);
                request_auth(fd);

            } else if (poll_fds[i].revents & POLLIN) {
                handle_packet_server(poll_fds[i].fd, clients, &num_clients,db);
            }
        }
    }


    return 0;
}
