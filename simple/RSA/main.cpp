#include "RSA.h"

int main(){
    jiao::RSA rsa1("key/public_key.pem", "key/private_key.pem");
    jiao::RSA rsa2("key2/public_key.pem", "key2/private_key.pem");

    auto res = rsa2.encrypt("C++也是调包侠");
    std::cout << rsa1.decrypt(res);
    return 0;
}