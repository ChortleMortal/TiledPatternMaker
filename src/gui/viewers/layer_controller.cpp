#include "gui/viewers/layer_controller.h"
#include "gui/top/system_view.h"
#include "gui/top/view_controller.h"
#include "sys/sys.h"

LayerController::LayerController(Type viewType, Model model, ZLevel zlevel, QString name) : Layer(viewType,model, zlevel, name)
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
    SystemView * view    = Sys::systemViewController->getView();

    connect(view, &SystemView::sig_mousePressed,          this, &LayerController::slot_mousePressed);
    connect(view, &SystemView::sig_mouseDragged,          this, &LayerController::slot_mouseDragged);
    connect(view, &SystemView::sig_mouseTranslate,        this, &LayerController::slot_mouseTranslate);
    connect(view, &SystemView::sig_mouseMoved,            this, &LayerController::slot_mouseMoved);
    connect(view, &SystemView::sig_mouseReleased,         this, &LayerController::slot_mouseReleased);
    connect(view, &SystemView::sig_mouseDoublePressed,    this, &LayerController::slot_mouseDoublePressed);

    connect(view, &SystemView::sig_wheel_scale,           this, &LayerController::slot_wheel_scale);
    connect(view, &SystemView::sig_wheel_rotate,          this, &LayerController::slot_wheel_rotate);

    connect(view, &SystemView::sig_deltaScale,            this, &LayerController::slot_scale);
    connect(view, &SystemView::sig_deltaRotate,           this, &LayerController::slot_rotate);
    connect(view, &SystemView::sig_deltaMoveX,            this, &LayerController::slot_moveX);
    connect(view, &SystemView::sig_deltaMoveY,            this, &LayerController::slot_moveY);

    connect(view, &SystemView::sig_prepare,               this, &LayerController::slot_prepare);
    connect(view, &SystemView::sig_enlarge,               this, &LayerController::slot_enlarge);
    connect(view, &SystemView::sig_restore,               this, &LayerController::slot_restore);

    connect(_viewControl, &ViewController::sig_breakaway,   this, &LayerController::slot_breakaway);
    connect(_viewControl, &ViewController::sig_lock,        this, &LayerController::slot_lock);
    connect(_viewControl, &ViewController::sig_solo,        this, &LayerController::slot_solo);
}

bool LayerController::validateSignal()
{
    return !isLocked();
}

void LayerController::slot_mouseTranslate(uint sigid, QPointF pt)
{
    if (!validateSignal()) return;

    Xform xf = getModelXform();

    pt /= xf.getScale();

    xf.setTranslate(xf.getTranslate() + pt);
    setModelXform(xf,true,sigid);
}

void LayerController::slot_wheel_scale(uint sigid, qreal delta)
{
    if (!validateSignal()) return;

    Xform xf = getModelXform();
    xf.setScale(xf.getScale() * (1.0 + delta));
    setModelXform(xf,true,sigid);
}

void LayerController::slot_wheel_rotate(uint sigid, qreal delta)
{
    if (!validateSignal()) return;

    Xform xf = getModelXform();
    xf.setRotateDegrees(xf.getRotateDegrees() + delta);
    setModelXform(xf,true,sigid);
}

void LayerController::slot_scale(uint sigid, int amount)
{
    if (!validateSignal()) return;

    Xform xf = getModelXform();
    xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
    setModelXform(xf,true,sigid);
}

void LayerController::slot_rotate(uint sigid, int amount)
{
    if (!validateSignal()) return;

    Xform xf = getModelXform();
    xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
    setModelXform(xf,true,sigid);
}

void LayerController::slot_moveX(uint sigid, qreal amount)
{
    if (!validateSignal()) return;

    Xform xf = getModelXform();

    amount /= xf.getScale();

    xf.setTranslateX(xf.getTranslateX() + amount);
    setModelXform(xf,true,sigid);
}

void LayerController::slot_moveY(uint sigid, qreal amount)
{
    if (!validateSignal()) return;

    Xform xf = getModelXform();

    amount /= xf.getScale();

    xf.setTranslateY(xf.getTranslateY() + amount);
    setModelXform(xf,true,sigid);
}

void LayerController::slot_solo(Layer * l, bool set)
{
    solo(l,set);
}

void LayerController::slot_lock(Layer * l, bool set)
{
    if (l != this)
        return;

    lock(set);
}

void LayerController::slot_breakaway(Layer * l, bool set)
{
    if (l != this)
        return;

    breakaway(set);
}

void LayerController::slot_prepare()
{
    preEnlargeCopy = getModelXform();
    //qDebug() << "saved" << preEnlargeCopy.info(4);
}

void LayerController::slot_enlarge(uint sigid, Xform xf)
{
    setModelXform(xf,true,sigid);
    //qDebug() << "enlarged" << xf.info(4);
}

void LayerController::slot_restore(uint sigid)
{
    setModelXform(preEnlargeCopy,true,sigid);
    //qDebug() << "restored" << preEnlargeCopy.info(4);
}
