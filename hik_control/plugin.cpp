#include "plugin.h"

void Plugin::create(QWidget *parent)
{
    w = new HikControl(parent);
}

void Plugin::start()
{
    w->show();
}
