#include "safe.h"

#include "util.h"

namespace SAFE {

Safe::Safe()
    : publicKey(nullptr, EVP_PKEY_free),
      privateKey(nullptr, EVP_PKEY_free),
      cipherText(nullptr, fclose),
      plainText(nullptr, fclose) {}

Safe::~Safe() {};

void Safe::loadPubKeyFromFile(const std::string& fileName) {
  // 使用 std::unique_ptr 管理 FILE*，并绑定 fclose 作为删除器
  std::unique_ptr<FILE, decltype(&fclose)> fptr(fopen(fileName.c_str(), "rb"),
                                                fclose);
  UTIL::ERRIF(fptr == nullptr, fileName, "打开公钥");

  // 使用 OpenSSL 的 PEM_read_PUBKEY 函数读取公钥
  EVP_PKEY* rawKey = PEM_read_PUBKEY(fptr.get(), nullptr, nullptr, nullptr);
  UTIL::ERRIF(rawKey == nullptr, fileName, "读取公钥");

  // 将 rawKey 交给 std::unique_ptr 管理，并绑定 EVP_PKEY_free 作为删除器
  publicKey.reset(rawKey);  // publicKey 已经使用自定义删除器声明
}

void Safe::loadPriKeyFromFile(const std::string& fileName) {
  // 使用 std::unique_ptr 管理 FILE*，并绑定 fclose 作为删除器
  std::unique_ptr<FILE, decltype(&fclose)> fptr(fopen(fileName.c_str(), "rb"),
                                                fclose);
  UTIL::ERRIF(fptr == nullptr, fileName, "打开私钥");

  // 使用 OpenSSL 的 PEM_read_PrivateKey 函数读取私钥
  EVP_PKEY* rawKey = PEM_read_PrivateKey(fptr.get(), nullptr, nullptr, nullptr);
  UTIL::ERRIF(rawKey == nullptr, fileName, "读取私钥");

  // 将 rawKey 交给 std::unique_ptr 管理，并绑定 EVP_PKEY_free 作为删除器
  privateKey.reset(rawKey);  // privateKey 已经使用自定义删除器声明
}

}  // namespace SAFE