#pragma once
#ifndef IMAGE_LAYER_H
#define IMAGE_LAYER_H

#include <QMouseEvent>
#include "misc/layer_controller.h"

// An ImageLayerView (based on QObject) takes a QPixmap and is added to the View
class ImageLayerView : public LayerController
{
    Q_OBJECT

public:
    ImageLayerView(QString name);

    void setPixmap(QPixmap & pm) { this->pixmap = pm; }

    void paint(QPainter *painter) override;

    const Xform &   getModelXform() override;
    void            setModelXform(const Xform & xf, bool update) override;

    eViewType iamaLayer() override { return VIEW_IMAGE; }
    void iamaLayerController() override {}

 public slots:
    void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;

private slots:
    void slot_deleteAction();

protected:

private:
    QPixmap pixmap;
    QString title;
};

#endif // IMAGE_LAYER_H
