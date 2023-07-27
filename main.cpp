#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif

#if DISPLAY && CONTROL
#include "demo.h"
#elif !DISPLAY && CONTROL
#include "hikcontrol.h"
#endif

#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, ".65001");
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    QApplication a(argc, argv);
    a.setFont(QFont("Microsoft YaHei", 10));
#if DISPLAY && CONTROL
    Demo w;
    w.show();
#elif !DISPLAY && CONTROL
    HikControl w;
    w.show();
#endif
    return a.exec();
}
