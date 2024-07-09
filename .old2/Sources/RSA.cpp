#include "../Headers/RSA.h"

namespace jiao{

void RSA::show(std::string str) {
    for(int i=0; i<(int)str.size(); ++i)
        printf("%02X", (unsigned char)str[i]);
}

void RSA::ERR(const std::string& msg) {
    std::cerr << "错误！" << msg << std::endl;
    ERR_print_errors_fp(stderr);
    exit(1);
}

void RSA::WAR(const std::string& msg) {
    std::cerr << "警告！" << msg << std::endl;
}

std::string RSA::read_file(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs){
        WAR("无法打开文件 " + path);
        return "";
    }
    std::string content;
    ifs.seekg(0, std::ios::end);    // 将文件流读取位置设置到文件末尾
    content.resize(ifs.tellg());    // 设置 content 的大小
    ifs.seekg(0, std::ios::beg);    // 文件流定位到文件开头
    ifs.read(&content[0], content.size());
    ifs.close();
    return content;
}

RSA::RSA(const std::string& public_key_path, const std::string& private_key_path, bool flg) {
    OpenSSL_add_all_algorithms();       // 初始化 OpenSSL 库
    
    // 读取公钥和私钥
    std::string public_key_str;
    std::string private_key_str;

    if(flg==0)
        public_key_str = read_file(public_key_path),
        private_key_str = read_file(private_key_path);
    else
        public_key_str = public_key_path,
        private_key_str = private_key_path;

    // 外部使用的公钥
    pubk = public_key_str;

    // 初始化公钥
    if(public_key_str.empty()) {
        WAR("公钥为空");
    } else {
        BIO *bio = BIO_new_mem_buf(public_key_str.c_str(), -1);
        /*  bio 一种 I/O 抽象，可以从文件、内存、网络等读取数据
            public_key_str.c_str()  为数据源
            -1                      表示字符串长度由函数自动计算
        */
        pub_key = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
        /*  读取公钥
            bio         为公钥字符串
            nullptr     为公钥结构体
            nullptr     为密码回调函数
            nullptr     为密码
        */
        BIO_free(bio);
        pub_st = 1;
    }

    // 初始化私钥
    if(private_key_str.empty()) {
        WAR("私钥为空");
    } else {
        BIO *bio = BIO_new_mem_buf(private_key_str.c_str(), -1);
        pri_key = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        pri_st = 1;
    }

}

RSA::~RSA() {
    EVP_PKEY_free(pub_key);             // 释放公钥
    EVP_PKEY_free(pri_key);             // 释放私钥
    EVP_cleanup();                      // 清理 OpenSSL 库
}

std::vector<uint8_t> RSA::encrypt(const std::vector<uint8_t>& plain_text) {
    if (!pub_st){
        WAR("公钥未初始化");
        return std::vector<uint8_t>();
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pub_key, nullptr);
    if (!ctx)
        ERR("无法初始化加密上下文");
    if (EVP_PKEY_encrypt_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
        ERR("无法设置加密参数");
    size_t outlen=0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, (unsigned char *)plain_text.data(), plain_text.size()) <= 0)
        ERR("无法计算加密后的长度");
    unsigned char *out = (unsigned char *)OPENSSL_malloc(outlen);
    if (!out)
        ERR("无法分配内存");
    if (EVP_PKEY_encrypt(ctx, out, &outlen, (unsigned char *)plain_text.data(), plain_text.size()) <= 0)
        ERR("无法加密");
    
    EVP_PKEY_CTX_free(ctx);
    return std::vector<uint8_t>(out, out+outlen);
}

std::vector<uint8_t> RSA::decrypt(const std::vector<uint8_t>& cipher_text) {
    if (!pri_st){
        WAR("私钥未初始化");
        return std::vector<uint8_t>();
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pri_key, nullptr);
    if (!ctx) ERR("无法初始化解密上下文");
    if (EVP_PKEY_decrypt_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
        ERR("无法设置解密参数");
    size_t outlen=0;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, (unsigned char *)cipher_text.data(), cipher_text.size()) <= 0)
        ERR("无法计算解密后的长度");
    unsigned char *out = (unsigned char *)OPENSSL_malloc(outlen);
    if (!out) ERR("无法分配内存");
    if (EVP_PKEY_decrypt(ctx, out, &outlen, (unsigned char *)cipher_text.data(), cipher_text.size()) <= 0)
        ERR("无法解密");
    
    EVP_PKEY_CTX_free(ctx);
    return std::vector<uint8_t>(out, out+outlen);
}


}