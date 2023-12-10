// -lssl -lcrypto

#include <memory>
#include <string>
#include <openssl/pem.h>
#include <openssl/rsa.h>

std::string RSA_Encrypt(const std::string& public_key, const std::string& plain_text) {
  // 将 PEM 格式的公钥转换为 RSA 结构体
  auto rsa = RSA_new();
  if (rsa == nullptr) {
    throw std::runtime_error("Failed to create RSA structure");
  }
  if (!PEM_read_bio_RSAPublicKey(
          BIO_new_mem_buf((const unsigned char*)public_key.c_str(), -1), rsa, nullptr, nullptr)) {
    RSA_free(rsa);
    throw std::runtime_error("Failed to read public key from PEM");
  }

  // 检查公钥是否有效
  if (rsa->n == nullptr || rsa->e == nullptr) {
    RSA_free(rsa);
    throw std::runtime_error("Public key is invalid");
  }

  // 将明文转换为字节数组
  auto plain_text_bytes = reinterpret_cast<const unsigned char*>(plain_text.c_str());
  int plain_text_length = plain_text.length();

  // 检查明文长度是否超过 RSA 结构体的大小
  if (plain_text_length > RSA_size(rsa)) {
    plain_text_length = RSA_size(rsa);
  }

  // 分配内存用于存储加密后的密文
  auto encrypted_text = new unsigned char[plain_text_length];

  // 使用 RSA_PKCS1_PADDING 填充方式进行公钥加密
  int encrypted_text_length = RSA_public_encrypt(
      plain_text_length, plain_text_bytes, encrypted_text, rsa, RSA_PKCS1_PADDING);

  // 检查加密是否成功
  if (encrypted_text_length == -1) {
    RSA_free(rsa);
    delete[] encrypted_text;
    throw std::runtime_error("RSA encryption failed");
  }

  // 将加密后的密文转换为字符串
  std::string encrypted_text_string(reinterpret_cast<char*>(encrypted_text), encrypted_text_length);

  // 释放内存并清理资源
  RSA_free(rsa);
  delete[] encrypted_text;

  return encrypted_text_string;
}


std::string RSA_Decrypt(const std::string& Private_key, const std::string& Encrypted_text) {
  // 将 PEM 格式的私钥转换为 RSA 结构体
  BIO* bio = BIO_new_mem_buf((char*)Private_key.c_str(), -1);  // 创建内存 BIO 对象
  RSA* rsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL); // 从 BIO 对象读取私钥
  BIO_free(bio);   // 释放 BIO 对象

  // 将密文转换为字节数组
  const unsigned char* encrypted_text_bytes = reinterpret_cast<const unsigned char*>(Encrypted_text.c_str()); // 获取密文字符串的字节数组
  int encrypted_text_length = Encrypted_text.length(); // 获取密文长度

  // 分配内存用于存储解密后的明文
  unsigned char* decrypted_text = new unsigned char[RSA_size(rsa)]; // 依据 RSA 结构体大小分配内存

  // 使用 RSA_PKCS1_PADDING 填充方式进行私钥解密
  int decrypted_text_length = RSA_private_decrypt(encrypted_text_length, encrypted_text_bytes, decrypted_text, rsa, RSA_PKCS1_PADDING);

  // 检查解密是否成功
  if (decrypted_text_length == -1) {
    RSA_free(rsa);  // 释放 RSA 结构体
    delete[] decrypted_text;  // 释放解密后明文内存
    throw std::runtime_error("RSA 解密失败");
  }

  // 将解密后的明文转换为字符串
  std::string decrypted_text_string(reinterpret_cast<char*>(decrypted_text), decrypted_text_length); // 将解密后明文转换为字符串

  // 释放内存并清理资源
  RSA_free(rsa);  // 释放 RSA 结构体
  delete[] decrypted_text;  // 释放解密后明文内存

  return decrypted_text_string; // 返回解密后的明文
}
