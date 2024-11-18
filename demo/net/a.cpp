#include <iostream>
#include <boost/asio.hpp>
#include <map>
#include <cstring>

using namespace boost::asio;

class CC{
public:
    CC(const uint16_t port){
        _io  = new io_context();
        _ac4 = new ip::tcp::acceptor(*_io, ip::tcp::endpoint(ip::address::from_string("0.0.0.0"), port));
        _ac6 = new ip::tcp::acceptor(*_io, ip::tcp::endpoint(ip::address::from_string("::"), port+1));
        _start_accept(*_ac4);
        _start_accept(*_ac6);
        _start_input();
        _io->run();
    }

    bool conn(std::string ip, int port){
        ip::tcp::socket sock(*_io);
        ip::tcp::endpoint ep(ip::address::from_string(ip), port);
        sock.connect(ep);
        int fd = sock.native_handle();
        if(fd < 0){
            std::cerr << "连接失败！" << std::endl;
            return false;
        }
        _cs.insert(std::make_pair(fd, std::move(sock)));
        _start_read(_cs.at(fd));
        return true;
    }

    void send(int fd, std::string msg){
        if(_cs.find(fd) == _cs.end()){
            std::cerr << "连接不存在！" << std::endl;
            return;
        }
        _cs.at(fd).write_some(boost::asio::buffer(msg));
    }

private:
    void _start_accept(ip::tcp::acceptor& ac){
        ac.async_accept(
            [this, &ac](const boost::system::error_code& ec, ip::tcp::socket sock){
                if(!ec){
                    int fd = sock.native_handle();
                    std::cerr << fd << " 连接成功！ " << std::endl;
                    // 保存连接
                    _cs.insert(std::make_pair(fd, std::move(sock)));
                    _start_read(_cs.at(fd));
                    _start_accept(ac);
                }else{
                    throw std::runtime_error(" 连接失败！ " + ec.message());
                }
            }
        );
    }

    void _start_read(ip::tcp::socket& sock){
        char buf[2560];
        memset(buf, '\0', sizeof buf);
        sock.async_read_some(
            boost::asio::buffer(buf, 2560),
            [&, this](const boost::system::error_code& ec, std::size_t len){
                if(!ec){
                    buf[len] = '\0';
                    std::cout << sock.native_handle() << " " << len << " " << buf << std::endl;

                    _start_read(sock);
                }else{
                    if(ec == boost::asio::error::eof){
                        std::cout << sock.native_handle() << " 连接关闭！ " << std::endl;
                        _cs.erase(sock.native_handle());
                    }else{
                        throw std::runtime_error(std::to_string(sock.native_handle()) + " 读取数据失败！ " + ec.message());
                    }
                }
            }
        );
    }

    void _start_input(){
        auto timer = new boost::asio::steady_timer(*_io, boost::asio::chrono::milliseconds(1));
        timer->async_wait(
            [this, timer](const boost::system::error_code& ec){
                if(!ec){
                    std::string cmd;
                    if(std::cin.rdbuf() -> in_avail() > 0)  // 如果输入缓冲区有数据
                        std::getline(std::cin, cmd);        // 读取数据
                    
                    if(cmd.substr(0, 4) == "conn"){         // 如果是连接命令
                        std::string ip = cmd.substr(5, cmd.find(' ', 5)-5);
                        std::string port = cmd.substr(cmd.find(' ', 5)+1);
                        conn(ip, std::stoi(port));
                    }else if(cmd.substr(0, 4) == "send"){   // 如果是发送命令
                        std::string fd = cmd.substr(5, cmd.find(' ', 5)-5);
                        std::string msg = cmd.substr(cmd.find(' ', 5)+1);
                        if(_cs.find(std::stoi(fd)) != _cs.end())
                            send(std::stoi(fd), msg);
                        else
                            std::cerr << "连接不存在！" << std::endl;
                    }
                    _start_input();
                }else{
                    throw std::runtime_error("输入失败！ " + ec.message());
                }
            }
        );

    }

    std::map<int, ip::tcp::socket>      _cs;    // 连接
    boost::asio::io_context*            _io;    // io 上下文
    ip::tcp::acceptor*                  _ac4;   // ipv4 监听器
    ip::tcp::acceptor*                  _ac6;   // ipv6 监听器
};

int main(int argc, char* argv[]){
    std::cin.sync_with_stdio(false);            // 关闭同步
    try{
        CC cc(std::stoul(argv[1]));
    }catch(std::exception& e){
        std::cerr << e.what() << std::endl;
        std::cerr << errno << " " << strerror(errno) << std::endl;
    }
    return 0;
}