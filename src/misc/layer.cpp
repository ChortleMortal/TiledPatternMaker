#include <QPainter>
#include "misc/layer.h"
#include "misc/sys.h"
#include "geometry/transform.h"
#include "settings/configuration.h"
#include "panels/controlpanel.h"

int Layer::refs = 0;

Layer::Layer(QString name, bool unique)
{
    _name   = name;
    _unique = unique;
    visible = true;
    zlevel  = STANDARD_ZLEVEL;
    debug   = 0;    // DEBUG_LAYER | DEBUG_TFORM | DEBUG_XFORM;

    connectSignals();
    refs++;
}

Layer::Layer(const Layer & other) : QObject()
{
    _name           = other._name;
    _unique         = other._unique;
    visible         = other.visible;
    layerTransform  = other.layerTransform;
    invertedLayer   = other.invertedLayer;
    subLayers       = other.subLayers;
    zlevel          = other.zlevel;
    xf_model        = other.xf_model;
    debug           = other.debug;

    connectSignals();
    refs++;
}

Layer::Layer(LayerPtr other)
{
    _name           = other->_name;
    _unique         = other->_unique;
    visible         = other->visible;
    layerTransform  = other->layerTransform;
    invertedLayer   = other->invertedLayer;
    subLayers       = other->subLayers;
    zlevel          = other->zlevel;
    xf_model        = other->xf_model;
    debug           = other->debug;

    connectSignals();
    refs++;
}

Layer::~Layer()
{
    refs--;
}

void Layer::connectSignals()
{
    config      = Sys::config;
    view        = Sys::view;
    viewControl = Sys::viewController;
    
    connect(this, &Layer::sig_refreshView, viewControl, &ViewController::slot_reconstructView);

    //auto panel = ControlPanel::getInstance();
    //connect(panel, &ControlPanel::sig_id_layers,  this,  &Layer::id_layer);
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
        view->update();
    }
}

void Layer::forceRedraw()
{
    view->update();
}

bool Layer::isSelected()
{
    auto lp = Sys::selectedLayer;
    return (lp == this);
}

QTransform  Layer::getCanvasTransform()
{
    Canvas & canvas  = viewControl->getCanvas();
    QTransform t     = canvas.getCanvasTransform();
    if (debug & DEBUG_TFORM) qDebug().noquote() << "Canvas transform:" << getLayerName() << Transform::info(t);
    return t;
}

QTransform  Layer::getModelTransform()
{
    QTransform t =  getModelXform().toQTransform(getCanvasTransform());
    if (debug & DEBUG_TFORM) qDebug().noquote() << "Model transform:" << getLayerName() << Transform::info(t);
    return t;
}

QTransform Layer::getLayerTransform()
{
    if (layerTransform.isIdentity())
    {
        // compute Transform
        computeLayerTransform();
    }
    if (debug & DEBUG_LAYER) qDebug().noquote() << "Layer transform:" << getLayerName() << Transform::info(layerTransform);
    return layerTransform;
}

void Layer::computeLayerTransform()
{
    layerTransform   = getCanvasTransform() * getModelTransform();
    invertedLayer  = layerTransform.inverted();
    if (debug & DEBUG_TFORM) qDebug().noquote() << "Computed Layer:" << getLayerName() << Transform::info(layerTransform);
}

void Layer::setCenterScreenUnits(QPointF spt)
{
    qDebug().noquote() << "Layer::setCenterScreenUnits" << getLayerName() << spt;

    if (layerTransform.isIdentity())
    {
        computeLayerTransform();
    }

    // save in model units
    QPointF mpt = invertedLayer.map(spt);
    Xform xf = getModelXform();
    xf.setModelCenter(mpt);
    setModelXform(xf,false);
    computeLayerTransform();

    // adjust so that image does not jump
    QPointF adjCenter = getCenterScreenUnits();
    QPointF diff      = spt - adjCenter;
    xf = getModelXform();
    xf.setTranslateX(xf.getTranslateX() + diff.x());
    xf.setTranslateY(xf.getTranslateY() + diff.y());
    setModelXform(xf,false);
    computeLayerTransform();
}

QPointF Layer::getCenterScreenUnits()
{
    if (layerTransform.isIdentity())
    {
        computeLayerTransform();
        //qDebug() << "Layer::getCenter()" << Transform::toInfoString(qtr_layer);
    }
    return layerTransform.map(getModelXform().getModelCenter());
}

QPointF Layer::getCenterModelUnits()
{
    return getModelXform().getModelCenter();
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
    if (config->showCenterDebug || Sys::showCenterMouse)
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
    qInfo().noquote() << getLayerName() << (isUnique() ? "unique" : "common");
}
