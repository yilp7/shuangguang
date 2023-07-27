#include "hikcontrol.h"
#include "ui_hikcontrol.h"

HikControl::HikControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HikControl),
    logged_in(false),
    lens_btn(NULL),
    speed(4)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);

    NET_DVR_Init();

    lens_btn = new QButtonGroup();
    lens_btn->addButton(ui->ZOOM_OUT_BTN, 0);
    lens_btn->addButton(ui->ZOOM_IN_BTN, 1);
    lens_btn->addButton(ui->FOCUS_FAR_BTN, 2);
    lens_btn->addButton(ui->FOCUS_NEAR_BTN, 3);
    connect(lens_btn, SIGNAL(buttonPressed(int)), this, SLOT(lens_btn_pressed(int)));
    connect(lens_btn, SIGNAL(buttonReleased(int)), this, SLOT(lens_btn_released(int)));

    QSlider *speed_slider = ui->SPEED_SLIDER;
    speed_slider->setMinimum(1);
    speed_slider->setMaximum(7);
    speed_slider->setSingleStep(1);
    speed_slider->setValue(4);
    ui->SPEED_EDIT->setText("4");

    QStringList sl;
    sl << "10" << "15" << "20" << "25";
    ui->FPS_OPTION->addItems(sl);
    ui->FPS_OPTION->setCurrentIndex(0);
}

HikControl::~HikControl()
{
    delete ui;
}

bool HikControl::log_in()
{
    NET_DVR_DEVICEINFO_V40 temp_device_info;
    memset(&temp_device_info, 0, sizeof(NET_DVR_DEVICEINFO_V40));
    NET_DVR_USER_LOGIN_INFO usr_info;
    memset(&usr_info, 0, sizeof(NET_DVR_USER_LOGIN_INFO));

    sprintf_s(usr_info.sDeviceAddress, 129, ui->IP_EDIT->text().simplified().toLatin1().data());
    sprintf_s(usr_info.sUserName, 129, "admin");
    sprintf_s(usr_info.sPassword, 129, "abcd1234");
    usr_info.wPort = 8000;
    LONG lLoginID = NET_DVR_Login_V40(&usr_info, &temp_device_info);

    if(lLoginID == -1) {
        QMessageBox::warning(this, "PROMPT", "Login to the device failed!");
        qDebug() << "login error: " << NET_DVR_GetLastError();
        return false;
    }
    device_info.lLoginID       = lLoginID;
    device_info.iDeviceChanNum = temp_device_info.struDeviceV30.byChanNum;
    device_info.iIPChanNum     = temp_device_info.struDeviceV30.byIPChanNum;
    device_info.iStartChan     = temp_device_info.struDeviceV30.byStartChan;
    device_info.iIPStartChan   = temp_device_info.struDeviceV30.byStartDChan;

    NET_DVR_COMPRESSIONCFG_V30 dev_config = {0};
    DWORD size = 0;
    NET_DVR_GetDVRConfig(device_info.lLoginID, NET_DVR_GET_COMPRESSCFG_V30, 1, &dev_config, sizeof(NET_DVR_COMPRESSIONCFG_V30), &size);
    dev_config.struNormHighRecordPara.dwVideoFrameRate = 10;
    NET_DVR_SetDVRConfig(device_info.lLoginID, NET_DVR_SET_COMPRESSCFG_V30, 1, &dev_config, sizeof(NET_DVR_COMPRESSIONCFG_V30));
    qDebug() << "get fps error:  " << NET_DVR_GetLastError();
    ui->FPS_OPTION->setCurrentIndex(0);

    NET_ITC_ICRCFG dev_icr_config = {0};
    NET_DVR_GetDVRConfig(device_info.lLoginID, NET_ITC_GET_ICRCFG, 1, &dev_icr_config, sizeof(NET_ITC_ICRCFG), &size);
    dev_icr_config.uICRParam.struICRManualSwitch.bySubSwitchMode = 1;
    NET_DVR_SetDVRConfig(device_info.lLoginID, NET_ITC_GET_ICRCFG, 1, &dev_icr_config, sizeof(NET_ITC_ICRCFG));

    return true;
}

