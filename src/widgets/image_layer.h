#pragma once
#ifndef IMAGE_LAYER_H
#define IMAGE_LAYER_H

#include <QMouseEvent>
#include "misc/layer_controller.h"

// An ImageLayer (based on QObject) takes a QPixmap and is added to the View
class ImageLayer : public LayerController
{
    Q_OBJECT

public:
    ImageLayer(QString name);

    void setPixmap(QPixmap & pm) { this->pixmap = pm; }

    void paint(QPainter *painter) override;

    virtual void iamaLayer() override {}
    virtual void iamaLayerController() override {}

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;

    virtual const Xform  & getCanvasXform() override { return xf; }
    virtual void           setCanvasXform(const Xform & xf) override { this->xf = xf; }

private slots:
    void slot_deleteAction();

protected:
    class ViewControl   * view;

private:
    QPixmap pixmap;
    Xform   xf;
    QString title;
};

#endif // IMAGE_LAYER_H
