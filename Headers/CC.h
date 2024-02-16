#pragma once

#include "RSA.h"

#include <boost/asio.hpp>
#include <iostream>
#include <map>

using namespace boost::asio;

// 通信类
class CC {
public:
    CC(int port, const std::string pubk, const std::string prik);
    ~CC();
    void once();    // 执行一步
    int conn(std::string ip, int port);
    void send(int fd, std::string msg, bool flg=1);

private:
    void update();   // 更新消息框

    jiao::RSA* rsa;

    io_context* io;
    ip::tcp::acceptor* ac4;
    ip::tcp::acceptor* ac6;

    std::map<int, boost::asio::ip::tcp::socket> cs;
    std::map<int, jiao::RSA*> ks;

    void start_read(ip::tcp::socket& socket);
    void start_accept(ip::tcp::acceptor& ac);
};