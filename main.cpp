#pragma execution_character_set("utf-8")

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("音视频媒体合成工具");
    w.show();
    return a.exec();
}
