#include <QPainter>

#include "gui/top/controlpanel.h"
#include "gui/viewers/layer.h"
#include "model/settings/configuration.h"
#include "sys/geometry/transform.h"
#include "sys/sys.h"

int Layer::refs = 0;

Layer::Layer(eViewType viewType,eModelType modelType, QString name)
{
    _viewType    = viewType;
    _modelType   = modelType;
    _name        = name;
    visible      = true;
    _viewControl = Sys::viewController;
    zlevel       = STANDARD_ZLEVEL;
    debug        = 0; //DEBUG_LAYER | DEBUG_TFORM | DEBUG_XFORM;

    initLayer();
    connectSignals();
    refs++;
}

Layer::Layer(const Layer & other) : QObject()
{
    _viewType       = other._viewType;
    _modelType      = other._modelType;
    _name           = other._name;
    _useSMX         = other._useSMX;
    _viewControl    = other._viewControl;
    visible         = other.visible;
    layerTransform  = other.layerTransform;
    invertedLayer   = other.invertedLayer;
    subLayers       = other.subLayers;
    zlevel          = other.zlevel;
    xf_model        = other.xf_model;
    debug           = other.debug;

    initLayer();
    connectSignals();
    refs++;
}

Layer::Layer(LayerPtr other)
{
    _viewType       = other->_viewType;
    _modelType      = other->_modelType;
    _name           = other->_name;
    _useSMX         = other->_useSMX;
    _viewControl    = other->_viewControl;
    visible         = other->visible;
    layerTransform  = other->layerTransform;
    invertedLayer   = other->invertedLayer;
    subLayers       = other->subLayers;
    zlevel          = other->zlevel;
    xf_model        = other->xf_model;
    debug           = other->debug;

    initLayer();
    connectSignals();
    refs++;
}

Layer::~Layer()
{
    refs--;
}

void Layer::initLayer()
{
    _solo      = false;
    _lockState = 0;
    if (modelType() == PRIMARY)
    {
        _useSMX = false;
    }
    else
    {
        Q_ASSERT(modelType() == DERIVED);
        _useSMX = true;
    }
}

void Layer::connectSignals()
{
    connect(this, &Layer::sig_reconstructView, _viewControl, &SystemViewController::slot_reconstructView);
    connect(this, &Layer::sig_updateView,      _viewControl, &SystemViewController::slot_updateView);

    connect(_viewControl, &SystemViewController::sig_resetLayers, this, &Layer::initLayer);


    connect(Sys::controlPanel, &ControlPanel::sig_id_layers, this, &Layer::id_layer);
}

bool Layer::sortByZlevel(LayerPtr s1, LayerPtr s2)
{
    return s1->zlevel < s2->zlevel;
}

bool Layer::sortByZlevelP(Layer * s1, Layer * s2)
{
    return s1->zlevel < s2->zlevel;
}

void Layer::addSubLayer(LayerPtr item)
{
    item->setLoc(pos);
    item->setZValue(zlevel);

    subLayers.push_back(item);
}

void Layer::removeSubLayer(LayerPtr item)
{
   subLayers.removeAll(item);
}

void Layer::forceLayerRecalc(bool update)
{
    layerTransform.reset();
    getLayerTransform();    // recalc
    if (update)
    {
        emit sig_updateView();
    }
}

void Layer::forceRedraw()
{
    emit sig_updateView();
}

bool Layer::isSelected()
{
    auto lp = Sys::selectedLayer;
    return (lp == this);
}

const Xform & Layer::getModelXform()
{
    if (isBreakaway() == false)
    {
        if (debug & DEBUG_XFORM) qDebug().noquote() << "GET" << layerName() <<  "SMX  " << xf_model.info();
        return viewControl()->getSystemModelXform();
    }
    else
    {
        if (debug & DEBUG_XFORM) qDebug().noquote() << "GET" << layerName() <<  "local" << xf_model.info();
        return xf_model;
    }
}

void  Layer::setModelXform(const Xform & xf, bool update, uint sigid)
{
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << layerName() << xf.info();

    if (isPrimary() && (this == viewControl()->getSelectedPrimaryLayer().get()))
    {
        // set the selected primary and SMX
        if (debug & DEBUG_XFORM) qDebug().noquote() << "SET" << layerName() << "local" << xf.info();
        xf_model = xf;
        if (debug & DEBUG_XFORM) qDebug().noquote() << "SET" << layerName() << "SMX  " << xf.info();
        viewControl()->setSystemModelXform(xf,sigid);
    }
    else if (isBreakaway() == false)
    {
        // all the other viewers
        if (debug & DEBUG_XFORM) qDebug().noquote() << "SET" << layerName() << "SMX  " << xf.info();
        viewControl()->setSystemModelXform(xf,sigid);
    }
    else
    {
        // breakaway
        if (debug & DEBUG_XFORM) qDebug().noquote() << "SET" << layerName() << "local" << xf.info();
        xf_model = xf;
    }
    if (update)
    {
        forceLayerRecalc(update);
    }
}

