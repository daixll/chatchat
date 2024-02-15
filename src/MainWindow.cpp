#include "include/MainWindow.h"

using namespace boost::asio;

std::map<int, ip::tcp::socket> cs;

void MainWindow::start_read(ip::tcp::socket& socket){
    char data[1024];
    // 异步读取数据
    socket.async_read_some(buffer(data, sizeof data),
        [&, this](const boost::system::error_code& ec, std::size_t len){
            if(!ec){
                data[len-1] = '\0';
                std::cout << socket.native_handle() << " RECV MESSAGE " << len << " : " << data << std::endl;
                // 消息框更新
                _chatRecord[socket.native_handle()]->append(QString::fromStdString(data));
                start_read(socket);
            }else{
                std::cout << socket.native_handle() << " RECV ERROR! " << ec.message() << " " << error::operation_aborted << std::endl;
                // 删除消息框
                return ;
            }
        });
}

void MainWindow::start_accept(ip::tcp::acceptor& ac){
    ac.async_accept(
        [this, &ac](const boost::system::error_code& ec, ip::tcp::socket socket){
        if(!ec){
            int fd = socket.native_handle();
            std::cout << fd << " ACC SUCCESS! " << std::endl;
            // 增加一个窗口
            _listWidget->addItem(QString::number(fd));
            _chatRecord[fd] = new QTextEdit(this);
            _chatRecord[fd] -> setReadOnly(true);
            _stackedWidget->addWidget(_chatRecord[fd]);

            cs.insert(std::make_pair(fd, std::move(socket)));

            start_read(cs.at(fd));

            start_accept(ac);
        }
        else
            std::cout << "ACC ERROR! " << ec.message() << std::endl;
    });
}

MainWindow::~MainWindow()
{
    delete _centralWidget;
    delete _layout;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("chat!chat!");
    resize(666, 666);

    _rrLayout       = new QHBoxLayout(this);
    _SEND           = new QPushButton("发送", this);

    _rightLayout    = new QVBoxLayout(this);
    _layout         = new QHBoxLayout(this);
    _centralWidget  = new QWidget(this);
    _menuBar        = new QMenuBar(this);

    _login          = new QAction("登录", this);
    _addFriend      = new QAction("添加好友", this);

    _listWidget     = new QListWidget(this);
    _stackedWidget  = new QStackedWidget(this);
    _sendTextEdit   = new QTextEdit(this);

    _rrLayout -> addWidget(_sendTextEdit);
    _rrLayout -> addWidget(_SEND);

    // 右边收发
    _rightLayout -> addWidget(_stackedWidget);
    _rightLayout -> addLayout(_rrLayout);
    
    // 左边列表
    _layout -> addWidget(_listWidget);
    _layout -> addLayout(_rightLayout);
    // 创建菜单栏
    _menuBar -> addAction(_login);
    _menuBar -> addAction(_addFriend);
    setMenuBar(_menuBar);
    // 设置大小
    _listWidget -> setFixedWidth(111);
    _sendTextEdit -> setFixedHeight(111);
    _SEND -> setFixedHeight(111);
    _SEND -> setFixedWidth(50);
    _SEND -> setStyleSheet("background-color: rgb(255, 0, 0);");

    _centralWidget -> setLayout(_layout); // 设置中央部件的布局管理器
    setCentralWidget(_centralWidget);     // 设置中央部件

    // 连接QAction的triggered信号到一个槽函数
    connect(_login, &QAction::triggered, this, &MainWindow::login);
    connect(_addFriend, &QAction::triggered, this, &MainWindow::addFriend);
    connect(_SEND, &QPushButton::clicked, this, &MainWindow::send);
    connect(_listWidget, &QListWidget::currentRowChanged, _stackedWidget, &QStackedWidget::setCurrentIndex);

    // Boost ASIO
    io = new io_context();
    ip::tcp::endpoint ep(ip::address::from_string("0.0.0.0"), 10086);
    ac = new ip::tcp::acceptor(*io, ep);
    start_accept(*ac);
    
    // 启动异步IO
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this](){
        io->poll();
    });
    timer->start(10);
}

void MainWindow::login(){
    QDialog *dialog = new QDialog(this);
    QVBoxLayout *layout = new QVBoxLayout;

    QLineEdit *lineEdit1 = new QLineEdit(this);
    QLineEdit *lineEdit2 = new QLineEdit(this);
    QPushButton *button = new QPushButton(tr("上号!(还没写)"), this);

    layout->addWidget(lineEdit1);
    layout->addWidget(lineEdit2);
    layout->addWidget(button);

    dialog->setLayout(layout);
    
    connect(button, &QPushButton::clicked, this, [&, this, dialog](){
        dialog->close();
    });

    dialog->exec();
}

void MainWindow::addFriend()
{
    QDialog *dialog = new QDialog(this);
    QVBoxLayout *layout = new QVBoxLayout;

    QLineEdit *lineEdit = new QLineEdit(this);
    QPushButton *button = new QPushButton(tr("添加好友"), this);

    layout->addWidget(lineEdit);
    layout->addWidget(button);

    dialog->setLayout(layout);
    dialog->exec();
}

void MainWindow::send()
{
    QString text = _sendTextEdit->toPlainText();

    text.replace("\n", "\n>> ");
    _sendTextEdit->clear();
}