#ifndef DEMO_H
#define DEMO_H

#include <QtCore>
#include <QtWidgets>
#include <QTextCodec>
#include <QSerialPort>
#include "GeneralDef.h"
#include "opencv2/opencv.hpp"
#include "mylabel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Demo; }
QT_END_NAMESPACE

class Demo : public QMainWindow
{
    Q_OBJECT

public:
    Demo(QWidget *parent = nullptr);
    ~Demo();

public:
    static void CALLBACK real_data_call_back_0(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser);
    static void CALLBACK real_data_call_back_1(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser);
    static void CALLBACK dec_call_back(long nPort, char *pBuf, long nSize, FRAME_INFO *pFrameInfo, long nUser, long Resever);
    static void process_real_data(DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, int num);

    static void move_to_dest(QString src, QString dst);

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);
//    bool eventFilter(QObject *obj, QEvent *e);

private:
    bool log_in(int num);
    void get_device_resource_cfg(int num);
    void start_playing(int source);
    void stop_playing(int source);
    void start_recording(int source);
    void stop_recording(int source);
    void send_ctrl_cmd(uchar dir);
    void setup_com(QSerialPort **com, QString com_num);
    void communicate_display();
    void stop();
    uchar check_sum();
    void send_ptz_cmd(uchar dir);
    void move(float angle, bool vertical);

    void read_command_file();

    static void haze_removal(cv::Mat &src, cv::Mat &res, int radius, float omega, float t0, int guided_radius, float eps);
    static std::vector<uchar> estimate_atmospheric_light_avg(cv::Mat &src, cv::Mat &dark);
    static std::vector<uchar> estimate_atmospheric_light_brightest(cv::Mat &src, cv::Mat &dark);
    static void dark_channel(cv::Mat &src, cv::Mat &dark, cv::Mat &inter, int r);
    static void guided_filter(cv::Mat &p, cv::Mat &I, int r, float eps, int fast);

private slots:
    void control_button_pressed(int id);
    void control_button_released(int id);
    void lens_button_pressed(int id);
    void lens_button_released(int id);
    void login_button_clicked(int id);
    void play_button_clicked(int id);
    void record_button_clicked(int id);
    void capture_button_clicked(int id);
    void on_PATH_BROWSE_clicked();

    void on_SPEED_SLIDER_1_sliderMoved(int position);
    void on_SPEED_SLIDER_2_sliderMoved(int position);
//    void on_SPEED_SLIDER_3_sliderMoved(int position);
    void on_SPEED_EDIT_1_textEdited(const QString &arg1);
    void on_SPEED_EDIT_2_textEdited(const QString &arg1);
//    void on_SPEED_EDIT_3_textEdited(const QString &arg1);

    void on_RELAY_SWITCH_1_clicked();
    void on_RELAY_SWITCH_2_clicked();
//    void on_RELAY_SWITCH_3_clicked();
//    void on_RELAY_SWITCH_4_clicked();
    void on_RELAY_ALL_ON_BTN_clicked();
    void on_RELAY_ALL_OFF_BTN_clicked();
    void on_WIPER_BTN_clicked();

    void on_FPS_OPTION_1_currentIndexChanged(int index);
    void on_FPS_OPTION_2_currentIndexChanged(int index);

    void on_FILTER_CHK_stateChanged(int arg1);
    void on_DEHAZE_CHK_stateChanged(int arg1);
    void on_ENHANCE_CHK_stateChanged(int arg1);
    void on_INVERT_CHK_stateChanged(int arg1);

    void ptz_control_button_pressed(int id);
    void ptz_control_button_released(int id);
    void on_STOP_BTN_clicked();
    void on_SPEED_SLIDER_sliderMoved(int position);
    void on_SPEED_EDIT_textEdited(const QString &arg1);
    void on_GET_ANGLE_BTN_clicked();
    void on_SET_ANGLE_BTN_clicked();
    void point_ptz_to_target(QPoint target);

private:
    Ui::Demo *ui;

    long                h_play[2];                         // handle
    LOCAL_DEVICE_INFO   device_info[2];
    bool                logged_in[2];
    bool                playing[2];
    bool                recording[2];
    long                port[2];
    MyLabel*            display[2];
    QComboBox*          fps_option[2];
    QLineEdit*          ip_edit[2];
    void                (CALLBACK*real_data_cb[2])(LONG, DWORD, BYTE*, DWORD, void*);
    cv::Mat             img_rgb[2];
    int                 seq_idx;
    cv::Mat             seq[8];
    cv::Mat             sum;
    int                 w[2];
    int                 h[2];
    QMutex              cb_mutex[2];
    int                 speed[3];
    cv::VideoWriter     vid_out[2];

    QButtonGroup*       control_grp;
    QButtonGroup*       lens_grp;
    QButtonGroup*       login_grp;
    QButtonGroup*       play_grp;
    QButtonGroup*       record_grp;
    QButtonGroup*       capture_grp;
    QString             save_path;
    QString             TEMP_PATH;

    QSerialPort         *com;
    bool                relay_on[4];
    bool                wiper_on;
    bool                average;
    bool                dehaze;
    bool                enhance;
    bool                invert;
    uchar               buffer_out[7];
    uchar               buffer_in[7];
    int                 out_len;
    int                 in_len;
    float               angle_h;
    float               angle_v;

//    QTcpSocket          ftp_cmd;
//    QTcpSocket          ftp_data;
    QString             message;
    QString             vid_name[2];

    LONG                com_handle[2];

    NET_DVR_COMPRESSIONCFG_V30 dev_config[2];

    QString             zoom_in;
    QString             zoom_out;
    QString             focus_near;
    QString             focus_far;

};
#endif // DEMO_H
