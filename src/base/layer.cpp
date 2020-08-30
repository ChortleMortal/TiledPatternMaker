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
#include "viewers/workspace_viewer.h"

int Layer::refs = 0;

Layer::Layer(QString name) : QObject()
{
    wsViewer = WorkspaceViewer::getInstance();
    config   = Configuration::getInstance();

    view = View::getInstance();
    connect(view, &View::sig_deltaScale,    this, &Layer::slot_scale);
    connect(view, &View::sig_deltaRotate,   this, &Layer::slot_rotate);
    connect(view, &View::sig_deltaMoveY,    this, &Layer::slot_moveY);
    connect(view, &View::sig_deltaMoveX,    this, &Layer::slot_moveX);

    connect(view, &View::sig_mousePressed,      this, &Layer::slot_mousePressed);
    connect(view, &View::sig_mouseTranslate,    this, &Layer::slot_mouseTranslate);
    connect(view, &View::sig_wheel_scale,       this, &Layer::slot_wheel_scale);
    connect(view, &View::sig_wheel_rotate,      this, &Layer::slot_wheel_rottate);
    connect(view, &View::sig_setCenter,         this, &Layer::slot_setCenter);

    this->name = name;

    visible = true;
    zlevel  = 0;

    refs++;
}

Layer::Layer(const Layer & other) : QObject()
{
    wsViewer = WorkspaceViewer::getInstance();
    config   = Configuration::getInstance();

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

    view = View::getInstance();

    connect(view, &View::sig_deltaScale,    this, &Layer::slot_scale,  Qt::UniqueConnection);
    connect(view, &View::sig_deltaRotate,   this, &Layer::slot_rotate, Qt::UniqueConnection);
    connect(view, &View::sig_deltaMoveY,    this, &Layer::slot_moveY,  Qt::UniqueConnection);
    connect(view, &View::sig_deltaMoveX,    this, &Layer::slot_moveX,  Qt::UniqueConnection);

    connect(view, &View::sig_mousePressed,      this, &Layer::slot_mousePressed);
    connect(view, &View::sig_mouseTranslate,              this, &Layer::slot_mouseTranslate);
    connect(view, &View::sig_wheel_scale,       this, &Layer::slot_wheel_scale);
    connect(view, &View::sig_wheel_rotate,      this, &Layer::slot_wheel_rottate);
    connect(view, &View::sig_setCenter,         this, &Layer::slot_setCenter);

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

void Layer::forceUpdateLayer()
{
    qtr_layer.reset();
    getLayerTransform();    // recalcs
    view->update();
}

void Layer::forceRedraw()
{
    view->update();
}

QTransform  Layer::getViewTransform()
{
    return wsViewer->getViewTransform(config->viewerType);
}

QTransform  Layer::getCanvasTransform()
{
    return xf_canvas.toQTransform(getViewTransform());
}

QTransform Layer::getLayerTransform()
{
    if (qtr_layer.isIdentity())
    {
        if (!xf_canvas.hasCenter)
        {
            QTransform t   = getViewTransform().inverted();
            QPointF center = t.map(view->rect().center());
            xf_canvas.setCenter(center);
        }
        // compute Transform
        computeLayerTransform();
    }
    //qDebug().noquote() << "Layer transform:" << Transform::toInfoString(qtr_layer);
    return qtr_layer;
}

void Layer::computeLayerTransform()
{
    qtr_layer   = getViewTransform() * getCanvasTransform();
    qtr_invert  = qtr_layer.inverted();
    //qDebug().noquote() << "qtr_layer:" << name << Transform::toInfoString(qtr_layer);
}

void Layer::setCenter (QPointF pt)
{
    qDebug() << "Layer::setCenter=" << pt;
    if (qtr_invert.isIdentity())
    {
        computeLayerTransform();
        qDebug() << "Layer::setCenter" << Transform::toInfoString(qtr_layer);
    }
    qDebug() << "Layer::setCenter" << Transform::toInfoString(qtr_layer);
    xf_canvas.setCenter(qtr_invert.map(pt));
}

void Layer::slot_setCenter(QPointF pt)
{
    qDebug() << "Layer::slot_setCenter=" << pt;
    if (qtr_invert.isIdentity())
    {
        computeLayerTransform();
        qDebug() << "Layer::slot_setCenter" << Transform::toInfoString(qtr_layer);
    }
    qDebug() << "Layer::slot_setCenter" << getName() << Transform::toInfoString(qtr_layer);
    xf_canvas.setCenter(qtr_invert.map(pt));
}

QPointF Layer::getCenter()
{
    if (qtr_invert.isIdentity())
    {
        computeLayerTransform();
        qDebug() << "Layer::getCenter()" << Transform::toInfoString(qtr_layer);
    }
    return qtr_layer.map(xf_canvas.getCenter());
}

void Layer::setCanvasXform(Xform & xf)
{
    xf_canvas = xf;
    forceUpdateLayer();
}

Xform Layer::getCanvasXform()
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
    if (config->kbdMode == KBD_MODE_XFORM_VIEW)
    {
        xf_canvas.setTranslateX(xf_canvas.getTranslateX() + pt.x());
        xf_canvas.setTranslateY(xf_canvas.getTranslateY() + pt.y());
        forceUpdateLayer();
    }
    else if (config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        BkgdImgPtr bip = wsViewer->getBkgdImage();
        if (bip)
        {
            Xform xf = bip->getXform();
            xf.setTranslateX(xf.getTranslateX() + pt.x());
            xf.setTranslateY(xf.getTranslateY() + pt.y());
            bip->setXform(xf);
            bip->bkgdTransformChanged(true);
        }
    }
}

