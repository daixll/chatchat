#include <iostream>
#include <string>
#include <fstream>
#include "RSA.h"

int main(){
    std::ifstream ifs_pub("pub_key");
    std::string pub_key;
    
    std::string t;
    while(getline(ifs_pub, t))
        pub_key += t;

    std::ifstream ifs_pri("pri_key");
    std::string pri_key;

    while(getline(ifs_pri, t))
        pri_key += t;

    std::string plain_text = "Hello World!";
    std::string encrypted_text = RSA_Encrypt(pub_key, plain_text);
    std::cout << "Encrypted text: " << encrypted_text << std::endl;
    std::string decrypted_text = RSA_Decrypt(pri_key, encrypted_text);
    std::cout << "Decrypted text: " << decrypted_text << std::endl;

    return 0;
}