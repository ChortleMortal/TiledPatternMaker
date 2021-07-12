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

////////////////////////////////////////////////////////////////////////////
//
// TilingViewer.java
//
// TilingViewer gets a Tiling instance and displays the tiling in a
// view window.  It always draws as much of the tiling as necessary to
// fill the whole view area, using the modified polygon-filling algorithm
// in geometry.FillRegion.
//
// TilingViewer also has a test application that accepts the name of
// a built-in tiling on the command line or the specification of a tiling
// on System.in and displays that tiling.  Access it using
// 		java tile.TilingViewer

#include "viewers/tiling_view.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "tile/tiling.h"
#include "tile/placed_feature.h"
#include "geometry/transform.h"
#include "viewers/view.h"
#include "base/geo_graphics.h"
#include "settings/configuration.h"

TilingViewPtr TilingView::spThis;

TilingViewPtr TilingView::getSharedInstance()
{
    if (!spThis)
    {
        spThis = std::make_shared<TilingView>();
    }
    return spThis;
}

TilingView::TilingView() : Layer("TilingView")
{
}

void TilingView::paint(QPainter *painter)
{
    if (!tiling)
    {
        return;

    }
    qDebug() << "TilingView::paint";

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);
    layerPen = QPen(Qt::red,3);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);  // DAC - draw goes to receive which goes to draw placed feature
                // DAC - receive is really '2draw one tile'

   drawCenter(painter);
}

void TilingView::draw(GeoGraphics *g2d )
{
    fill(g2d);
}

// The FillRegion algorithm relies on a callback to send information
// about which translational units to draw.  Here, we get T1 and T2,
// build a transform and draw the PlacedFeatures in the translational
// unit at that location.
void TilingView::receive(GeoGraphics * gg, int h, int v )
{
    //qDebug() << "fill TilingView::receive:"  << h << v;
    QPointF   pt = (tiling->getTrans1() * static_cast<qreal>(h)) + (tiling->getTrans2() * static_cast<qreal>(v));
    QTransform T = QTransform::fromTranslate(pt.x(),pt.y());

    gg->pushAndCompose(T);

    for (auto it = tiling->getPlacedFeatures().begin(); it != tiling->getPlacedFeatures().end(); it++)
    {
        PlacedFeaturePtr pf = *it;
        drawPlacedFeature(gg, pf);
    }

    gg->pop();
}

void TilingView::drawPlacedFeature(GeoGraphics * g2d, PlacedFeaturePtr pf)
{
    //qDebug().noquote() << "PlacedFeat:" << pf->getFeature().get() <<  "transform:" << Transform::toInfoString(t);

    FeaturePtr f  = pf->getFeature();
    EdgePoly ep   = pf->getPlacedEdgePoly();

    for (auto it = ep.begin(); it != ep.end(); it++)
    {
        EdgePtr edge = *it;
        if (edge->getType() == EDGETYPE_LINE)
        {
            g2d->drawLine(edge->getLine(),layerPen);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            g2d->drawChord(edge->v1->pt,edge->v2->pt,edge->getArcCenter(),layerPen,QBrush(),edge->isConvex());
        }
    }
}

void TilingView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    if (view->getMouseMode() == MOUSE_MODE_CENTER && btn == Qt::LeftButton)
    {
        setCenterScreenUnits(spt);
        forceLayerRecalc();
        emit sig_refreshView();
    }
}

void TilingView::slot_mouseDragged(QPointF spt)
{ Q_UNUSED(spt); }

void TilingView::slot_mouseTranslate(QPointF spt)
{
    switch (Layer::config->kbdMode)
    {
    case KBD_MODE_XFORM_TILING:
        if (tiling)
        {
            QTransform T = getFrameTransform();
            qreal scale = Transform::scalex(T);
            QPointF mpt = spt/scale;
            QTransform tt = QTransform::fromTranslate(mpt.x(),mpt.y());
            for (auto pfp : qAsConst(tiling->getPlacedFeatures()))
            {
                QTransform t = pfp->getTransform();
                t *= tt;
                pfp->setTransform(t);
            }
        }
       break;

    case KBD_MODE_XFORM_VIEW:
        xf_canvas.setTranslateX(xf_canvas.getTranslateX() + spt.x());
        xf_canvas.setTranslateY(xf_canvas.getTranslateY() + spt.y());
        forceLayerRecalc();
        break;

    case KBD_MODE_XFORM_SELECTED:
        if (isSelected())
        {
            xf_canvas.setTranslateX(xf_canvas.getTranslateX() + spt.x());
            xf_canvas.setTranslateY(xf_canvas.getTranslateY() + spt.y());
            forceLayerRecalc();
        }
        break;

    default:
        break;
    }
}

void TilingView::slot_mouseMoved(QPointF spt)
{ Q_UNUSED(spt); }
void TilingView::slot_mouseReleased(QPointF spt)
{ Q_UNUSED(spt); }
void TilingView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

void TilingView::slot_wheel_scale(qreal delta)
{
    switch (Layer::config->kbdMode)
    {
    case KBD_MODE_XFORM_TILING:
        if (tiling)
        {
            qreal sc = 1.0 + delta;
            QTransform ts;
            ts.scale(sc,sc);
            for (auto pfp : qAsConst(tiling->getPlacedFeatures()))
            {
                QTransform t = pfp->getTransform();
                t *= ts;
                pfp->setTransform(t);
            }
        }
        break;

    case KBD_MODE_XFORM_VIEW:
        xf_canvas.setScale(xf_canvas.getScale() + delta);
        forceLayerRecalc();
        break;

    case KBD_MODE_XFORM_SELECTED:
        if (isSelected())
        {
            xf_canvas.setScale(xf_canvas.getScale() + delta);
            forceLayerRecalc();
        }
        break;

    default:
        break;
    }
}

