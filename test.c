//
// Created by jude on 29/03/2026.
//
#include <sodium.h>
#include <stdio.h>

int main(void)
{
    if (sodium_init() < 0) {
        return 1;
    }

    unsigned char pk[crypto_sign_PUBLICKEYBYTES];
    unsigned char sk[crypto_sign_SECRETKEYBYTES];

    crypto_sign_keypair(pk, sk);

    printf("Public key:\n");
    for (int i = 0; i < crypto_sign_PUBLICKEYBYTES; i++)
        printf("%02x", pk[i]);

    printf("\n\nPrivate key:\n");
    for (int i = 0; i < crypto_sign_SECRETKEYBYTES; i++)
        printf("%02x", sk[i]);

    printf("\n");

    return 0;
}