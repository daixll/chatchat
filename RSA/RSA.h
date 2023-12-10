#define DEBUG 1
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/rand.h>
#include <string>
#include <fstream>
#include <iostream>
namespace jiao {

class RSA {

public:
    RSA(const std::string& public_key_path, const std::string& private_key_path);
    ~RSA();

    std::string encrypt(const std::string& plain_text);
    std::string decrypt(const std::string& cipher_text);

private:
    EVP_PKEY *pub_key;
    EVP_PKEY *pri_key;

    void ERR(const std::string& msg) {
        std::cerr << "错误！" << msg << std::endl;
        exit(1);
    }

    std::string read_file(const std::string& path) {
        std::ifstream ifs(path);
        if (!ifs) 
            ERR("无法打开文件 " + path);
        std::string content;
        ifs.seekg(0, std::ios::end);
        content.resize(ifs.tellg());
        ifs.seekg(0, std::ios::beg);
        ifs.read(&content[0], content.size());
        ifs.close();
        return content;
    }
};


RSA::RSA(const std::string& public_key_path, const std::string& private_key_path) {
    // 初始化 OpenSSL 库
    OpenSSL_add_all_algorithms();
    // 更强的随机数生成器
    RAND_load_file("/dev/urandom", 32);
    // 读取公钥和私钥
    std::string public_key_str  = read_file(public_key_path);
    std::string private_key_str = read_file(private_key_path);
    // 初始化公钥
    BIO *bio = BIO_new_mem_buf(public_key_str.c_str(), -1);
    pub_key = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    // 初始化私钥
    bio = BIO_new_mem_buf(private_key_str.c_str(), -1);
    pri_key = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
}

RSA::~RSA() {
    EVP_PKEY_free(pub_key);             // 释放公钥
    EVP_PKEY_free(pri_key);             // 释放私钥
    EVP_cleanup();                      // 清理 OpenSSL 库
}

std::string RSA::encrypt(const std::string& plain_text) {
    // 初始化加密上下文
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pub_key, nullptr);
    if (!ctx) ERR("无法初始化加密上下文");
    // 设置加密参数
    if (EVP_PKEY_encrypt_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
        ERR("无法设置加密参数");
    // 计算加密后的长度
    size_t outlen;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, (unsigned char *)plain_text.c_str(), plain_text.size()) <= 0)
        ERR("无法计算加密后的长度");
    // 加密
    unsigned char *out = (unsigned char *)OPENSSL_malloc(outlen);
    if (!out) ERR("无法分配内存");
    if (EVP_PKEY_encrypt(ctx, out, &outlen, (unsigned char *)plain_text.c_str(), plain_text.size()) <= 0)
        ERR("无法加密");
    // 释放加密上下文
    EVP_PKEY_CTX_free(ctx);
    // 返回加密后的字符串
    return std::string((char *)out, outlen);
}

std::string RSA::decrypt(const std::string& cipher_text) {
    // 初始化解密上下文
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pri_key, nullptr);
    if (!ctx) ERR("无法初始化解密上下文");
    // 设置解密参数
    if (EVP_PKEY_decrypt_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
        ERR("无法设置解密参数");
    // 计算解密后的长度
    size_t outlen;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, (unsigned char *)cipher_text.c_str(), cipher_text.size()) <= 0)
        ERR("无法计算解密后的长度");
    // 解密
    unsigned char *out = (unsigned char *)OPENSSL_malloc(outlen);
    if (!out) ERR("无法分配内存");
    if (EVP_PKEY_decrypt(ctx, out, &outlen, (unsigned char *)cipher_text.c_str(), cipher_text.size()) <= 0)
        ERR("无法解密");
    // 释放解密上下文
    EVP_PKEY_CTX_free(ctx);
    // 返回解密后的字符串
    return std::string((char *)out, outlen);
}

}   // namespace jiao