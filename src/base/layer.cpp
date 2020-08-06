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
#include "base/view.h"
#include "geometry/transform.h"
#include "viewers/workspace_viewer.h"

int Layer::refs = 0;

Layer::Layer(QString name) : QObject()
{
    canvas   = Canvas::getInstance();
    wsViewer = WorkspaceViewer::getInstance();
    config   = Configuration::getInstance();

    connect(canvas, &Canvas::sig_deltaScale,    this, &Layer::slot_scale);
    connect(canvas, &Canvas::sig_deltaRotate,   this, &Layer::slot_rotate);
    connect(canvas, &Canvas::sig_deltaMoveY,    this, &Layer::slot_moveY);
    connect(canvas, &Canvas::sig_deltaMoveX,    this, &Layer::slot_moveX);

    view = View::getInstance();
    connect(view, &View::sig_mousePressed,      this, &Layer::slot_mousePressed);

    this->name = name;

    visible = true;
    zlevel  = 0;

    refs++;
}

Layer::Layer(const Layer & other) : QObject()
{
    canvas   = Canvas::getInstance();
    wsViewer = WorkspaceViewer::getInstance();
    config   = Configuration::getInstance();

    visible     = other.visible;
    xf_canvas   = other.xf_canvas;
    name        = other.name;
    qtr_view    = other.qtr_view;
    qtr_layer   = other.qtr_layer;
    qtr_invert  = other.qtr_invert;
    layerPen    = other.layerPen;
    name        = other.name;
    visible     = other.visible;
    subLayers   = other.subLayers;
    zlevel      = other.zlevel;

    connect(canvas, &Canvas::sig_deltaScale,    this, &Layer::slot_scale,  Qt::UniqueConnection);
    connect(canvas, &Canvas::sig_deltaRotate,   this, &Layer::slot_rotate, Qt::UniqueConnection);
    connect(canvas, &Canvas::sig_deltaMoveY,    this, &Layer::slot_moveY,  Qt::UniqueConnection);
    connect(canvas, &Canvas::sig_deltaMoveX,    this, &Layer::slot_moveX,  Qt::UniqueConnection);

    view = View::getInstance();
    connect(view, &View::sig_mousePressed,      this, &Layer::slot_mousePressed);

    refs++;
}

Layer::~Layer()
{
    refs--;
}

void Layer::paint(QPainter * painter)
{
    //qDebug() << "Layer paint subs =" << subLayers.count();

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
    view->update();
}

void Layer::forceRedraw()
{
    view->update();
}

// FIXME - should be a push not a pull
QTransform Layer::getLayerTransform()
{
    if (qtr_layer.isIdentity())
    {
        if (!xf_canvas.hasCenter)
        {
            QTransform bT = wsViewer->getViewTransform(config->viewerType);
            QTransform iT = bT.inverted();

            QPointF center = view->rect().center();    // TODO - this should maybe be the canvas/design center
               center = iT.map(center);
               xf_canvas.setCenter(center);
        }
        // compute Transform
        computeLayerTransform();
    }
    //qDebug().noquote() << "Layer transform:" << Transform::toInfoString(qtr_layer);
    return qtr_layer;
}

// FIXME - should be a push not a pull
QTransform Layer::getViewTransform()
{
    if (qtr_layer.isIdentity())
    {
        getLayerTransform();
    }
    //qDebug().noquote() << "View transform:" << Transform::toInfoString(qtr_view);
    return qtr_view;
}

// FIXME - should be a push not a pull
// DAC - this taprats method did not work when the view (canvas) was not square
// This has been refactored
void Layer::computeLayerTransform()
{
    QTransform qtr_canvas;
    qtr_canvas  = xf_canvas.toQTransform(qtr_view);                 // qtr_view used to map the center
    qtr_view    = wsViewer->getViewTransform(config->viewerType);    // FIXME - maybe should be the current view
    qtr_layer   = qtr_view * qtr_canvas;
    qtr_invert  = qtr_layer.inverted();

    //qDebug().noquote() << "qtr_view:" << name << Transform::toInfoString(qtr_view);
    //qDebug().noquote() << "layerXT :" << name << Transform::toInfoString(qtr_canvas);
    //qDebug().noquote() << "layerT  :" << name << Transform::toInfoString(qtr_layer);
}

void Layer::setCenter (QPointF pt)
{
    //qDebug() << "setRotateCenter=" << pt;
    xf_canvas.setCenter(qtr_invert.map(pt));
}

QPointF Layer::getCenter()
{
    return qtr_layer.map(xf_canvas.getCenter());
}

void Layer::setCanvasXform(Xform & xf)
{
    xf_canvas = xf;
    qtr_layer.reset();
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

void Layer::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
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
