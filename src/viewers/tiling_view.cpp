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
#include "misc/geo_graphics.h"
#include "geometry/edge.h"
#include "geometry/fill_region.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"
#include "viewers/viewcontrol.h"
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

TilingView::TilingView() : LayerController("TilingView")
{
}

void TilingView::paint(QPainter *painter)
{
    if (!tiling)
    {
        return;

    }
    qDebug() << "TilingView::paint";

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    layerPen = QPen(Qt::red,3);

    QTransform tr = getLayerTransform();
    //qDebug().noquote() << "Tiling View" << Transform::toInfoString(tr);
    GeoGraphics gg(painter,tr);
    draw(&gg);  // DAC - draw goes to receive which goes to draw placed tile
                // DAC - receive is really '2d draw one tile'

   drawCenter(painter);
}

void TilingView::draw(GeoGraphics *gg)
{
    FillRegion flood(tiling,tiling->getData().getFillData());
    Placements placements = flood.getPlacements(config->repeatMode);

    auto placed = tiling->getData().getPlacedTiles();
    for (auto T : placements)
    {
        gg->pushAndCompose(T);

        for (auto it = placed.begin(); it != placed.end(); it++)
        {
            PlacedTilePtr pf = *it;
            drawPlacedYTile(gg, pf);
        }

        gg->pop();
    }
}

void TilingView::drawPlacedYTile(GeoGraphics * g2d, PlacedTilePtr pf)
{
    //qDebug().noquote() << "PlacedFeat:" << pf->getTile().get() <<  "transform:" << Transform::toInfoString(t);

    TilePtr f  = pf->getTile();
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
            g2d->drawArc(edge->v1->pt,edge->v2->pt,edge->getArcCenter(),edge->isConvex(),layerPen);
        }
        else if (edge->getType() == EDGETYPE_CHORD)
        {
            g2d->drawChord(edge->v1->pt,edge->v2->pt,edge->getArcCenter(),edge->isConvex(),layerPen);
        }
    }
}

void TilingView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{ Q_UNUSED(spt); Q_UNUSED(btn); }

void TilingView::slot_mouseDragged(QPointF spt)
{ Q_UNUSED(spt); }

void TilingView::slot_mouseTranslate(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        if (tiling)
        {
            QTransform T = getFrameTransform();
            qreal scale = Transform::scalex(T);
            QPointF mpt = spt/scale;
            QTransform tt = QTransform::fromTranslate(mpt.x(),mpt.y());
            auto & placed = tiling->getData().getPlacedTiles();
            for (auto pfp : qAsConst(placed))
            {
                QTransform t = pfp->getTransform();
                t *= tt;
                pfp->setTransform(t);
            }
        }
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateX(xf.getTranslateX() + spt.x());
        xf.setTranslateY(xf.getTranslateY() + spt.y());
        setCanvasXform(xf);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_SELECTED))
    {
        if (isSelected())
        {
            Xform xf = getCanvasXform();
            xf.setTranslateX(xf.getTranslateX() + spt.x());
            xf.setTranslateY(xf.getTranslateY() + spt.y());
            setCanvasXform(xf);
        }
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
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        if (tiling)
        {
            qreal sc = 1.0 + delta;
            QTransform ts;
            ts.scale(sc,sc);

            auto & placed = tiling->getData().getPlacedTiles();
            for (auto pfp : qAsConst(placed))
            {
                QTransform t = pfp->getTransform();
                t *= ts;
                pfp->setTransform(t);
            }
        }
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setCanvasXform(xf);
    }
}

void TilingView::slot_wheel_rotate(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        if (tiling)
        {
            QTransform tr;
            tr.rotate(delta);
            Xform xf(tr);
            xf.setModelCenter(getCenterModelUnits());
            QTransform tr2 = xf.toQTransform(QTransform());

            auto & placed = tiling->getData().getPlacedTiles();
            for (auto pfp : qAsConst(placed))
            {
                QTransform t = pfp->getTransform();
                t *= tr2;
                pfp->setTransform(t);
            }
        }
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setCanvasXform(xf);
    }
}

void TilingView::slot_scale(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        if (tiling)
        {
            qreal scale = 1.0 + (0.01 * amount);
            auto & placed = tiling->getDataAccess(true).getPlacedTiles();
            for (auto pfp : qAsConst(placed))
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
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setCanvasXform(xf);
    }
}

void TilingView::slot_rotate(int amount)
{
    if (!view->isActiveLayer(this)) return;

    qDebug() << "TilingMaker::slot_rotate" << amount;
    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        if (tiling)
        {
            qreal qdelta = 0.01 * amount;
            auto & placed = tiling->getDataAccess(true).getPlacedTiles();
            for (auto pfp : qAsConst(placed))
            {
                QTransform t = pfp->getTransform();
                t *= QTransform().rotateRadians(qdelta);
                pfp->setTransform(t);
            }
        }
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setCanvasXform(xf);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_SELECTED))
    {
        if (isSelected())
        {
            Xform xf = getCanvasXform();
            xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
            setCanvasXform(xf);
        }
    }
}

void TilingView:: slot_moveX(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        if (tiling)
        {
            qreal qdelta = 0.01 * amount;
            auto & placed = tiling->getDataAccess(true).getPlacedTiles();
            for (auto pfp : qAsConst(placed))
            {
                QTransform t = pfp->getTransform();
                t *= QTransform::fromTranslate(qdelta,0.0);
                pfp->setTransform(t);
            }
        }
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setCanvasXform(xf);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_SELECTED))
    {
        if (isSelected())
        {
            Xform xf = getCanvasXform();
            xf.setTranslateX(xf.getTranslateX() + amount);
            setCanvasXform(xf);
        }
    }
}

void TilingView::slot_moveY(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        if (tiling)
        {
            qreal qdelta = 0.01 * amount;
            auto & placed = tiling->getDataAccess(true).getPlacedTiles();
            for (auto pfp : qAsConst(placed))
            {
                QTransform t = pfp->getTransform();
                t *= QTransform::fromTranslate(0.0,qdelta);
                pfp->setTransform(t);
            }
        }
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateY(xf.getTranslateY() + amount);
        setCanvasXform(xf);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_SELECTED))
    {
        if (isSelected())
        {
            Xform xf = getCanvasXform();
            xf.setTranslateY(xf.getTranslateY() + amount);
            setCanvasXform(xf);
        }
    }
}
