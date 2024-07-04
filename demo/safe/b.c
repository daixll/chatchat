#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <openssl/pem.h>

#define MBLOCK 256 // 256byte, 2048 bit

int8_t ERRIF(int8_t flg, const char* target, const char* msg){
    if(flg == 1){
        perror(target);
        perror(msg);
        exit(1);
    }
    return 1;
}

EVP_PKEY* load_key_from_file(const char* filename, int8_t flg){
    FILE* fptr = fopen(filename, "rb");                 // 以二进制方式打开文件
    ERRIF(fptr == NULL, filename, "打开文件错误");

    EVP_PKEY* pkey = NULL;                              // 密钥结构体
    if(flg == 0)
        pkey = PEM_read_PUBKEY(fptr, NULL, NULL, NULL);
    else
        pkey = PEM_read_PrivateKey(fptr, NULL, NULL, NULL);
    ERRIF(pkey == NULL, filename, "读取密钥错误");
    
    fclose(fptr);                                       // 关闭文件
    return pkey;
}

int main(char arg, char* argc[]){
    EVP_PKEY* pub_key = load_key_from_file(argc[1], 1); // 私钥 1
    FILE* fptr_in = fopen(argc[2], "rb");               // 密文 2
    ERRIF(fptr_in == NULL, argc[2], "打开文件错误");
    FILE* fptr_out = fopen(argc[3], "wb");              // 明文 3

    // 计算文件大小
    fseek(fptr_in, 0, SEEK_END);
    size_t flen = ftell(fptr_in);

    // 申请内存空间
    char* ptr1 = malloc(MBLOCK);
    ERRIF(ptr1 == NULL, "ptr1", "内存申请失败");
    char* ptr2 = malloc(MBLOCK * 1000000);
    ERRIF(ptr2 == NULL, "ptr2", "内存申请失败");

    // 初始化加解密上下文
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pub_key, NULL);
    ERRIF(ctx == NULL, "ctx", "初始化加解密上下文失败");
    ERRIF(EVP_PKEY_decrypt_init(ctx) <= 0, "ctx", "初始化解密失败");
    ERRIF(EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0, "ctx", "设置加密填充参数失败");

    // 分段读取文件并解密
    size_t Outlen = 0;
    for(size_t i=0; i<flen; i+=MBLOCK){
        // 计算每块的长度 i,i+len
        fseek(fptr_in, i, SEEK_SET);
        size_t len = i+MBLOCK > flen ? flen-i : MBLOCK;
        // 将密文读入内存
        size_t read_len = fread(ptr1, 1, len, fptr_in);
        ERRIF(read_len != len, argc[2], "密文 -> 内存 [错误]");
        /* 解密 */
        // 确定解密后长度
        size_t outlen = 0;
        ERRIF(EVP_PKEY_decrypt(ctx, NULL, &outlen, ptr1, len) <= 0, "ctx", "计算解密后长度失败");
        // 解密
        ERRIF(EVP_PKEY_decrypt(ctx, ptr2+Outlen, &outlen, ptr1, len) <= 0, "ctx", "解密失败");
        Outlen += outlen;
    }
    // 将内存写入明文
    size_t write_len = fwrite(ptr2, 1, Outlen, fptr_out);
    ERRIF(write_len != Outlen, argc[3], "内存 -> 明文 [错误]");

    // 释放资源
    EVP_PKEY_CTX_free(ctx);

    free(ptr2);
    free(ptr1);
    fclose(fptr_out);
    fclose(fptr_in);
    return 0;
}