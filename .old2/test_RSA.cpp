#include "Headers/RSA.h"

int main(){
    jiao::RSA* one = new jiao::RSA("key/one/public_key.pem", "key/one/private_key.pem");
    jiao::RSA* two = new jiao::RSA("key/two/public_key.pem", "key/two/private_key.pem");

    // 获取加密内容
    std::string s = "加密内容不能过长，后期考虑分段加密";

    // 加密
    auto ss = one->encrypt(std::vector<uint8_t>(s.begin(), s.end()));
    one->show(std::string(ss.begin(), ss.end()));

    std::cout << "\n";

    // 解密
    auto sss = one->decrypt(ss);
    std::cout << std::string(sss.begin(), sss.end());

    return 0;
}