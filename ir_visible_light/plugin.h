#ifndef PLUGIN_H
#define PLUGIN_H

#include <QObject>
#include <QtPlugin>
#include "plugininterface.h"
#include "demo.h"

class Plugin : public QObject, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.zkss.glass.addons.ir-visible-light" FILE "ir_visible_light/plugin_ir_visible_light.json")
    Q_INTERFACES(PluginInterface)

public:
    void start() override;

    Demo w;
};

#endif // PLUGIN_H
