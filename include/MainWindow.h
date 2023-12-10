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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QString _serverIP;  // 服务器IP
    int     _serverPort;// 服务器端口

    QHBoxLayout *_rrLayout;     // 左侧布局
    QPushButton *_SEND;         // 发送按钮

    QVBoxLayout *_rightLayout;  // 右侧上下布局
    QHBoxLayout *_layout;       // 左右布局
    QWidget     *_centralWidget;// 中央部件
    QMenuBar    *_menuBar;      // 菜单栏

    QAction     *_login;        // 登录
    QAction     *_addFriend;    // 添加好友
    QAction     *_changeServer; // 切换服务器

    void send();                // 发送消息
    void login();               // 登录
    void addFriend();           // 添加好友
    void changeServer();        // 切换服务器

    QListWidget *_listWidget;       // 声明一个列表部件
    QTextEdit   *_receiveTextEdit;  // 接收消息
    QTextEdit   *_sendTextEdit;     // 发送消息
};