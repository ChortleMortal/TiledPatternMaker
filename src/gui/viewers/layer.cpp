#include <QPainter>

#include "gui/top/controlpanel.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/layer.h"
#include "model/settings/configuration.h"
#include "sys/geometry/transform.h"
#include "sys/sys.h"

int Layer::refs = 0;

Layer::Layer(Type type, Model model, ZLevel zlevel, QString name)
{
    _type        = type;
    _model       = model;
    _zlevel      = zlevel;
    _opacity     = 1.0;
    _name        = name;
    _clipable    = false;
    _viewControl = Sys::systemViewController;

    visible      = true;
    debug        = 0; //DEBUG_LAYER | DEBUG_TFORM | DEBUG_XFORM;
    centerConv   = false;
    bkgdColor   = Qt::black;

    initLayer();
    connectSignals();

    refs++;
}

Layer::Layer(const Layer & other) : QObject()
{
    _type           = other._type;
    _model          = other._model;
    _name           = other._name;
    _useSMX         = other._useSMX;
    _viewControl    = other._viewControl;
    _zlevel         = other._zlevel;
    _opacity        = other._opacity;
    _clipable       = other._clipable;
    visible         = other.visible;

    layerTransform  = other.layerTransform;
    subLayers       = other.subLayers;
    xf_model        = other.xf_model;
    debug           = other.debug;
    centerConv      = other.centerConv;
    bkgdColor       = other.bkgdColor;

    initLayer();
    connectSignals();

    refs++;
}

Layer::Layer(LayerPtr other)
{
    _type           = other->_type;
    _model          = other->_model;
    _name           = other->_name;
    _useSMX         = other->_useSMX;
    _viewControl    = other->_viewControl;
    _zlevel         = other->_zlevel;
    _opacity        = other->_opacity;
    _clipable       = other->_clipable;

    visible         = other->visible;
    layerTransform  = other->layerTransform;
    subLayers       = other->subLayers;
    xf_model        = other->xf_model;
    debug           = other->debug;
 	centerConv      = other->centerConv;
    bkgdColor       = other->bkgdColor;

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
    if (model() == Model::Primary)
    {
        _useSMX = false;
    }
    else
    {
        Q_ASSERT(model() == Model::Derived);
        _useSMX = true;
    }
}

void Layer::connectSignals()
{
    connect(this, &Layer::sig_reconstructView, _viewControl, &ViewController::slot_reconstructView);
    connect(this, &Layer::sig_updateView,      _viewControl, &ViewController::slot_updateView);

    connect(_viewControl, &ViewController::sig_resetLayers, this, &Layer::initLayer);


    connect(Sys::controlPanel, &ControlPanel::sig_id_layers, this, &Layer::id_layer);
}

bool Layer::sortByZlevel(LayerPtr s1, LayerPtr s2)
{
    return s1->_zlevel < s2->_zlevel;
}

bool Layer::sortByZlevelP(Layer * s1, Layer * s2)
{
    return s1->_zlevel < s2->_zlevel;
}

void Layer::addSubLayer(LayerPtr item)
{
    item->setLoc(pos);
    item->setZLevel(_zlevel);

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
    Canvas * canvas = viewControl()->getCanvas();
    QTransform t     = canvas->getTransform();
    if (debug & DEBUG_TFORM) qDebug().noquote() << "Canvas transform:" << layerName() << Transform::info(t);
    return t;
}

QTransform  Layer::getModelTransform()
{
    QTransform t = modelTransformOf(getModelXform());
    if (debug & DEBUG_TFORM) qDebug().noquote() << "Model transform:" << layerName() << Transform::info(t);
    return t;
}

QTransform Layer::modelTransformOf(const Xform & modelXf)
{
    QPointF center = viewControl()->getCanvas()->getCenter();
    QPointF trans  = modelXf.getTranslate();
    return modelXf.getTransform(center - trans);  // NOTE - 24APR25 this is a major change
}

