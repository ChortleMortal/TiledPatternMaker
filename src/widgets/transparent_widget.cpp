#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QSettings>

#include "widgets/transparent_widget.h"
#include "tiledpatternmaker.h"

extern class TiledPatternMaker * theApp;

/////////////////////////////////////////////
///
/// Transparent Widget
///
/////////////////////////////////////////////

TransparentImageWidget::TransparentImageWidget(QString name)
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

    // also
    setAttribute(Qt::WA_DeleteOnClose);
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
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
        oldPos = event->globalPos();
#else
        oldPos = event->globalPosition().toPoint();
#endif
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

#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    QPoint delta = event->globalPos() - oldPos;
    move(x() + delta.x(), y() + delta.y());
    oldPos = event->globalPos();
#else
    QPoint delta = event->globalPosition().toPoint() - oldPos;
    move(x() + delta.x(), y() + delta.y());
    oldPos = event->globalPosition().toPoint();
#endif

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