void TilingView::slot_wheel_rotate(qreal delta)
{
    switch (Layer::config->kbdMode)
    {
    case KBD_MODE_XFORM_TILING:
        if (tiling)
        {
            QTransform tr;
            tr.rotate(delta);
            Xform xf(tr);
            xf.setCenter(getCenterModelUnits());
            QTransform tr2 = xf.toQTransform(QTransform());
            qDebug() << Transform::toInfoString(tr);
            qDebug() << Transform::toInfoString(tr2);
            for (auto pfp : qAsConst(tiling->getPlacedFeatures()))
            {
                QTransform t = pfp->getTransform();
                t *= tr2;
                pfp->setTransform(t);
            }
        }
        break;

    case KBD_MODE_XFORM_VIEW:
        xf_canvas.setRotateDegrees(xf_canvas.getRotateDegrees() + delta);
        forceLayerRecalc();
        break;

    case KBD_MODE_XFORM_SELECTED:
        if (isSelected())
        {
            xf_canvas.setRotateDegrees(xf_canvas.getRotateDegrees() + delta);
            forceLayerRecalc();
        }
        break;

    default:
        break;
    }
}

void TilingView::slot_scale(int amount)
{
    switch (Layer::config->kbdMode)
    {
    case KBD_MODE_XFORM_TILING:
        if (tiling)
        {
            qreal scale = 1.0 + (0.01 * amount);
            for (auto pfp : qAsConst(tiling->getPlacedFeatures()))
            {
                QTransform t = pfp->getTransform();
                qDebug() << "t0" << Transform::toInfoString(t);
                QTransform t1 = t.scale(scale,scale);

                t = pfp->getTransform();
                QTransform t2 = t *QTransform::fromScale(scale,scale);

                qDebug() << "t1" << Transform::toInfoString(t1);
                qDebug() << "t2" << Transform::toInfoString(t2);

                t = pfp->getTransform();
                // scales position too
                t *= QTransform::fromScale(scale,scale);
                pfp->setTransform(t);
            }
        }
        break;

    case KBD_MODE_XFORM_VIEW:
        xf_canvas.setScale(xf_canvas.getScale() + static_cast<qreal>(amount)/100.0);
        forceLayerRecalc();
        break;

    case KBD_MODE_XFORM_SELECTED:
        if (isSelected())
        {
            xf_canvas.setScale(xf_canvas.getScale() + static_cast<qreal>(amount)/100.0);
            forceLayerRecalc();
        }
        break;

    default:
        break;
    }
}

void TilingView::slot_rotate(int amount)
{
    qDebug() << "TilingMaker::slot_rotate" << amount;
    switch (Layer::config->kbdMode)
    {
    case KBD_MODE_XFORM_TILING:
        if (tiling)
        {
            qreal qdelta = 0.01 * amount;
            for (auto pfp : qAsConst(tiling->getPlacedFeatures()))
            {
                QTransform t = pfp->getTransform();
                t *= QTransform().rotateRadians(qdelta);
                pfp->setTransform(t);
            }
        }
        break;

    case KBD_MODE_XFORM_VIEW:
        xf_canvas.setRotateRadians(xf_canvas.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        forceLayerRecalc();
        break;

    case KBD_MODE_XFORM_SELECTED:
        if (isSelected())
        {
            xf_canvas.setRotateRadians(xf_canvas.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
            forceLayerRecalc();
        }
        break;

    default:
        break;
    }
}

void TilingView:: slot_moveX(int amount)
{
    switch (Layer::config->kbdMode)
    {
    case  KBD_MODE_XFORM_TILING:
        if (tiling)
        {
            qreal qdelta = 0.01 * amount;
            for (auto pfp : qAsConst(tiling->getPlacedFeatures()))
            {
                QTransform t = pfp->getTransform();
                t *= QTransform::fromTranslate(qdelta,0.0);
                pfp->setTransform(t);
            }
        }
        break;

    case KBD_MODE_XFORM_VIEW:
        xf_canvas.setTranslateX(xf_canvas.getTranslateX() + amount);
        forceLayerRecalc();
        break;

    case KBD_MODE_XFORM_SELECTED:
        if (isSelected())
        {
            xf_canvas.setTranslateX(xf_canvas.getTranslateX() + amount);
            forceLayerRecalc();
        }
        break;

    default:
        break;
    }
}

void TilingView::slot_moveY(int amount)
{
    switch (Layer::config->kbdMode)
    {
    case KBD_MODE_XFORM_TILING:
        if (tiling)
        {
            qreal qdelta = 0.01 * amount;
            for (auto pfp : qAsConst(tiling->getPlacedFeatures()))
            {
                QTransform t = pfp->getTransform();
                t *= QTransform::fromTranslate(0.0,qdelta);
                pfp->setTransform(t);
            }
        }
        break;

    case KBD_MODE_XFORM_VIEW:
        xf_canvas.setTranslateY(xf_canvas.getTranslateY() + amount);
        forceLayerRecalc();
        break;

    case KBD_MODE_XFORM_SELECTED:
        if (isSelected())
        {
            xf_canvas.setTranslateY(xf_canvas.getTranslateY() + amount);
            forceLayerRecalc();
        }
        break;

    default:
        break;
    }
}
