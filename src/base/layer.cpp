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
#include "geometry/transform.h"
#include "viewers/view.h"
#include "settings/configuration.h"
#include "viewers/viewcontrol.h"

int Layer::refs = 0;

Layer::Layer(QString name) : QObject()
{
    this->name = name;
    visible    = true;
    zlevel     = 0;
    connectSignals();
    refs++;
}

Layer::Layer(const Layer & other) : QObject()
{
    view    =  View::getInstance();
    config  = Configuration::getInstance();

    visible     = other.visible;
    xf_canvas   = other.xf_canvas;
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
    view    =  View::getInstance();
    config  = Configuration::getInstance();

    visible     = other->visible;
    xf_canvas   = other->xf_canvas;
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
    view    =  View::getInstance();
    config  = Configuration::getInstance();

    connect(view, &View::sig_mousePressed,          this, &Layer::slot_mousePressed);
    connect(view, &View::sig_mouseDragged,          this, &Layer::slot_mouseDragged);
    connect(view, &View::sig_mouseTranslate,        this, &Layer::slot_mouseTranslate);
    connect(view, &View::sig_mouseMoved,            this, &Layer::slot_mouseMoved);
    connect(view, &View::sig_mouseReleased,         this, &Layer::slot_mouseReleased);
    connect(view, &View::sig_mouseDoublePressed,    this, &Layer::slot_mouseDoublePressed);

    connect(view, &View::sig_wheel_scale,       this, &Layer::slot_wheel_scale);
    connect(view, &View::sig_wheel_rotate,      this, &Layer::slot_wheel_rotate);

    connect(view, &View::sig_deltaScale,    this, &Layer::slot_scale,  Qt::UniqueConnection);
    connect(view, &View::sig_deltaRotate,   this, &Layer::slot_rotate, Qt::UniqueConnection);
    connect(view, &View::sig_deltaMoveX,    this, &Layer::slot_moveX,  Qt::UniqueConnection);
    connect(view, &View::sig_deltaMoveY,    this, &Layer::slot_moveY,  Qt::UniqueConnection);

    ViewControl * vcontrol = ViewControl::getInstance();
    connect(this, &Layer::sig_refreshView, vcontrol, &ViewControl::slot_refreshView);

}

void Layer::paint(QPainter * painter)
{
    qDebug().noquote() << "Layer::paint" << getName() <<": subs =" << subLayers.count();

    std::stable_sort(subLayers.begin(), subLayers.end(), sortByZlevel);

    painter->save();
    painter->translate(getLoc());
    qreal rot = xf_canvas.getRotateDegrees();
    painter->rotate(rot);

    for (auto& layer : subLayers)
    {
        layer->paint(painter);
    }

    painter->restore();
}

bool Layer::sortByZlevel(LayerPtr s1, LayerPtr s2)
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
    LayerPtr lp = config->selectedLayer.lock();
    if (lp)
    {
        Layer * layer = lp.get();
        if (layer == this)
        {
            return true;
        }
    }
    return false;
}

QTransform  Layer::getFrameTransform()
{
    QTransform t = view->frameSettings.getTransform(config->getViewerType());

    //qDebug().noquote() << "Frame transform:" << Transform::toInfoString(t);

    return t;
}

QTransform  Layer::getCanvasTransform()
{
    QTransform t =  xf_canvas.toQTransform(getFrameTransform());

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
    //qDebug().noquote() << "frame:" << Transform::toInfoString(getFrameTransform()) << "canvas" << Transform::toInfoString(getCanvasTransform());
    qtr_layer   = getFrameTransform() * getCanvasTransform();
    qtr_invert  = qtr_layer.inverted();
    //qDebug().noquote() << "computed qtr_layer:" << name << Transform::toInfoString(qtr_layer);
}

void Layer::setCenterScreenUnits(QPointF spt)
{
    if (qtr_invert.isIdentity())
    {
        computeLayerTransform();
    }

    // save in model units
    QPointF mpt = qtr_invert.map(spt);
    xf_canvas.setCenter(mpt);
    computeLayerTransform();

    QPointF diff = spt - getCenterScreenUnits();
    qDebug() << "diff" << diff;
    xf_canvas.setTranslateX(xf_canvas.getTranslateX() + diff.x());
    xf_canvas.setTranslateY(xf_canvas.getTranslateY() + diff.y());
    computeLayerTransform();

    emit sig_center();
}

QPointF Layer::getCenterScreenUnits()
{
    if (qtr_invert.isIdentity())
    {
        computeLayerTransform();
        //qDebug() << "Layer::getCenter()" << Transform::toInfoString(qtr_layer);
    }
    return qtr_layer.map(xf_canvas.getCenter());
}

QPointF Layer::getCenterModelUnits()
{
    return xf_canvas.getCenter();
}

void Layer::setCanvasXform(const Xform & xf)
{
    xf_canvas = xf;
    forceLayerRecalc();
}

const Xform & Layer::getCanvasXform()
{
    return xf_canvas;
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


void Layer::setLoc(QPointF loc)
{
    //qDebug() << name << "pos=" << pos << "new" << loc;
    pos += loc;
}

void Layer::setZValue(int z)
{
    zlevel = z;
}

void  Layer::drawCenter(QPainter * painter)
{
    if (config->showCenterDebug || config->showCenterMouse)
    {
        QPointF pt = getCenterScreenUnits();
        //qDebug() << "Layer::drawCenter:" << pt;
        qreal len = 13;
        QColor green(Qt::green);
        painter->setPen(QPen(green));
        green.setAlpha(128);
        painter->setBrush(QBrush(green));
        painter->drawEllipse(pt,len,len);
        painter->setPen(QPen(Qt::blue));
        painter->drawLine(QPointF(pt.x()-len,pt.y()),QPointF(pt.x()+len,pt.y()));
        painter->drawLine(QPointF(pt.x(),pt.y()-len),QPointF(pt.x(),pt.y()+len));
    }

}