QTransform Layer::getUndistortedLayerTransform()
{
    // used by painter crops, which frame the canvas rather than warping with its content
    Canvas * canvas = viewControl()->getCanvas();
    return canvas->getUndistortedTransform() * getModelTransform();
}

void Layer::computeLayerTransform()
{
    layerTransform        = getCanvasTransform() * getModelTransform();
    if (debug & DEBUG_TFORM) qDebug().noquote() << "Computed transfrom:" << layerName() << Transform::info(layerTransform);
}

QTransform Layer::getLayerTransform()
{
    if (layerTransform.isIdentity())
    {
        // compute Transform
        computeLayerTransform();
        if (centerConv)
        {
            correctLegacyCenter(getLegacyCenter());
            centerConv = false;
        }
    }
    if (debug & DEBUG_LAYER) qDebug().noquote() << "Layer transform:" << layerName() << Transform::info(layerTransform);
    return layerTransform;
}

bool Layer::correctLegacyCenter(QPointF mpt)
{
    qInfo() << "Layer::correctLegacyLayer" << mpt << _name;

    bool rv = false;
    Xform xf = getModelXform();

    if (!mpt.isNull())
    {
        // one time conversion after loading for previous use of 'center'
        qreal canvasScale = Transform::scalex(getCanvasTransform());
        qreal layerScale  = Transform::scalex(getLayerTransform());

        QPointF correction = mpt * (canvasScale-layerScale);
        if (!correction.isNull())
        {
            qWarning() << "applying correction for legacy center" << correction << "to translate" << xf.getTranslate();
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
    auto t = getLayerTransform().inverted();
    qreal scale = Transform::scalex(t);
    return val * scale;
}

QPointF Layer::screenToModel(QPointF pt)
{
    auto t = getLayerTransform().inverted();
    return t.map(pt);
}

QPoint Layer::screenToModel(QPoint pt)
{
    auto t = getLayerTransform().inverted();
    return t.map(pt);
}

QRectF  Layer::screenToModel(QRectF rect)
{
    auto t = getLayerTransform().inverted();
    return t.mapRect(rect);
}

QRect Layer::screenToModel(QRect rect)
{
    auto t = getLayerTransform().inverted();
    return t.mapRect(rect);
}

QPointF Layer::screenToModel(int x, int y)
{
    qreal xx = static_cast<qreal>(x);
    qreal yy = static_cast<qreal>(y);

    auto t = getLayerTransform().inverted();
    return t.map(QPointF(xx, yy));
}

QPolygonF Layer::screenToModel(QPolygonF poly)
{
    auto t = getLayerTransform().inverted();
    return t.map(poly);
}

Circle Layer::screenToModel(Circle c)
{
    auto       t = getLayerTransform().inverted();
    QPointF cent = t.map(c.centre);
    qreal radius = Transform::scalex(t) * c.radius;
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

QRect Layer::modelToScreen(QRect rect)
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

void  Layer::drawLayerModelCenter(QPainter * painter)
{
    if (Sys::showCenterDebug || Sys::showCenterMouse)
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
    qInfo().noquote() << layerName() << sLayerType[(int)type()];
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


const QString sLayerType[] = {
    E2STR(LegacyDesign),
    E2STR(Mosaic),
    E2STR(Prototype),
    E2STR(MotifMaker),
    E2STR(Tiling),
    E2STR(TilingMaker),
    E2STR(MapEditor),
    E2STR(BkgdImage),
    E2STR(Grid),
    E2STR(ConstructionLines),
    E2STR(Crop),
    E2STR(DebugLayer),
    E2STR(BMPImage),
};

const QString sLayerType2[] = {
    "Legacy Design Layer",
    "Mosaic Layer",
    "Prototype Layer",
    "Motif Maker Layer",
    "Tiling Layer",
    "Tiling Maker Layer",
    "Map Editor Layer",
    "Background Image Layer",
    "Grid Layer",
    "Construction Lines Layer",
    "Crop Layer",
    "Debug Layer",
    "Image Viewer Layer"
};

