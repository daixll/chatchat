#include <QApplication>
#include "include/MainWindow.h"

int main(int argc, char *argv[]){
    QApplication app(argc, argv);
    MainWindow* window = new MainWindow(nullptr, argv[1]);
    window -> show();
    return app.exec();
}