void HikControl::get_device_resource_cfg()
{
    NET_DVR_IPPARACFG_V40 ip_access_cfg;
    memset(&ip_access_cfg, 0, sizeof(ip_access_cfg));
    DWORD ret;

    device_info.bIPRet = NET_DVR_GetDVRConfig(device_info.lLoginID, NET_DVR_GET_IPPARACFG_V40, 0, &ip_access_cfg, sizeof(NET_DVR_IPPARACFG_V40), &ret);

    for(int i = 0; i < MAX_ANALOG_CHANNUM; i++) {
        if (i < device_info.iDeviceChanNum) {
            sprintf(device_info.struChanInfo[i].chChanName, "camera%d", i + device_info.iStartChan);
            device_info.struChanInfo[i].iChanIndex = i + device_info.iStartChan;  //通道号
            //不支持ip接入,9000以下设备不支持禁用模拟通道
            device_info.struChanInfo[i].bEnable = device_info.bIPRet ? ip_access_cfg.byAnalogChanEnable[i] : TRUE;
//            qDebug("%d\n", device_info[num].struChanInfo[i].iChanIndex);
        }
        else {
            sprintf(device_info.struChanInfo[i].chChanName, "");
            device_info.struChanInfo[i].iChanIndex = -1;
            device_info.struChanInfo[i].bEnable = FALSE;
        }
    }
    //支持IP接入，9000设备
    if (device_info.bIPRet) {
        //数字通道
        for(int i = 0; i < MAX_IP_CHANNEL; i++) {
            //ip通道在线
            if(ip_access_cfg.struStreamMode[i].uGetStream.struChanInfo.byEnable) {
                device_info.struChanInfo[i + MAX_ANALOG_CHANNUM].bEnable = TRUE;
                device_info.struChanInfo[i + MAX_ANALOG_CHANNUM].iChanIndex = i + ip_access_cfg.dwStartDChan;
                sprintf(device_info.struChanInfo[i + MAX_ANALOG_CHANNUM].chChanName, "IP Camera %d", i + 1);
            }
            else {
                device_info.struChanInfo[i + MAX_ANALOG_CHANNUM].bEnable = FALSE;
                device_info.struChanInfo[i + MAX_ANALOG_CHANNUM].iChanIndex = -1;
            }
        }
    }
}

void HikControl::on_CONNECT_BTN_clicked()
{
    if(logged_in) {
        NET_DVR_Logout_V30(device_info.lLoginID);
        device_info.lLoginID = -1;
    }
    else {
        if(!log_in()) return;
        get_device_resource_cfg();
    }

    logged_in ^= 1;
    ui->CONNECT_BTN->setIcon(QIcon(QString(":/icons/operations/operations/") + (logged_in ? "disconnect.png" : "connect.png")));
}

void HikControl::lens_btn_pressed(int id)
{
    switch (id) {
    case 0: NET_DVR_PTZControlWithSpeed_Other(device_info.lLoginID, 1, ZOOM_OUT, 0, speed);   break;
    case 1: NET_DVR_PTZControlWithSpeed_Other(device_info.lLoginID, 1, ZOOM_IN, 0, speed);    break;
    case 2: NET_DVR_PTZControlWithSpeed_Other(device_info.lLoginID, 1, FOCUS_FAR, 0, speed);  break;
    case 3: NET_DVR_PTZControlWithSpeed_Other(device_info.lLoginID, 1, FOCUS_NEAR, 0, speed); break;
    default:                                                                                  break;                                                              break;
    }
    qDebug() << "lens control start: " << NET_DVR_GetLastError();
}

void HikControl::lens_btn_released(int id)
{
    switch (id) {
    case 0: NET_DVR_PTZControlWithSpeed_Other(device_info.lLoginID, 1, ZOOM_OUT, 1, speed);   break;
    case 1: NET_DVR_PTZControlWithSpeed_Other(device_info.lLoginID, 1, ZOOM_IN, 1, speed);    break;
    case 2: NET_DVR_PTZControlWithSpeed_Other(device_info.lLoginID, 1, FOCUS_FAR, 1, speed);  break;
    case 3: NET_DVR_PTZControlWithSpeed_Other(device_info.lLoginID, 1, FOCUS_NEAR, 1, speed); break;
    default:                                                                                  break;
    }
    qDebug() << "lens control stop:  " << NET_DVR_GetLastError();
}

void HikControl::on_SPEED_SLIDER_sliderMoved(int position)
{
    speed = position;
}

void HikControl::on_SPEED_EDIT_editingFinished()
{
    speed = ui->SPEED_EDIT->text().toInt();
    if (speed > 7) speed = 7;
    if (speed < 1) speed = 1;
    ui->SPEED_SLIDER->setValue(speed);
    ui->SPEED_EDIT->setText(QString::asprintf("%d", speed));
}

void HikControl::on_ICR_CHK_stateChanged(int arg1)
{
    if (!logged_in) return;
    DWORD size = 0;
    NET_ITC_ICRCFG dev_icr_config = {0};
    NET_DVR_GetDVRConfig(device_info.lLoginID, NET_ITC_GET_ICRCFG, 1, &dev_icr_config, sizeof(NET_ITC_ICRCFG), &size);
    qDebug() << "get icr error:  " << NET_DVR_GetLastError();
    qDebug() << "switch" << dev_icr_config.bySwitchType;
    dev_icr_config.uICRParam.struICRManualSwitch.bySubSwitchMode = arg1 + 1;
    NET_DVR_SetDVRConfig(device_info.lLoginID, NET_ITC_GET_ICRCFG, 1, &dev_icr_config, sizeof(NET_ITC_ICRCFG));
}
