#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QSettings>

#include "gui/widgets/transparent_widget.h"

/////////////////////////////////////////////
///
/// Transparent Widget
///
/////////////////////////////////////////////

TransparentImageWidget::TransparentImageWidget(QString name) :  ImageWidget()
{
    title    = name;
    dragging = false;

    // this makes it transparent
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlag(Qt::FramelessWindowHint);
    onTop = true;

    // this gives some MapLayer handle to grab on to
    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(40);
    setStyleSheet("color: rgba(255,0,0,64);");
}

void TransparentImageWidget::setContentSize(QSize sz)
{
    int w = lineWidth();
    int frameWidth  = frameGeometry().width()  - geometry().width()  + (w * 2);
    int frameHeight = frameGeometry().height() - geometry().height() + (w * 2);
    resize(sz.width() + frameWidth, sz.height() + frameHeight);
    update();
}

void TransparentImageWidget::paintEvent(QPaintEvent * event)
{
    QLabel::paintEvent(event);

    QPainter painter(this);
    painter.setPen(Qt::blue);
    painter.setFont(QFont("Arial", 20));
    painter.drawText(QPointF(10,30), title);
}

void TransparentImageWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        oldPos = event->globalPosition().toPoint();
        dragging = true;
    }
    else if (event->button() == Qt::RightButton)
    {
        close();    // deletes
    }
}

void TransparentImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    dragging = false;
    oldPos   = QPoint();
}

void TransparentImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!dragging)
    {
        return;
    }

    QPoint delta = event->globalPosition().toPoint() - oldPos;
    move(x() + delta.x(), y() + delta.y());
    oldPos = event->globalPosition().toPoint();

    ImageWidget::mouseMoveEvent(event);
}

void TransparentImageWidget::keyPressEvent( QKeyEvent *k )
{
    if (k->key() == 'T')
    {
        onTop = !onTop;
        setWindowFlag(Qt::WindowStaysOnTopHint,onTop);
    }
    else
    {
        ImageWidget::keyPressEvent(k);
    }
}