void Layer::slot_moveX(int amount)
{
    if (config->kbdMode == KBD_MODE_XFORM_VIEW)
    {
        xf_canvas.setTranslateX(xf_canvas.getTranslateX() + amount);
        forceUpdateLayer();
    }
    else if (config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        BkgdImgPtr bip = wsViewer->getBkgdImage();
        if (bip)
        {
            Xform xf = bip->getXform();
            xf.setTranslateX(xf.getTranslateX() + amount);
            bip->setXform(xf);
            bip->bkgdTransformChanged(true);
        }
    }
}

void Layer::slot_moveY(int amount)
{
    if (config->kbdMode == KBD_MODE_XFORM_VIEW)
    {
        xf_canvas.setTranslateY(xf_canvas.getTranslateY() + amount);
        forceUpdateLayer();
    }
    else if (config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        BkgdImgPtr bip = wsViewer->getBkgdImage();
        if (bip)
        {
            Xform xf = bip->getXform();
            xf.setTranslateY(xf.getTranslateY() + amount);
            bip->setXform(xf);
            bip->bkgdTransformChanged(true);
        }
    }
}

void Layer::slot_rotate(int amount)
{
    if (config->kbdMode == KBD_MODE_XFORM_VIEW)
    {
        xf_canvas.setRotateRadians(xf_canvas.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        forceUpdateLayer();
    }
    else if (config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        BkgdImgPtr bip = wsViewer->getBkgdImage();
        if (bip)
        {
            Xform xf = bip->getXform();
            xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
            bip->setXform(xf);
            bip->bkgdTransformChanged(true);
        }
    }
}

void Layer::slot_wheel_rottate(qreal delta)
{
    if (config->kbdMode == KBD_MODE_XFORM_VIEW)
    {
        xf_canvas.setRotateDegrees(xf_canvas.getRotateDegrees() + delta);
        forceUpdateLayer();
    }
    else if (config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        BkgdImgPtr bip = wsViewer->getBkgdImage();
        if (bip)
        {
            Xform xf = bip->getXform();
            xf.setRotateDegrees(xf.getRotateDegrees() + delta);
            bip->setXform(xf);
            bip->bkgdTransformChanged(true);
        }
    }
}

void Layer::slot_scale(int amount)
{
    if (config->kbdMode == KBD_MODE_XFORM_VIEW)
    {
        xf_canvas.setScale(xf_canvas.getScale() + static_cast<qreal>(amount)/100.0);
        forceUpdateLayer();
    }
    else if (config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        BkgdImgPtr bip = wsViewer->getBkgdImage();
        if (bip)
        {
            Xform xf = bip->getXform();
            xf.setScale(xf.getScale() + static_cast<qreal>(amount)/100.0);
            bip->setXform(xf);
            bip->bkgdTransformChanged(true);
        }
    }
}

void Layer::slot_wheel_scale(qreal delta)
{
    if (config->kbdMode == KBD_MODE_XFORM_VIEW)
    {
        xf_canvas.setScale(xf_canvas.getScale() + delta);
        forceUpdateLayer();
    }
    else if (config->kbdMode == KBD_MODE_XFORM_BKGD)
    {
        BkgdImgPtr bip = wsViewer->getBkgdImage();
        if (bip)
        {
            Xform xf = bip->getXform();
            xf.setScale(xf.getScale() + delta);
            bip->setXform(xf);
            bip->bkgdTransformChanged(true);
        }
    }
}

void Layer::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    qDebug() << getName() << config->kbdMode;

    if (config->kbdMode != KBD_MODE_CENTER)
    {
        return;
    }
    if (btn != Qt::LeftButton)
    {
        return;
    }
    xf_canvas.setCenter(qtr_invert.map(spt));
    view->update();
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
        QPointF pt = getCenter();
        qDebug() << "Layer::drawCenter:" << pt;
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
