#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QSettings>

#include "widgets/image_widget.h"
#include "engine/image_engine.h"
#include "misc/sys.h"

/////////////////////////////////////////////
///
/// Image Widget
///
/////////////////////////////////////////////

ImageWidget::ImageWidget()
{
    setMouseTracking(true);
    setMaximumSize(Sys::MAX_WIDTH, Sys::MAX_HEIGHT);
    setAttribute(Qt::WA_DeleteOnClose);
}

ImageWidget::~ImageWidget()
{
    QSettings s;
    s.setValue("imageWidgetPos",pos());
}

void ImageWidget::keyPressEvent(QKeyEvent * k)
{
    if (k->key() == 'Q')
    {
        close();
    }
    else
    {
        emit sig_keyPressed(k);
    }
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
    Sys::imageEngine->sig_colorPick(col);
}

void ImageWidget::slot_closeMe()
{
    //qDebug() << "ImageWidget::slot_closeMe";
    QTimer::singleShot(500, this, &ImageWidget::close);
}






