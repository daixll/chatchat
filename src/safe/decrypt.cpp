#include <thread>
#include <vector>
#include <memory>
#include <cstdio>
#include <cstdint>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include "safe.h"
#include "util.h"

const size_t MBLOCK = 256;  // 2048-bit RSA, 256 bytes

namespace SAFE {

Decrypt::Decrypt() : Safe() {}
Decrypt::~Decrypt() {}

void Decrypt::decrypt(const std::string& inFileName,
                      const std::string& outFileName) {
  cipherText.reset(fopen(inFileName.c_str(), "rb"));
  UTIL::ERRIF(cipherText == nullptr, inFileName, "打开密文");
  plainText.reset(fopen(outFileName.c_str(), "wb"));
  UTIL::ERRIF(plainText == nullptr, outFileName, "打开明文");

  // 计算文件大小
  fseek(cipherText.get(), 0, SEEK_END);
  size_t fLen = ftell(cipherText.get());
  fseek(cipherText.get(), 0, SEEK_SET);

  // 块的数量
  size_t total_blocks = (fLen + MBLOCK - 1) / MBLOCK;

  // 确定每块的大小
  auto mPtr = std::make_unique<std::vector<std::vector<uint8_t>>>();
  mPtr->resize(total_blocks);

  // 引入多线程，分段读取文件并解密
  std::vector<std::thread> threads;
  threads.reserve(total_blocks); // 预留空间

  for (size_t i = 0, idx = 0; i < fLen; i += MBLOCK, idx++) {
    // 计算当前块的大小
    size_t len = (i + MBLOCK > fLen) ? (fLen - i) : MBLOCK;

    // 重置大小
    (*mPtr)[idx].resize(len);

    // 读取密文到内存
    UTIL::ERRIF(fread((*mPtr)[idx].data(), 1, len, cipherText.get()) != len,
                inFileName, "密文 -> 内存");

    // 多线程解密
    threads.emplace_back([&, idx, len]() {
      // 初始化加解密上下文
      std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> ctx(
          EVP_PKEY_CTX_new(privateKey.get(), nullptr), EVP_PKEY_CTX_free);

      UTIL::ERRIF(ctx == nullptr, "ctx", "初始化加解密上下文");
      UTIL::ERRIF(EVP_PKEY_decrypt_init(ctx.get()) <= 0, "ctx", "初始化解密");
      UTIL::ERRIF(
          EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0,
          "ctx", "设置解密填充参数");

      // 确定解密后长度
      size_t lenOut = 0;
      UTIL::ERRIF(
          EVP_PKEY_decrypt(ctx.get(), nullptr, &lenOut,
                           (*mPtr)[idx].data(), len) <= 0,
          inFileName, "确定解密长度");

      // 解密后的数据
      std::vector<uint8_t> decryptedData(lenOut);

      // 解密
      UTIL::ERRIF(
          EVP_PKEY_decrypt(ctx.get(), decryptedData.data(), &lenOut,
                           (*mPtr)[idx].data(), len) <= 0,
          "ctx", "解密");

      // 重置大小
      decryptedData.resize(lenOut);
      (*mPtr)[idx] = std::move(decryptedData);
    });
  }

  // 等待所有线程完成
  for (auto& t : threads) {
    if (t.joinable()) {
      t.join();
    }
  }

  // 写入到明文
  for (const auto& block : *mPtr) {
    UTIL::ERRIF(fwrite(block.data(), 1, block.size(), plainText.get()) != block.size(),
                outFileName, "内存 -> 明文");
  }
}

}  // namespace SAFE