#ifndef FTPSETTINGS_H
#define FTPSETTINGS_H

#include <libssh2.h>

#include <QtCore>
#include <QtWidgets>

namespace Ui {
class FTPSettings;
}

struct AuthInfoGrp {
    QString ip;
    QString usr;
    QString pwd;
};

struct IPSettings {
    QString ip;
    QString netmask;
    QString gateway;
    QString dns;
};

class FTPSettings : public QWidget
{
    Q_OBJECT

public:
    explicit FTPSettings(QWidget *parent = nullptr);
    ~FTPSettings();

    LIBSSH2_SESSION *login(libssh2_socket_t &sock, int &result);
    QString exec_cmd(LIBSSH2_SESSION *session, const char* cmd);
    void exit_ssh(LIBSSH2_SESSION *session, libssh2_socket_t &sock);

protected:
    // @override
    void keyPressEvent(QKeyEvent *event);

private slots:
    void on_CONNECT_TO_NANOPI_BTN_clicked();

    void on_SEND_CMD_BTN_clicked();

    void on_GET_CONFIG_BTN_clicked();

    void on_RESET_BTN_clicked();

    void on_SET_CONFIG_FTP_BTN_clicked();

    void on_SET_CONFIG_IP_BTN_clicked();

private:
    Ui::FTPSettings *ui;

    bool logged_in;

    libssh2_socket_t sock;
    LIBSSH2_SESSION *session;

    struct AuthInfoGrp nanopi;
    struct AuthInfoGrp device;
    struct AuthInfoGrp ftp;

    int time_interval;
    int time_start;
    int time_end;

    struct IPSettings nanopi_ip;
};

#endif // FTPSETTINGS_H
