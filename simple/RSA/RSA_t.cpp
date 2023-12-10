#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY_SIZE 2048

void handleErrors() {
    fprintf(stderr, "Error occurred.\n");
    exit(EXIT_FAILURE);
}

void encryptAndDecryptExample() {
    EVP_PKEY_CTX *ctx = NULL;
    unsigned char *out = NULL, *decrypted = NULL;
    size_t outlen, decryptedlen;

    // Generate RSA key pair
    EVP_PKEY *keypair = NULL;
    EVP_PKEY_CTX *keygen_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    EVP_PKEY_keygen_init(keygen_ctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(keygen_ctx, KEY_SIZE);
    EVP_PKEY_keygen(keygen_ctx, &keypair);
    EVP_PKEY_CTX_free(keygen_ctx);

    // Input data
    const char *inputData = "Hello, OpenSSL!";
    size_t inputDataLen = strlen(inputData);

    // Encrypt
    ctx = EVP_PKEY_CTX_new(keypair, NULL);
    if (!ctx) handleErrors();

    if (EVP_PKEY_encrypt_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
        handleErrors();

    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, (unsigned char *)inputData, inputDataLen) <= 0)
        handleErrors();

    out = (unsigned char *)OPENSSL_malloc(outlen);
    if (!out) handleErrors();

    if (EVP_PKEY_encrypt(ctx, out, &outlen, (unsigned char *)inputData, inputDataLen) <= 0)
        handleErrors();

    // Decrypt
    if (EVP_PKEY_decrypt_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
        handleErrors();

    if (EVP_PKEY_decrypt(ctx, NULL, &decryptedlen, out, outlen) <= 0)
        handleErrors();

    decrypted = (unsigned char *)OPENSSL_malloc(decryptedlen);
    if (!decrypted) handleErrors();

    if (EVP_PKEY_decrypt(ctx, decrypted, &decryptedlen, out, outlen) <= 0)
        handleErrors();

    // Print results
    printf("Original Data: %s\n", inputData);
    printf("Encrypted Data: ");
    for (size_t i = 0; i < outlen; ++i)
        printf("%02X", out[i]);
    printf("\n");

    printf("Decrypted Data: %s\n", decrypted);

    // Cleanup
    OPENSSL_free(out);
    OPENSSL_free(decrypted);
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(keypair);
}

int main() {
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();

    // Seed the random number generator
    RAND_load_file("/dev/urandom", 32);

    // Run the example
    encryptAndDecryptExample();

    // Clean up OpenSSL
    EVP_cleanup();

    return 0;
}
