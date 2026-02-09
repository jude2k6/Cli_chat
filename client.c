//
// Created by jude on 03/02/2026.
//

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>


int getServerFd() {
    int status;
    struct addrinfo hints;
    struct addrinfo *service_info; // will point to the results
    int socket_fd;
    int trys =0;

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream socket

    status = getaddrinfo("localhost", "5000", &hints, &service_info);
    socket_fd = socket(service_info->ai_family, service_info->ai_socktype, service_info->ai_protocol);
    int connection = connect(socket_fd, service_info->ai_addr, service_info->ai_addrlen);
    while ((connection == -1) && trys<10){
        connection = connect(socket_fd, service_info->ai_addr, service_info->ai_addrlen);
        trys++;

    }
    if (connection ==-1) {
        perror("Failed to connect to server: ");
        exit(-1);
    }
    return socket_fd;
}



void recieveMessage(int sender_fd) {
    char buf[256];
    int nbytes = recv(sender_fd, buf, sizeof(buf), 0);
    printf("Message: %s\n\n\n", buf);
}


void recieveMessage(int sender_fd) {
    char buf[256];
    int nbytes = recv(sender_fd, buf, sizeof(buf), 0);
    printf("Message: %s\n\n\n", buf);
}

void sendMessage(struct pollfd poll_fds[]) {
    char buf[256];
    fgets(buf, sizeof(buf), stdin);

   int bytes_sent= send(poll_fds[0].fd, buf, sizeof(buf), 0);
    printf("Sent %d,bytes to server \n\n\n",bytes_sent);
}


struct pollfd packPollFd(int socket_fd) {
    struct pollfd p;
    p.fd = socket_fd;
    p.events = POLLIN;
    p.revents = 0;
    return p;
}

int main() {
    int server_fd = getServerFd();
    struct pollfd poll_fds[2];
    poll_fds[0] = packPollFd(server_fd);
    poll_fds[1] = packPollFd(0);


    int ready;
    while (1) {
        ready = poll(poll_fds, 2, -1);
        if (ready == -1) {
            printf("Error");
            return -1;
        }
        for (int i = 0; i < 2; ++i) {
            if ((i == 0) && (poll_fds[i].revents & POLLIN)) {
                recieveMessage(poll_fds[0].fd);
            } else if ((i == 1) && (poll_fds[i].revents & POLLIN)) {
                sendMessage(poll_fds);
            }
        }
    }

    return 0;
}
