#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif

#include "demo.h"

#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, ".65001");
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    QApplication a(argc, argv);
    a.setFont(QFont("Microsoft YaHei", 10));
    Demo w;
    w.show();
    return a.exec();
}
