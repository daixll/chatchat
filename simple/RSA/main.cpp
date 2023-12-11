#include "RSA.h"



int main(){
    jiao::RSA rsa("key/public_key.pem", "key/private_key.pem");

    auto res = rsa.encrypt("C++也是调包侠");

    std::ofstream ofs("res.txt");
    ofs << res;
    ofs.close();

    std::ifstream ifs("res.txt");
    res="";
    if(1){
        ifs.seekg(0, std::ios::end);
        res.resize(ifs.tellg());
        ifs.seekg(0, std::ios::beg);
        ifs.read(&res[0], res.size());
        ifs.close();
    }

    rsa.show(res);

    std::cout << rsa.decrypt(res);
    return 0;
}