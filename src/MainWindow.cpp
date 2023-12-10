#include "include/MainWindow.h"

MainWindow::~MainWindow()
{
    delete _centralWidget;
    delete _layout;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    _serverIP   = "dxll.love";
    _serverPort = 6666;

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
    _changeServer   = new QAction("切换服务器", this);

    _listWidget     = new QListWidget(this);
    _receiveTextEdit= new QTextEdit(this);
    _sendTextEdit   = new QTextEdit(this);

    _rrLayout -> addWidget(_sendTextEdit);
    _rrLayout -> addWidget(_SEND);

    // 右边收发
    _rightLayout -> addWidget(_receiveTextEdit);
    _rightLayout -> addLayout(_rrLayout);
    // 左边列表
    _layout -> addWidget(_listWidget);
    _layout -> addLayout(_rightLayout);
    // 创建菜单栏
    _menuBar -> addAction(_login);
    _menuBar -> addAction(_addFriend);
    _menuBar -> addAction(_changeServer);
    setMenuBar(_menuBar);
    // 设置大小
    _listWidget -> setFixedWidth(111);
    _sendTextEdit -> setFixedHeight(111);
    _SEND -> setFixedHeight(111);
    _SEND -> setFixedWidth(50);
    // 接收框设置为只读
    _receiveTextEdit -> setReadOnly(true);

    _centralWidget -> setLayout(_layout); // 设置中央部件的布局管理器
    setCentralWidget(_centralWidget);     // 设置中央部件

    // 连接QAction的triggered信号到一个槽函数
    connect(_login, &QAction::triggered, this, &MainWindow::login);
    connect(_addFriend, &QAction::triggered, this, &MainWindow::addFriend);
    connect(_changeServer, &QAction::triggered, this, &MainWindow::changeServer);
    connect(_SEND, &QPushButton::clicked, this, &MainWindow::send);
}

void MainWindow::login(){
    QDialog *dialog = new QDialog(this);
    QVBoxLayout *layout = new QVBoxLayout;

    QLineEdit *lineEdit1 = new QLineEdit(this);
    QLineEdit *lineEdit2 = new QLineEdit(this);
    QPushButton *button = new QPushButton(tr("登录 / 注册"), this);

    layout->addWidget(lineEdit1);
    layout->addWidget(lineEdit2);
    layout->addWidget(button);

    dialog->setLayout(layout);
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

void MainWindow::changeServer()
{
    QDialog *dialog = new QDialog(this);

    QHBoxLayout *hLayout = new QHBoxLayout;
    QLineEdit *lineEdit1 = new QLineEdit(this);
    QLineEdit *lineEdit2 = new QLineEdit(this);
    hLayout->addWidget(lineEdit1);
    hLayout->addWidget(lineEdit2);

    lineEdit1->setText(_serverIP);
    lineEdit2->setText(QString::number(_serverPort));


    QVBoxLayout *vLayout = new QVBoxLayout;
    QPushButton *button = new QPushButton(tr("切换服务器"), this);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(button);

    dialog->setLayout(vLayout);
    dialog->exec();
}

void MainWindow::send()
{
    QString text = _sendTextEdit->toPlainText();

    text.replace("\n", "\n>> ");
    _receiveTextEdit->append(">> " + text);
    _sendTextEdit->clear();
}