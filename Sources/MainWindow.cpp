#include "../Headers/MainWindow.h"

MainWindow::~MainWindow(){}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("chat!chat!");
    resize(666, 666);

    // 右下
    SendEdit_.setFixedHeight(111);
    SendButton_.setFixedHeight(111);
    SendButton_.setText("发送");
    SendButton_.setStyleSheet("background-color: rgb(255, 0, 0);");
    RDLayout_.addWidget(&SendEdit_);
    RDLayout_.addWidget(&SendButton_);

    // 右上
    RLayout_.addWidget(&StackWidget_);
    RLayout_.addLayout(&RDLayout_);

    // 总体
    ListWidget_.setFixedWidth(111);
    Layout_.addWidget(&ListWidget_);
    Layout_.addLayout(&RLayout_);

    setCentralWidget(&CentralWidget_);
    CentralWidget_.setLayout(&Layout_);

    // 菜单栏
    Login_.setText("登录");
    MenuBar_.addAction(&Login_);
    AddFriend_.setText("添加好友");
    MenuBar_.addAction(&AddFriend_);
    Status_.setText("未登陆");
    Status_.setAlignment(Qt::AlignCenter);
    MenuBar_.setCornerWidget(&Status_);
    
    setMenuBar(&MenuBar_);

    connect(&Login_, &QAction::triggered, this, [&, this](){
        // 显示登录窗口
        QDialog dialog(this);
        dialog.setWindowTitle("登录");

        QVBoxLayout layout(&dialog);
        dialog.setLayout(&layout);

        QLineEdit l1(&dialog);
        QLineEdit l2(&dialog);
        QLineEdit l3(&dialog);
        QPushButton l4(&dialog);
        
        l1.setPlaceholderText("port");
        l2.setPlaceholderText("公钥");
        l3.setPlaceholderText("私钥");
        l4.setText("登录");
        
        layout.addWidget(&l1);
        layout.addWidget(&l2);
        layout.addWidget(&l3);
        layout.addWidget(&l4);

        connect(&l4, &QPushButton::clicked, this, [&, this](){
            // 登录
            if(l1.text().isEmpty() || l2.text().isEmpty() || l3.text().isEmpty())
                return;
            Status_.setText(l1.text());
            
            // 启动监听 初始化自己的密钥对
            cc = new CC(l1.text().toInt(), l2.text().toStdString(), l3.text().toStdString());
            
            // 伪异步IO
            QTimer *timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, [&](){
                cc->once();
                // 需要监视socket，以便更新界面
            });
            timer->start(100);

            dialog.close();
        });

        dialog.exec();
    });

    connect(&AddFriend_, &QAction::triggered, this, [&, this](){
        // 显示添加好友窗口
        QDialog dialog(this);
        dialog.setWindowTitle("添加好友");

        QVBoxLayout layout(&dialog);
        dialog.setLayout(&layout);

        QLineEdit l1(&dialog);
        QLineEdit l2(&dialog);
        //QLineEdit l3(&dialog);
        QPushButton l4(&dialog);
        
        l1.setPlaceholderText("好友的IP");
        l2.setPlaceholderText("好友的端口");
        //l3.setPlaceholderText("好友的公钥");
        l4.setText("添加");
        
        layout.addWidget(&l1);
        layout.addWidget(&l2);
        //layout.addWidget(&l3);
        layout.addWidget(&l4);

        connect(&l4, &QPushButton::clicked, this, [&, this](){
            // 添加好友
            if(l1.text().isEmpty())
                return;
            // 主动连接，同时发送公钥
            int fd = cc->conn(l1.text().toStdString(), l2.text().toInt());

            ListWidget_.addItem(QString::number(fd));
            ChatRecord_[fd] = new QTextEdit(this);
            ChatRecord_[fd]->setReadOnly(true);
            StackWidget_.addWidget(ChatRecord_[fd]);
            
            dialog.close();
        });

        dialog.exec();
    });

    connect(&SendButton_, &QPushButton::clicked, this, [&, this](){
        // 发送消息
        if(SendEdit_.toPlainText().isEmpty())
            return;
        
        int fd = ListWidget_.currentItem()->text().toInt();
        std::cout << "SEND TO " << fd << std::endl;
        cc->send(fd, SendEdit_.toPlainText().toStdString());

        SendEdit_.clear();
    });

    connect(&ListWidget_, &QListWidget::currentRowChanged, &StackWidget_, &QStackedWidget::setCurrentIndex);
}