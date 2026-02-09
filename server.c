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
#include <stdlib.h>
#include <errno.h>


#define MAX_CLIENTS 10


int getListener() {
    struct addrinfo hints;
    struct addrinfo *service_info; // will point to the results
    int socket_fd, new_fd;

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

void handleConnection(struct pollfd poll_fds[], nfds_t *nConnections) {
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;
    int new_fd = accept(poll_fds[0].fd, (struct sockaddr *) &their_addr, &addr_size);
    struct pollfd new_fd_struct;
    new_fd_struct.fd = new_fd;
    new_fd_struct.events = POLLIN;
    new_fd_struct.revents = 0;
    poll_fds[*nConnections + 1] = new_fd_struct;
    *nConnections +=1;
    printf("Connect client on fd %d\n\n\n",new_fd);
}

void handleDisconnection(int remove_index, struct pollfd poll_fds[], nfds_t *nConnections) {
    printf("Removed client on fd %d \n\n\n",poll_fds[remove_index].fd);
    poll_fds[remove_index] = poll_fds[*nConnections];
    *nConnections -=1;

}

struct pollfd packPollFd(int socket_fd) {
    struct pollfd p;
    p.fd = socket_fd;
    p.events = POLLIN;
    p.revents = 0;
    return  p;
}

void handleClient(int sender_fd, struct pollfd poll_fds[], nfds_t *nConnections) {
    char buf[256];
    int nbytes = recv(sender_fd, buf, sizeof(buf), 0);
    if (nbytes <= 0) {

        if (nbytes == 0) {
            handleDisconnection(sender_fd,poll_fds,nConnections);
        }
    } else {
        printf("Message from %d: %s \n\n\n", sender_fd,buf); //forward message to all other clients


        for (int i = 1; i <= *nConnections; ++i) {
            if (poll_fds[i].fd != sender_fd) {
                send(poll_fds[i].fd,buf,nbytes,0);
            }

        }
    }
}



int main() {
    int listener_fd = getListener();

    struct pollfd poll_fds[MAX_CLIENTS];
    poll_fds[0] = packPollFd(listener_fd);

    nfds_t nConnections = 0;

    int ready;

    while (1) {
        printf("in while loop\n\n\n");
        ready = poll(poll_fds, nConnections + 1, -1);
        if (ready == -1) {
            printf("Error");
            return -1;
        }
        printf("Number of ready on poll %d \n\n\n",ready);

        for (nfds_t i = 0; i < nConnections + 1; i++) {
            if ((i == 0) && (poll_fds[i].revents & POLLIN)) {

                printf("Trying to handle connection\n\n\n");
                handleConnection(poll_fds, &nConnections);
            } else if (poll_fds[i].revents & POLLIN) {

                handleClient(poll_fds[i].fd, poll_fds, &nConnections);
            }
        }
    }


    return 0;
}
