#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QSettings>

#include "gui/widgets/image_widget.h"
#include "sys/engine/image_engine.h"
#include "sys/sys.h"

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

QPixmap ImageWidget::removeAlphaChannel(const QPixmap &src)
{
    // From ChatGPT : Create a new QPixmap with the same size as the source, but without an alpha channel
    QPixmap opaquePixmap(src.size());
    opaquePixmap.fill(Qt::white); // Fill with an opaque color (white)

    // Create a QPainter to paint the original pixmap onto the new opaque pixmap
    QPainter painter(&opaquePixmap);
    painter.drawPixmap(0, 0, src);
    painter.end();

    return opaquePixmap;
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
    emit Sys::imageEngine->sig_colorPick(col);
}

void ImageWidget::slot_closeMe()
{
    //qDebug() << "ImageWidget::slot_closeMe";
    QTimer::singleShot(500, this, &ImageWidget::close);
}






