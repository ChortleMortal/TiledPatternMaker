#include <QPainter>
#include <QDebug>
#include <QMenu>
#include "widgets/image_layer.h"
#include "viewers/viewcontrol.h"

// An ImageLayer (based on QObject) takes a QPixmap and is added to the View
ImageLayer::ImageLayer(QString name) : LayerController("Image")
{
    title = name;
    view  = ViewControl::getInstance();
    connect(this, &ImageLayer::sig_refreshView, view, &ViewControl::slot_refreshView);
}

void ImageLayer::paint(QPainter *painter)
{
    if (pixmap.isNull())
        return;

    painter->save();

    const Xform & xf = getCanvasXform();
    painter->setTransform(xf.getTransform());
    QRectF rect = pixmap.rect();
    painter->drawPixmap(rect,pixmap,rect);

    painter->setPen(Qt::blue);
    painter->setFont(QFont("Arial", 20));
    painter->drawText(QPointF(10,30), title);

    painter->restore();
}

void ImageLayer::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    if (btn == Qt::RightButton)
    {
        qDebug() << "ImageLayer -right click";
        QMenu menu(view);
        menu.addAction("Delete",this,&ImageLayer::slot_deleteAction);
        menu.exec(view->mapToGlobal(spt.toPoint()));
    }
}

void ImageLayer::slot_mouseDragged(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageLayer::slot_mouseMoved(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageLayer::slot_mouseReleased(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageLayer::slot_mouseDoublePressed(QPointF spt)
{
    Q_UNUSED(spt);
}

void ImageLayer::slot_deleteAction()
{
    view->removeImage(this);
    emit sig_refreshView();
}



