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
  // Open the ciphertext file in binary read mode
  cipherText.reset(fopen(inFileName.c_str(), "rb"));
  UTIL::ERRIF(cipherText == nullptr, inFileName, "打开密文");

  // Open the plaintext file in binary write mode
  plainText.reset(fopen(outFileName.c_str(), "wb"));
  UTIL::ERRIF(plainText == nullptr, outFileName, "打开明文");

  // Calculate the size of the ciphertext file
  fseek(cipherText.get(), 0, SEEK_END);
  size_t fLen = ftell(cipherText.get());
  fseek(cipherText.get(), 0, SEEK_SET); // Rewind to the beginning of the file

  // Calculate the total number of blocks
  size_t total_blocks = (fLen + MBLOCK - 1) / MBLOCK;

  // Pre-allocate the vector to hold all blocks
  auto mPtr = std::make_unique<std::vector<std::vector<uint8_t>>>();
  mPtr->resize(total_blocks);

  // Vector to hold all threads
  std::vector<std::thread> threads;
  threads.reserve(total_blocks); // Reserve space to prevent reallocations

  for (size_t i = 0, idx = 0; i < fLen; i += MBLOCK, idx++) {
    // Calculate the length of the current block
    size_t len = (i + MBLOCK > fLen) ? (fLen - i) : MBLOCK;

    // Resize the current block vector to hold the ciphertext
    (*mPtr)[idx].resize(len);

    // Read the ciphertext block into memory
    UTIL::ERRIF(fread((*mPtr)[idx].data(), 1, len, cipherText.get()) != len,
                inFileName, "密文 -> 内存");

    // Spawn a thread to decrypt this block
    threads.emplace_back([&, idx, len]() {
      // Initialize decryption context
      std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> ctx(
          EVP_PKEY_CTX_new(privateKey.get(), nullptr), EVP_PKEY_CTX_free);

      UTIL::ERRIF(ctx == nullptr, "ctx", "初始化加解密上下文");
      UTIL::ERRIF(EVP_PKEY_decrypt_init(ctx.get()) <= 0, "ctx", "初始化解密");
      UTIL::ERRIF(
          EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0,
          "ctx", "设置解密填充参数");

      // Determine the length of the decrypted data
      size_t lenOut = 0;
      UTIL::ERRIF(
          EVP_PKEY_decrypt(ctx.get(), nullptr, &lenOut,
                           (*mPtr)[idx].data(), len) <= 0,
          inFileName, "确定解密长度");

      // Allocate buffer for decrypted data
      std::vector<uint8_t> decryptedData(lenOut);

      // Perform the decryption
      UTIL::ERRIF(
          EVP_PKEY_decrypt(ctx.get(), decryptedData.data(), &lenOut,
                           (*mPtr)[idx].data(), len) <= 0,
          "ctx", "解密");

      // Resize and store the decrypted data
      decryptedData.resize(lenOut);
      (*mPtr)[idx] = std::move(decryptedData);
    });
  }

  // Wait for all threads to complete
  for (auto& t : threads) {
    if (t.joinable()) {
      t.join();
    }
  }

  // Write all decrypted data to the plaintext file
  for (const auto& block : *mPtr) {
    UTIL::ERRIF(fwrite(block.data(), 1, block.size(), plainText.get()) != block.size(),
                outFileName, "内存 -> 明文");
  }
}

}  // namespace SAFE