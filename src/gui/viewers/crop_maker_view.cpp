#include <QApplication>

#include "gui/model_editors/crop_edit/mouse_edit_crop.h"
#include "gui/top/controlpanel.h"
#include "gui/viewers/crop_maker_view.h"
#include "gui/viewers/map_editor_view.h"
#include "model/makers/crop_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_manager.h"
#include "model/styles/style.h"
#include "sys/geometry/crop.h"

using std::make_shared;

CropMakerView::CropMakerView() : LayerController(VIEW_CROP,PRIMARY,"Crop Maker")
{
    debugMouse  = false;
    cropMaker   = nullptr;
    makerType   = CM_UNDEFINED;
    setZLevel(CROP_VIEW_ZLEVEL);
    connect(this, &CropMakerView::sig_delegateView, Sys::controlPanel, &ControlPanel::slot_delegateView);
}

QTransform CropMakerView::getLayerTransform()
{
    QTransform t;
    switch (makerType)
    {
    case CM_UNDEFINED:
        break;

    case CM_MOSAIC:
    case CM_PAINTER:
    {
        auto mosaic = Sys::mosaicMaker->getMosaic();
        auto style  = mosaic->getFirstRegularStyle();
        t           = style->getLayerTransform();
    }   break;

    case CM_MAPED:
        t = Sys::mapEditorView->getLayerTransform();
        break;
    }
    return t;
}

QTransform CropMakerView::getCanvasTransform()
{
    QTransform t;
    switch (makerType)
    {
    case CM_UNDEFINED:
        break;

    case CM_MOSAIC:
    case CM_PAINTER:
    {
        auto mosaic = Sys::mosaicMaker->getMosaic();
        auto style  = mosaic->getFirstRegularStyle();
        t           = style->getCanvasTransform();
    }   break;

    case CM_MAPED:
        t = Sys::mapEditorView->getCanvasTransform();
        break;
    }
    return t;
}

QTransform CropMakerView::getModelTransform()
{
    QTransform t;
    switch (makerType)
    {
    case CM_UNDEFINED:
        break;

    case CM_MOSAIC:
    case CM_PAINTER:
    {
        auto mosaic = Sys::mosaicMaker->getMosaic();
        auto style  = mosaic->getFirstRegularStyle();
        t           = style->getModelTransform();
    }   break;

    case CM_MAPED:
        t = Sys::mapEditorView->getModelTransform();
        break;
    }
    return t;
}

const Xform & CropMakerView::getModelXform()
{
    switch (makerType)
    {
    case CM_UNDEFINED:
        break;

    case CM_MOSAIC:
    case CM_PAINTER:
    {
        auto mosaic = Sys::mosaicMaker->getMosaic();
        auto style  = mosaic->getFirstRegularStyle();
        return style->getModelXform();
    }   break;

    case CM_MAPED:
        return Sys::mapEditorView->getModelXform();
        break;
    }
    return unityXf;
}

void CropMakerView::aquire(CropMaker *ed, eCropMaker maker)
{
    // aquire always grabs the viewer
    cropMaker  = ed;
    makerType  = maker;
}

void CropMakerView::release(eCropMaker maker)
{
    if (maker == makerType)
    {
        cropMaker  = nullptr;
        makerType  = CM_UNDEFINED;
    }
}

void CropMakerView::setShowCrop(eCropMaker maker, bool state)
{
    if (maker == makerType)
    {
        emit sig_delegateView(VIEW_CROP,state);     // thread safe
    }
}

bool CropMakerView::getShowCrop(eCropMaker maker)
{
    if (maker == makerType)
    {
        return Sys::viewController->isEnabled(VIEW_CROP);
    }
    else
    {
        return false;
    }
}

void CropMakerView::paint(QPainter *painter)
{
    if (makerType != CM_UNDEFINED)
    {
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        // draw
        draw(painter,getLayerTransform());
    }
}

void CropMakerView::draw(QPainter *painter , QTransform t)
{
    if (mouseInteraction)
    {
        mouseInteraction->draw(painter,mousePos,t);
    }
    else if (cropMaker)
    {
        CropPtr crop = cropMaker->getCrop();
        if (crop)
        {
            crop->draw(painter,t,false);
        }
    }
}

//////////////////////////////////////////////////////////////////
///
/// slots and evzents
///
//////////////////////////////////////////////////////////////////

void CropMakerView::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    if (!viewControl()->isEnabled(VIEW_CROP))
        return;

    if (!cropMaker)
        return;

    auto crop = cropMaker->getCrop();
    if (!crop)
        return;

    setMousePos(spt);

    auto t = getLayerTransform();
    mouseInteraction = make_shared<MouseEditCrop>(mousePos,crop.get(),t);
    mouseInteraction->updateDragging(mousePos,t);
}

void CropMakerView::slot_mouseDragged(QPointF spt)
{
    if (!viewControl()->isEnabled(VIEW_CROP)) return;

    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << mousePos << screenToModel(mousePos);

    if (mouseInteraction)
    {
        mouseInteraction->updateDragging(mousePos,getLayerTransform());
    }

    forceRedraw();
}

void CropMakerView::slot_mouseMoved(QPointF spt)
{
    if (!viewControl()->isEnabled(VIEW_CROP)) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << mousePos;

    forceRedraw();
}

void CropMakerView::slot_mouseReleased(QPointF spt)
{
    if (!viewControl()->isEnabled(VIEW_CROP)) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "release" << mousePos << screenToModel(mousePos);

    if (mouseInteraction)
    {
        mouseInteraction->endDragging(mousePos,getLayerTransform());
        mouseInteraction.reset();
    }
}

void CropMakerView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

void CropMakerView::setMousePos(QPointF pt)
{
    Qt::KeyboardModifiers km = QApplication::keyboardModifiers();
    if (km & Qt::ControlModifier)
    {
        mousePos.setY(pt.y());
    }
    else if (km & Qt::ShiftModifier)
    {
        mousePos.setX(pt.x());
    }
    else
    {
        mousePos = pt;
    }
}
