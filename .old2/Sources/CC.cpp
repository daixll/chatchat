#include "../Headers/CC.h"

using namespace boost::asio;

CC::CC(const int port, const std::string pubk, const std::string prik){
    rsa = new jiao::RSA(pubk, prik, true);
    io = new io_context();
    auto ep4 = ip::tcp::endpoint(ip::address::from_string("0.0.0.0"), port);
    ac4 = new ip::tcp::acceptor(*io, ep4);
    auto ep6 = ip::tcp::endpoint(ip::address::from_string("::"), port+1);
    ac6 = new ip::tcp::acceptor(*io, ep6);

    start_accept(*ac4);
    start_accept(*ac6);
}

void CC::init2(QListWidget* lw, QStackedWidget* sw, std::map<int, QTextEdit*>* cr){
    lw_ = lw, sw_ = sw, cr_ = cr;   // 引入列表，堆叠窗口，消息框
}

CC::~CC(){
    delete io;
    delete ac4;
    delete ac6;
}

void CC::once(){
    io->poll();
}

void CC::start_accept(ip::tcp::acceptor& ac){
    ac.async_accept(
        [this, &ac](const boost::system::error_code& ec, ip::tcp::socket socket){
        if(!ec){
            int fd = socket.native_handle();
            std::cout << fd << " ACC SUCCESS! " << std::endl;
            
            //--- 增加窗口
            lw_->addItem(QString::number(fd));
            cr_->insert(std::make_pair(fd, new QTextEdit()));
            cr_->at(fd)->setReadOnly(true);
            sw_->addWidget(cr_->at(fd));
            //--- over

            cs.insert(std::make_pair(fd, std::move(socket)));

            start_read(cs.at(fd));  // 首次会读取到对方的公钥
            send(fd, rsa->pubk, 0); // 发送自己的公钥
            start_accept(ac);
        }
        else
            std::cout << " ACC ERROR! " << ec.message() << std::endl;
    });
}

void CC::start_read(ip::tcp::socket& socket){
    char data[10240];
    memset(data, '\0', sizeof data);
    // 异步读取数据
    socket.async_read_some(buffer(data, sizeof data),
        [&, this](const boost::system::error_code& ec, std::size_t len){
            if(!ec){
                //char data_copy[10240];
                //memset(data_copy, '\0', sizeof data_copy);
                //memcpy(data_copy, data, sizeof data);
                std::cout << socket.native_handle() << " RECV MESSAGE " << len << " : ";
                rsa->show(std::string(data, data + len));
                std::cout << std::endl;
                //<< data << std::endl;
                
                if(ks.find(socket.native_handle()) == ks.end()){
                    // 保存对方的公钥
                    ks.insert(std::make_pair(socket.native_handle(), new jiao::RSA(data,"",true)));
                }else{
                    // 使用自己的私钥解密
                    auto msg = rsa->decrypt(std::vector<uint8_t>(data, data+len));
                    std::cout << "DECRYPTED MESSAGE " << msg.size() << " : " << msg.data() << std::endl;

                    // 消息框更新
                    cr_->at(socket.native_handle())->append(QString::fromStdString(std::string(msg.begin(), msg.end())));

                }

                start_read(socket);
            }else{
                std::cout << socket.native_handle() << " RECV ERROR! " << ec.message() << " " << error::operation_aborted << std::endl;
                // 删除列表
                lw_->takeItem(lw_->row(lw_->findItems(QString::number(socket.native_handle()), Qt::MatchExactly).at(0)));
                // 删除消息框
                cr_->erase(socket.native_handle());
                // 删除 socket
                cs.erase(socket.native_handle());

                return ;
            }
        });
}

int CC::conn(std::string ip, int port){
    ip::tcp::socket socket(*io);
    ip::tcp::endpoint ep(ip::address::from_string(ip), port);
    socket.connect(ep);
    int fd = socket.native_handle();
    cs.insert(std::make_pair(socket.native_handle(), std::move(socket)));
    start_read(cs.at(fd));
    std::cout << "CONNECTED TO " << fd << std::endl;

    //sleep(1);
    send(fd, rsa->pubk, 0); // 发送自己的公钥
    return fd;
}

void CC::send(int fd, std::string msg, bool flg){   // flg 为 1 时使用 fd 的公钥加密
    // 使用 fd 的公钥加密
    if(flg){
        auto mmsg = ks.at(fd)->encrypt(std::vector<uint8_t>(msg.begin(), msg.end()));
        cs.at(fd).write_some(buffer(mmsg));
    }else{
        cs.at(fd).write_some(buffer(msg));
    }
}