QTransform  Layer::getCanvasTransform()
{
    Canvas & canvas  = viewControl()->getCanvas();
    QTransform t     = canvas.getTransform();
    if (debug & DEBUG_TFORM) qDebug().noquote() << "Canvas transform:" << layerName() << Transform::info(t);
    return t;
}

QTransform  Layer::getModelTransform()
{
    QPointF center        = viewControl()->getCanvas().getCenter();
    const Xform & modelXf = getModelXform();
    QPointF trans         = modelXf.getTranslate();
    QTransform t          = modelXf.getTransform(center - trans);  // NOTE - 24APR25 this is a major change
    if (debug & DEBUG_TFORM) qDebug().noquote() << "Model transform:" << layerName() << Transform::info(t);
    return t;
}

void Layer::computeLayerTransform()
{
    layerTransform              = getCanvasTransform() * getModelTransform();
    invertedLayer               = layerTransform.inverted();
    if (debug & DEBUG_TFORM) qDebug().noquote() << "Comput transfrom:" << layerName() << Transform::info(layerTransform);
}

QTransform Layer::getLayerTransform()
{
    if (layerTransform.isIdentity())
    {
        // compute Transform
        computeLayerTransform();
    }
    if (debug & DEBUG_LAYER) qDebug().noquote() << "Layer transform:" << layerName() << Transform::info(layerTransform);
    return layerTransform;
}

bool Layer::correctLegacyLayer(QPointF mpt)
{
    qInfo() << "Layer::correctLegacyLayer" << mpt << _name;

    bool rv = false;
    Xform xf = getModelXform();

    if (!mpt.isNull())
    {
        // one time conversion after loading for previous use of 'center'
        qreal canvasScale = Transform::scalex(getCanvasTransform());
        //qreal modelScale  = Transform::scalex(getModelTransform());
        qreal layerScale  = Transform::scalex(getLayerTransform());
        //qDebug() << "canvas scale" << canvasScale << "model scale" << modelScale << "layer scale" << layerScale;

        QPointF correction = mpt * (canvasScale-layerScale);
        if (!correction.isNull())
        {
            //qWarning() << "applying correction for legacy center" << correction << "to translate" << xf.getTranslate();
            xf.applyTranslate(correction);
            setModelXform(xf,false,Sys::nextSigid());
            rv = true;
        }
    }

    qreal scale   = xf.getScale();
    if (!Loose::equals(1.0,scale))
    {
        QPointF trans = xf.getTranslate();
        trans /= scale;
        xf.setTranslate(trans);
        setModelXform(xf,false,Sys::nextSigid());
        rv = true;
    }

    if (rv == true)
    {
        forceLayerRecalc(false);
    }

    return rv;
}

QPointF Layer::getCenterScreenUnits()
{
    if (layerTransform.isIdentity())
    {
        computeLayerTransform();
    }
    // NOTE - in this new world is alwyays zero
    return modelToScreen(QPointF(0,0));
}

QPointF Layer::getCenterModelUnits()
{
    // NOTE - in this new world is alwyays zero
    return QPointF(0,0);
}

qreal Layer::screenToModel(qreal val)
{
    getLayerTransform();
    qreal scale = Transform::scalex(invertedLayer);
    return val * scale;
}

QPointF Layer::screenToModel(QPointF pt)
{
    getLayerTransform();
    return invertedLayer.map(pt);
}

QPoint Layer::screenToModel(QPoint pt)
{
    getLayerTransform();
    return invertedLayer.map(pt);
}

QRectF  Layer::screenToModel(QRectF rect)
{
    getLayerTransform();
    return invertedLayer.mapRect(rect);
}

QPointF Layer::screenToModel(int x, int y)
{
    qreal xx = static_cast<qreal>(x);
    qreal yy = static_cast<qreal>(y);

    getLayerTransform();
    return invertedLayer.map(QPointF(xx, yy));
}

QPolygonF Layer::screenToModel(QPolygonF poly)
{
    getLayerTransform();
    return invertedLayer.map(poly);
}

Circle Layer::screenToModel(Circle c)
{
    getLayerTransform();
    QPointF cent = invertedLayer.map(c.centre);
    qreal radius = Transform::scalex(invertedLayer) * c.radius;
    Circle circ(cent,radius);
    return circ;
}

QPointF Layer::modelToScreen(QPointF pt)
{
    getLayerTransform();
    return layerTransform.map(pt);
}

QRectF Layer::modelToScreen(QRectF rect)
{
    getLayerTransform();
    return layerTransform.mapRect(rect);
}

