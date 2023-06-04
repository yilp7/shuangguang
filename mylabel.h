#ifndef MYLABEL_H
#define MYLABEL_H

#include <QMainWindow>
#include <QLabel>
#include <QMouseEvent>
#include "opencv2/core.hpp"

class MyLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MyLabel(QWidget *parent = 0);

public:
    void update_roi(QPoint pos);

private:
//  @override
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

signals:
    void set_pixmap(QPixmap pm);

    void ptz_target(QPoint target);

public:
    QPoint   lefttop;
    QPoint   center;
    QPoint   prev_pos;
    bool     grab;
    int      curr_scale;
    float    scale[5];
    cv::Rect rect_params;
    QRect    reserved_geometry;

};

#endif // MYLABEL_H
