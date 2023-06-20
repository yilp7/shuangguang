#include "demo.h"
#include "./ui_demo.h"

Demo* wnd;

// serial call back
void CALLBACK fsdcb(LONG lSerialHandle, LONG lChannel, char *pRecvDataBuffer, DWORD dwBufSize, void *pUser){
    QByteArray read(pRecvDataBuffer);
    QString send_str;
    for (int i = 0; i < dwBufSize; i++) send_str += QString::asprintf(" %02X", (uchar)read.data()[i]);
    qDebug("received %s", qPrintable(send_str));
}

Demo::Demo(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::Demo),
    h_play{-1, -1},
    logged_in{false},
    playing{false},
    recording{false},
    com(NULL),
    relay_on{true, true, true, true},
    wiper_on(false),
    average(false),
    dehaze(false),
    enhance(false),
    invert(false),
    buffer_out{0},
    buffer_in{0},
    in_len(7),
    angle_h(0),
    angle_v(0)
{
    ui->setupUi(this);
    wnd = this;

    NET_DVR_Init();

    read_command_file();

//    ui->NAME->setStyleSheet("color:#2069B1;");
    ui->SPEED_EDIT_1->hide();
    ui->SPEED_EDIT_2->hide();
//    ui->DEHAZE_BTN->hide();

    display[0] = ui->DISPLAY_1;
    display[1] = ui->DISPLAY_2;
    connect(display[0], SIGNAL(set_pixmap(QPixmap)), display[0], SLOT(setPixmap(QPixmap)));
    connect(display[1], SIGNAL(set_pixmap(QPixmap)), display[1], SLOT(setPixmap(QPixmap)));

    ip_edit[0] = ui->IP_EDIT_1;
    ip_edit[1] = ui->IP_EDIT_2;

    display[0]->setPixmap(QPixmap::fromImage(QImage(":/logo/yjs5.png").scaled(display[0]->size() * 2 / 3, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    display[1]->setPixmap(QPixmap::fromImage(QImage(":/logo/yjs5.png").scaled(display[1]->size() * 2 / 3, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

    save_path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
//    save_path += "C:/Users/";
//    save_path += QStandardPaths::writableLocation(QStandardPaths::HomeLocation).section("/", -1, -1);
//    save_path += "/Pictures";
    ui->PATH_EDIT->setText(save_path);
    TEMP_PATH = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

//    control_grp = new QButtonGroup();
//    control_grp->addButton(ui->LEFT_UP_BTN_1, 0);
//    control_grp->addButton(ui->UP_BTN_1, 1);
//    control_grp->addButton(ui->RIGHT_UP_BTN_1, 2);
//    control_grp->addButton(ui->LEFT_BTN_1, 3);
//    control_grp->addButton(ui->AUTO_BTN_1, 4);
//    control_grp->addButton(ui->RIGHT_BTN_1, 5);
//    control_grp->addButton(ui->LEFT_DOWN_BTN_1, 6);
//    control_grp->addButton(ui->DOWN_BTN_1, 7);
//    control_grp->addButton(ui->RIGHT_DOWN_BTN_1, 8);
//    connect(control_grp, SIGNAL(buttonPressed(int)), this, SLOT(control_button_pressed(int)));
//    connect(control_grp, SIGNAL(buttonReleased(int)), this, SLOT(control_button_released(int)));

    lens_grp = new QButtonGroup();
    lens_grp->addButton(ui->ZOOM_OUT_BTN_1, 1);
    lens_grp->addButton(ui->ZOOM_IN_BTN_1, 2);
    lens_grp->addButton(ui->FOCUS_FAR_BTN_1, 3);
    lens_grp->addButton(ui->FOCUS_NEAR_BTN_1, 4);
//    lens_grp->addButton(ui->IRIS_CLOSE_BTN_1, 5);
//    lens_grp->addButton(ui->IRIS_OPEN_BTN_1, 6);
    lens_grp->addButton(ui->ZOOM_OUT_BTN_2, 7);
    lens_grp->addButton(ui->ZOOM_IN_BTN_2, 8);
    lens_grp->addButton(ui->FOCUS_FAR_BTN_2, 9);
    lens_grp->addButton(ui->FOCUS_NEAR_BTN_2, 10);
//    lens_grp->addButton(ui->IRIS_CLOSE_BTN_2, 11);
//    lens_grp->addButton(ui->IRIS_OPEN_BTN_2, 12);
    connect(lens_grp, SIGNAL(buttonPressed(int)), this, SLOT(lens_button_pressed(int)));
    connect(lens_grp, SIGNAL(buttonReleased(int)), this, SLOT(lens_button_released(int)));

    play_grp = new QButtonGroup();
    play_grp->addButton(ui->PLAY_BTN_1, 0);
    play_grp->addButton(ui->PLAY_BTN_2, 1);
    connect(play_grp, SIGNAL(buttonClicked(int)), this, SLOT(play_button_clicked(int)), Qt::QueuedConnection);

    record_grp = new QButtonGroup();
    record_grp->addButton(ui->RECORD_BTN_1, 0);
    record_grp->addButton(ui->RECORD_BTN_2, 1);
    connect(record_grp, SIGNAL(buttonClicked(int)), this, SLOT(record_button_clicked(int)), Qt::QueuedConnection);

    capture_grp = new QButtonGroup();
    capture_grp->addButton(ui->CAPTURE_BTN_1, 0);
    capture_grp->addButton(ui->CAPTURE_BTN_2, 1);
    connect(capture_grp, SIGNAL(buttonClicked(int)), this, SLOT(capture_button_clicked(int)), Qt::QueuedConnection);

    QSlider *speed_slider = ui->SPEED_SLIDER_1;
    speed_slider->setMinimum(1);
    speed_slider->setMaximum(7);
    speed_slider->setSingleStep(1);
    speed_slider->setValue(4);
    speed_slider = ui->SPEED_SLIDER_2;
    speed_slider->setMinimum(1);
    speed_slider->setMaximum(7);
    speed_slider->setSingleStep(1);
    speed_slider->setValue(4);
//    speed_slider = ui->SPEED_SLIDER_3;
//    speed_slider->setMinimum(1);
//    speed_slider->setMaximum(64);
//    speed_slider->setSingleStep(1);
//    speed_slider->setValue(20);
    ui->SPEED_EDIT_1->setText("4");
    ui->SPEED_EDIT_2->setText("4");
//    ui->SPEED_EDIT_3->setText("20");

    real_data_cb[0] = &real_data_call_back_0;
    real_data_cb[1] = &real_data_call_back_1;

    QStringList sl;
    sl << "10" << "15" << "20" << "25";
    ui->FPS_OPTION_1->addItems(sl);
    ui->FPS_OPTION_1->setCurrentIndex(0);
    ui->FPS_OPTION_2->addItems(sl);
    ui->FPS_OPTION_2->setCurrentIndex(0);
    fps_option[0] = ui->FPS_OPTION_1;
    fps_option[1] = ui->FPS_OPTION_2;

//    com.setPortName("COM8");
//    if (com.open(QIODevice::ReadWrite)) {
//        qDebug("%s connected\n", com.portName().toLatin1().data());

//        com.setBaudRate(9600);
//        com.setDataBits(QSerialPort::Data8);
//        com.setParity(QSerialPort::NoParity);
//        com.setStopBits(QSerialPort::OneStop);
//        com.setFlowControl(QSerialPort::NoFlowControl);
//    }

    control_grp = new QButtonGroup();
    control_grp->addButton(ui->LEFT_UP_BTN, 0);
    control_grp->addButton(ui->UP_BTN, 1);
    control_grp->addButton(ui->RIGHT_UP_BTN, 2);
    control_grp->addButton(ui->LEFT_BTN, 3);
    control_grp->addButton(ui->AUTO_BTN, 4);
    control_grp->addButton(ui->RIGHT_BTN, 5);
    control_grp->addButton(ui->LEFT_DOWN_BTN, 6);
    control_grp->addButton(ui->DOWN_BTN, 7);
    control_grp->addButton(ui->RIGHT_DOWN_BTN, 8);
    connect(control_grp, SIGNAL(buttonPressed(int)), this, SLOT(ptz_control_button_pressed(int)));
    connect(control_grp, SIGNAL(buttonReleased(int)), this, SLOT(ptz_control_button_released(int)));

    QSlider *ptz_speed = ui->SPEED_SLIDER;
    ptz_speed->setMinimum(1);
    ptz_speed->setMaximum(64);
    ptz_speed->setSingleStep(1);
    ptz_speed->setValue(16);
    ui->SPEED_EDIT->setText("16");
    speed[2] = 16;

    com = new QSerialPort();
    setup_com(&com, ui->COM_EDIT->text());

    connect(ui->DISPLAY_2, SIGNAL(ptz_target(QPoint)), this, SLOT(point_ptz_to_target(QPoint)));

//    this->show();
}

Demo::~Demo()
{
    delete ui;
    NET_DVR_Cleanup();
}

void CALLBACK Demo::real_data_call_back_0(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser) {
    process_real_data(dwDataType, pBuffer, dwBufSize, 0);
}

void CALLBACK Demo::real_data_call_back_1(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser) {
    process_real_data(dwDataType, pBuffer, dwBufSize, 1);
}

void Demo::process_real_data(DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, int num) {
    switch(dwDataType) {
    case NET_DVR_SYSHEAD:
        if(!PlayM4_GetPort(&(wnd->port[num]))) break;
        if(dwBufSize > 0) {
            if(!PlayM4_SetStreamOpenMode(wnd->port[num], STREAME_REALTIME)) break;
            if(!PlayM4_OpenStream(wnd->port[num], pBuffer, dwBufSize, 1024*1024)) break;
            PlayM4_SetDecCallBackMend(wnd->port[num], dec_call_back, num);
            if(!PlayM4_Play(wnd->port[num], nullptr)) break;
        }
        break;
    case NET_DVR_STREAMDATA:
        if(dwBufSize > 0) PlayM4_InputData(wnd->port[num], pBuffer, dwBufSize);
        break;
    default:
        if(dwBufSize > 0) PlayM4_InputData(wnd->port[num], pBuffer, dwBufSize);
        break;
    }
}

void Demo::move_to_dest(QString src, QString dst)
{
    QFile::rename(src, dst);
}

void Demo::resizeEvent(QResizeEvent *event)
{
    QRect window = this->geometry();
//    qDebug("window: %d, %d", window.width(), window.height());
    QRect region[2] = {display[0]->geometry(), display[1]->geometry()};
    QPoint center[2] = {display[0]->center, display[1]->center};
    int height = (window.height() - 20 - 40 - 16) * 16 / 31,
        width = height * 5 / 3;
    int width_constraint = window.width() - 20 - 250,
        height_constraint = width_constraint * 3 / 5;
    if (height < height_constraint) height_constraint = height, width_constraint = height * 5 / 3;
    if (width > width_constraint) width = width_constraint, height = height_constraint;
//    qDebug("w1 %d, h1 %d", width, height * 15 / 16);
//    qDebug("w2 %d, h2 %d", width, height);

    center[0] = center[0] * width / region[0].width();
    center[1] = center[1] * width / region[1].width();
    region[0].setSize(QSize(width, height * 15 / 16));
    region[1].setRect(250, 20 + height * 15 / 16 + 40, width, height);

    cb_mutex[0].lock();
    cb_mutex[1].lock();
    display[0]->setGeometry(region[0]);
    display[0]->update_roi(center[0]);
    display[1]->setGeometry(region[1]);
    display[1]->update_roi(center[1]);
    cb_mutex[0].unlock();
    cb_mutex[1].unlock();

//    ui->SWITCH_GRP->move(20, 265 + (region[1].top() - 420) / 2);
    ui->CONTROL_1->move(20, region[0].bottom() - 171);
    ui->CONTROL_2->move(20, region[1].y());
    ui->PTZ_GRP->move(20, region[1].y() + 180);
    ui->LOGO->move(40, window.height() - 70);
    ui->PLAY_GRP_2->move(250, region[1].y() - 20);
    if (!playing[0]) display[0]->setPixmap(QPixmap::fromImage(QImage(":/logo/yjs5.png").scaled(display[0]->size() * 2 / 3, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    if (!playing[1]) display[1]->setPixmap(QPixmap::fromImage(QImage(":/logo/yjs5.png").scaled(display[1]->size() * 2 / 3, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
//    qDebug("display region1 x: %d, y: %d, w: %d, h: %d\n", region[0].x(), region[0].y(), region[0].width(), region[0].height());
//    qDebug("display region2 x: %d, y: %d, w: %d, h: %d\n", region[1].x(), region[1].y(), region[1].width(), region[1].height());

    event->accept();
}

void Demo::haze_removal(cv::Mat &src, cv::Mat &res, int radius, float omega, float t0, int guided_radius, float eps)
{
//    LARGE_INTEGER t1, t2, tc;
//    QueryPerformanceFrequency(&tc);
//    QueryPerformanceCounter(&t1);

    int step = src.step, h = src.rows, w = src.cols, ch = src.channels();
    cv::Mat dark(h, w, CV_8UC1, 255), inter(h, w, src.type());

    dark_channel(src, dark, inter, radius);
//    cv::imwrite("res/dark.bmp", dark);

//    QueryPerformanceCounter(&t2);
//    printf("- dark channel: %f\n", (double)(t2.QuadPart - t1.QuadPart) / (double)tc.QuadPart * 1e3);
//    t1 = t2;

    uchar A_avg;
    std::vector<uchar> A = estimate_atmospheric_light_avg(src, dark);

    if (src.channels() == 3) A_avg = (uint(A[0]) + A[1] + A[2]) / 3;
    else A_avg = A[0];
//    printf("%hhu, %hhu, %hhu\n", A[0], A[1], A[2]);

//    QueryPerformanceCounter(&t2);
//    printf("- A estimation: %f\n", (double)(t2.QuadPart - t1.QuadPart) / (double)tc.QuadPart * 1e3);
//    t1 = t2;

    cv::Mat transmission(h, w, CV_32FC1);
    dark.convertTo(transmission, CV_32FC1);
    transmission = 1 - omega * transmission / A_avg;
    guided_filter(transmission, src, guided_radius, eps, 3);
//    cv::imwrite("res/transmission.bmp", transmission * 255.0);

//    QueryPerformanceCounter(&t2);
//    printf("- guided filter: %f\n", (double)(t2.QuadPart - t1.QuadPart) / (double)tc.QuadPart * 1e3);
//    t1 = t2;

    uchar *ptr1 = src.data, *ptr2 = res.data;
    float *ptr3 = (float*)transmission.data;
//    uchar *ptr3 = dark.data;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            for (int k = 0; k < ch; k++) ptr2[i * step + j * ch + k] = cv::saturate_cast<uchar>((ptr1[i * step + j * ch + k] - A[k]) / std::max(ptr3[i * w + j], t0) + A[k]);
//            for (int k = 0; k < ch; k++) ptr2[i * step + j * ch + k] = cv::saturate_cast<uchar>((ptr1[i * step + j * ch + k] - (uchar)A[k]) / ptr3[i * w + j] + (uchar)A[k]);
//            for (int k = 0; k < ch; k++) ptr2[i * step + j * ch + k] = cv::saturate_cast<uchar>((ptr1[i * step + j * ch + k] - A) / max(1 - omega * ptr3[i * w + j] / A, t0) + A);
//            for (int k = 0; k < ch; k++) ptr2[i * step + j * ch + k] = cv::saturate_cast<uchar>((ptr1[i * step + j * ch + k] - A) / max(transmission.at<float>(i, j), t0) + A);
        }
    }

//    QueryPerformanceCounter(&t2);
//    printf("- reconstruct: %f\n", (double)(t2.QuadPart - t1.QuadPart) / (double)tc.QuadPart * 1e3);
}

std::vector<uchar> Demo::estimate_atmospheric_light_avg(cv::Mat &src, cv::Mat &dark)
{
    uchar *ptr = dark.data;
    int hash[256] = {0};
    for (int i = dark.total() - 1; i >= 0; i--) hash[ptr[i]]++;
    int h = src.rows, w = src.cols, step = src.step, ch = src.channels();
    int count = 0, top = dark.total() / 1000, threshold = 255;
    for (; count < top && threshold; threshold--) count += hash[threshold];

    std::vector<uint> _A(ch);
    for (int k = 0; k < ch; k++) _A[k] = 0;
    std::vector<uchar> A(ch);
    uchar *ptr1 = src.data, *ptr2 = dark.data;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if (ptr2[i * w + j] <= threshold) continue;
            for (int k = 0; k < ch; k++) _A[k] += ptr1[i * step + j * ch + k];
        }
    }
    for (int k = 0; k < ch; k++) A[k] = _A[k] / count;

    return A;
}

std::vector<uchar> Demo::estimate_atmospheric_light_brightest(cv::Mat &src, cv::Mat &dark)
{
    int w = src.cols, step = src.step, ch = src.channels();
    std::vector<uint> _A(ch);
    std::vector<uchar> A(ch);
    uchar *ptr1 = src.data, *ptr2 = dark.data;

    int heap_size = src.total() / 1000;
    std::vector<std::pair<uchar, int>> max_heap;
    for (int i = 0; i < heap_size; i++) max_heap.push_back(std::make_pair(ptr2[i], i));
    std::make_heap(max_heap.begin(), max_heap.end(), [](const std::pair<uchar, int>& a, const std::pair<uchar, int>& b) { return a.first > b.first; });
    for (int i = dark.total() - 1; i >= heap_size; i--) {
        if(ptr2[i] > max_heap.front().first) {
            max_heap.push_back(std::make_pair(ptr2[i], i));
            std::push_heap(max_heap.begin(), max_heap.end());
            std::pop_heap(max_heap.begin(), max_heap.end());
            max_heap.pop_back();
        }
    }
    std::vector<uchar*> vec_a;
    for (std::pair<uchar, int> p: max_heap) vec_a.push_back(src.data + (p.second / w * step + p.second % w * src.channels()));
    if (src.channels() == 3) std::sort(vec_a.begin(), vec_a.end(), [](const uchar* a, const uchar* b) { return uint(a[0]) + a[1] + a[2] > uint(b[0]) + b[1] + b[2]; });
    else                     std::sort(vec_a.begin(), vec_a.end(), [](const uchar* a, const uchar* b) { return a[0] > b[0]; });
    for (int i = 0; i < src.channels(); i++) A[i] = vec_a.front()[i];

//    if (ch == 3) {
//        for (int k = 0; k < 3; k++) _A[k] = std::accumulate(max_heap.begin(), max_heap.end(), (uint)0, [ptr1, w, ch, step, k](uint res, const std::pair<uchar, int>& a){ return res + ptr1[a.second / w * step + a.second % w * ch + k]; });
//        for (int k = 0; k < 3; k++) A[k] = _A[k] / heap_size;
//    }
}

void Demo::dark_channel(cv::Mat &src, cv::Mat &dark, cv::Mat &inter, int r)
{
    int h = src.rows, w = src.cols, ch = src.channels(), step = src.step;
    uchar *ptr1 = src.data, *ptr2 = dark.data;
    uchar curr_min, temp;

    // using erode as min filter for efficiency
    cv::Mat min_filter = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * r + 1, 2 * r + 1), cv::Point(r, r));

    inter = src.clone();
    cv::erode(src, inter, min_filter);

    ptr1 = inter.data;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            curr_min = ptr1[i * step + j * ch];
            for (int k = 1; k < ch; k++) {
                curr_min = curr_min > (temp = ptr1[i * step + j * ch + k]) ? temp : curr_min;
            }
            ptr2[i * w + j] = curr_min;
        }
    }
}

void Demo::guided_filter(cv::Mat &p, cv::Mat &I, int r, float eps, int fast)
{
    cv::Mat I_f, p_f;
    cv::Mat mean_p, mean_I, corr_I, corr_Ip, var_I, cov_Ip, a, b, mean_a, mean_b;
    if (I.channels() == 3) cv::cvtColor(I, I_f, cv::COLOR_RGB2GRAY);
    else I_f = I.clone();
    I_f.convertTo(I_f, CV_32FC1);
    p.convertTo(p_f, CV_32FC1);
    cv::resize(I_f, I_f, cv::Size(I.cols / fast, I.rows / fast));
    cv::resize(p_f, p_f, cv::Size(p.cols / fast, p.rows / fast));
    I_f /= 255.0;

    cv::boxFilter(I_f, mean_I, CV_32FC1, cv::Size(2 * r + 1, 2 * r + 1));
    cv::boxFilter(p_f, mean_p, CV_32FC1, cv::Size(2 * r + 1, 2 * r + 1));
    cv::boxFilter(I_f.mul(I_f), corr_I, CV_32FC1, cv::Size(2 * r + 1, 2 *r + 1));
    cv::boxFilter(I_f.mul(p_f), corr_Ip, CV_32FC1, cv::Size(2 * r + 1, 2 * r + 1));

    var_I = corr_I - mean_I.mul(mean_I);
    cov_Ip = corr_Ip - mean_I.mul(mean_p);

    a = cov_Ip.mul(1 / (var_I + eps));
    b = mean_p - a.mul(mean_I);

    cv::boxFilter(a, mean_a, CV_32FC1, cv::Size(2 * r + 1, 2 * r + 1));
    cv::boxFilter(b, mean_b, CV_32FC1, cv::Size(2 * r + 1, 2 * r + 1));

    p_f = mean_a.mul(I_f) + mean_b;
    cv::resize(p_f, p_f, p.size());
    p_f.convertTo(p, p.type());
}

void CALLBACK Demo::dec_call_back(long nPort, char *pBuf, long nSize, FRAME_INFO *pFrameInfo, long nUser, long Resever) {
    if(pFrameInfo->nType == T_YV12) {
        if (!wnd->cb_mutex[nUser].tryLock(50)) return;
        wnd->w[nUser] = pFrameInfo->nWidth;
        wnd->h[nUser] = pFrameInfo->nHeight;

        MyLabel *disp = wnd->display[nUser];
        cv::cvtColor(cv::Mat(pFrameInfo->nHeight * 3 / 2, pFrameInfo->nWidth, CV_8UC1, pBuf), wnd->img_rgb[nUser], cv::COLOR_YUV2RGB_YV12);
        cv::Rect rect(disp->rect_params.tl() * wnd->w[nUser] / disp->width(), disp->rect_params.br() * wnd->w[nUser] / disp->width());
        if (rect.height > pFrameInfo->nHeight) rect.height = pFrameInfo->nHeight;

        if (!nUser) {
            if (wnd->sum.empty()) {
                wnd->seq_idx = 0;
                wnd->sum = cv::Mat::zeros(wnd->h[nUser], wnd->w[nUser], CV_16UC3);
                for (auto &m : wnd->seq) m = cv::Mat(wnd->h[nUser], wnd->w[nUser], CV_16UC3);
            }
            wnd->sum -= wnd->seq[wnd->seq_idx];
            wnd->img_rgb[nUser].convertTo(wnd->seq[wnd->seq_idx], CV_16UC3);
            wnd->sum += wnd->seq[wnd->seq_idx];
            wnd->seq_idx = (wnd->seq_idx + 1) % 8;
            if (wnd->average) wnd->sum.convertTo(wnd->img_rgb[nUser], CV_8UC3, 1 / 8.);
            if (wnd->dehaze) haze_removal(wnd->img_rgb[nUser], wnd->img_rgb[nUser], 7, 0.98, 0.1, 60, 0.001), wnd->img_rgb[nUser] *= 1.2;
            if (wnd->enhance) {
                wnd->img_rgb[nUser] = ~wnd->img_rgb[nUser];
                haze_removal(wnd->img_rgb[nUser], wnd->img_rgb[nUser], 7, 0.98, 0.1, 60, 0.001);
                wnd->img_rgb[nUser] = ~wnd->img_rgb[nUser];
            }
            if (wnd->invert) cv::flip(wnd->img_rgb[nUser], wnd->img_rgb[nUser], -1);
        }

        cv::Mat frame = wnd->img_rgb[nUser](rect);
        cv::resize(frame, frame, cv::Size(disp->width(), disp->height()), cv::INTER_AREA), frame *= 1.2;

//        if (wnd->dehaze) haze_removal(frame, frame, 7, 0.95, 0.1, 60, 0.01);

//        disp->setPixmap(QPixmap::fromImage(QImage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888)));
        emit disp->set_pixmap(QPixmap::fromImage(QImage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888)));

        wnd->cb_mutex[nUser].unlock();
    }
}

bool Demo::log_in(int num) {
//    data_exchange(true);

    NET_DVR_DEVICEINFO_V40 temp_device_info;
    memset(&temp_device_info, 0, sizeof(NET_DVR_DEVICEINFO_V40));
    NET_DVR_USER_LOGIN_INFO usr_info;
    memset(&usr_info, 0, sizeof(NET_DVR_USER_LOGIN_INFO));

    sprintf_s(usr_info.sDeviceAddress, 129, ip_edit[num]->text().toLatin1().data());
    sprintf_s(usr_info.sUserName, 129, "admin");
    sprintf_s(usr_info.sPassword, 129, "abcd1234");
    usr_info.wPort = 8000;
    LONG lLoginID = NET_DVR_Login_V40(&usr_info, &temp_device_info);

    if(lLoginID == -1) {
        QMessageBox::warning(this, "PROMPT", "Login to the device failed!");
        return false;
    }
    device_info[num].lLoginID       = lLoginID;
    device_info[num].iDeviceChanNum = temp_device_info.struDeviceV30.byChanNum;
    device_info[num].iIPChanNum     = temp_device_info.struDeviceV30.byIPChanNum;
    device_info[num].iStartChan     = temp_device_info.struDeviceV30.byStartChan;
    device_info[num].iIPStartChan   = temp_device_info.struDeviceV30.byStartDChan;

    DWORD size = 0;
    NET_DVR_GetDVRConfig(device_info[num].lLoginID, NET_DVR_GET_COMPRESSCFG_V30, 1, dev_config + num, sizeof(NET_DVR_COMPRESSIONCFG_V30), &size);
    dev_config[num].struNormHighRecordPara.dwVideoFrameRate = 10;
    NET_DVR_SetDVRConfig(device_info[num].lLoginID, NET_DVR_SET_COMPRESSCFG_V30, 1, dev_config + num, sizeof(NET_DVR_COMPRESSIONCFG_V30));
    fps_option[num]->setCurrentIndex(0);

    return true;
}

void Demo::get_device_resource_cfg(int num) {
    NET_DVR_IPPARACFG_V40 ip_access_cfg;
    memset(&ip_access_cfg, 0, sizeof(ip_access_cfg));
    DWORD ret;

    device_info[num].bIPRet = NET_DVR_GetDVRConfig(device_info[num].lLoginID, NET_DVR_GET_IPPARACFG_V40, 0, &ip_access_cfg, sizeof(NET_DVR_IPPARACFG_V40), &ret);

    for(int i = 0; i < MAX_ANALOG_CHANNUM; i++) {
        if (i < device_info[num].iDeviceChanNum) {
            sprintf(device_info[num].struChanInfo[i].chChanName, "camera%d", i + device_info[num].iStartChan);
            device_info[num].struChanInfo[i].iChanIndex = i + device_info[num].iStartChan;  //通道号
            //不支持ip接入,9000以下设备不支持禁用模拟通道
            device_info[num].struChanInfo[i].bEnable = device_info[num].bIPRet ? ip_access_cfg.byAnalogChanEnable[i] : TRUE;
//            qDebug("%d\n", device_info[num].struChanInfo[i].iChanIndex);
        }
        else {
            sprintf(device_info[num].struChanInfo[i].chChanName, "");
            device_info[num].struChanInfo[i].iChanIndex = -1;
            device_info[num].struChanInfo[i].bEnable = FALSE;
        }
    }
    //支持IP接入，9000设备
    if (device_info[num].bIPRet) {
        //数字通道
        for(int i = 0; i < MAX_IP_CHANNEL; i++) {
            //ip通道在线
            if(ip_access_cfg.struStreamMode[i].uGetStream.struChanInfo.byEnable) {
                device_info[num].struChanInfo[i + MAX_ANALOG_CHANNUM].bEnable = TRUE;
                device_info[num].struChanInfo[i + MAX_ANALOG_CHANNUM].iChanIndex = i + ip_access_cfg.dwStartDChan;
                sprintf(device_info[num].struChanInfo[i + MAX_ANALOG_CHANNUM].chChanName, "IP Camera %d", i + 1);
            }
            else {
                device_info[num].struChanInfo[i + MAX_ANALOG_CHANNUM].bEnable = FALSE;
                device_info[num].struChanInfo[i + MAX_ANALOG_CHANNUM].iChanIndex = -1;
            }
        }
    }
}

void Demo::start_playing(int source) {
    login_button_clicked(source);

    NET_DVR_CLIENTINFO client_info;
//    client_info.hPlayWnd     = (HWND)display[source]->winId();
    client_info.hPlayWnd     = NULL;
    client_info.lChannel     = 1;
    client_info.lLinkMode    = 0;
    client_info.sMultiCastIP = NULL;

//    NET_DVR_PREVIEWINFO preview_info;
////    preview_info.hPlayWnd     = (HWND)display[source]->winId();
//    preview_info.hPlayWnd     = NULL;
//    preview_info.lChannel     = 1;
//    preview_info.dwStreamType = 0;
//    preview_info.dwLinkMode   = 0;
//    preview_info.bBlocked     = 1;

//    qDebug("Channel number:%ld\n", client_info.lChannel);
    h_play[source] = NET_DVR_RealPlay_V30(device_info[source].lLoginID, &client_info, real_data_cb[source], NULL, TRUE);
//    h_play[source] = NET_DVR_RealPlay_V30(device_info[source].lLoginID, &client_info, NULL, NULL, TRUE);
//    h_play[source] = NET_DVR_RealPlay_V40(device_info[source].lLoginID, &preview_info, real_data_cb[source], NULL);
//    h_play[source] = NET_DVR_RealPlay_V40(device_info[source].lLoginID, &preview_info, NULL, NULL);
    if(h_play[source] == -1) {
        QMessageBox::warning(this, "PROMPT", QString::asprintf("error playing, code %ld", NET_DVR_GetLastError()));
        return;
    }

    playing[source] = true;
//    ui->PLAY_BTN->setText("Stop");
    play_grp->button(source)->setIcon(QIcon(":/icons/video_control/play_stop.png"));
    display[source]->update_roi(QPoint());
    display[source]->grab = true;

//    if (source) {
//        grab_thread = new GrabThread(this);
//        grab_thread->start();
//    }
}

void Demo::stop_playing(int source) {
    if (h_play[source] == -1) return;
    if (recording[source]) {
        stop_recording(source);
        QThread::msleep(1000); // wait for stop_save_real_data to complete
    }

    NET_DVR_StopRealPlay(h_play[source]);

    h_play[source] = -1;
    playing[source] = false;
//    ui->PLAY_BTN->setText("Play");
    play_grp->button(source)->setIcon(QIcon(":/icons/video_control/play_start.png"));
    display[source]->setPixmap(QPixmap::fromImage(QImage(":/logo/yjs5.png").scaled(display[source]->size() * 2 / 3, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

//    if (source) {
//        grab_thread->quit();
//        grab_thread->wait();
//        delete grab_thread;
//    }
//    display[source]->grab = false;
}

void Demo::start_recording(int source) {
    vid_name[source] = QDateTime::currentDateTime().toString("yyyyMMddhhmmss") +
            QString::asprintf("_ch%02d_dev%d.avi", device_info[source].struChanInfo[0].iChanIndex, source + 1);
    qDebug() << save_path + "/" + vid_name[source];
    NET_DVR_SaveRealData(h_play[source], (TEMP_PATH + "/" + vid_name[source]).toLatin1().data());
//    vid_out[source].open(rec_name.toLatin1().data(), cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), 25, cv::Size(w[source], h[source]), true);
    recording[source] = true;
    record_grp->button(source)->setIcon(QIcon(":/icons/video_control/record_stop.png"));
}

void Demo::stop_recording(int source) {
    NET_DVR_StopSaveRealData(h_play[source]);
//    vid_out[source].release();
    recording[source] = false;
    record_grp->button(source)->setIcon(QIcon(":/icons/video_control/record_start.png"));

    std::thread t(Demo::move_to_dest, TEMP_PATH + "/" + vid_name[source], save_path + "/" + vid_name[source]);
    t.detach();
}

void Demo::send_ctrl_cmd(uchar dir)
{
    uchar buffer_out[7] = {0};
    buffer_out[0] = 0xFF;
    buffer_out[1] = 0x01;
    buffer_out[2] = 0x00;
    buffer_out[3] = dir;
    buffer_out[4] = dir < 5 ? speed[2] : 0x00;
    buffer_out[5] = dir > 5 ? speed[2] : 0x00;
    int sum = 0;
    for (int i = 1; i < 6; i++) sum += buffer_out[i];
    buffer_out[6] = sum & 0xFF;
    NET_DVR_SerialSend(com_handle[0], 1, (char*)&buffer_out, 7);
//    com.write(QByteArray((char*)&send, 7));
    QString send_str;
    for (int i = 0; i < 7; i++) send_str += QString::asprintf(" %02X", buffer_out[i]);
    qDebug("%s", qPrintable(send_str));
}

void Demo::control_button_pressed(int id) {
//    qDebug("%dp\n", id);
    switch (id) {
    case 0: send_ctrl_cmd(0x08); send_ctrl_cmd(0x04); break;
    case 1: send_ctrl_cmd(0x08);                      break;
    case 2: send_ctrl_cmd(0x08); send_ctrl_cmd(0x02); break;
    case 3: send_ctrl_cmd(0x04);                      break;
//    case 4: send_ctrl_cmd(0x00); break;
    case 5: send_ctrl_cmd(0x02);                      break;
    case 6: send_ctrl_cmd(0x10); send_ctrl_cmd(0x04); break;
    case 7: send_ctrl_cmd(0x10);                      break;
    case 8: send_ctrl_cmd(0x10); send_ctrl_cmd(0x02); break;
    default:                                          break;                                                              break;
    }
}

void Demo::control_button_released(int id) {
    long long send = 0x010000000001FF;
    NET_DVR_SerialSend(com_handle[0], 1, (char*)&send, 7);
//    com.write(QByteArray((char*)&send, 7));
    QString send_str;
    for (int i = 0; i < 7; i++) send_str += QString::asprintf(" %02X", ((uchar*)&send)[i]);
    qDebug("%s", qPrintable(send_str));
}

void Demo::lens_button_pressed(int id)
{
    int ret, grp = id < 7;
//    qDebug() << grp;
    if (grp) {
        switch (id) {
        case 1: ret = NET_DVR_PTZControlWithSpeed(h_play[0], ZOOM_OUT, 0, speed[0]);   break;
        case 2: ret = NET_DVR_PTZControlWithSpeed(h_play[0], ZOOM_IN, 0, speed[0]);    break;
        case 3: ret = NET_DVR_PTZControlWithSpeed(h_play[0], FOCUS_FAR, 0, speed[0]);  break;
        case 4: ret = NET_DVR_PTZControlWithSpeed(h_play[0], FOCUS_NEAR, 0, speed[0]); break;
        case 5: ret = NET_DVR_PTZControlWithSpeed(h_play[0], IRIS_CLOSE, 0, speed[0]); break;
        case 6: ret = NET_DVR_PTZControlWithSpeed(h_play[0], IRIS_OPEN, 0, speed[0]);  break;
        default:ret = 1;                                                               break;
        }
    }
    else {
        QString send_str, temp;
        QByteArray send;
        switch (id - 6) {
/*
        case 1: send = 0x410000400001FF; goto send_data;
        case 2: send = 0x210000200001FF; goto send_data;
        case 3: send = 0x810000800001FF; goto send_data;
        case 4: send = 0x020000000101FF; goto send_data;
        case 5: send = 0x050000000401FF; goto send_data;
        case 6: send = 0x030000000201FF; goto send_data;
*/
//        case 1: send_str = "AA07009D0201020154EBAA"; goto send_data;
/*
        case 1: send_str = "FF0B004000004B"; goto send_data;
        case 2: send_str = "FF0B002000002B"; goto send_data; //8101040702FF
        case 3: send_str = "FF0B010000000C"; goto send_data;
        case 4: send_str = "FF0B008000008B"; goto send_data;
*/
/*
        case 1: send_str = "FF010040000041"; goto send_data;
        case 2: send_str = "FF010020000021"; goto send_data;
        case 3: send_str = "FF010100000002"; goto send_data;
        case 4: send_str = "FF010080000081"; goto send_data;
*/
        case 1: send_str = zoom_out;   goto send_data;
        case 2: send_str = zoom_in;    goto send_data; //8101040702FF
        case 3: send_str = focus_far;  goto send_data;
        case 4: send_str = focus_near; goto send_data;
        default: return;
        }
        send_data:
//        com.write(QByteArray((char*)&send, 8));
        bool ok;
//        qDebug() << "send_str" << send_str;
        for (int i = 0; i < 7; i++) send.append(send_str.midRef(i * 2, 2).toInt(&ok, 16));
        for (int i = 0; i < 7; i++) temp += QString::asprintf(" %02X", (uchar)send[i]);
        qDebug() << "sent" << temp;
        NET_DVR_SerialSend(com_handle[0], 1, send.data(), 11);
    }
}

void Demo::lens_button_released(int id)
{
    int ret, grp = id < 7;
    if (grp) {
        switch (id) {
        case 1: ret = NET_DVR_PTZControlWithSpeed(h_play[0], ZOOM_OUT, 1, speed[0]);   break;
        case 2: ret = NET_DVR_PTZControlWithSpeed(h_play[0], ZOOM_IN, 1, speed[0]);    break;
        case 3: ret = NET_DVR_PTZControlWithSpeed(h_play[0], FOCUS_FAR, 1, speed[0]);  break;
        case 4: ret = NET_DVR_PTZControlWithSpeed(h_play[0], FOCUS_NEAR, 1, speed[0]); break;
        case 5: ret = NET_DVR_PTZControlWithSpeed(h_play[0], IRIS_CLOSE, 1, speed[0]); break;
        case 6: ret = NET_DVR_PTZControlWithSpeed(h_play[0], IRIS_OPEN, 1, speed[0]);  break;
        default:ret = 1;                                                               break;
        }
    }
    else {
        QString send_str, temp;
        QByteArray send;
        switch (id - 6) {
/*
        case 1: send_str = "FF0B000000000B"; goto send_data; //8101040700FF
        case 2: send_str = "FF0B000000000B"; goto send_data;
        case 3: send_str = "FF0B000000000B"; goto send_data;
        case 4: send_str = "FF0B000000000B"; goto send_data;
*/
        case 1:
        case 2:
        case 3:
//        case 4: send_str = "FF010000000001"; goto send_data;
//        case 4: send_str = "FF0B000000000B"; goto send_data;
        case 4: send_str = lens_stop; goto send_data;
        default: return;
        }
        send_data:
//        com.write(QByteArray((char*)&send, 8));
        bool ok;
        for (int i = 0; i < 7; i++) send.append(send_str.mid(i * 2, 2).toInt(&ok, 16));
        for (int i = 0; i < 7; i++) temp += QString::asprintf(" %02X", (uchar)send[i]);
        qDebug("%s", qPrintable(temp));
        NET_DVR_SerialSend(com_handle[0], 1, send.data(), 11);
    }
}

void Demo::login_button_clicked(int id) {
    if(logged_in[id]) {
        if (recording[id]) {
            stop_recording(id);
            QThread::msleep(1000); // wait for stop_save_real_data to complete
        }
        if (playing[id]) stop_playing(id);

        NET_DVR_Logout_V30(device_info[id].lLoginID);
        device_info[id].lLoginID = -1;
        if (id) NET_DVR_SerialStop(com_handle[0]);
    }
    if(!log_in(id)) return;
    get_device_resource_cfg(id);
    if (id) {
        NET_DVR_DECODERCFG_V40 rs485;
        DWORD size;
        NET_DVR_GetDVRConfig(device_info[1].lLoginID, NET_DVR_GET_DECODERCFG_V40, 0xFFFFFFFF, &rs485, sizeof(NET_DVR_DECODERCFG_V40), &size);
        rs485.dwBaudRate = 9; // 9: 9600; 14: 115200
        NET_DVR_SetDVRConfig(device_info[1].lLoginID, NET_DVR_GET_DECODERCFG_V40, 0xFFFFFFFF, &rs485, sizeof(NET_DVR_DECODERCFG_V40));
        NET_DVR_SERIALSTART_V40 serial = {sizeof(NET_DVR_SERIALSTART_V40), 2, 1, {0}};
        com_handle[0] = NET_DVR_SerialStart_V40(device_info[1].lLoginID, &serial, sizeof(NET_DVR_SERIALSTART_V40), fsdcb, NULL);
        qDebug() << "serial 485: " << NET_DVR_GetLastError();

//        NET_DVR_RS232CFG_V30 rs232;
//        DWORD size;
//        NET_DVR_GetDVRConfig(device_info[1].lLoginID, NET_DVR_GET_RS232CFG_V30, 0xFFFFFFFF, &rs232, sizeof(NET_DVR_RS232CFG_V30), &size);
//        memcpy(rs232.struRs232, new NET_DVR_SINGLE_RS232{9, 3, 0, 0, 0, 2}, sizeof(NET_DVR_SINGLE_RS232));
//        qDebug() << rs232.struRs232->dwBaudRate;
//        qDebug() << rs232.struRs232->dwWorkMode;
//        serial = {sizeof(NET_DVR_SERIALSTART_V40), 1, 0, {0}};
//        com_handle[1] = NET_DVR_SerialStart_V40(device_info[1].lLoginID, &serial, sizeof(NET_DVR_SERIALSTART_V40), fsdcb, NULL);
//        qDebug() << "serial 232: " << NET_DVR_GetLastError();
    }

    logged_in[id] = !logged_in[id];
}

void Demo::play_button_clicked(int id) {
    playing[id] ? stop_playing(id) : start_playing(id);
}

void Demo::record_button_clicked(int id) {
    if(h_play[id] == -1) {
        QMessageBox::warning(this, "PROMPT", "Please select a channel to play");
        return;
    }
    recording[id] ? stop_recording(id) : start_recording(id);
}

void Demo::capture_button_clicked(int id) {
    if(h_play[id] == -1) {
        QMessageBox::warning(this, "PROMPT", "Please select a channel to play");
        return;
    }
//    data_exchange(true);

    QString pic_name = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");

    int ret = 0;
    if (true) {
    // bmp
        pic_name += ".bmp";
        cb_mutex[id].lock();
        cv::Mat save = img_rgb[id].clone();
        cb_mutex[id].unlock();
        cv::cvtColor(save, save, cv::COLOR_RGB2BGR);
        cv::imwrite((TEMP_PATH + "/" + pic_name).toLatin1().data(), save);
//        ret = NET_DVR_CapturePicture(h_play[id], pic_name.toLatin1().data());
        std::thread t(Demo::move_to_dest, TEMP_PATH + "/" + pic_name, save_path + "/" + pic_name);
        t.detach();
    }
    // jpg
    else {
        pic_name += ".jpg";
        NET_DVR_JPEGPARA jpg_para = {0};
        // 0: "CIF", 1: "QCIF", 2: "D1", 3: "UXGA", 4: "SVGA", 5: "HD720p", 6: "VGA", 7: "XVGA", 8: "HD900p"
        jpg_para.wPicSize = 7;
        // 0: "Normal", 1: "Fine", 2: "Superfine"
        jpg_para.wPicQuality = 2;

        ret = NET_DVR_CaptureJPEGPicture(device_info[id].lLoginID,
                                         device_info[id].struChanInfo[0].iChanIndex,
                                         &jpg_para,
                                         pic_name.toLatin1().data());
    }
    qDebug() << save_path + "/" + pic_name;
//    qDebug("capture error %d (0x%x)\n", ret, ret);
}

void Demo::on_PATH_BROWSE_clicked()
{
    QString temp = QFileDialog::getExistingDirectory(this, "Select folder", save_path);
    if (!temp.isEmpty()) save_path = temp;
    ui->PATH_EDIT->setText(save_path);
}

void Demo::on_SPEED_SLIDER_1_sliderMoved(int position)
{
    speed[0] = position;
    uchar buffer_out[9] = {0};
    buffer_out[0] = 0xB0;
    buffer_out[1] = 0x01;
    buffer_out[4] = 0x20;
    buffer_out[5] = 0x00;
    buffer_out[6] = 0x00;
    buffer_out[7] = 0x00;

    buffer_out[2] = speed[0];
    buffer_out[3] = speed[0];

    buffer_out[8] = (2 * (uint)speed[0] + 0x21) & 0xFF;

//    com.write(QByteArray((char*)&buffer_out, 9));
    NET_DVR_SerialSend(com_handle[0], 1, (char*)&buffer_out, 9);
    QString send_str;
    for (int i = 0; i < 8; i++) send_str += QString::asprintf(" %02X", (uchar)buffer_out[i]);
    qDebug("%s", qPrintable(send_str));
    ui->SPEED_EDIT_1->setText(QString::asprintf("%d", position));
}

void Demo::on_SPEED_SLIDER_2_sliderMoved(int position)
{
    speed[1] = position;
    ui->SPEED_EDIT_2->setText(QString::asprintf("%d", position));
}
/*
void Demo::on_SPEED_SLIDER_3_sliderMoved(int position)
{
    speed[2] = position;
    ui->SPEED_EDIT_2->setText(QString::asprintf("%d", position));
}
*/
void Demo::on_SPEED_EDIT_1_textEdited(const QString &arg1)
{
    speed[0] = (char)arg1.toInt();
    if (speed[0] > 7) speed[0] = 7;
    if (speed[0] < 1) speed[0] = 1;
    ui->SPEED_SLIDER_1->setValue(speed[0]);
    ui->SPEED_EDIT_1->setText(QString::asprintf("%d", speed[0]));
}

void Demo::on_SPEED_EDIT_2_textEdited(const QString &arg1)
{
    speed[1] = (char)arg1.toInt();
    if (speed[1] > 7) speed[1] = 7;
    if (speed[1] < 1) speed[1] = 1;
    ui->SPEED_SLIDER_2->setValue(speed[1]);
    ui->SPEED_EDIT_2->setText(QString::asprintf("%d", speed[1]));
}
/*
void Demo::on_SPEED_EDIT_3_textEdited(const QString &arg1)
{
    speed[2] = (char)arg1.toInt();
    if (speed[1] > 64) speed[1] = 64;
    if (speed[1] < 1) speed[1] = 1;
    ui->SPEED_SLIDER_3->setValue(speed[1]);
    ui->SPEED_EDIT_3->setText(QString::asprintf("%d", speed[1]));
}
*/
void Demo::on_RELAY_SWITCH_1_clicked()
{
    long long send = relay_on[0] ? 0x098C00FF00000502 : 0xF9CD000000000502;
//    com.write(QByteArray((char*)&send, 8));
    NET_DVR_SerialSend(com_handle[0], 1, (char*)&send, 8);
    QString send_str;
    for (int i = 0; i < 8; i++) send_str += QString::asprintf(" %02X", ((uchar*)&send)[i]);
    qDebug("%s", qPrintable(send_str));
    relay_on[0] ^= 1;
    ui->RELAY_SWITCH_1->setText(relay_on[0] ? "下电" : "上电");
}

void Demo::on_RELAY_SWITCH_2_clicked()
{
    long long send = relay_on[1] ? 0xC9DD00FF01000502 : 0x399C000001000502;
//    com.write(QByteArray((char*)&send, 8));
    NET_DVR_SerialSend(com_handle[0], 1, (char*)&send, 8);
    QString send_str;
    for (int i = 0; i < 8; i++) send_str += QString::asprintf(" %02X", ((uchar*)&send)[i]);
    qDebug("%s", qPrintable(send_str));
    relay_on[1] ^= 1;
    ui->RELAY_SWITCH_2->setText(relay_on[1] ? "下电" : "上电");
}
/*
void Demo::on_RELAY_SWITCH_3_clicked()
{
    long long send = relay_on[2] ? 0xC92D00FF02000502 : 0x396C000002000502;
//    com.write(QByteArray((char*)&send, 8));
    NET_DVR_SerialSend(com_handle, 1, (char*)&send, 8);
    QString send_str;
    for (int i = 0; i < 8; i++) send_str += QString::asprintf(" %02X", ((uchar*)&send)[i]);
    qDebug("%s", qPrintable(send_str));
    relay_on[2] ^= 1;
    ui->RELAY_SWITCH_3->setText(relay_on[2] ? "O" : "X");
}

void Demo::on_RELAY_SWITCH_4_clicked()
{
    long long send = relay_on[3] ? 0x097C00FF03000502 : 0xF93D000003000502;
//    com.write(QByteArray((char*)&send, 8));
    NET_DVR_SerialSend(com_handle, 1, (char*)&send, 8);
    QString send_str;
    for (int i = 0; i < 8; i++) send_str += QString::asprintf(" %02X", (((uchar*)&send)[i]));
    qDebug("%s", qPrintable(send_str));
    relay_on[3] ^= 1;
    ui->RELAY_SWITCH_4->setText(relay_on[3] ? "O" : "X");
}
*/
void Demo::on_RELAY_ALL_ON_BTN_clicked()
{
    QString send_str = "020F000000080100BE80", temp;
    bool ok;
    QByteArray send;
    for (int i = 0; i < 10; i++) send.append(send_str.mid(i * 2, 2).toInt(&ok, 16));
    for (int i = 0; i < 10; i++) temp += QString::asprintf(" %02X", (uchar)send[i]);
    qDebug("%s", qPrintable(temp));
    NET_DVR_SerialSend(com_handle[0], 1, send.data(), 10);
    qDebug() << NET_DVR_GetLastError();
    ui->RELAY_SWITCH_1->setText("下电");
    ui->RELAY_SWITCH_2->setText("下电");
//    ui->RELAY_SWITCH_3->setText("O");
//    ui->RELAY_SWITCH_4->setText("O");
    relay_on[0] = relay_on[1] = relay_on[2] = relay_on[3] = true;
}

void Demo::on_RELAY_ALL_OFF_BTN_clicked()
{
    QString send_str = "020F0000000801FFFEC0", temp;
    bool ok;
    QByteArray send;
    for (int i = 0; i < 10; i++) send.append(send_str.mid(i * 2, 2).toInt(&ok, 16));
    for (int i = 0; i < 10; i++) temp += QString::asprintf(" %02X", (uchar)send[i]);
    qDebug("%s", qPrintable(temp));
    NET_DVR_SerialSend(com_handle[0], 1, send.data(), 10);
    ui->RELAY_SWITCH_1->setText("上电");
    ui->RELAY_SWITCH_2->setText("上电");
//    ui->RELAY_SWITCH_3->setText("X");
//    ui->RELAY_SWITCH_4->setText("X");
    relay_on[0] = relay_on[1] = relay_on[2] = relay_on[3] = false;
}

void Demo::on_WIPER_BTN_clicked()
{
    long long send = wiper_on ? 0x0D01000B0001FF : 0x0B0100090001FF;
    NET_DVR_SerialSend(com_handle[0], 1, (char*)&send, 7);
    QString send_str;
    for (int i = 0; i < 7; i++) send_str += QString::asprintf(" %02X", ((uchar*)&send)[i]);
    qDebug("%s", qPrintable(send_str));
    qDebug() << NET_DVR_GetLastError();
    wiper_on ^= 1;
    ui->WIPER_BTN->setText(wiper_on ? "雨刷关" : "雨刷开");
}

void Demo::keyPressEvent(QKeyEvent *event)
{
    static QLineEdit *edit;

    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (!(this->focusWidget())) break;
        edit = qobject_cast<QLineEdit*>(this->focusWidget());
        if (!edit) break;

        if (edit == ui->IP_EDIT_1) login_button_clicked(0);
        else if (edit == ui->IP_EDIT_2) login_button_clicked(1);
        else if (edit == ui->COM_EDIT) {
            if (com) com->close();
            setup_com(&com, edit->text());
        }
        else if (edit == ui->ANGLE_H_EDIT || edit == ui->ANGLE_V_EDIT) on_SET_ANGLE_BTN_clicked();
        this->focusWidget()->clearFocus();
        break;
    case Qt::Key_Escape:
        this->focusWidget()->clearFocus();
        break;
    default:
        break;
    }
}

void Demo::on_FPS_OPTION_1_currentIndexChanged(int index)
{
    int idx;
    switch (index * 5 + 10) {
    case 10: idx = 10; break;
    case 15: idx = 14; break;
    case 20: idx = 13; break;
    case 25: idx = 17; break;
    default: idx = -1; break;
    }

    DWORD size = 0;
    NET_DVR_GetDVRConfig(device_info[0].lLoginID, NET_DVR_GET_COMPRESSCFG_V30, 1, dev_config + 0, sizeof(NET_DVR_COMPRESSIONCFG_V30), &size);
    dev_config[0].struNormHighRecordPara.dwVideoFrameRate = idx;
    NET_DVR_SetDVRConfig(device_info[0].lLoginID, NET_DVR_SET_COMPRESSCFG_V30, 1, dev_config + 0, sizeof(NET_DVR_COMPRESSIONCFG_V30));
}

void Demo::on_FPS_OPTION_2_currentIndexChanged(int index)
{
    int idx;
    switch (index * 5 + 10) {
    case 10: idx = 10; break;
    case 15: idx = 14; break;
    case 20: idx = 13; break;
    case 25: idx = 17; break;
    default: idx = -1; break;
    }

    DWORD size = 0;
    NET_DVR_GetDVRConfig(device_info[1].lLoginID, NET_DVR_GET_COMPRESSCFG_V30, 1, dev_config + 1, sizeof(NET_DVR_COMPRESSIONCFG_V30), &size);
    dev_config[1].struNormHighRecordPara.dwVideoFrameRate = idx;
    NET_DVR_SetDVRConfig(device_info[1].lLoginID, NET_DVR_SET_COMPRESSCFG_V30, 1, dev_config + 1, sizeof(NET_DVR_COMPRESSIONCFG_V30));
}

void Demo::on_FILTER_CHK_stateChanged(int arg1)
{
    cb_mutex[0].lock();
    average = arg1;
    if (!average) sum.release();
    cb_mutex[0].unlock();
}

void Demo::on_DEHAZE_CHK_stateChanged(int arg1)
{
    dehaze = arg1;
//    QString send_str = dehaze ? "800104370201FF" : "800104370202FF", temp;
//    bool ok;
//    QByteArray send;
//    for (int i = 0; i < 7; i++) send.append(send_str.mid(i * 2, 2).toInt(&ok, 16));
//    for (int i = 0; i < 7; i++) temp += QString::asprintf(" %02X", (uchar)send[i]);
//    qDebug("%s", qPrintable(temp));
//    NET_DVR_SerialSend(com_handle[0], 1, send.data(), 7);
}

void Demo::on_ENHANCE_CHK_stateChanged(int arg1)
{
    enhance = arg1;
}

void Demo::on_INVERT_CHK_stateChanged(int arg1)
{
    invert = arg1;
}

void Demo::setup_com(QSerialPort **com, QString com_num)
{
    if (*com) delete *com;
    *com = new QSerialPort;
    (*com)->setPortName("COM" + com_num);
    qDebug("%p\n", com);

    if ((*com)->open(QIODevice::ReadWrite)) {
        qDebug("COM%s connected\n", qPrintable(com_num));
        ui->COM->setStyleSheet("color:#000000;");

        (*com)->setBaudRate(9600);
        (*com)->setDataBits(QSerialPort::Data8);
        (*com)->setParity(QSerialPort::NoParity);
        (*com)->setStopBits(QSerialPort::OneStop);
        (*com)->setFlowControl(QSerialPort::NoFlowControl);

        // send initial data
    }
    else {
        ui->COM->setStyleSheet("color:#FF0000;");
        delete *com;
        *com = NULL;
    }
}

void Demo::communicate_display()
{
    QString str_s("sent    "), str_r("received");

    for (int i = 0; i < 7; i++) str_s += QString::asprintf(" %02X", buffer_out[i]);
    if (com) com->write(QByteArray((char*)buffer_out, 7));

    if (com) {
        memset(buffer_in, 0, 7);
        QByteArray data, temp;
        in_len = 0;
        while(com->waitForReadyRead(100) && in_len < 7) {
            if((temp = com->readAll()).isEmpty()) continue;
            in_len += temp.size();
            data.append(temp);
        }
        memcpy(buffer_in, data, 7);
        for (int i = 0; i < in_len; i++) str_r += QString::asprintf(" %02X", buffer_in[i]);
    }

    QThread().msleep(10);
}

void Demo::stop()
{
    *(long long*)buffer_out = 0x010000000001FF;
    communicate_display();
}

uchar Demo::check_sum()
{
    int sum = 0;
    for (int i = 1; i < 6; i++) sum += buffer_out[i];
    return sum & 0xFF;
}

void Demo::ptz_control_button_pressed(int id) {
    qDebug("%dp\n", id);
    switch(id){
    case 0: send_ptz_cmd(0x08); send_ptz_cmd(0x04);  break;
    case 1: send_ptz_cmd(0x08);                      break;
    case 2: send_ptz_cmd(0x08); send_ptz_cmd(0x02);  break;
    case 3: send_ptz_cmd(0x04);                      break;
    case 4: *(long long*)buffer_out = 0x7F7700070001FF; communicate_display(); break;
    case 5: send_ptz_cmd(0x02);                      break;
    case 6: send_ptz_cmd(0x10); send_ptz_cmd(0x04);  break;
    case 7: send_ptz_cmd(0x10);                      break;
    case 8: send_ptz_cmd(0x10); send_ptz_cmd(0x02);  break;
    default:                                         break;
    }
}

void Demo::send_ptz_cmd(uchar dir)
{
    buffer_out[0] = 0xFF;
    buffer_out[1] = 0x01;
    buffer_out[2] = 0x00;
    buffer_out[3] = dir;
    buffer_out[4] = dir < 5 ? speed[2] : 0x00;
    buffer_out[5] = dir > 5 ? speed[2] : 0x00;
    buffer_out[6] = check_sum();

    communicate_display();
}

void Demo::move(float angle, bool vertical)
{
    int ang = angle * 100;
    if (ang < 0) ang += 36000;
    buffer_out[0] = 0xFF;
    buffer_out[1] = 0x01;
    buffer_out[2] = 0x00;
    buffer_out[3] = vertical ? 0x4D : 0x4B;
    buffer_out[4] = (ang >> 8) & 0xFF;
    buffer_out[5] = ang & 0xFF;
    buffer_out[6] = check_sum();

    communicate_display();
}

void Demo::read_command_file()
{
    QFile user_cmd("user_cmd");
    user_cmd.open(QIODevice::ReadOnly);
    if (user_cmd.isOpen()) {
        zoom_out   = user_cmd.readLine(17).simplified();
        zoom_in    = user_cmd.readLine(17).simplified();
        focus_far  = user_cmd.readLine(17).simplified();
        focus_near = user_cmd.readLine(17).simplified();
        lens_stop  = user_cmd.readLine(17).simplified();
//        qDebug() << zoom_in << zoom_out << focus_far << focus_near;
    }
    else {
        zoom_in    = "FF0B008000008B";
        zoom_out   = "FF0B010000000C";
        focus_near = "FF0B002000002B";
        focus_far  = "FF0B004000004B";
        lens_stop  = "FF0B000000000B";
    }

    user_cmd.close();
}

void Demo::ptz_control_button_released(int id) {
    if (id == 4) return;
    qDebug("%dr\n", id);
    stop();
}

void Demo::on_STOP_BTN_clicked()
{
    stop();
}

void Demo::on_SPEED_SLIDER_sliderMoved(int position)
{
    speed[2] = position;
    ui->SPEED_EDIT->setText(QString::asprintf("%d", speed[2]));
}

void Demo::on_SPEED_EDIT_textEdited(const QString &arg1)
{
    speed[2] = (char)arg1.toInt();
    if (speed[2] > 64) speed[2] = 64;
    if (speed[0] < 1) speed[2] = 1;
    ui->SPEED_SLIDER->setValue(speed[2]);
    ui->SPEED_EDIT->setText(QString::asprintf("%d", speed[2]));
}

void Demo::on_GET_ANGLE_BTN_clicked()
{
    *(long long*)buffer_out = 0x520000510001FF;
    communicate_display();
    angle_h = ((buffer_in[4] << 8) + buffer_in[5]) / 100.0;
    ui->ANGLE_H_EDIT->setText(QString::asprintf("%06.2f", angle_h));

    QThread::msleep(30);

    *(long long*)buffer_out = 0x540000530001FF;
    communicate_display();
    angle_v = ((buffer_in[4] << 8) + buffer_in[5]) / 100.0;
    ui->ANGLE_V_EDIT->setText(QString::asprintf("%05.2f", angle_v > 40 ? angle_v - 360 : angle_v));
}

void Demo::on_SET_ANGLE_BTN_clicked()
{
    angle_h = ui->ANGLE_H_EDIT->text().toFloat();
    move(angle_h, false);
    ui->ANGLE_H_EDIT->setText(QString::asprintf("%06.2f", angle_h));

    angle_v = ui->ANGLE_V_EDIT->text().toFloat();
    if (angle_v > 40) angle_v = 40;
    if (angle_v < -40) angle_v = -40;
    move(angle_v, true);
    ui->ANGLE_V_EDIT->setText(QString::asprintf("%05.2f", angle_v));
}

void Demo::point_ptz_to_target(QPoint target)
{
    if (!playing[1]) return;
    static int display_width, display_height;
    //TODO config params for max zoom
//    static float tot_h = 15, tot_v = 12;// large FOV
    static float tot_h = 2.8, tot_v = 2.2;//small FOV
    display_width = ui->DISPLAY_2->width();
    display_height = ui->DISPLAY_2->height();
    angle_h += target.x() * tot_h / display_width - tot_h / 2;
    angle_v += target.y() * tot_v / display_height - tot_v / 2;

    move(angle_h, false);
    QThread::msleep(30);
    move(angle_v, true);
}
