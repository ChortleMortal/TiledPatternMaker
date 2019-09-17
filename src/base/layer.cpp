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
#include "base/canvas.h"
#include "geometry/Transform.h"
#include "viewers/workspaceviewer.h"

int Layer::refs = 0;

Layer::Layer(QString name) : QObject(), QGraphicsItemGroup()
{
    canvas   = Canvas::getInstance();
    wsViewer = WorkspaceViewer::getInstance();
    config   = Configuration::getInstance();

    connect(canvas, &Canvas::sig_deltaScale,    this, &Layer::slot_scale);
    connect(canvas, &Canvas::sig_deltaRotate,   this, &Layer::slot_rotate);
    connect(canvas, &Canvas::sig_deltaMoveV,    this, &Layer::slot_moveY);
    connect(canvas, &Canvas::sig_deltaMoveH,    this, &Layer::slot_moveX);

    this->name = name;

    refs++;
}

Layer::Layer(const Layer & other) : QObject(), QGraphicsItemGroup()
{
    canvas  = Canvas::getInstance();

    setVisible(other.isVisible());

    layerXform    = other.layerXform;
    name         = other.name;

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
    layerT.reset();
    canvas->update();
}

void Layer::forceRedraw()
{
    canvas->update();
}

QTransform Layer::getLayerTransform()
{
    if(layerT.isIdentity())
    {
        computeLayerTransform();
    }
    //qDebug().noquote() << "Layer transform:" << Transform::toInfoString(layerT);
    return layerT;
}


// DAC - this taprats method did not work when the view (canvas) was not square
// This has been refactored
void Layer::computeLayerTransform()
{
    baseT             = wsViewer->getViewTransform(config->viewerType);
    layerT            = baseT * layerXform.computeTransform();
    invT              = layerT.inverted();

    qDebug().noquote() << "baseT :" << name << Transform::toInfoString(baseT);
    //qDebug().noquote() << "XForm :" << name << Transform::toInfoString(layerXform.getTransform());
    qDebug().noquote() << "layerT:" << name << Transform::toInfoString(layerT);
}

void Layer::setRotateCenter (QPointF pt)
{
    //qDebug() << "setRotateCenter=" << pt;
    layerXform.rotateCenter = pt;
}

QPointF Layer::getRotateCenter() const
{
    return layerXform.rotateCenter;
}

void Layer::setDeltas(Xform & xf)
{
    layerXform = xf;
}

Xform Layer::getDeltas()
{
    return layerXform;
}

QPointF Layer::screenToWorld(QPointF pt)
{
    getLayerTransform();
    return invT.map(pt);
}

QPointF Layer::screenToWorld(int x, int y)
{
    qreal xx = static_cast<qreal>(x);
    qreal yy = static_cast<qreal>(y);

    getLayerTransform();
    return invT.map(QPointF(xx, yy));
}

QPointF Layer::worldToScreen(QPointF pt)
{
    getLayerTransform();
    return layerT.map(pt);
}

QLineF Layer::worldToScreen(QLineF line)
{
    QLineF aline;
    getLayerTransform();
    aline.setP1(layerT.map(line.p1()));
    aline.setP2(layerT.map(line.p2()));
    return aline;
}

void Layer::slot_moveX(int amount)
{
    if (canvas->getMode() != MODE_TRANSFORM) return;

    layerXform.translateX += amount;

    forceUpdateLayer();
}

void Layer::slot_moveY(int amount)
{
    if (canvas->getMode() != MODE_TRANSFORM) return;

    layerXform.translateY += -amount;

    forceUpdateLayer();
}

void Layer::slot_rotate(int amount)
{
    if (canvas->getMode() != MODE_TRANSFORM) return;

    layerXform.rotation += qDegreesToRadians(static_cast<qreal>(amount));

    forceUpdateLayer();
}

void Layer::slot_scale(int amount)
{
    if (canvas->getMode() != MODE_TRANSFORM) return;

    layerXform.scale += static_cast<qreal>(amount)/100.0;

    forceUpdateLayer();
}
