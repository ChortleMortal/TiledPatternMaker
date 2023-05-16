#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QSettings>

#include "widgets/transparentwidget.h"
#include "tiledpatternmaker.h"
#include "misc/defaults.h"

extern class TiledPatternMaker * theApp;

/////////////////////////////////////////////
///
/// Image Widget
///
/////////////////////////////////////////////

ImageWidget::ImageWidget()
{
    setMouseTracking(true);
    setMaximumSize(MAX_WIDTH,MAX_HEIGHT);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(theApp, &TiledPatternMaker::sig_closeAllImageViewers,this, &ImageWidget::slot_closeMe);
}

ImageWidget::~ImageWidget()
{
    QSettings s;
    s.setValue("imageWidgetPos",pos());
}

void ImageWidget::keyPressEvent( QKeyEvent *k )
{
    theApp->imageKeyPressed(k);
}

void ImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    QSettings s;
    s.setValue("imageWidgetPos",pos());
}

void ImageWidget::mousePressEvent(QMouseEvent *event)
{
    QPoint pt;
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    pt = event->pos();
#else
    pt = event->position().toPoint();
#endif

    QPixmap pm = grab(QRect(pt,QSize(1,1)));
    QImage img = pm.toImage();
    QColor col = QColor(img.pixel(0,0));
    qDebug() << "color=" << col;
    emit theApp->sig_colorPick(col);
}

void ImageWidget::slot_closeMe()
{
    //qDebug() << "ImageWidget::slot_closeMe";
    QTimer::singleShot(500, this, &ImageWidget::close);
}

/////////////////////////////////////////////
///
/// Transparent Widget
///
/////////////////////////////////////////////

TransparentWidget::TransparentWidget(QString name)
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

void TransparentWidget::paintEvent(QPaintEvent * event)
{
    QLabel::paintEvent(event);

    QPainter painter(this);
    painter.setPen(Qt::blue);
    painter.setFont(QFont("Arial", 20));
    painter.drawText(QPointF(10,30), title);
}

void TransparentWidget::mousePressEvent(QMouseEvent *event)
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

void TransparentWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    dragging = false;
    oldPos   = QPoint();
}

void TransparentWidget::mouseMoveEvent(QMouseEvent *event)
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

void TransparentWidget::keyPressEvent( QKeyEvent *k )
{
    int key = k->key();
    switch (key)
    {
    case 'Q':
        close();    // deletes
        break;
    case 'T':
        onTop = !onTop;
        setWindowFlag(Qt::WindowStaysOnTopHint,onTop);
        break;
    default:
        break;
    }
}