QLineF Layer::modelToScreen(QLineF line)
{
    QLineF aline;
    getLayerTransform();
    aline.setP1(layerTransform.map(line.p1()));
    aline.setP2(layerTransform.map(line.p2()));
    return aline;
}

QPolygonF Layer::modelToScreen(QPolygonF poly)
{
    getLayerTransform();
    QPolygonF poly2 = layerTransform.map(poly);
    return poly2;
}

Circle Layer::modelToScreen(Circle c)
{
    getLayerTransform();
    QPointF cent = layerTransform.map(c.centre);
    qreal radius = Transform::scalex(layerTransform) * c.radius;
    Circle circ(cent,radius);
    return circ;
}

void Layer::setLoc(QPointF loc)
{
    //qDebug() << name << "pos=" << pos << "new" << loc;
    pos += loc;
}

void Layer::setZValue(int z)
{
    zlevel = z;
}

void  Layer::drawLayerModelCenter(QPainter * painter)
{
    if (Sys::config->showCenterDebug || Sys::showCenterMouse)
    {
        QPointF pt = getCenterScreenUnits();
        drawCenterSymbol(painter,pt,QColor(Qt::green),QColor(Qt::blue));
    }
}

void Layer::drawCenterSymbol(QPainter * painter, QPointF spt, QColor circleColor, QColor xColor)
{
    painter->save();
    qreal len = 13;
    circleColor.setAlpha(128);
    painter->setPen(QPen(circleColor));
    painter->setBrush(QBrush(circleColor));
    painter->drawEllipse(spt,len,len);
    painter->setPen(QPen(xColor));
    painter->drawLine(QPointF(spt.x()-len,spt.y()),QPointF(spt.x()+len,spt.y()));
    painter->drawLine(QPointF(spt.x(),spt.y()-len),QPointF(spt.x(),spt.y()+len));
    painter->restore();
}

// this is only called for legacy designs where the layer has sub-layers
void Layer::paint(QPainter * painter)
{
    //qDebug().noquote() << "Layer::paint" << getName() <<": sub-levels =" << subLayers.count();

    std::stable_sort(subLayers.begin(), subLayers.end(), sortByZlevel);

    painter->save();

    painter->translate(getLoc());
    qreal rot = getModelXform().getRotateDegrees();
    painter->rotate(rot);

    for (const auto & layer : std::as_const(subLayers))
    {
        layer->paint(painter);
    }

    painter->restore();
}

void Layer::id_layer()
{
    qInfo().noquote() << layerName() << sViewerType[viewType()];
}

//////////////////////////////////////////////////////////////////////////////////////
///
///  Brekaway/SMX logic (SMX is System Model Xfeorm)
///
//////////////////////////////////////////////////////////////////////////////////////


void Layer::_setUserLock(bool set)
{
    if (set)
        _lockState |= USER_LOCK;
    else
        _lockState &= ~USER_LOCK;
}

void Layer::_setSoloLock(bool set)
{
    if (set)
        _lockState |= SOLO_LOCK;
    else
        _lockState &= ~SOLO_LOCK;
}

void Layer::breakaway(bool set)
{
    if (isBreakaway() == set)
        return;

    if (viewControl()->getSelectedPrimaryLayer().get() == this)
        return;

    // alayer can be in breakaway but not locked or soloed
    if (set)
    {
        xf_model = getModelXform();     // copies from SMX to local
        _setBreakaway(true);
    }
    else
    {
        _setBreakaway(false);
        forceLayerRecalc(true);
    }
}

void Layer::lock(bool set)
{
    if (set)
    {
        if (_lockState & USER_LOCK)
            return;

        _setUserLock(true);

        // a locked layer does not respond to keyboard events
        // so its model xform needs to be copied into the local xform
        // when unlocked, a derived layer will pop back into
        // alignment with other derived layers and primary layers stay where they are
        breakaway(true);
    }
    else
    {
        if ((_lockState & USER_LOCK) == 0)
            return;

        _setUserLock(false);
    }
}

// soloing a layer means all other layers aare locked and the selkected layer is put into breakaway, and remains there
// turning off solo for the layer turns  of the locks on all the others layers
// if another layer goes into solo while another layer is soloed, the first solo layer muste be turned off;.
// but in effect the selected layer is mmade solo and all other layers are locked, if another layer is soloed it is taken out and locked
void Layer::solo(Layer * layer, bool set)
{
    if (layer == this)
    {
        if (set)
        {
            // the solo layer is set into solo mode
            _setSolo(true);
            _setSoloLock(false);
            breakaway(true);
        }
        else
        {
            _setSolo(false);
            _setSoloLock(false);
        }
    }
    else
    {
        // other layers
        if (set)
        {
            _setSolo(false);
            _setSoloLock(true);
        }
        else
        {
            _setSolo(false);    // should already be false
            _setSoloLock(false);
        }
    }
}


