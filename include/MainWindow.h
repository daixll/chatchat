#pragma once

#include <QMainWindow>
#include <QLayout>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>
#include <QMenuBar>
#include <QAction>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>

#include <boost/asio.hpp>
#include <iostream>
#include <map>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QHBoxLayout *_rrLayout;     // 右侧下方布局
    QTextEdit   *_sendTextEdit; // 发送消息
    QPushButton *_SEND;         // 发送按钮

    QVBoxLayout *_rightLayout;  // 右侧上下布局
    QHBoxLayout *_layout;       // 左右布局
    QWidget     *_centralWidget;// 中央部件
    QMenuBar    *_menuBar;      // 菜单栏

    QAction     *_login;        // 登录
    QAction     *_addFriend;    // 添加好友

    void send();                // 发送消息
    void login();               // 登录
    void addFriend();           // 添加好友

    QListWidget *_listWidget;       // 声明一个列表部件
    QStackedWidget *_stackedWidget; // 堆叠部件
    std::map<int, QTextEdit*> _chatRecord; // 聊天记录
    
    boost::asio::io_context* io;
    boost::asio::ip::tcp::acceptor* ac;
    void start_read(boost::asio::ip::tcp::socket& socket);
    void start_accept(boost::asio::ip::tcp::acceptor& ac);
};