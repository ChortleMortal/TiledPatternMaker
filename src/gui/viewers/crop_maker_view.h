#pragma once
#ifndef CROP_VIEW_H
#define CROP_VIEW_H

#include "gui/viewers/layer_controller.h"
#include "model/makers/crop_maker.h"

typedef std::shared_ptr<class MouseEditCrop> CropMouseActionPtr;

class CropMaker;

class CropMakerView : public LayerController
{
    Q_OBJECT

public:
    CropMakerView();
    ~CropMakerView() {}

    void        aquire(CropMaker * ed, eCropMaker maker);
    void        release(eCropMaker maker);
    void        unloadLayerContent() override   { release(makerType);}

    CropMaker * getMaker()          { return cropMaker; }
    eCropMaker  getMakerType()      { return makerType; }

    void        setShowCrop(eCropMaker maker, bool state);
    bool        getShowCrop(eCropMaker maker);

    void        paint(QPainter * painter) override;
    void        draw(QPainter *, QTransform t);

    virtual void  iamaLayerController() override {}

    virtual QTransform getLayerTransform() override;
    virtual QTransform getCanvasTransform() override;
    virtual QTransform getModelTransform() override;
    virtual const Xform &   getModelXform() override;

signals:
    void    sig_delegateView(eViewType vt, bool delegate);

public slots:
    void slot_mousePressed(QPointF spt, Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;

    void    setMousePos(QPointF pt);
    QPointF getMousePos() { return mousePos; }

protected:

private:
    CropMaker         * cropMaker;

    CropMouseActionPtr mouseInteraction;   // used by menu
    QPointF            mousePos;            // used by menu
    bool               debugMouse;
    eCropMaker         makerType;
    const Xform        unityXf;
};

#endif
