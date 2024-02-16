#pragma once

#include "CC.h"
#include "RSA.h"

#include <QMainWindow>
#include <QMenuBar>
#include <QLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDialog>
#include <QTimer>

#include <map>

// 主窗口
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QWidget CentralWidget_;                             // 中央部件
    QMenuBar MenuBar_;                                  // 菜单栏
        QAction Login_;                                 // 登录
        QAction AddFriend_;                             // 添加好友
        QLabel Status_;                                 // 状态

    QHBoxLayout Layout_;                                // 总体布局
        QListWidget ListWidget_;                        // 列表
        QVBoxLayout RLayout_;                           // 右侧布局
            QStackedWidget StackWidget_;                // 堆叠部件
                std::map<int, QTextEdit*> ChatRecord_;  // 聊天记录
            QHBoxLayout RDLayout_;                      // 右侧下方布局
                QTextEdit SendEdit_;                    // 发送消息框
                QPushButton SendButton_;                // 发送按钮
    
    CC*         cc;
};