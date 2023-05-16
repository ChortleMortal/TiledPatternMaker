#pragma once
#ifndef CROP_VIEW_H
#define CROP_VIEW_H

#include "misc/layer_controller.h"

typedef std::shared_ptr<class CropView>   CropViewPtr;
typedef std::shared_ptr<class MouseEditCrop> CropMouseActionPtr;

class CropMaker;

class CropView : public LayerController
{
public:
    static CropViewPtr getSharedInstance();

    CropView();  // dont use this
    ~CropView() {}

    void          init(CropMaker * ed);

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
    void                setMouseInteraction(CropMouseActionPtr action) { mouse_interaction = action; }
    void                resetMouseInteraction() { mouse_interaction.reset(); }
    CropMouseActionPtr  getMouseInteraction() { return mouse_interaction; }

private:
    static CropViewPtr  spThis;
    QPointF             mousePos;             // used by menu
    bool                debugMouse;

    CropMouseActionPtr  mouse_interaction;    // used by menu

    CropMaker          * cropMaker;
    Configuration      * config;
};

#endif
