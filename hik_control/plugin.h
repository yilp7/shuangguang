#ifndef PLUGIN_H
#define PLUGIN_H

#include <QObject>
#include <QtPlugin>
#include "plugininterface.h"
#include "hikcontrol.h"

class Plugin : public QObject, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.zkss.glass.addons.hik_control" FILE "hik_control/plugin_hik_control.json")
    Q_INTERFACES(PluginInterface)

public:
    void create(QWidget *parent) override;
    void start() override;

    HikControl *w;
};

#endif // PLUGIN_H
