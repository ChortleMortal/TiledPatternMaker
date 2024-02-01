#include <QApplication>
#include <QtMath>
#include <QDebug>

#include "misc/border.h"
#include "makers/crop_maker/mouse_edit_border.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "viewers/border_view.h"
#include "viewers/crop_view.h"
#include "viewers/view.h"

BorderView * BorderView::mpThis = nullptr;

BorderView * BorderView::getInstance()
{
    if (!mpThis)
    {
        mpThis = new BorderView();
    }
    return mpThis;
}

void BorderView::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

BorderView::BorderView() : LayerController("BorderView",false)
{
    config      = Configuration::getInstance();
    cropView    = CropViewer::getInstance();
    debugMouse  = false;
    setZValue(BORDER_ZLEVEL);
}


void BorderView::paint(QPainter *painter)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    auto b = border.lock();
    if (b)
    {
        if (b->getRequiresConversion())
        {
            b->setRequiresConversion(false);
            b->legacy_convertToModelUnits();
        }

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

void BorderView::setModelXform(const Xform & xf, bool update)
{
    Q_ASSERT(!_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << xf.toInfoString() << (isUnique() ? "unique" : "common");
    viewControl->setCurrentModelXform(xf,update);
}

const Xform & BorderView::getModelXform()
{
    Q_ASSERT(!_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << viewControl->getCurrentModelXform().toInfoString() << (isUnique() ? "unique" : "common");
    return viewControl->getCurrentModelXform();
}


void BorderView::slot_wheel_rotate(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setModelXform(xf,true);
    }
}

void BorderView::slot_scale(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setModelXform(xf,true);
    }
}

void BorderView::slot_rotate(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setModelXform(xf,true);
    }
}

void BorderView:: slot_moveX(qreal amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        amount = screenToWorld(amount);
        Xform xf = getModelXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setModelXform(xf,true);
    }
}

void BorderView::slot_moveY(qreal amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        amount = screenToWorld(amount);
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
    if (cropView->getShowCrop())
        return;
    if (!ControlPanel::getInstance()->isVisiblePage(PAGE_BORDER_MAKER))
        return;

    setMousePos(spt);

    auto b = border.lock();
    if (b)
    {
        auto c = std::dynamic_pointer_cast<Crop>(b);
        auto mec = std::make_shared<MouseEditBorder>(this,mousePos,c);
        setMouseInteraction(mec);
        mec->updateDragging(mousePos);
    }
}

void BorderView::slot_mouseDragged(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;
    if (cropView->getShowCrop()) return;

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
        Xform xf = getModelXform();
        xf.setTranslateX(xf.getTranslateX() + pt.x());
        xf.setTranslateY(xf.getTranslateY() + pt.y());
        setModelXform(xf,true);
    }
}

void BorderView::slot_mouseMoved(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;
    if (cropView->getShowCrop()) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << mousePos;
}

void BorderView::slot_mouseReleased(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;
    if (cropView->getShowCrop()) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "release" << mousePos << screenToWorld(mousePos);

    BorderMouseActionPtr mma = getMouseInteraction();
    if (mma)
    {
        mma->endDragging(mousePos);
        resetMouseInteraction();
        auto b = border.lock();
        if (b)
        {
            b->setRequiresConstruction(true);
            forceRedraw();
        }
    }
}

void BorderView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

void BorderView::slot_wheel_scale(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode( KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setModelXform(xf,true);
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
