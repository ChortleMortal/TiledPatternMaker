#include <QPainter>
#include <QDebug>
#include <QMenu>

#include "gui/viewers/image_view.h"
#include "gui/top/system_view_controller.h"

#ifdef __linux__
#include "model/settings/configuration.h"
#include "gui/widgets/image_widget.h"
#endif

// An ImageViewer (based on QObject) takes a QPixmap and is added to the View
ImageViewer::ImageViewer() : LayerController(VIEW_BMP_IMAGE,PRIMARY,"Image Viewer")
{
    setZLevel(IMAGE_ZLEVEL);
    connect(this, &ImageViewer::sig_reconstructView, viewControl(),    &SystemViewController::slot_reconstructView);
}

void ImageViewer::load(QPixmap & pm)
{
#ifdef __linux__
    pixmap = ImageWidget::removeAlphaChannel(pm);
#else
    pixmap = pm;
#endif
}

void ImageViewer::unloadLayerContent()
{
    pixmap = QPixmap();
    Xform xform;
    setModelXform(xform,true,Sys::nextSigid());
}

void ImageViewer::paint(QPainter *painter)
{
    if (pixmap.isNull())
        return;

    painter->save();
    
    const Xform & xf = getModelXform();
    painter->setTransform(xf.getTransform());

#ifdef __linux__
    if (!Sys::config->compare_transparent)
    {
        painter->fillRect(pixmap.rect(),Qt::black);
    }
#endif
    painter->drawPixmap(0,0,pixmap);

    painter->restore();
}

void ImageViewer::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    Q_UNUSED(spt);
    Q_UNUSED(btn);
}

void ImageViewer::slot_mouseDragged(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageViewer::slot_mouseMoved(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageViewer::slot_mouseReleased(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageViewer::slot_mouseDoublePressed(QPointF spt)
{
    Q_UNUSED(spt);
}




