#pragma once
#ifndef CROP_VIEW_H
#define CROP_VIEW_H

#include "gui/viewers/layer_controller.h"
#include "model/makers/crop_maker.h"

typedef std::shared_ptr<class MouseEditCrop> CropMouseActionPtr;

class CropMaker;

class CropViewer : public LayerController
{
public:
    CropViewer();
    ~CropViewer() {}

    void        aquire(CropMaker * ed, eCropMaker maker);
    void        release(eCropMaker maker);

    CropMaker * getMaker()          { return cropMaker; }
    eCropMaker  getMakerType()      { return makerType; }

    void        setShowCrop(eCropMaker maker, bool state);
    bool        getShowCrop(eCropMaker maker);

    void        paint(QPainter * painter) override;
    void        draw(QPainter *, QTransform t);

    const Xform &   getModelXform() override;
    void            setModelXform(const Xform & xf, bool update) override;

public slots:
    void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseTranslate(QPointF pt)      override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;
    void slot_setCenter(QPointF spt) override;

    void slot_wheel_scale(qreal delta)  override;
    void slot_wheel_rotate(qreal delta) override;

    void slot_scale(int amount)  override;
    void slot_rotate(int amount) override;
    void slot_moveX(qreal amount)  override;
    void slot_moveY(qreal amount)  override;

    eViewType iamaLayer() override { return VIEW_CROP; }
    void      iamaLayerController() override {}

    void    setMousePos(QPointF pt);
    QPointF getMousePos() { return mousePos; }

protected:

private:
    CropMaker         * cropMaker;

    CropMouseActionPtr mouseInteraction;   // used by menu
    QPointF            mousePos;            // used by menu
    bool               debugMouse;
    bool               showCrop;
    eCropMaker         makerType;
};

#endif
