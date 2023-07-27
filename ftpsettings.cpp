#include "ftpsettings.h"
#include "ui_ftpsettings.h"

FTPSettings::FTPSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FTPSettings),
    logged_in(false),
    session(NULL),
    nanopi{"", "root", "fa"}
{
    ui->setupUi(this);

    ui->NANOPI_IP_EDT->setFocus();
}

FTPSettings::~FTPSettings()
{
    delete ui;
}

LIBSSH2_SESSION *FTPSettings::login(libssh2_socket_t &sock, int &result)
{
    uint32_t hostaddr = inet_addr(ui->NANOPI_IP_EDT->text().toLatin1().constData());
    struct sockaddr_in sin;
    const char *fingerprint;
    LIBSSH2_SESSION *session = NULL;
    QString fingerprint_hex("Fingerprint:");

    WSADATA wsadata;
    if (result = WSAStartup(MAKEWORD(2, 0), &wsadata)) {
        qWarning() << "WSAStartup failed with error:" << result;
        return NULL;
    }
    if (result = libssh2_init(0)) {
        qWarning() << "libssh2 initialization failed, error" << result;
        return NULL;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == LIBSSH2_INVALID_SOCKET) {
        qWarning() << "failed to create socket!";
        return NULL;
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    sin.sin_addr.s_addr = hostaddr;
    qInfo() << qPrintable(QString::asprintf("Connecting to %s:%d as user %s", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port), nanopi.usr.toLatin1().constData()));
    if (::connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in))) {
        qWarning() << "failed to connect!";
        return NULL;
    }

    session = libssh2_session_init();
    if (!session) {
        qWarning() << "Could not initialize SSH session!";
        return NULL;
    }

    libssh2_trace(session, ~0);

    if (result = libssh2_session_handshake(session, sock)) {
        qWarning() << "Failure establishing SSH session:" << result;
        return session;
    }

    fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
    for (int i = 0; i < 20; i++) fingerprint_hex += QString::asprintf(" %02X", (uchar)fingerprint[i]);
    qInfo() << qPrintable(fingerprint_hex);

    if (result = libssh2_userauth_password(session, nanopi.usr.toLatin1().constData(), nanopi.pwd.toLatin1().constData())) qWarning() << "Authentication by password failed!";
    else qInfo() << "Authentication by password succeeded.";

    libssh2_session_set_timeout(session, 5000);

    return session;
}

QString FTPSettings::exec_cmd(LIBSSH2_SESSION *session, const char *cmd)
{
    LIBSSH2_CHANNEL *channel;
    QString output_stdout, output_stderr;

    channel = libssh2_channel_open_session(session);
    if (!channel) {
        qWarning() << "Unable to open a session";
        return "";
    }

    if (libssh2_channel_exec(channel, cmd)) {
        qWarning() << "Unable to request command on channel";
        return "";
    }

    if (QString(cmd).startsWith("sudo ")) libssh2_channel_write(channel, "pi\n", 3);

    QElapsedTimer t;
    t.start();
    while (!libssh2_channel_eof(channel)) {
        char buf[1024] = {0};
        ssize_t err = libssh2_channel_read(channel, buf, sizeof(buf));
        if (err < 0) {
            qWarning() << "Unable to read response from stdout:" << (int)err;
            break;
        }
        else output_stdout += buf;
        memset(buf, 0, 1024);
        err = libssh2_channel_read_stderr(channel, buf, sizeof(buf));
        if (err < 0) {
            qWarning() << "Unable to read response from stderr:" << (int)err;
            break;
        }
        else output_stderr += buf;
        if (t.elapsed() > 2000) {
            qWarning() << "Read response timeout";
            break;
        }
    }
    qInfo() << qPrintable(output_stdout.trimmed());
    qWarning() << qPrintable(output_stderr.trimmed());

    libssh2_channel_get_exit_status(channel);
    if (libssh2_channel_close(channel)) qWarning() << "Unable to close channel";

    if (channel) {
        libssh2_channel_free(channel);
        channel = NULL;
    }

    return output_stdout.simplified();
}

void FTPSettings::exit_ssh(LIBSSH2_SESSION *session, libssh2_socket_t &sock)
{
    if (session) {
        libssh2_session_disconnect(session, "Normal Shutdown");
        libssh2_session_free(session);
    }

    if (sock != LIBSSH2_INVALID_SOCKET) shutdown(sock, 2);

    closesocket(sock);
    qInfo() << "Exiting ssh";
    libssh2_exit();
}

