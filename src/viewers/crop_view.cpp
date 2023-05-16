#include <QApplication>

#include "geometry/crop.h"
#include "makers/crop_maker/crop_maker.h"
#include "makers/crop_maker/mouse_edit_crop.h"
#include "makers/prototype_maker/prototype.h"
#include "settings/configuration.h"
#include "viewers/crop_view.h"
#include "viewers/viewcontrol.h"

using std::make_shared;


CropViewPtr CropView::spThis;

CropViewPtr CropView::getSharedInstance()
{
    if (!spThis)
    {
        spThis = make_shared<CropView>();
    }
    return spThis;
}

CropView::CropView() : LayerController("Crop")
{
    config      = Configuration::getInstance();
    debugMouse  = false;
}

void CropView::init(CropMaker *ed)
{
    cropMaker = ed;
}

void CropView::paint(QPainter *painter)
{
    if (cropMaker->getState() == CROPMAKER_STATE_ACTIVE)
    {
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        // draw
        draw(painter,getLayerTransform());
    }
}

void CropView::draw(QPainter *painter , QTransform t)
{
    CropMouseActionPtr cma = getMouseInteraction();
    if (cma)
    {
        cma->draw(painter,mousePos,t);
    }
    else
    {
        CropPtr crop = cropMaker->getActiveCrop();
        if (crop)
        {
            crop->draw(painter,t,false);
        }
    }
}

void CropView::slot_wheel_rotate(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setCanvasXform(xf);
    }
}

void CropView::slot_scale(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setCanvasXform(xf);
    }
}

void CropView::slot_rotate(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setCanvasXform(xf);
    }
}

void CropView:: slot_moveX(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setCanvasXform(xf);
    }
}

void CropView::slot_moveY(int amount)
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

void CropView::slot_setCenter(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        setCenterScreenUnits(spt);
    }
}

void CropView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    if (!view->isActiveLayer(this))
        return;

    if (cropMaker->getState() != CROPMAKER_STATE_ACTIVE)
        return;

    setMousePos(spt);

    auto t = getLayerTransform();
    auto mec = make_shared<MouseEditCrop>(mousePos,cropMaker->getCrop(),t);
    setMouseInteraction(mec);
    getMouseInteraction()->updateDragging(mousePos,t);
}


void CropView::slot_mouseDragged(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << mousePos << screenToWorld(mousePos);

    CropMouseActionPtr mma = getMouseInteraction();
    if (mma)
    {
        mma->updateDragging(mousePos,getLayerTransform());
    }

    forceRedraw();
}

void CropView::slot_mouseTranslate(QPointF pt)
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

void CropView::slot_mouseMoved(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << mousePos;

    forceRedraw();
}

void CropView::slot_mouseReleased(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "release" << mousePos << screenToWorld(mousePos);

    CropMouseActionPtr mma = getMouseInteraction();
    if (mma)
    {
        mma->endDragging(mousePos,getLayerTransform());
        resetMouseInteraction();
    }
}

void CropView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

void CropView::slot_wheel_scale(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode( KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setCanvasXform(xf);
    }
}

void CropView::setMousePos(QPointF pt)
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
