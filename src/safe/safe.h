#pragma once
#include <openssl/pem.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <memory>

namespace SAFE {

class Safe {
 public:
  Safe();
  ~Safe();
  void loadPubKeyFromFile(const std::string& fileName);
  void loadPriKeyFromFile(const std::string& fileName);

 protected:
  std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> publicKey,
      privateKey;  // 保存公钥和私钥
  std::unique_ptr<FILE, decltype(&fclose)> cipherText,
      plainText;  // 指向密文和明文
};

class Encrypt : public Safe {
 public:
  Encrypt();
  ~Encrypt();

  void encrypt(const std::string& inFileName,
               const std::string& outFileName);  // 文件 -> 文件
  // std::string encrypt(const std::string& str);   // 字符串 -> 字符串
  // private:
};

class Decrypt : public Safe {
 public:
  Decrypt();
  ~Decrypt();

  void decrypt(const std::string& inFileName,
               const std::string& outFileName);  // 文件 -> 文件

  // private:
};

}  // namespace SAFE