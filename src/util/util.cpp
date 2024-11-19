#include "util.h"

namespace UTIL {
bool ERRIF(bool flg, const std::string &target, const std::string &msg) {
  if (flg) {
    std::cerr << "失败" << std::endl;
    std::cerr << target << std::endl;
    std::cerr << msg << std::endl;
    exit(1);
  }
  //std::cout << "成功：" + target + " " + msg + " " << std::endl;
  return true;
}
}  // namespace UTIL