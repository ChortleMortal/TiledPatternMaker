#include <QPainter>
#include "misc/layer.h"
#include "geometry/transform.h"
#include "settings/configuration.h"
#include "viewers/viewcontrol.h"

int Layer::refs = 0;

Layer::Layer(QString name) : QObject()
{
    this->name = name;
    visible    = true;
    zlevel     = STANDARD_ZLEVEL;
    connectSignals();
    refs++;
}

Layer::Layer(const Layer & other) : QObject()
{
    visible     = other.visible;
    name        = other.name;
    qtr_layer   = other.qtr_layer;
    qtr_invert  = other.qtr_invert;
    layerPen    = other.layerPen;
    name        = other.name;
    visible     = other.visible;
    subLayers   = other.subLayers;
    zlevel      = other.zlevel;
    connectSignals();
    refs++;
}

Layer::Layer(LayerPtr other) : QObject()
{
    visible     = other->visible;
    name        = other->name;
    qtr_layer   = other->qtr_layer;
    qtr_invert  = other->qtr_invert;
    layerPen    = other->layerPen;
    name        = other->name;
    visible     = other->visible;
    subLayers   = other->subLayers;
    zlevel      = other->zlevel;
    connectSignals();
    refs++;
}

Layer::~Layer()
{
    refs--;
}

void Layer::connectSignals()
{
    config   = Configuration::getInstance();
    view     = ViewControl::getInstance();

    connect(this, &Layer::sig_refreshView, view, &ViewControl::slot_refreshView);
}

// this is only called for legacy designs where the layer has sub-layers
void Layer::paint(QPainter * painter)
{
    //qDebug().noquote() << "Layer::paint" << getName() <<": sub-levels =" << subLayers.count();

    std::stable_sort(subLayers.begin(), subLayers.end(), sortByZlevel);

    painter->save();
    painter->translate(getLoc());
    qreal rot = getCanvasXform().getRotateDegrees();
    painter->rotate(rot);

    for (const auto & layer : subLayers)
    {
        layer->paint(painter);
    }

    painter->restore();
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
    qtr_layer.reset();
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
    auto lp = config->selectedLayer;
    return (lp == this);
}

QTransform  Layer::getFrameTransform()
{
    QTransform t = view->getViewSettings().getTransform(view->getMostRecent());

    //qDebug().noquote() << "Frame transform:" << Transform::toInfoString(t);

    return t;
}

QTransform  Layer::getCanvasTransform()
{
    QTransform t =  getCanvasXform().toQTransform(getFrameTransform());

    //qDebug().noquote() << "Canvas transform:" << Transform::toInfoString(t);

    return t;
}

QTransform Layer::getLayerTransform()
{
    if (qtr_layer.isIdentity())
    {
        // compute Transform
        computeLayerTransform();
    }
    //qDebug().noquote() << "Layer transform:" << Transform::toInfoString(qtr_layer);
    return qtr_layer;
}

void Layer::computeLayerTransform()
{
    //qDebug().noquote() << "OLD      qtr_layer:" << getName() << Transform::toInfoString(qtr_layer);
    //qDebug().noquote() << "frame:" << Transform::toInfoString(getFrameTransform()) << "canvas" << Transform::toInfoString(getCanvasTransform());
    qtr_layer   = getFrameTransform() * getCanvasTransform();
    qtr_invert  = qtr_layer.inverted();
    //qDebug().noquote() << "computed qtr_layer:" << getName() << Transform::toInfoString(qtr_layer);
}

void Layer::setCenterScreenUnits(QPointF spt)
{
    qDebug().noquote() << "Layer::setCenterScreenUnits" << getName() << spt;

    if (qtr_layer.isIdentity())
    {
        computeLayerTransform();
    }

    // save in model units
    QPointF mpt = qtr_invert.map(spt);
    Xform xf = getCanvasXform();
    xf.setModelCenter(mpt);
    setCanvasXform(xf);
    computeLayerTransform();

    // adjust so that image does not jump
    QPointF adjCenter = getCenterScreenUnits();
    QPointF diff      = spt - adjCenter;
    xf = getCanvasXform();
    xf.setTranslateX(xf.getTranslateX() + diff.x());
    xf.setTranslateY(xf.getTranslateY() + diff.y());
    setCanvasXform(xf);
    computeLayerTransform();
}

QPointF Layer::getCenterScreenUnits()
{
    if (qtr_layer.isIdentity())
    {
        computeLayerTransform();
        //qDebug() << "Layer::getCenter()" << Transform::toInfoString(qtr_layer);
    }
    return qtr_layer.map(getCanvasXform().getModelCenter());
}

QPointF Layer::getCenterModelUnits()
{
    return getCanvasXform().getModelCenter();
}

void Layer::setCanvasXform(const Xform & xf)
{
    view->setCurrentXform(xf);
    forceLayerRecalc();
}

const Xform & Layer::getCanvasXform()
{
    return view->getCurrentXform();
}

qreal Layer::screenToWorld(qreal val)
{
    getLayerTransform();
    qreal scale = Transform::scalex(qtr_invert);
    return val * scale;
}

QPointF Layer::screenToWorld(QPointF pt)
{
    getLayerTransform();
    return qtr_invert.map(pt);
}

QRectF  Layer::screenToWorld(QRectF rect)
{
    getLayerTransform();
    return qtr_invert.mapRect(rect);
}

QPointF Layer::screenToWorld(int x, int y)
{
    qreal xx = static_cast<qreal>(x);
    qreal yy = static_cast<qreal>(y);

    getLayerTransform();
    return qtr_invert.map(QPointF(xx, yy));
}

QPolygonF Layer::screenToWorld(QPolygonF poly)
{
    getLayerTransform();
    return qtr_invert.map(poly);
}

QPointF Layer::worldToScreen(QPointF pt)
{
    getLayerTransform();
    return qtr_layer.map(pt);
}

QRectF Layer::worldToScreen(QRectF rect)
{
    getLayerTransform();
    return qtr_layer.mapRect(rect);
}

QLineF Layer::worldToScreen(QLineF line)
{
    QLineF aline;
    getLayerTransform();
    aline.setP1(qtr_layer.map(line.p1()));
    aline.setP2(qtr_layer.map(line.p2()));
    return aline;
}

QPolygonF Layer::worldToScreen(QPolygonF poly)
{
    getLayerTransform();
    QPolygonF poly2 = qtr_layer.map(poly);
    return poly2;
}

Circle Layer::worldToScreen(Circle c)
{
    getLayerTransform();
    QPointF cent = qtr_layer.map(c.centre);
    qreal radius = Transform::scalex(qtr_layer) * c.radius;
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
    if (config->showCenterDebug || config->showCenterMouse)
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
