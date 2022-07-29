#include <QApplication>

#include "misc/border.h"
#include "makers/crop_maker/mouse_edit_border.h"
#include "settings/configuration.h"
#include "viewers/border_view.h"
#include "viewers/viewcontrol.h"

using std::make_shared;

const bool debugMouse = false;

BorderViewPtr BorderView::spThis;

BorderViewPtr BorderView::getSharedInstance()
{
    if (!spThis)
    {
        spThis = make_shared<BorderView>();
    }
    return spThis;
}

BorderView::BorderView() : LayerController("Border")
{
    config = Configuration::getInstance();
    setZValue(BORDER_ZLEVEL);
}


void BorderView::paint(QPainter *painter)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    auto b = border.lock();
    if (b)
    {
        painter->save();
        b->draw(painter,getLayerTransform());
        painter->restore();

        if (getMouseInteraction())
        {
            painter->save();
            getMouseInteraction()->draw(painter,mousePos);
            painter->restore();
        }
    }
}

void BorderView::slot_wheel_rotate(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setCanvasXform(xf);
    }
}

void BorderView::slot_scale(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setCanvasXform(xf);
    }
}

void BorderView::slot_rotate(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setCanvasXform(xf);
    }
}

void BorderView:: slot_moveX(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setCanvasXform(xf);
    }
}

void BorderView::slot_moveY(int amount)
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

void BorderView::slot_setCenter(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        setCenterScreenUnits(spt);
    }
}

void BorderView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    if (!view->isActiveLayer(this))
        return;


    setMousePos(spt);

    if (border.lock())
    {
        auto b = border.lock();
        auto c = std::dynamic_pointer_cast<Crop>(b);
        auto mec = make_shared<MouseEditBorder>(mousePos,c);
        setMouseInteraction(mec);
        mec->updateDragging(mousePos);
    }
}


void BorderView::slot_mouseDragged(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;


    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << mousePos << screenToWorld(mousePos);

    BorderMouseActionPtr mec = getMouseInteraction();
    if (mec)
    {
        mec->updateDragging(mousePos);
    }

    forceRedraw();
}

void BorderView::slot_mouseTranslate(QPointF pt)
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

void BorderView::slot_mouseMoved(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;


    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << mousePos;

    forceRedraw();
}

void BorderView::slot_mouseReleased(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;


    setMousePos(spt);

    if (debugMouse) qDebug() << "release" << mousePos << screenToWorld(mousePos);

    BorderMouseActionPtr mma = getMouseInteraction();
    if (mma)
    {
        mma->endDragging(mousePos);
        resetMouseInteraction();
    }
}

void BorderView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

void BorderView::slot_wheel_scale(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode( KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setCanvasXform(xf);
    }
}

void BorderView::setMousePos(QPointF pt)
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
