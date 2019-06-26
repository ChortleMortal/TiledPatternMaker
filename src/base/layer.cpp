/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/layer.h"
#include "base/configuration.h"
#include "base/workspace.h"
#include "base/canvas.h"
#include "geometry/Point.h"

int Layer::refs = 0;

Layer::Layer(QString name) : QObject(), QGraphicsItemGroup()
{
    canvas  = Canvas::getInstance();

    Bounds b(-10.0,10.0,20.0,0.0);
    setBounds(b);

    layerTransform = nullptr;
    inverse   = nullptr;

    setRotateCenter(QPointF(400.0,400.0));  // default;

    connect(canvas, &Canvas::sig_deltaScale,    this, &Layer::slot_scale);
    connect(canvas, &Canvas::sig_deltaRotate,   this, &Layer::slot_rotate);
    connect(canvas, &Canvas::sig_deltaMoveV,    this, &Layer::slot_moveY);
    connect(canvas, &Canvas::sig_deltaMoveH,    this, &Layer::slot_moveX);

    this->name = name;

    refs++;
}

Layer::Layer(const Layer & layer) : QObject(), QGraphicsItemGroup()
{
    canvas  = Canvas::getInstance();

    setVisible(layer.isVisible());

    bounds = layer.bounds;
    deltas = layer.deltas;

    rotateCenter = layer.rotateCenter;

    layerTransform = nullptr;
    inverse   = nullptr;

    name = layer.name;

    refs++;
}

void Layer::addToGroup(QGraphicsItem * item)
{
    QGraphicsItemGroup::addToGroup(item);
}

void Layer::removeFromGroup(QGraphicsItem * item)
{
    QGraphicsItemGroup::removeFromGroup(item);
}

void Layer::forceUpdateLayer()
{
    layerTransform.reset();
    inverse.reset();
    canvas->update();
}

void Layer::forceRedraw()
{
    canvas->update();
}

TransformPtr Layer::getLayerTransform()
{
    computeLayerTransform();
    //qDebug().noquote() << "Layer transform:" << transform->toString();
    return layerTransform;
}

#define DACFIX

// DAC - this taprats method did not work when the view (canvas) was not square
// so fixes have been made
void Layer::computeLayerTransform()
{
    static bool debug = false;

    if( !layerTransform)
    {
        //qDebug() << "computeLayerTransform";
        Bounds b = bounds + deltas;
        if (debug) qDebug() << "left top width theta" << b.left << b.top << b.width << b.theta;

        QRectF rect  = canvas->sceneRect();
        qreal aspect = rect.width() / rect.height();
        qreal height = b.width / aspect;
        qreal scalex = rect.width()/b.width;

        Transform first  = Transform::translate(-b.left, - (b.top - height));
        Transform second = Transform::scale(scalex);
        Transform third  = Transform::translate(0.0,(rect.width() - rect.height())/2.0);
        Transform fourth = Transform::rotateAroundPoint(rotateCenter, b.theta);
        Transform full   = fourth.compose( third.compose(second.compose(first)));

        layerTransform = make_shared<Transform>(full);
        qDebug() << "Layer: " << name << layerTransform->toString();
        inverse        = make_shared<Transform>(full.invert());
    }
}

void Layer::setRotateCenter (QPointF pt)
{
    //qDebug() << "setRotateCenter=" << pt;
    rotateCenter = pt;
}

Bounds Layer::getAdjustedBounds()
{
    Bounds b = bounds + deltas;
    return b;
}

QPolygonF Layer::getBoundary()
{
    QSizeF d    = canvas->sceneRect().size();

    qreal wwidth  = d.width();
    qreal wheight = d.height();

    QPolygonF ret;

    computeLayerTransform();

    ret << inverse->apply( 0.0, 0.0 );
    ret << inverse->apply( wwidth, 0.0 );
    ret << inverse->apply( wwidth, wheight );
    ret << inverse->apply( 0.0, wheight );

    return ret;
}

QPointF Layer::screenToWorld(QPointF pt)
{
    if (layerTransform == nullptr)
    {
        computeLayerTransform();
    }

    return inverse->apply(pt);
}

QPointF Layer::screenToWorld(int x, int y)
{
    qreal xx = static_cast<qreal>(x);
    qreal yy = static_cast<qreal>(y);

    if (layerTransform == nullptr)
    {
        computeLayerTransform();
    }

    return inverse->apply(QPointF(xx, yy));
}

QPointF Layer::worldToScreen(QPointF pt)
{
    if (layerTransform == nullptr)
    {
        computeLayerTransform();
    }

    return layerTransform->apply(pt);
}

void Layer::slot_moveX(int amount)
{
    qreal delta  = static_cast<qreal>(amount) / 50.0;
    deltas.left -= delta;

    forceUpdateLayer();
}

void Layer::slot_moveY(int amount)
{
    qreal delta = static_cast<qreal>(amount) / 50.0;
    deltas.top += delta;

    forceUpdateLayer();
}

void Layer::slot_rotate(int amount)
{
    qreal delta   = qDegreesToRadians(static_cast<qreal>(amount));
    deltas.theta += delta;

    forceUpdateLayer();
}

void Layer::slot_scale(int amount)
{
    QRectF     d  = canvas->sceneRect();
    qreal aspect  = d.width() / d.height();
    qreal width   = bounds.width + deltas.width;
    qreal height  = width / aspect;

    double r      = static_cast<qreal>(amount) /100.0;
    deltas.left  -= width  * r * 0.5;
    deltas.top   += height * r * 0.5;
    deltas.width += width  * r;

    forceUpdateLayer();
}
