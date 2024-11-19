#include <thread>
#include <vector>

#include "safe.h"
#include "util.h"

const size_t MBLOCK = 212;  // 214 byte, 2048 bit，保险起见，留两个字节

namespace SAFE {

Encrypt::Encrypt() : Safe() {}

Encrypt::~Encrypt() {}

void Encrypt::encrypt(const std::string& inFileName,
                      const std::string& outFileName) {
  plainText.reset(fopen(inFileName.c_str(), "rb"));
  UTIL::ERRIF(plainText == nullptr, inFileName, "打开明文");
  cipherText.reset(fopen(outFileName.c_str(), "wb"));
  UTIL::ERRIF(cipherText == nullptr, outFileName, "打开密文");

  // 计算文件大小
  fseek(plainText.get(), 0, SEEK_END);  // 移动文件指针到文件末尾
  size_t fLen = ftell(plainText.get());  // 获取文件指针位置，即文件大小

  // 引入多线程，分段读取文件并加密
  std::vector<std::thread> threads;

  // 保存每块的数据
  std::unique_ptr<std::vector<std::vector<uint8_t>>> mPtr =
      std::make_unique<std::vector<std::vector<uint8_t>>>();

  for (size_t i = 0, idx = 0; i < fLen; i += MBLOCK, idx++) {
    // 确定每块的大小 [i,i+len)
    fseek(plainText.get(), i, SEEK_SET);
    size_t len = i + MBLOCK > fLen ? fLen - i : MBLOCK;

    // 为每块分配内存
    mPtr->push_back(std::vector<uint8_t>(len));

    // 将数据保存到 mPtr
    UTIL::ERRIF(fread((*mPtr)[idx].data(), 1, len, plainText.get()) != len,
                inFileName, "原文 -> 内存");

    // 多线程加密
    threads.push_back(std::thread([=, &mPtr]() {
      // 初始化加解密上下文
      std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> ctx(
          EVP_PKEY_CTX_new(publicKey.get(), nullptr), EVP_PKEY_CTX_free);

      UTIL::ERRIF(ctx == nullptr, "ctx", "初始化加解密上下文");
      UTIL::ERRIF(EVP_PKEY_encrypt_init(ctx.get()) <= 0, "ctx", "初始化加密");
      UTIL::ERRIF(
          EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0,
          "ctx", "设置加密填充参数");

      // 确定加密后长度
      size_t lenOut;
      UTIL::ERRIF(
          EVP_PKEY_encrypt(ctx.get(), nullptr, &lenOut,
                           (const uint8_t*)(*mPtr)[idx].data(), len) <= 0,
          "ctx", "确定加密后长度");

      // 加密
      std::unique_ptr<std::vector<uint8_t>> t(new std::vector<uint8_t>(lenOut));
      UTIL::ERRIF(
          EVP_PKEY_encrypt(ctx.get(), t->data(), &lenOut,
                           (const uint8_t*)(*mPtr)[idx].data(), len) <= 0,
          "ctx", "加密");

      // 写入到 mPtr
      (*mPtr)[idx].resize(lenOut);
      (*mPtr)[idx] = *t;
    }));
  }

  // 等待所有线程完成
  for (auto& t : threads) {
    if (t.joinable()) {
      t.join();
    }
  }

  // 将内存写入密文
  for (const auto& v : *mPtr) {
    UTIL::ERRIF(fwrite(v.data(), 1, v.size(), cipherText.get()) != v.size(),
                outFileName, "内存 -> 密文");
  }
}

}  // namespace SAFE