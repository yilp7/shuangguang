#ifndef HIKCONTROL_H
#define HIKCONTROL_H

#include <QtCore>
#include <QtWidgets>
#include <QSerialPort>
#include "GeneralDef.h"

namespace Ui {
class HikControl;
}

class HikControl : public QWidget
{
    Q_OBJECT

public:
    explicit HikControl(QWidget *parent = nullptr);
    ~HikControl();

    bool log_in();
    void get_device_resource_cfg();

private slots:
    void on_CONNECT_BTN_clicked();
    void lens_btn_pressed(int id);
    void lens_btn_released(int id);
    void on_SPEED_SLIDER_sliderMoved(int position);
    void on_SPEED_EDIT_editingFinished();

    void on_ICR_CHK_stateChanged(int arg1);

private:
    Ui::HikControl*     ui;

    LOCAL_DEVICE_INFO   device_info;
    bool                logged_in;

    QButtonGroup*       lens_btn;
    int                 speed;
};

#endif // HIKCONTROL_H
