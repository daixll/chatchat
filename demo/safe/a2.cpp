#include <openssl/pem.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>  // For malloc, free
#include <string.h>

#define BLOCK_SIZE 1024 // 1KB

void handle_openssl_error() {
    unsigned long err_code;
    const char *err_msg;

    while ((err_code = ERR_get_error()) != 0) {
        err_msg = ERR_error_string(err_code, NULL);
        fprintf(stderr, "OpenSSL Error: %s\n", err_msg);
    }
}

EVP_PKEY* load_public_key_from_file(const char* file_path) {
    EVP_PKEY* pkey = NULL;
    FILE* fp = fopen(file_path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file: %s\n", file_path);
        return NULL;
    }

    pkey = PEM_read_PUBKEY(fp, NULL, NULL, NULL);
    fclose(fp);

    if (pkey == NULL) {
        fprintf(stderr, "Error reading public key from file: %s\n", file_path);
        ERR_print_errors_fp(stderr);
    }

    return pkey;
}

EVP_PKEY* load_private_key_from_file(const char* file_path) {
    EVP_PKEY* pkey = NULL;
    FILE* fp = fopen(file_path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file: %s\n", file_path);
        return NULL;
    }

    pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
    fclose(fp);

    if (pkey == NULL) {
        fprintf(stderr, "Error reading private key from file: %s\n", file_path);
        ERR_print_errors_fp(stderr);
    }

    return pkey;
}

void encrypt_with_public_key(EVP_PKEY* pkey, const unsigned char* in, size_t inlen, const char* output_file) {
    EVP_PKEY_CTX *ctx;
    unsigned char *out;
    size_t outlen;

        ctx = EVP_PKEY_CTX_new(pkey, NULL);
        if (!ctx) {
            fprintf(stderr, "Error creating context\n");
            return;
        }
        if (EVP_PKEY_encrypt_init(ctx) <= 0) {
            fprintf(stderr, "Error initializing encryption\n");
            EVP_PKEY_CTX_free(ctx);
            return;
        }
        if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
            fprintf(stderr, "Error setting RSA padding\n");
            EVP_PKEY_CTX_free(ctx);
            return;
        }

    for (size_t i = 0; i < inlen; i += BLOCK_SIZE) {
        size_t block_size = (i + BLOCK_SIZE <= inlen) ? BLOCK_SIZE : inlen - i;

        /* Determine buffer length */
        if (EVP_PKEY_encrypt(ctx, NULL, &outlen, in+i, block_size) <= 0) {
            fprintf(stderr, "Error determining buffer length\n");
            EVP_PKEY_CTX_free(ctx);
            return;
        }

        out = (unsigned char*)malloc(block_size);  // Allocate memory using malloc
        if (!out) {
            fprintf(stderr, "Error allocating memory\n");
            EVP_PKEY_CTX_free(ctx);
            return;
        }

        if (EVP_PKEY_encrypt(ctx, out, &outlen, in+i, block_size) <= 0) {
            fprintf(stderr, "Error encrypting data\n");
            free(out);  // Free memory using free
            EVP_PKEY_CTX_free(ctx);
            return;
        }

        /* Save encrypted data to file */
        FILE* outfile = fopen(output_file, "ab");
        if (!outfile) {
            fprintf(stderr, "Error opening output file: %s\n", output_file);
            free(out);  // Free memory using free
            EVP_PKEY_CTX_free(ctx);
            return;
        }
        fwrite(out, 1, block_size, outfile);
        fclose(outfile);
        printf("Encrypted data saved to file: %s\n", output_file);
    }
    /* Clean up */
    free(out);  // Free memory using free
    EVP_PKEY_CTX_free(ctx);
}

