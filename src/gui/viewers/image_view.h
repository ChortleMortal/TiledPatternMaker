#pragma once
#ifndef IMAGE_VIEWER_H
#define IMAGE_VIEWER_H

#include <QMouseEvent>
#include "gui/viewers/layer_controller.h"

// An ImageViewer (based on QObject) takes a QPixmap and is added to the View
class ImageViewer : public LayerController
{
    Q_OBJECT

public:
    ImageViewer();

    void load(QPixmap & pm);
    void unloadLayerContent() override;
    bool isLoaded() { return !pixmap.isNull(); }

    const QPixmap & getPixmap() { return pixmap; }

    QSize  getSize() { return pixmap.size(); }

    void paint(QPainter *painter) override;

    void iamaLayerController() override {}

signals:
    void sig_keyPressed(QKeyEvent *);

 public slots:
    void slot_mousePressed(QPointF spt, Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;

protected:

private:
    QPixmap pixmap;
};

#endif // IMAGE_LAYER_H
