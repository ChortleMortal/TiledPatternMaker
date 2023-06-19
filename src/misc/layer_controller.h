#pragma once
#ifndef TPM_LAYER_CONTROLLER_H
#define TPM_LAYER_CONTROLLER_H

#include <QPen>
#include "misc/layer.h"

typedef std::shared_ptr<class Layer> LayerPtr;
typedef std::shared_ptr<class LayerController> LayerCtrlPtr;

class LayerController : public Layer
{
    Q_OBJECT

public:
    LayerController(QString name);
    LayerController(const LayerController & other);
    LayerController(LayerCtrlPtr other);
    virtual ~LayerController();

    virtual void iamaLayerController() = 0;

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) = 0;
    virtual void slot_mouseDragged(QPointF spt)       = 0;
    virtual void slot_mouseMoved(QPointF spt)         = 0;
    virtual void slot_mouseReleased(QPointF spt)      = 0;
    virtual void slot_mouseDoublePressed(QPointF spt) = 0;

    virtual void slot_setCenter(QPointF spt);

    virtual void slot_mouseTranslate(QPointF pt);
    virtual void slot_wheel_scale(qreal delta);
    virtual void slot_wheel_rotate(qreal delta);

    virtual void slot_scale(int amount);
    virtual void slot_rotate(int amount);
    virtual void slot_moveX(int amount);
    virtual void slot_moveY(int amount);

protected:
    void connectSignals();
};


#endif
