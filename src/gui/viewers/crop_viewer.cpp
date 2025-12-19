#include <QApplication>

#include "gui/model_editors/crop_edit/mouse_edit_crop.h"
#include "gui/top/controlpanel.h"
#include "gui/viewers/crop_viewer.h"
#include "gui/viewers/gui_modes.h"
#include "model/makers/crop_maker.h"
#include "model/prototypes/prototype.h"
#include "sys/geometry/crop.h"

using std::make_shared;

CropViewer::CropViewer() : LayerController(VIEW_CROP,DERIVED,"Crop Maker")
{
    debugMouse  = false;
    cropMaker   = nullptr;
    makerType   = CM_UNDEFINED;

    connect(this, &CropViewer::sig_delegateView, Sys::controlPanel, &ControlPanel::slot_delegateView);
}

void CropViewer::aquire(CropMaker *ed, eCropMaker maker)
{
    // aquire always grabs the viewer
    cropMaker  = ed;
    makerType  = maker;
}

void CropViewer::release(eCropMaker maker)
{
    if (maker == makerType)
    {
        cropMaker  = nullptr;
        makerType  = CM_UNDEFINED;
    }
}

void CropViewer::setShowCrop(eCropMaker maker, bool state)
{
    if (maker == makerType)
    {
        emit sig_delegateView(VIEW_CROP,state);     // thread safe
    }
}

bool CropViewer::getShowCrop(eCropMaker maker)
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

void CropViewer::paint(QPainter *painter)
{
    if (makerType != CM_UNDEFINED)
    {
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        // draw
        draw(painter,getLayerTransform());
    }
}

void CropViewer::draw(QPainter *painter , QTransform t)
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

void CropViewer::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
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
    mouseInteraction = make_shared<MouseEditCrop>(mousePos,crop,t);
    mouseInteraction->updateDragging(mousePos,t);
}

void CropViewer::slot_mouseDragged(QPointF spt)
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

void CropViewer::slot_mouseMoved(QPointF spt)
{
    if (!viewControl()->isEnabled(VIEW_CROP)) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << mousePos;

    forceRedraw();
}

void CropViewer::slot_mouseReleased(QPointF spt)
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

void CropViewer::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

void CropViewer::setMousePos(QPointF pt)
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
