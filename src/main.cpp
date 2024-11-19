#include "safe/safe.h"
#include "util/util.h"

bool flg;

const std::string TTT = "png";

std::string str1 = "demo/safe/data/raw." + TTT;
std::string str2 = "demo/safe/data/encrypt_output/mid.bin";
std::string str3 = "demo/safe/data/decrypt_output/out." + TTT;

int main() {
  SAFE::Encrypt enc;
  SAFE::Decrypt dec;

  enc.loadPubKeyFromFile("demo/safe/key/one/pub.pem");
  dec.loadPriKeyFromFile("demo/safe/key/one/pri.pem");

  enc.encrypt(str1, str2);
  dec.decrypt(str2, str3);
  return 0;
}