void decrypt_with_private_key(EVP_PKEY* pkey, const unsigned char* in, size_t inlen, const char* output_file) {
    EVP_PKEY_CTX *ctx;
    unsigned char *out;
    size_t outlen;

    ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) {
        fprintf(stderr, "Error creating context\n");
        return;
    }
    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        fprintf(stderr, "Error initializing decryption\n");
        EVP_PKEY_CTX_free(ctx);
        return;
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        fprintf(stderr, "Error setting RSA padding\n");
        EVP_PKEY_CTX_free(ctx);
        return;
    }

    for (size_t i = 0; i < inlen; i += BLOCK_SIZE) {
        size_t block_size = (i + BLOCK_SIZE <= inlen) ? BLOCK_SIZE : inlen - i;

        /* Determine buffer length */
        if (EVP_PKEY_decrypt(ctx, NULL, &outlen, in+i, block_size) <= 0) {
            fprintf(stderr, "Error determining buffer length\n");
            EVP_PKEY_CTX_free(ctx);
            return;
        }

        out = (unsigned char*)malloc(block_size);  // Allocate memory using malloc
        if (!out) {
            fprintf(stderr, "Error allocating memory\n");
            EVP_PKEY_CTX_free(ctx);
            return;
        }

        if (EVP_PKEY_decrypt(ctx, out, &outlen, in+i, block_size) <= 0) {
            fprintf(stderr, "Error decrypting data\n");
            free(out);  // Free memory using free
            EVP_PKEY_CTX_free(ctx);
            return;
        }

        /* Save decrypted data to file */
        FILE* outfile = fopen(output_file, "ab");
        if (!outfile) {
            fprintf(stderr, "Error opening output file\n");
            free(out);  // Free memory using free
            EVP_PKEY_CTX_free(ctx);
            return;
        }
        fwrite(out, 1, block_size, outfile);
        fclose(outfile);
        printf("Decrypted data saved to file: %s\n", output_file);
    }

    /* Clean up */
    free(out);  // Free memory using free
    EVP_PKEY_CTX_free(ctx);
}

int main() {
    const char* pubkey_file = "key/one/pub.pem";
    EVP_PKEY* pkey = load_public_key_from_file(pubkey_file);

    if (pkey) {
        const char* input_file = "data/txt.txt";
        const char* output_file = "data/encrypt_output/txt.bin";

        FILE* file = fopen(input_file, "rb");
        if (!file) {
            fprintf(stderr, "Error opening input file: %s\n", input_file);
            EVP_PKEY_free(pkey);
            return 1;
        }

        // Determine file size
        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Allocate buffer for file contents
        unsigned char* buffer = (unsigned char*)malloc(file_size);  // Allocate memory using malloc
        if (!buffer) {
            fprintf(stderr, "Error allocating memory\n");
            fclose(file);
            EVP_PKEY_free(pkey);
            return 1;
        }

        // Read file contents into buffer
        size_t bytes_read = fread(buffer, 1, file_size, file);
        fclose(file);

        if (bytes_read != file_size) {
            fprintf(stderr, "Error reading file\n");
            free(buffer);  // Free memory using free
            EVP_PKEY_free(pkey);
            return 1;
        }

        // Encrypt the file contents and save to output file
        encrypt_with_public_key(pkey, buffer, bytes_read, output_file);

        // Clean up
        free(buffer);  // Free memory using free
        EVP_PKEY_free(pkey);
    }

    const char* privkey_file = "key/one/pri.pem";
    pkey = load_private_key_from_file(privkey_file);
    if (pkey) {
        const char* input_file = "data/encrypt_output/txt.bin";
        const char* output_file = "data/decrypt_output/txt.txt";

        FILE* file = fopen(input_file, "rb");
        if (!file) {
            fprintf(stderr, "Error opening input file: %s\n", input_file);
            EVP_PKEY_free(pkey);
            return 1;
        }

                // Determine file size
        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Allocate buffer for file contents
        unsigned char* buffer = (unsigned char*)malloc(file_size);  // Allocate memory using malloc
        if (!buffer) {
            fprintf(stderr, "Error allocating memory\n");
            fclose(file);
            EVP_PKEY_free(pkey);
            return 1;
        }

        // Read file contents into buffer
        size_t bytes_read = fread(buffer, 1, file_size, file);
        fclose(file);

        if (bytes_read != file_size) {
            fprintf(stderr, "Error reading file\n");
            free(buffer);  // Free memory using free
            EVP_PKEY_free(pkey);
            return 1;
        }

        // Decrypt the file contents and save to output file
        decrypt_with_private_key(pkey, buffer, bytes_read, output_file);

        // Clean up
        free(buffer);  // Free memory using free
        EVP_PKEY_free(pkey);
    }

    return 0;
}
