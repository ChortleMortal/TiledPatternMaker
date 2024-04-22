#include <QApplication>

#include "geometry/crop.h"
#include "makers/crop_maker/crop_maker.h"
#include "makers/crop_maker/mouse_edit_crop.h"
#include "makers/prototype_maker/prototype.h"
#include "viewers/crop_view.h"
#include "viewers/view_controller.h"

using std::make_shared;

CropViewer::CropViewer() : LayerController("Crop Viewer",false)
{
    debugMouse  = false;
    cropMaker   = nullptr;
    makerType   = CM_UNDEFINED;
    setShowCrop(CM_UNDEFINED,false);
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
        showCrop = state;
    }
}

bool CropViewer::getShowCrop(eCropMaker maker)
{
    if (maker == makerType)
    {
        return showCrop;
    }
    else
    {
        return false;
    }
}

void CropViewer::paint(QPainter *painter)
{
    if (makerType != CM_UNDEFINED && showCrop)
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

void CropViewer::setModelXform(const Xform & xf, bool update)
{
    Q_ASSERT(!_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << xf.info() << (isUnique() ? "unique" : "common");
    viewControl->setCurrentModelXform(xf,update);
}

const Xform & CropViewer::getModelXform()
{
    Q_ASSERT(!_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << viewControl->getCurrentModelXform().info() << (isUnique() ? "unique" : "common");
    return viewControl->getCurrentModelXform();
}

void CropViewer::slot_wheel_rotate(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setModelXform(xf,true);
    }
}

void CropViewer::slot_scale(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setModelXform(xf,true);
    }
}

void CropViewer::slot_rotate(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setModelXform(xf,true);
    }
}

void CropViewer:: slot_moveX(qreal amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setModelXform(xf,true);
    }
}

void CropViewer::slot_moveY(qreal amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setTranslateY(xf.getTranslateY() + amount);
        setModelXform(xf,true);
    }
}

//////////////////////////////////////////////////////////////////
///
/// slots and evzents
///
//////////////////////////////////////////////////////////////////

void CropViewer::slot_setCenter(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        setCenterScreenUnits(spt);
    }
}

void CropViewer::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    if (!view->isActiveLayer(this))
        return;

    if (!showCrop)
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
    if (!view->isActiveLayer(this))
        return;

    if (!showCrop)
        return;

    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << mousePos << screenToModel(mousePos);

    if (mouseInteraction)
    {
        mouseInteraction->updateDragging(mousePos,getLayerTransform());
    }

    forceRedraw();
}

void CropViewer::slot_mouseTranslate(QPointF pt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setTranslateX(xf.getTranslateX() + pt.x());
        xf.setTranslateY(xf.getTranslateY() + pt.y());
        setModelXform(xf,true);
    }
}

void CropViewer::slot_mouseMoved(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << mousePos;

    forceRedraw();
}

void CropViewer::slot_mouseReleased(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

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

void CropViewer::slot_wheel_scale(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode( KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setModelXform(xf,true);
    }
}

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
