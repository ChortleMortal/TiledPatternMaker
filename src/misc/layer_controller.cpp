#include "misc/layer_controller.h"
#include "misc/sys.h"
#include "viewers/view_controller.h"

LayerController::LayerController(QString name, bool unique) : Layer(name,unique)
{
    connectSignals();
}

LayerController::LayerController(const LayerController & other) : Layer(other)
{
    connectSignals();
}

LayerController::LayerController(LayerCtrlPtr other) : Layer(other)
{
    connectSignals();
}

LayerController::~LayerController()
{
}

void LayerController::connectSignals()
{
    view    =  Sys::view;

    connect(view, &View::sig_mousePressed,          this, &LayerController::slot_mousePressed);
    connect(view, &View::sig_setCenter,             this, &LayerController::slot_setCenter);
    connect(view, &View::sig_mouseDragged,          this, &LayerController::slot_mouseDragged);
    connect(view, &View::sig_mouseTranslate,        this, &LayerController::slot_mouseTranslate);
    connect(view, &View::sig_mouseMoved,            this, &LayerController::slot_mouseMoved);
    connect(view, &View::sig_mouseReleased,         this, &LayerController::slot_mouseReleased);
    connect(view, &View::sig_mouseDoublePressed,    this, &LayerController::slot_mouseDoublePressed);

    connect(view, &View::sig_wheel_scale,           this, &LayerController::slot_wheel_scale);
    connect(view, &View::sig_wheel_rotate,          this, &LayerController::slot_wheel_rotate);

    connect(view, &View::sig_deltaScale,            this, &LayerController::slot_scale);
    connect(view, &View::sig_deltaRotate,           this, &LayerController::slot_rotate);
    connect(view, &View::sig_deltaMoveX,            this, &LayerController::slot_moveX);
    connect(view, &View::sig_deltaMoveY,            this, &LayerController::slot_moveY);
    
    connect(this, &Layer::sig_refreshView, Sys::viewController, &ViewController::slot_reconstructView);

}

void LayerController::slot_mouseTranslate(QPointF pt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setTranslateX(xf.getTranslateX() + pt.x());
        xf.setTranslateY(xf.getTranslateY() + pt.y());
        setModelXform(xf,true);
    }
}

void LayerController::slot_wheel_scale(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setModelXform(xf,true);
    }
}

void LayerController::slot_wheel_rotate(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setModelXform(xf,true);
    }
}

void LayerController::slot_scale(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setModelXform(xf,true);
    }
}

void LayerController::slot_rotate(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setModelXform(xf,true);
    }
}

void LayerController::slot_moveX(qreal amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getModelXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setModelXform(xf,true);
    }
}

void LayerController::slot_moveY(qreal amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        qDebug().noquote() << "LayerController::slot_move" << getLayerName();
        Xform xf = getModelXform();
        xf.setTranslateY(xf.getTranslateY() + amount);
        setModelXform(xf,true);
    }
}

void LayerController::slot_setCenter(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        setCenterScreenUnits(spt);
    }
}
