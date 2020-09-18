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
#include "base/view.h"
#include "geometry/transform.h"
#include "base/workspace.h"

int Layer::refs = 0;

Layer::Layer(QString name, eLayerType ltype) : QObject()
{
    workspace = Workspace::getInstance();
    config    = Configuration::getInstance();

    connect(workspace, &View::sig_deltaScale,    this, &Layer::slot_scale);
    connect(workspace, &View::sig_deltaRotate,   this, &Layer::slot_rotate);
    connect(workspace, &View::sig_deltaMoveY,    this, &Layer::slot_moveY);
    connect(workspace, &View::sig_deltaMoveX,    this, &Layer::slot_moveX);

    connect(workspace, &View::sig_mousePressed,      this, &Layer::slot_mousePressed);
    connect(workspace, &View::sig_mouseTranslate,    this, &Layer::slot_mouseTranslate);
    connect(workspace, &View::sig_wheel_scale,       this, &Layer::slot_wheel_scale);
    connect(workspace, &View::sig_wheel_rotate,      this, &Layer::slot_wheel_rotate);
    connect(workspace, &View::sig_setCenter,         this, &Layer::slot_setCenterScreen);

    this->name = name;
    layerType  = ltype;
    visible = true;
    zlevel  = 0;

    refs++;
}

Layer::Layer(const Layer & other) : QObject()
{
    workspace = Workspace::getInstance();
    config   = Configuration::getInstance();

    layerType   = other.layerType;
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

    connect(workspace, &View::sig_deltaScale,    this, &Layer::slot_scale,  Qt::UniqueConnection);
    connect(workspace, &View::sig_deltaRotate,   this, &Layer::slot_rotate, Qt::UniqueConnection);
    connect(workspace, &View::sig_deltaMoveY,    this, &Layer::slot_moveY,  Qt::UniqueConnection);
    connect(workspace, &View::sig_deltaMoveX,    this, &Layer::slot_moveX,  Qt::UniqueConnection);

    connect(workspace, &View::sig_mousePressed,      this, &Layer::slot_mousePressed);
    connect(workspace, &View::sig_mouseTranslate,    this, &Layer::slot_mouseTranslate);
    connect(workspace, &View::sig_wheel_scale,       this, &Layer::slot_wheel_scale);
    connect(workspace, &View::sig_wheel_rotate,      this, &Layer::slot_wheel_rotate);
    connect(workspace, &View::sig_setCenter,         this, &Layer::slot_setCenterScreen);

    refs++;
}

Layer::~Layer()
{
    refs--;
}

void Layer::paint(QPainter * painter)
{
    qDebug().noquote() << "Layer::paint" << getName() <<": subs =" << subLayers.count();

    std::stable_sort(subLayers.begin(), subLayers.end(), sortByZlevel);

    painter->save();
    painter->translate(getLoc());
    qreal rot = xf_canvas.getRotateDegrees();
    painter->rotate(rot);

    for (auto layer : subLayers)
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
        workspace->update();
    }
}

void Layer::forceRedraw()
{
    workspace->update();
}

QTransform  Layer::getFrameTransform()
{
    return workspace->getFrameTransform(config->viewerType);
}

QTransform  Layer::getCanvasTransform()
{
    return xf_canvas.toQTransform(getFrameTransform());
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
    qtr_layer   = getFrameTransform() * getCanvasTransform();
    qtr_invert  = qtr_layer.inverted();
    //qDebug().noquote() << "qtr_layer:" << name << Transform::toInfoString(qtr_layer);
}

void Layer::setCenterScreen(QPointF spt)
{
    qDebug().noquote() << "Layer::setCenterScreen new=" << spt << "old=" << getCenterScreen()  << "diff = " << (spt - getCenterScreen());
    if (qtr_invert.isIdentity())
    {
        computeLayerTransform();
        qDebug().noquote() << "Layer::setCenterScreen" << Transform::toInfoString(qtr_layer);
    }
    qDebug().noquote() << "Layer::setCenterScreen" << getName() << Transform::toInfoString(qtr_layer);

    xf_canvas.setCenter(qtr_invert.map(spt));
    computeLayerTransform();

    QPointF diff = spt - getCenterScreen();
    xf_canvas.setTranslateX(xf_canvas.getTranslateX() + diff.x());
    xf_canvas.setTranslateY(xf_canvas.getTranslateY() + diff.y());
    computeLayerTransform();

    emit sig_center();
}

void Layer::setCenterModel(QPointF mpt)
{
    xf_canvas.setCenter(mpt);
    emit sig_center();

}
void Layer::slot_setCenterScreen(QPointF spt)
{
    setCenterScreen(spt);
}

