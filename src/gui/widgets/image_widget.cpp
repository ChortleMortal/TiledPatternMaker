#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QSettings>

#include "gui/widgets/image_widget.h"
#include "gui/top/system_view_controller.h"
#include "sys/engine/image_engine.h"
#include "sys/sys.h"
#include "sys/tiledpatternmaker.h"

extern  TiledPatternMaker * theApp;

/////////////////////////////////////////////
///
/// Image Widget
///
/////////////////////////////////////////////

ImageWidget::ImageWidget()
{
    setMouseTracking(true);
    setMaximumSize(Sys::MAX_WIDTH, Sys::MAX_HEIGHT);
    setScaledContents(false);

    connect(theApp, &TiledPatternMaker::sig_imageToPrimary, this, &ImageWidget::slot_moveToPrimary,Qt::QueuedConnection);
}

ImageWidget::~ImageWidget()
{}

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

void ImageWidget::closeEvent(QCloseEvent * event)
{
    QSettings s;
    s.setValue("imageWidgetPos",pos());

    QLabel::closeEvent(event);

    emit sig_closed(this);
}

void ImageWidget::setContentSize(QSize sz)
{
    resize(sz);
    update();
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
    pt = event->position().toPoint();

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

void ImageWidget::slot_moveToPrimary()
{
    QScreen * primary = qApp->primaryScreen();

    QWindow * wh = Sys::viewController->windowHandle();
    if (!wh)
    {
        return;
    }
    move(primary->geometry().center() - Sys::viewController->viewRect().center());
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    raise();  // for MacOS
    activateWindow();  // for windows
}

void ImageWidget::setPixmap(const QPixmap & pixmap)
{
    _pixmap = pixmap;
    QLabel::setPixmap(_pixmap);
}

void ImageWidget::setPixmap(const QPixmap & pixmap, qreal scale)
{
    _pixmap = pixmap;
    QLabel::setPixmap(pixmap.scaled((pixmap.size() * scale),Qt::KeepAspectRatio ,Qt::SmoothTransformation));
}


