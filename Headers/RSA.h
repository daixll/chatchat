#pragma once

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/rand.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>

namespace jiao {

// 加密类
class RSA {

public:
    RSA(const std::string& public_key_path="", const std::string& private_key_path="", bool flg=0);
    ~RSA();

    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& plain_text);
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& cipher_text);

    std::string pubk;   // 公钥

    void show(std::string& str);
private:
    EVP_PKEY *pub_key;
    EVP_PKEY *pri_key;
    bool pub_st=0;      // 公钥未初始化
    bool pri_st=0;      // 私钥未初始化

    void ERR(const std::string& msg);
    void WAR(const std::string& msg);
    std::string read_file(const std::string& path);
};
}   // namespace jiao