QPointF Layer::getCenterScreen()
{
    if (qtr_invert.isIdentity())
    {
        computeLayerTransform();
        //qDebug() << "Layer::getCenter()" << Transform::toInfoString(qtr_layer);
    }
    return qtr_layer.map(xf_canvas.getCenter());
}

QPointF Layer::getCenterModel()
{
    return xf_canvas.getCenter();
}

void Layer::setCanvasXform(const Xform & xf)
{
    xf_canvas = xf;
    forceLayerRecalc();
}

void Layer::updateCanvasXform(const Xform & xf)
{
    // does not touch the center
    xf_canvas.update(xf);
    forceLayerRecalc();
}

const Xform & Layer::getCanvasXform()
{
    return xf_canvas;
}

QPointF Layer::screenToWorld(QPointF pt)
{
    getLayerTransform();
    return qtr_invert.map(pt);
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

QLineF Layer::worldToScreen(QLineF line)
{
    QLineF aline;
    getLayerTransform();
    aline.setP1(qtr_layer.map(line.p1()));
    aline.setP2(qtr_layer.map(line.p2()));
    return aline;
}

void Layer::slot_mouseTranslate(QPointF pt)
{
    if (   (config->kbdMode == KBD_MODE_XFORM_VIEW && layerType != LTYPE_BACKGROUND)
        || (config->kbdMode == KBD_MODE_XFORM_BKGD && layerType == LTYPE_BACKGROUND)
        || (config->viewerType == VIEW_DESIGN))
    {
        xf_canvas.setTranslateX(xf_canvas.getTranslateX() + pt.x());
        xf_canvas.setTranslateY(xf_canvas.getTranslateY() + pt.y());
        forceLayerRecalc();
    }
}

void Layer::slot_moveX(int amount)
{
    if (   (config->kbdMode == KBD_MODE_XFORM_VIEW && layerType != LTYPE_BACKGROUND)
        || (config->kbdMode == KBD_MODE_XFORM_BKGD && layerType == LTYPE_BACKGROUND)
        || (config->viewerType == VIEW_DESIGN))
    {
        xf_canvas.setTranslateX(xf_canvas.getTranslateX() + amount);
        forceLayerRecalc();
    }
}

void Layer::slot_moveY(int amount)
{
    if (   (config->kbdMode == KBD_MODE_XFORM_VIEW && layerType != LTYPE_BACKGROUND)
        || (config->kbdMode == KBD_MODE_XFORM_BKGD && layerType == LTYPE_BACKGROUND)
        || (config->viewerType == VIEW_DESIGN))
    {
        xf_canvas.setTranslateY(xf_canvas.getTranslateY() + amount);
        forceLayerRecalc();
    }
}

void Layer::slot_rotate(int amount)
{
    if (   (config->kbdMode == KBD_MODE_XFORM_VIEW && layerType != LTYPE_BACKGROUND)
         || (config->kbdMode == KBD_MODE_XFORM_BKGD && layerType == LTYPE_BACKGROUND)
         || (config->viewerType == VIEW_DESIGN))
    {
        xf_canvas.setRotateRadians(xf_canvas.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        forceLayerRecalc();
    }
}

void Layer::slot_wheel_rotate(qreal delta)
{
    if (   (config->kbdMode == KBD_MODE_XFORM_VIEW && layerType != LTYPE_BACKGROUND)
        || (config->kbdMode == KBD_MODE_XFORM_BKGD && layerType == LTYPE_BACKGROUND)
        || (config->viewerType == VIEW_DESIGN))
    {
        xf_canvas.setRotateDegrees(xf_canvas.getRotateDegrees() + delta);
        forceLayerRecalc();
    }
}

void Layer::slot_scale(int amount)
{
    if (   (config->kbdMode == KBD_MODE_XFORM_VIEW && layerType != LTYPE_BACKGROUND)
        || (config->kbdMode == KBD_MODE_XFORM_BKGD && layerType == LTYPE_BACKGROUND)
        || (config->viewerType == VIEW_DESIGN))
    {
        xf_canvas.setScale(xf_canvas.getScale() + static_cast<qreal>(amount)/100.0);
        forceLayerRecalc();
    }
}

void Layer::slot_wheel_scale(qreal delta)
{
    if (   (config->kbdMode == KBD_MODE_XFORM_VIEW && layerType != LTYPE_BACKGROUND)
        || (config->kbdMode == KBD_MODE_XFORM_BKGD && layerType == LTYPE_BACKGROUND)
        || (config->viewerType == VIEW_DESIGN))
    {
        xf_canvas.setScale(xf_canvas.getScale() + delta);
        forceLayerRecalc();
    }
}

void Layer::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    qDebug() << getName() << config->kbdMode;

    if (config->kbdMode == KBD_MODE_CENTER && btn == Qt::LeftButton)
    {
        setCenterScreen(spt);
    }
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
    if (config->showCenter)
    {
        QPointF pt = getCenterScreen();
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
