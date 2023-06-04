#include "mylabel.h"
#include "qdebug.h"

MyLabel::MyLabel(QWidget *parent) : QLabel(parent)
  , lefttop(0, 0)
  , center(0, 0)
  , grab(false)
  , curr_scale(0)
//  , scale{ QSize(640, 512), QSize(480, 384), QSize(320, 256), QSize(160, 128), QSize(80, 64)}
//  , scale{ QSize(640, 400), QSize(480, 300), QSize(320, 200), QSize(160, 100), QSize(80, 50)}
  , scale{ 1.0f, 2.0f / 3, 1.0f / 2, 1.0f / 4, 1.0f / 8}
{
    reserved_geometry = geometry();
}

void MyLabel::update_roi(QPoint center)
{
    QPoint temp(scale[curr_scale] * this->width(), scale[curr_scale] * this->height());
    lefttop = center - temp / 2;
    if (lefttop.x() < 0) lefttop.setX(0);
    if (lefttop.y() < 0) lefttop.setY(0);
    if (lefttop.x() > this->width() - temp.x()) lefttop.setX(this->width() - temp.x());
    if (lefttop.y() > this->height() - temp.y()) lefttop.setY(this->height() - temp.y());

    rect_params.x = lefttop.x();
    rect_params.y = lefttop.y();
    rect_params.width = scale[curr_scale] * this->width();
    rect_params.height = scale[curr_scale] * this->height();
    this->center = lefttop + temp / 2;
}

void MyLabel::mousePressEvent(QMouseEvent *event)
{
    QLabel::mousePressEvent(event);

    if(event->button() != Qt::LeftButton) return;

//    qDebug("pos: %d, %d\n", event->x(), event->y());
//    qDebug("pos: %d, %d\n", event->globalX(), event->globalY());
//    qDebug("%s pressed\n", qPrintable(this->objectName()));

    if (grab) prev_pos = event->pos();
}

void MyLabel::mouseMoveEvent(QMouseEvent *event)
{
    QLabel::mouseMoveEvent(event);

    if (!grab) return;
//    qDebug("pos: %d, %d\n", event->x(), event->y());
//    qDebug("pos: %d, %d\n", event->globalX(), event->globalY());
//    qDebug("%s selecting\n", qPrintable(this->objectName()));

    update_roi(lefttop + QPoint((int)(scale[curr_scale] * this->width()), (int)(scale[curr_scale] * this->height())) / 2 - (event->pos() - prev_pos));
    prev_pos = event->pos();
}

void MyLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->modifiers() == Qt::AltModifier) emit ptz_target(event->pos());
}

void MyLabel::wheelEvent(QWheelEvent *event)
{
    QLabel::wheelEvent(event);

    if (!grab) return;
    QPoint center = lefttop + QPoint((int)(scale[curr_scale] * this->width() / 2), (int)(scale[curr_scale] * this->height() / 2));
    QLabel::wheelEvent(event);
    if(event->delta() > 0) {
        if (curr_scale >= 4) return;
        curr_scale++;
    }
    else {
        if (curr_scale <= 0) return;
        curr_scale--;
    }

//    if (curr_scale > 4) curr_scale = 4;
//    if (curr_scale < 0) curr_scale = 0;
    qDebug("current scale: %dx%d", (int)(scale[curr_scale] * this->width()), (int)(scale[curr_scale] * this->height()));

    update_roi(center);
}

void MyLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    QLabel::mouseDoubleClickEvent(event);

    static uint i = 0;
    if (i++ % 2 == 0) {
        reserved_geometry = geometry();
        setWindowFlags(Qt::Dialog);
        showFullScreen();
    }
    else {
        setGeometry(reserved_geometry);
        setWindowFlags(Qt::Widget);
        showNormal();
    };
}