void FTPSettings::keyPressEvent(QKeyEvent *event)
{
    static QLineEdit *edit;

    switch (event->key()) {
    case Qt::Key_Escape:
        if(!this->focusWidget()) return;
        this->focusWidget()->clearFocus();
        return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (!(this->focusWidget())) break;
        edit = qobject_cast<QLineEdit*>(this->focusWidget());
        if (!edit) break;

        if (edit == ui->NANOPI_IP_EDT) ui->CONNECT_TO_NANOPI_BTN->click();
        else if (edit == ui->CUSTOM_CMD_EDT) ui->SEND_CMD_BTN->click();
        this->focusWidget()->clearFocus();

        break;
    default: break;
    }
}

void FTPSettings::on_CONNECT_TO_NANOPI_BTN_clicked()
{
    int result;
    session = login(sock, result);
    if (result) exit_ssh(session, sock);
    logged_in = !result;
}

void FTPSettings::on_SEND_CMD_BTN_clicked()
{
    if (!session) return;

    exec_cmd(session, ui->CUSTOM_CMD_EDT->text().toLatin1().constData());
}

void FTPSettings::on_GET_CONFIG_BTN_clicked()
{
    if (!session) return;

    QString config = exec_cmd(session, "cat /home/pi/config");

    QStringList config_list = config.split(" ");
    if (config_list.size() < 8) return;
    ui->DEVICE_IP_EDT->setText(device.ip = config_list[1]);
    ui->DEVICE_USR_EDT->setText(device.usr = config_list[2]);
    ui->DEVICE_PWD_EDT->setText(device.pwd = config_list[3]);
    ui->FTP_SERVER_EDT->setText(ftp.ip = config_list[5]);
    ui->FTP_USR_EDT->setText(ftp.usr = config_list[6]);
    ui->FTP_PWD_EDT->setText(ftp.pwd = config_list[7]);

    config = exec_cmd(session, "cat /etc/network/interfaces");

    config_list.clear();
    config_list = config.split(" ");
    if (config_list.size() < 8) return;
    int length = config_list.size();
    ui->STATIC_IP_EDT->setText(nanopi_ip.ip = config_list[length - 7]);
    ui->SUBNET_MASK_EDT->setText(nanopi_ip.netmask = config_list[length - 5]);
    ui->GATEWAY_EDT->setText(nanopi_ip.gateway = config_list[length - 3]);
    ui->DNS_SERVER_EDT->setText(nanopi_ip.dns = config_list[length - 1]);
}

void FTPSettings::on_RESET_BTN_clicked()
{
    ui->DEVICE_IP_EDT->setText("192.168.1.200");
    ui->DEVICE_USR_EDT->setText("admin");
    ui->DEVICE_PWD_EDT->setText("abcd1234");
    ui->FTP_SERVER_EDT->setText("101.200.166.95:9020");
    ui->FTP_USR_EDT->setText("ZONGZUOBIAO");
    ui->FTP_PWD_EDT->setText("ZONGZUOBIAO");
    ui->STATIC_IP_EDT->setText("192.168.1.199");
    ui->SUBNET_MASK_EDT->setText("255.255.255.0");
    ui->GATEWAY_EDT->setText("192.168.1.1");
    ui->DNS_SERVER_EDT->setText("4.2.2.1");
}

void FTPSettings::on_SET_CONFIG_FTP_BTN_clicked()
{
    if (!session) return;

    QString cmd = "echo device                             > /home/pi/config && "
                  "echo " + ui->DEVICE_IP_EDT->text()  + ">> /home/pi/config && "
                  "echo " + ui->DEVICE_USR_EDT->text() + ">> /home/pi/config && "
                  "echo " + ui->DEVICE_PWD_EDT->text() + ">> /home/pi/config && "
                  "echo                                   >> /home/pi/config && "
                  "echo ftp                               >> /home/pi/config && "
                  "echo " + ui->FTP_SERVER_EDT->text() + ">> /home/pi/config && "
                  "echo " + ui->FTP_USR_EDT->text() + "   >> /home/pi/config && "
                  "echo " + ui->FTP_PWD_EDT->text() + "   >> /home/pi/config";

    exec_cmd(session, cmd.toLatin1().constData());
}

void FTPSettings::on_SET_CONFIG_IP_BTN_clicked()
{
    if (!session) return;

    QString cmd = "sed -i '/address /s/" + nanopi_ip.ip + "/" + ui->STATIC_IP_EDT->text() + "/'           /etc/network/interfaces && "
                  "sed -i '/netmask /s/" + nanopi_ip.netmask + "/" + ui->SUBNET_MASK_EDT->text() + "/'    /etc/network/interfaces && "
                  "sed -i '/gateway /s/" + nanopi_ip.gateway + "/" + ui->SUBNET_MASK_EDT->text() + "/'    /etc/network/interfaces && "
                  "sed -i '/dns-nameservers /s/" + nanopi_ip.dns + "/" + ui->DNS_SERVER_EDT->text() + "/' /etc/network/interfaces";

    exec_cmd(session, cmd.toLatin1().constData());
}
