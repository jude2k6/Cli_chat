//
// Created by jude on 12/02/2026.
//

#ifndef UNTITLED3_AUTH_H
#define UNTITLED3_AUTH_H


#include <stdio.h>
#include <unistd.h>
#include <sodium.h>
#include <sqlite3.h>
#include  <sys/stat.h>


#include "../network/packet.h"
#include "../db/db.h"


#define NONCE_SIZE 16


// this function needs to be expanded preopely rn its just a test function
void request_auth(int fd) {
    printf("Trying to send auth request\n");
    packet_t packet;
    packet.header.id = 1;
    packet.header.type = AUTH_REQ;
    packet.payload = malloc(crypto_sign_SECRETKEYBYTES);
    unsigned char nonce[crypto_sign_SECRETKEYBYTES];
    randombytes_buf(nonce, crypto_sign_SECRETKEYBYTES);
    memcpy(packet.payload, nonce, crypto_sign_SECRETKEYBYTES);
    packet.header.length = crypto_sign_SECRETKEYBYTES;
    int bytes = forward_packet(&packet, fd);
    free(packet.payload);
    printf("auth packet sent %d\n", bytes);
}

void send_auth(packet_t packet, int server_fd) {
    unsigned char nonce[crypto_sign_SECRETKEYBYTES];
    memcpy(nonce, packet.payload, crypto_sign_SECRETKEYBYTES);
    const char *home = getenv("HOME");
    char path[512];
    snprintf(path, sizeof(path), "%s/.CLI_Chat/SecretKey", home);
    FILE *file = fopen(path, "rb");
    unsigned char private_key[crypto_sign_SECRETKEYBYTES];
    if (file == NULL) {



        file = fopen(path, "wb+");
        unsigned char pk[crypto_sign_PUBLICKEYBYTES];
        unsigned char sk[crypto_sign_SECRETKEYBYTES];
        crypto_sign_keypair(pk, sk);
        fwrite(sk,crypto_sign_SECRETKEYBYTES, 1, file);
        fclose(file);
        send_auth(packet, server_fd);
    }
    fread(private_key,crypto_sign_SECRETKEYBYTES, 1, file);
    unsigned char sig[crypto_sign_SECRETKEYBYTES];
    unsigned long long sig_len = crypto_sign_SECRETKEYBYTES;
    crypto_sign_detached(sig, &sig_len, nonce, crypto_sign_SECRETKEYBYTES, private_key);
    packet_t send_packet;
    send_packet.header.id = 0;
    send_packet.header.type = AUTH_SEND;
    send_packet.header.length = crypto_sign_SECRETKEYBYTES;
    send_packet.payload = malloc(crypto_sign_SECRETKEYBYTES);
    memcpy(send_packet.payload, sig, crypto_sign_SECRETKEYBYTES);
    forward_packet(&send_packet, server_fd);
    free(packet.payload);
}

int confirm_auth(packet_t packet, sqlite3 *db) {
    printf("Verification started");;

    uint32_t uid = ntohl(packet.header.id);
    unsigned char *public_key = get_public_key(db, uid);
    unsigned char sig[crypto_sign_SECRETKEYBYTES];
    memcpy(sig, packet.payload,crypto_sign_SECRETKEYBYTES);
    unsigned char nonce[crypto_sign_SECRETKEYBYTES];
    memcpy(nonce, packet.payload + crypto_sign_SECRETKEYBYTES,crypto_sign_SECRETKEYBYTES);
    int auth = crypto_sign_verify_detached(sig, nonce,NONCE_SIZE, public_key);
    free(public_key);

    printf("Client verified");;
    return auth;
}

int register_user() {

}


#endif //UNTITLED3_AUTH_H
