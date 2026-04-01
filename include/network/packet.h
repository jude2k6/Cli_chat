//
// Created by jude on 08/02/2026.
//

#ifndef UNTITLED3_PACKET_H
#define UNTITLED3_PACKET_H
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>


#define MESSAGE 0
#define SYSTEM 1
#define COMMAND 2
#define AUTH_REQ 3
#define AUTH_SEND 4
#define REGISTER 5

#define HEADER_WIRE_SIZE 7


typedef struct Header {
    uint32_t id;
    uint16_t length;
    uint8_t type;
} Header;

typedef struct packet_t {
    Header header;
    uint8_t *payload;
} packet_t;








#include <stdio.h>
#include <ctype.h>

void hexdump(const void *buf, size_t len) {
    const unsigned char *data = buf;

    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);

        if ((i + 1) % 16 == 0) {
            printf(" | ");
            for (size_t j = i - 15; j <= i; j++) {
                printf("%c", isprint(data[j]) ? data[j] : '.');
            }
            printf("\n");
        }
    }
    printf("\n");
}

int forward_packet(packet_t *packet,int fd) {

    int buffer_size = HEADER_WIRE_SIZE + packet->header.length;
    uint8_t buffer[buffer_size];
    int payload_size = packet->header.length;
    packet->header.id = htonl(packet->header.id);
    packet->header.length = htons(packet->header.length);
    uint8_t *buffer_offset = buffer;
    memcpy(buffer_offset, &packet->header.id,sizeof(uint32_t) );
    buffer_offset+=sizeof(uint32_t);
    memcpy(buffer_offset, &packet->header.length, sizeof(uint16_t) );
    buffer_offset+=sizeof(uint16_t);
    memcpy(buffer_offset, &packet->header.type, sizeof(uint8_t) );
    buffer_offset+=sizeof(uint8_t);
    memcpy(buffer_offset, packet->payload,payload_size );
    hexdump(buffer,72);
    int num = send(fd,buffer,buffer_size,0);
    return num;
}




packet_t recieve_packet(const int fd) {
    packet_t packet;
    Header header;
    uint8_t header_buffer[HEADER_WIRE_SIZE ];
    uint8_t* buffer_offset = header_buffer;
    int total = 0;
    while (total < HEADER_WIRE_SIZE) {
        total += recv(fd, header_buffer+total, HEADER_WIRE_SIZE-total, 0);
    }

    hexdump(header_buffer,7);
    memcpy(&header.id,buffer_offset,sizeof(uint32_t));
    buffer_offset+=sizeof(uint32_t);
    memcpy(&header.length,buffer_offset,sizeof(uint16_t));
    buffer_offset+=sizeof(uint16_t);
    memcpy(&header.type,buffer_offset,sizeof(uint8_t));
    header.length = ntohs(header.length);
    header.id = ntohl(header.id);
    packet.header = header;
    packet.payload = (uint8_t *) malloc(header.length);
    total = 0;
    while (total < sizeof(header)) {
       total += recv(fd, packet.payload, packet.header.length, 0);
    }


    return packet;
}


#endif //UNTITLED3_PACKET_H
