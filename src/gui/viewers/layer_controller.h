#pragma once
#ifndef TPM_LAYER_CONTROLLER_H
#define TPM_LAYER_CONTROLLER_H

#include <QPen>
#include "gui/viewers/layer.h"

typedef std::shared_ptr<class LayerController> LayerCtrlPtr;

class LayerController : public Layer
{
    Q_OBJECT

public:
    LayerController(eViewType viewType,eModelType modelType, QString name);
    LayerController(const LayerController & other);
    LayerController(LayerCtrlPtr other);
    virtual ~LayerController();

    virtual void iamaLayerController() = 0;

public slots:
    virtual void slot_mousePressed(QPointF spt, Qt::MouseButton btn) = 0;
    virtual void slot_mouseDragged(QPointF spt)       = 0;
    virtual void slot_mouseMoved(QPointF spt)         = 0;
    virtual void slot_mouseReleased(QPointF spt)      = 0;
    virtual void slot_mouseDoublePressed(QPointF spt) = 0;

    virtual void slot_mouseTranslate(uint sigid, QPointF pt);
    virtual void slot_wheel_scale(uint sigid, qreal delta);
    virtual void slot_wheel_rotate(uint sigid, qreal delta);

    virtual void slot_scale(uint sigid, int amount);
    virtual void slot_rotate(uint sigid, int amount);
    virtual void slot_moveX(uint sigid, qreal amount);
    virtual void slot_moveY(uint sigid, qreal amount);

    void slot_breakaway(Layer * l, bool set);
    void slot_lock(Layer * l, bool set);
    void slot_solo(Layer * l, bool set);

protected:
    bool validateSignal();
    void connectSignals();

private:
};

#endif
