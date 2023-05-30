#pragma once
#ifndef CROP_VIEW_H
#define CROP_VIEW_H

#include "misc/layer_controller.h"

typedef std::shared_ptr<class MouseEditCrop> CropMouseActionPtr;

class CropMaker;

class CropViewer : public LayerController
{
public:
    static CropViewer * getInstance();
    static void         releaseInstance();

    void    init(CropMaker * ed);
    CropMaker * getMaker() { return cropMaker; }

    void    setShowCrop(bool state) { _showCrop = state; }
    bool    getShowCrop()           { return _showCrop; }

    virtual void  paint(QPainter * painter) override;
    virtual void  draw(QPainter *, QTransform t);

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseTranslate(QPointF pt)      override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;
    virtual void slot_setCenter(QPointF spt) override;

    virtual void slot_wheel_scale(qreal delta)  override;
    virtual void slot_wheel_rotate(qreal delta) override;

    virtual void slot_scale(int amount)  override;
    virtual void slot_rotate(int amount) override;
    virtual void slot_moveX(int amount)  override;
    virtual void slot_moveY(int amount)  override;

    virtual void iamaLayer() override {}
    virtual void iamaLayerController() override {}

    void    setMousePos(QPointF pt);
    QPointF getMousePos() { return mousePos; }

protected:

private:
    CropViewer();
    ~CropViewer() {}

    static CropViewer * mpThis;
    CropMaker         * cropMaker;
    Configuration     * config;

    CropMouseActionPtr mouseInteraction;   // used by menu
    QPointF            mousePos;            // used by menu
    bool               debugMouse;
    bool                _showCrop;

};

#endif
