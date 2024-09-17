#include <QPainter>
#include <QDebug>
#include <QMenu>
#include "model/settings/configuration.h"
#include "gui/widgets/image_layer.h"
#include "gui/top/view_controller.h"
#ifdef __linux__
#include "gui/widgets/image_widget.h"
#endif

// An ImageLayerView (based on QObject) takes a QPixmap and is added to the View
ImageLayerView::ImageLayerView(QString name) : LayerController("Image",true)
{
    title = name;
    setZValue(1);
    connect(this, &ImageLayerView::sig_reconstructView, viewControl, &ViewController::slot_reconstructView);
}

void ImageLayerView::setPixmap(QPixmap & pm)
{
#ifdef __linux__
    pixmap = ImageWidget::removeAlphaChannel(pm);
#else
       pixmap = pm;
#endif
}

void ImageLayerView::paint(QPainter *painter)
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

void ImageLayerView::setModelXform(const Xform & xf, bool update)
{
    Q_ASSERT(_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << xf.info() << (isUnique() ? "unique" : "common");
    xf_model = xf;
    forceLayerRecalc(update);
}

const Xform & ImageLayerView::getModelXform()
{
    Q_ASSERT(_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "GET" << getLayerName() << xf_model.info() << (isUnique() ? "unique" : "common");
    return xf_model;
}

void ImageLayerView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    if (btn == Qt::RightButton)
    {
        qDebug() << "ImageLayer -right click";
        QMenu menu(Sys::view);
        menu.addAction("Delete",this,&ImageLayerView::slot_deleteAction);
        menu.exec(Sys::view->mapToGlobal(spt.toPoint()));
    }
}

void ImageLayerView::slot_mouseDragged(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageLayerView::slot_mouseMoved(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageLayerView::slot_mouseReleased(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageLayerView::slot_mouseDoublePressed(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageLayerView::slot_deleteAction()
{
    viewControl->removeImage(this);
    emit sig_reconstructView();
}



