#include <QPainter>
#include <QDebug>
#include <QMenu>
#include "widgets/image_layer.h"
#include "viewers/view_controller.h"

// An ImageLayerView (based on QObject) takes a QPixmap and is added to the View
ImageLayerView::ImageLayerView(QString name) : LayerController("Image",true)
{
    title = name;
    connect(this, &ImageLayerView::sig_refreshView, viewControl, &ViewController::slot_reconstructView);
}

void ImageLayerView::paint(QPainter *painter)
{
    if (pixmap.isNull())
        return;

    painter->save();
    
    const Xform & xf = getModelXform();
    painter->setTransform(xf.getTransform());
    QRectF rect = pixmap.rect();
    painter->drawPixmap(rect,pixmap,rect);

    painter->setPen(Qt::blue);
    painter->setFont(QFont("Arial", 20));
    painter->drawText(QPointF(10,30), title);

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
        QMenu menu(view);
        menu.addAction("Delete",this,&ImageLayerView::slot_deleteAction);
        menu.exec(view->mapToGlobal(spt.toPoint()));
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
    emit sig_refreshView();
}



