﻿#include <QApplication>

#include "geometry/crop.h"
#include "makers/crop_maker/crop_maker.h"
#include "makers/crop_maker/mouse_edit_crop.h"
#include "makers/prototype_maker/prototype.h"
#include "settings/configuration.h"
#include "viewers/crop_view.h"
#include "viewers/viewcontrol.h"

using std::make_shared;

CropViewer * CropViewer::mpThis = nullptr;

CropViewer * CropViewer::getInstance()
{
    if (!mpThis)
    {
        mpThis = new CropViewer();
    }
    return mpThis;
}

void CropViewer::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

CropViewer::CropViewer() : LayerController("Crop Viewer")
{
    config      = Configuration::getInstance();
    debugMouse  = false;
    setShowCrop(false);
    cropMaker   = nullptr;
}

void CropViewer::init(CropMaker *ed)
{
    cropMaker = ed;
}

void CropViewer::paint(QPainter *painter)
{
    if (getShowCrop())
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

void CropViewer::slot_wheel_rotate(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setCanvasXform(xf);
    }
}

void CropViewer::slot_scale(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setCanvasXform(xf);
    }
}

void CropViewer::slot_rotate(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setCanvasXform(xf);
    }
}

void CropViewer:: slot_moveX(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setCanvasXform(xf);
    }
}

void CropViewer::slot_moveY(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateY(xf.getTranslateY() + amount);
        setCanvasXform(xf);
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

    if (!getShowCrop())
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

    if (!getShowCrop())
        return;

    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << mousePos << screenToWorld(mousePos);

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
        Xform xf = getCanvasXform();
        xf.setTranslateX(xf.getTranslateX() + pt.x());
        xf.setTranslateY(xf.getTranslateY() + pt.y());
        setCanvasXform(xf);
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

    if (debugMouse) qDebug() << "release" << mousePos << screenToWorld(mousePos);

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
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setCanvasXform(xf);
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
