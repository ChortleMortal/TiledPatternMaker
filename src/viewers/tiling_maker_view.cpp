////////////////////////////////////////////////////////////////////////////
//
// FeatureView.java
//
// It's unlikely this file will get used in the applet.  A TilingMakerView
// is a special kind of GeoView that assumes a subclass will maintain
// a collection of Tiles.  It knows how to draw Tiles quickly,
// and provides a bunch of services to subclasses for mouse-based
// interaction with features.

#include <QDebug>
#include <QApplication>
#include <QMenu>

#include "viewers/tiling_maker_view.h"
#include "enums/etilingmakermousemode.h"
#include "geometry/edge.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "misc/utilities.h"
#include "makers/tiling_maker/tile_selection.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/tiling_maker/tiling_mouseactions.h"
#include "misc/geo_graphics.h"
#include "settings/configuration.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"
#include "tile/tile.h"
#include "viewers/grid_view.h"
#include "viewers/viewcontrol.h"

using std::make_shared;

TilingMakerView * TilingMakerView::mpThis = nullptr;

TilingMakerView * TilingMakerView::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new TilingMakerView;
    }
    return mpThis;
}

void TilingMakerView::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

TilingMakerView::TilingMakerView() : LayerController("TilingMakerView")
{
    config      = Configuration::getInstance();
    debugMouse  = false;
    _hideTiling = false;
}

TilingMakerView::~TilingMakerView()
{
#ifdef EXPLICIT_DESTRUCTOR
    allplacedTiles.clear();
    in_tiling.clear();
#endif
}

void  TilingMakerView::setTiling(TilingPtr tiling)
{
    allPlacedTiles = tiling->getInTiling();

    // at this time allplacedTile and in_tiling are the same
    if (allPlacedTiles.size() > 0)
    {
        PlacedTilePtr pf = allPlacedTiles.first();
        QTransform T     = pf->getTransform();
        trans_origin     = T.map(pf->getTile()->getCenter());
    }

    wTiling = tiling;
}

void TilingMakerView::clearViewData()
{
    allPlacedTiles.clear();
    for (auto m : wMeasurements)
    {
        delete m;
    }
    wMeasurements.clear();
    tileSelector.reset();
    editPlacedTile.reset();
    mouse_interaction.reset();
}

void TilingMakerView::paint(QPainter *painter)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QColor bcolor = view->getViewSettings().getBkgdColor(VIEW_TILING_MAKER);
    lineColor = (bcolor == QColor(Qt::white) ? QColor(Qt::black) : QColor(Qt::white));
    layerPen = QPen(lineColor,3);

    QTransform tr = getLayerTransform();
    //qDebug() << "TilingMakerView::paint viewT="  << Transform::toInfoString(tr);

    GeoGraphics gg(painter,tr);
    draw(&gg);
    
    drawLayerModelCenter(painter);
}

void TilingMakerView::draw( GeoGraphics * g2d )
{
    //qDebug() << "TilingMakerView::draw";
    auto tiling = wTiling.lock();
    if (tiling && !_hideTiling && !editPlacedTile)
    {
        drawTiling(g2d);

        // draw translation vectors
        QPointF  tp1 =  tiling->getData().getTrans1();
        QPointF  tp2 =  tiling->getData().getTrans2();
        if (!tp1.isNull() && !tp2.isNull())
        {
            QLineF visibleT1;
            visibleT1.setP1(trans_origin);
            visibleT1.setP2(trans_origin + tp1);

            QLineF visibleT2;
            visibleT2.setP1(trans_origin);
            visibleT2.setP2(trans_origin + tp2);

            drawTranslationVectors(g2d,visibleT1.p1(),visibleT1.p2(),visibleT2.p1(),visibleT2.p2());

            QPen apen(Qt::red,3);
            for (const auto & line : constructionLines)
            {
                g2d->drawLine(line,apen);
            }
        }
    }

    if (tileSelector && !editPlacedTile)
    {
        //qDebug() << "current selection:"  << strTiliingSelection[currentSelection->getType()];
        switch (tileSelector->getType())
        {
        case INTERIOR:
            // nothing - handled by currentTile
            break;

        case EDGE:
            g2d->drawEdge(tileSelector->getPlacedEdge(), QPen(QColor(Qt::green),3));
            break;

        case VERTEX:
        case MID_POINT:
        case TILE_CENTER:
            g2d->drawCircle(tileSelector->getPlacedPoint(), 12, QPen(circle_color),QBrush(circle_color));
            break;

        case ARC_POINT:
            g2d->drawCircle(tileSelector->getPlacedPoint(), 12, QPen(circle_color), QBrush(circle_color));
            g2d->drawEdge(tileSelector->getPlacedEdge(), QPen(QColor(Qt::red),3));
            break;

        case SCREEN_POINT:
            g2d->drawCircle(tileSelector->getModelPoint(), 14, QPen(Qt::red), QBrush(Qt::red));
            break;
        }
    }

    if (tilingMaker->getTilingMakerMouseMode() == TM_EDIT_TILE_MODE)
    {
        QPolygonF p;
        if (editPlacedTile)
        {
            p = editPlacedTile->getPlacedPoints();
            drawTile(g2d, editPlacedTile, true, normal_color);
        }
        else if (tileSelector && tileSelector->getType() == INTERIOR)
        {
            p = tileSelector->getPlacedPolygon();
        }
        if (p.size())
        {
            QPen pen(Qt::blue);
            QBrush brush(Qt::blue);
            for (auto it = p.begin(); it != p.end(); it++)
            {
                g2d->drawCircle(*it,5,pen,brush);
            }
        }
    }

    drawMeasurements(g2d);

    drawAccum(g2d);

    drawMouseInteraction(g2d);

    if (!tileEditPoint.isNull())
    {
        g2d->drawCircle(tileEditPoint,10,QPen(Qt::red),QBrush());
    }
}

void TilingMakerView::drawTiling( GeoGraphics * g2d )
{
    determineOverlapsAndTouching();

    auto tiling = wTiling.lock();

    for (const auto & tile : allPlacedTiles)
    {
        QColor color = normal_color;
        if (tile->show())
        {
            if (tile == tilingMaker->getCurrentPlacedTile())
            {
                color = under_mouse_color;
            }
            else if (tileSelector
                     && ((tileSelector->getType() == INTERIOR) || (tileSelector->getType() == TILE_CENTER))
                     && tileSelector->getPlacedTile() == tile)
            {
                color = selected_color;
            }
            else if (tile->isOverlapping())
            {
                color = overlapping_color;
            }
            else if (tile->isTouching())
            {
                color = touching_color;
            }
            else if (tiling && tiling->getInTiling().contains(tile))
            {
                color = in_tiling_color;
            }
        }
        drawTile(g2d, tile, true, color);
    }
}

void TilingMakerView::drawTranslationVectors(GeoGraphics * g2d, QPointF t1_start, QPointF t1_end, QPointF t2_start, QPointF t2_end)
{
    qreal arrow_length = Transform::distFromInvertedZero(g2d->getTransform(),12.0);
    qreal arrow_width  = Transform::distFromInvertedZero(g2d->getTransform(),6.0);

    layerPen.setColor(construction_color);

    if (t1_start != t1_end)
    {
        g2d->drawLine(t1_start, t1_end, layerPen);
        g2d->drawArrow(t1_start, t1_end, arrow_length, arrow_width, construction_color);
        g2d->drawText(worldToScreen(t1_end) + QPointF(10,0),"T1");
    }

    if (t2_start != t2_end)
    {
        g2d->drawLine(t2_start, t2_end, layerPen);
        g2d->drawArrow(t2_start, t2_end, arrow_length, arrow_width, construction_color);
        g2d->drawText(worldToScreen(t2_end) + QPointF(10,0),"T2");
    }
}

void TilingMakerView::drawTile(GeoGraphics * g2d, PlacedTilePtr pf, bool draw_c, QColor icol )
{

    // fill the polygon
    EdgePoly ep = pf->getPlacedEdgePoly();
    if (pf->show())
    {
        QPen pen(icol, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        g2d->fillEdgePoly(ep,pen);
    }

    QPen pen2(lineColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    g2d->drawEdgePoly(ep,pen2);

    if (tilingMaker->getTilingMakerMouseMode() == TM_EDGE_CURVE_MODE)
    {
        for (const auto & edge : qAsConst(ep))
        {
            QPen apen = layerPen;
            apen.setColor(Qt::blue);
            g2d->drawCircle(edge->getArcCenter(),5,apen,QBrush());
        }
    }

    // draw center circle
    if (draw_c && pf->show())
    {
        QPolygonF pts = pf->getPlacedPoints();
        QPointF pt    = pf->getTile()->getCenter();
        pt            = pf->getTransform().map(pt);

        layerPen.setColor(Qt::red);
        g2d->drawCircle(pt,9,layerPen, QBrush());

        layerPen.setWidth(2);
        qreal len = screenToWorld(9.0);
        g2d->drawLine(QPointF(pt.x()-len,pt.y()),QPointF(pt.x()+len,pt.y()),layerPen);
        g2d->drawLine(QPointF(pt.x(),pt.y()-len),QPointF(pt.x(),pt.y()+len),layerPen);
    }
}

void TilingMakerView::drawAccum(GeoGraphics * g2d)
{
    if ( wAccum.size() == 0)
    {
        return;
    }

    layerPen.setColor(construction_color);
    QBrush brush(construction_color);
    for (auto it = wAccum.begin(); it != wAccum.end(); it++)
    {
        EdgePtr edge = *it;
        if (edge->getType() == EDGETYPE_LINE)
        {
            //qDebug() << "draw accum edge";
            QPointF p1 = edge->v1->pt;
            QPointF p2 = edge->v2->pt;
            g2d->drawCircle(p1,6,layerPen,brush);
            g2d->drawCircle(p2,6,layerPen,brush);
            g2d->drawLine(p1, p2,layerPen);
        }
        else if (edge->getType() == EDGETYPE_POINT)
        {
            //qDebug() << "draw accum point";
            QPointF p = edge->v1->pt;
            g2d->drawCircle(p,6,layerPen,brush);
        }
    }
}

void TilingMakerView::drawMeasurements(GeoGraphics *g2d)
{
    layerPen.setColor(construction_color);
    for (auto mm : wMeasurements)
    {
        g2d->drawLineDirect(mm->startS(), mm->endS(),layerPen);
        QString msg = QString("%1 (%2)").arg(QString::number(mm->lenS(),'f',2)).arg(QString::number(mm->lenW(),'f',8));
        g2d->drawText(mm->endS() + QPointF(10,0),msg);
    }
}

// hide tiling so bacground can be seen
void TilingMakerView::hideTiling(bool state)
{
    _hideTiling = state;
    forceRedraw();
}


//
// Tile, edge and vertex finding.
//
TileSelectorPtr TilingMakerView::findTile(QPointF spt)
{
    TileSelectorPtr nothingToIgnore;
    return findTile(spt, nothingToIgnore);
}

TileSelectorPtr TilingMakerView::findTile(QPointF spt, TileSelectorPtr ignore)
{
    QPointF wpt = screenToWorld(spt);

    for(auto placedTile : qAsConst(allPlacedTiles))
    {
        if (ignore && (ignore->getPlacedTile() == placedTile))
            continue;

        QPolygonF pgon = placedTile->getPlacedPoints();
        if (pgon.containsPoint(wpt,Qt::OddEvenFill))
        {
            return make_shared<InteriorTilleSelector>(placedTile);
        }
    }
    
    TileSelectorPtr sel;
    return sel;
}

TileSelectorPtr TilingMakerView::findVertex(QPointF spt)
{
    TileSelectorPtr nothingToIgnore;
    return findVertex(spt,nothingToIgnore);
}

TileSelectorPtr TilingMakerView::findVertex(QPointF spt,TileSelectorPtr ignore)
{
    for(auto it = allPlacedTiles.begin(); it != allPlacedTiles.end(); it++ )
    {
        PlacedTilePtr pf = *it;

        if (ignore && (ignore->getPlacedTile() == pf))
            continue;

        QPolygonF pgon = pf->getTilePoints();
        QTransform   T = pf->getTransform();
        for( int v = 0; v < pgon.size(); ++v )
        {
            QPointF a = pgon[v];
            QPointF b = T.map(a);
            QPointF c = worldToScreen(b);
            if (Point::dist2(spt,c) < 49.0 )
            {
                return make_shared<VertexTileSelector>(pf,a);
            }
        }
    }
    
    TileSelectorPtr sel;
    return sel;
}

TileSelectorPtr TilingMakerView::findMidPoint(QPointF spt)
{
    TileSelectorPtr nothingToIgnore;
    return findMidPoint(spt,nothingToIgnore);
}

TileSelectorPtr TilingMakerView::findMidPoint(QPointF spt, TileSelectorPtr ignore)
{
    TileSelectorPtr sel;

    for(auto it = allPlacedTiles.begin(); it != allPlacedTiles.end(); it++ )
    {
        PlacedTilePtr pf = *it;

        if (ignore && (ignore->getPlacedTile() == pf))
            continue;

        QTransform T = pf->getTransform();
        EdgePoly ep  = pf->getTileEdgePoly();
        for (const auto & edge : ep)
        {
            QPointF a     = edge->v1->pt;
            QPointF b     = edge->v2->pt;
            QPointF wmid  = edge->getMidPoint();
            QPointF aa    = T.map(a);
            QPointF bb    = T.map(b);
            QPointF wmidd = T.map(wmid);
            QPointF smid  = worldToScreen(wmidd);
            if (Point::dist2(spt,smid) < 49.0)
            {
                // Avoid selecting middle point if end-points are too close together.
                QPointF a2 = worldToScreen(aa);
                QPointF b2 = worldToScreen(bb);
                qreal screenDist = Point::dist2(a2,b2);
                if ( screenDist < (6.0 * 6.0 * 6.0 * 6.0) )
                {
                    qDebug() << "Screen dist too small = " << screenDist;
                    return sel;
                }
                return make_shared<MidPointTileSelector>(pf, edge, wmid);
            }
        }
    }

    return sel;
}

TileSelectorPtr TilingMakerView::findArcPoint(QPointF spt)
{
    TileSelectorPtr sel;

    for(const auto & pf : allPlacedTiles)
    {
        QTransform T   = pf->getTransform();
        EdgePoly epoly = pf->getTileEdgePoly();

        for(const auto & ep : epoly)
        {
            if (ep->getType() == EDGETYPE_CURVE || ep->getType() == EDGETYPE_CHORD)
            {
                QPointF a    = ep->getArcCenter();
                QPointF aa   = T.map(a);
                QPointF aad  = worldToScreen(aa);
                if (Point::dist2(spt,aad) < 49.0)
                {
                    return make_shared<ArcPointTileSelector>(pf, ep, a);
                }
            }
        }
    }

    return sel;
}

TileSelectorPtr TilingMakerView::findEdge(QPointF spt)
{
    TileSelectorPtr nothingToIgnore;
    return findEdge(spt, nothingToIgnore);
}

TileSelectorPtr TilingMakerView::findEdge(QPointF spt, TileSelectorPtr ignore )
{
    for (auto it = allPlacedTiles.begin(); it != allPlacedTiles.end(); it++ )
    {
        PlacedTilePtr pf = *it;

        if (ignore && (ignore->getPlacedTile() == pf))
            continue;

        EdgePoly epoly = pf->getTileEdgePoly();
        QTransform T   = pf->getTransform();

        for (int v = 0; v < epoly.size(); ++v)
        {
            EdgePtr edge  = epoly[v];
            QLineF  line  = edge->getLine();
            QLineF  lineW = T.map(line);
            QLineF  LineS = worldToScreen(lineW);

            if (Point::distToLine(spt, LineS) < 7.0)
            {
                return make_shared<EdgeTileSelector>(pf,edge);
            }
        }
    }
    
    TileSelectorPtr sel;
    return sel;
}

TileSelectorPtr TilingMakerView::findSelection(QPointF spt)
{
    TileSelectorPtr sel;

    if (      (sel = findVertex(spt)) )
        return sel;
    else if ( (sel = findMidPoint(spt)) )
        return sel;
    else if ( (sel = findEdge(spt)) )
        return sel;
    else
    {
        TileSelectorPtr sel2 = findTile(spt);
        if (sel2)
        {
            sel = findCenter(sel2->getPlacedTile(),spt);
            if (!sel)
                sel = sel2;
        }
    }
    return sel;
}

TileSelectorPtr TilingMakerView::findPoint(QPointF spt)
{
    TileSelectorPtr nothingToIgnore;
    return findPoint(spt,nothingToIgnore);
}

TileSelectorPtr TilingMakerView::findPoint(QPointF spt, TileSelectorPtr ignore)
{
    TileSelectorPtr sel = findVertex(spt,ignore);
    if (!sel)
    {
        sel = findMidPoint(spt,ignore);
    }
    if (!sel)
    {
        TileSelectorPtr sel2 = findTile(spt,ignore);
        if (sel2)
        {
            sel = findCenter(sel2->getPlacedTile(),spt);
        }
    }
    if (sel)
        qDebug() << "findPoint:" << sel->getTypeString();
    return sel;
}

QPointF TilingMakerView::findSelectionPointOrPoint(QPointF spt)
{
    TileSelectorPtr sel = findPoint(spt);
    if (!sel)
    {
        return screenToWorld(spt);
    }

    return sel->getPlacedPoint();
}

TileSelectorPtr TilingMakerView::findCenter(PlacedTilePtr pf, QPointF spt)
{
    EdgePoly  epoly = pf->getPlacedEdgePoly();
    QPointF    wpt  = epoly.calcCenter();
    QPointF    spt2 = worldToScreen(wpt);

    if (Point::isNear(spt,spt2))
    {
        TilePtr tile = pf->getTile();
        QPointF mpt  = tile->getCenter();
        return make_shared<CenterTileSelector>(pf, mpt);
    }
    
    TileSelectorPtr sel;
    return sel;
}

bool TilingMakerView::accumHasPoint(QPointF wpt)
{
    QPointF newpoint = worldToScreen(wpt);
    for (auto it = wAccum.begin(); it != wAccum.end(); it++)
    {
        EdgePtr edge = *it;
        QPointF existing = worldToScreen(edge->v1->pt);
        if (Point::isNear(newpoint,existing))
        {
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////
//
// Validating that features don't overlap when tiled.
//
// Note: we're not very stringent to avoid flagging polygons that would
//       only slightly overlap. This is more about including completely
//       unnecessary polygons that cover area already covered by another
//       copy, or other gross errors that are hard to see (especially once
//       the plane if fully tiled!).
//
// casper - original is replaced with a more robust implementation which
// also distinguishes between touching and overlapping.

void TilingMakerView::determineOverlapsAndTouching()
{
    for (const auto & tile : allPlacedTiles)
    {
        tile->clearViewState();
    }

    if (!config->tm_showOverlaps)
    {
        return;
    }

#if 0
    for (const auto & tile1 : allPlacedTiles)
    {
        if (!tile1->show()) continue;

        QPolygonF poly1 = tile1->getPlacedPoints();

        for (const auto & tile2 : allPlacedTiles)
        {
            if (!tile2->show()) continue;
            if (tile2 == tile1) continue;

            QPolygonF poly2 = tile2->getPlacedPoints();
            if (poly2.intersects(poly1))
            {
                QPolygonF poly3 = poly2.intersected(poly1);
                if (!poly3.isEmpty())
                {
                    qDebug() << "overlapping";
                    tile1->setOverlapping();
                    tile2->setOverlapping();
                }
                else
                {
                    qDebug() << "touching";
                    tile1->setTouching();
                    tile2->setTouching();
                }
            }
        }
    }
#else
    for (const auto & tile1 : allPlacedTiles)
    {
        if (!tile1->show())  continue;

        QPolygonF poly1 = tile1->getPlacedPoints();
        EdgePoly ep1    = tile1->getPlacedEdgePoly();
        for (const auto & tile2 : allPlacedTiles)
        {
            if (!tile2->show())  continue;
            if (tile2 ==  tile1) continue;

            QPolygonF poly2 = tile2->getPlacedPoints();
            EdgePoly ep2    = tile2->getPlacedEdgePoly();
#if 0
            if (isColinearAndTouching(ep1,ep2))
            {
                qDebug() << "touching";
                tile1->setTouching();
                tile2->setTouching();
            }
            else  if (poly2.intersects(poly1))
            {
                qDebug() << "overlapping";
                tile1->setOverlapping();
                tile2->setOverlapping();
            }
#else
            if (poly2.intersects(poly1))
            {
                if (isColinearAndTouching(ep1,ep2))
                {
                    //qDebug() << "touching";
                    tile1->setTouching();
                    tile2->setTouching();
                }
                else
                {
                    //qDebug() << "overlapping";
                    tile1->setOverlapping();
                    tile2->setOverlapping();
                }
            }
#endif
        }
    }
#endif
}

bool TilingMakerView::isColinearAndTouching(const EdgePoly & ep1, const EdgePoly & ep2, qreal tolerance)
{
    Q_UNUSED(tolerance);
    for (const EdgePtr & edge1 : ep1)
    {
        for (const EdgePtr &  edge2 : ep2)
        {
            if (isColinearAndTouching(edge1->getLine(),edge2->getLine()))
            {
                return true;
            }
        }
    }
    return false;
}

bool TilingMakerView::isColinearAndTouching(const QLineF & l1, const QLineF & l2, qreal tolerance)
{
    //static bool debug = false;
#if 1
    if (isColinear(l1,l2,tolerance))
    {
        if (l1.length() < l2.length())
        {
            // l1 shorter
            return (Point::isOnLine(l1.p1(),l2) || Point::isOnLine(l1.p2(),l2));
        }
        else
        {
            // l2 shorter, l1 longer or equal
            return  (Point::isOnLine(l2.p1(),l1) || Point::isOnLine(l2.p2(),l1));
        }
    }

#else
    if (   Utils::pointOnLine(l1,l2.p1()) || Utils::pointOnLine(l1,l2.p2())
        || Utils::pointAtEndOfLine(l1,l2.p1()) || Utils::pointAtEndOfLine(l1,l2.p2()))
    {
        return isColinear(l1,l2,tolerance);
    }

    if (debug) qDebug() << "    not touching";
#endif
    return false;
}

bool TilingMakerView::isColinear(const QLineF & l1, const QLineF & l2, qreal tolerance)
{
#if 1
    static bool debug = false;

    qreal angle = Utils::angle(l1,l2);
    if (debug) qDebug() << "    angle=" << angle;
    if ((qAbs(angle) < tolerance) || (qAbs(angle-180.0) < tolerance))
    {
        if (debug) qDebug() << "    colinear";
        return true;
    }
    else
    {
        if (debug) qDebug() << "NOT colinear";

        return false;
    }
#else
    Q_UNUSED(tolerance);
    qreal grad1 = (l1.y2() - l1.y1()) / (l1.x2() - l1.x1());
    qreal grad2 = (l2.y2() - l2.y1()) / (l2.x2() - l2.x1());
    return Loose::equals(grad1,grad2);

#endif
}

// used when creating a polygon
TileSelectorPtr TilingMakerView::findNearGridPoint(QPointF spt)
{
    TileSelectorPtr tsp;
    QPointF p;

    if (GridView::getInstance()->nearGridPoint(spt,p))
    {
        tsp = make_shared<ScreenTileSelector>(p);  // not really a vertex, but good enough
    }
    return tsp;
}

TileSelectorPtr TilingMakerView::findTileUnderMouse()
{
    return findTile(sMousePos);
}


void TilingMakerView::setMousePos(QPointF spt)
{
    Qt::KeyboardModifiers km = QApplication::keyboardModifiers();
    if (km == Qt::SHIFT)
    {
        sMousePos.setX(spt.x());
    }
    else if (km == Qt::CTRL)
    {
        sMousePos.setY(spt.y());
    }
    else
    {
        sMousePos = spt;
    }
}

void TilingMakerView::startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton)
{
    Q_ASSERT(tilingMaker->getTilingMakerMouseMode() == TM_NO_MOUSE_MODE);

    TileSelectorPtr sel = findSelection(spt);
    if(sel)
    {
        // do this first
        wAccum.clear();
        mouse_interaction.reset();

        switch (mouseButton)
        {
        case Qt::LeftButton:
            qDebug().noquote() << "left button selection:" << sel->getTypeString();
            switch (sel->getType())
            {
            case VERTEX:
            case TILE_CENTER:
                mouse_interaction = make_shared<JoinPoint>(sel, spt);
                break;
            case MID_POINT:
                mouse_interaction = make_shared<JoinMidPoint>(sel, spt);
                break;
            case EDGE:
                mouse_interaction = make_shared<JoinEdge>(sel, spt);
                break;
            case INTERIOR:
                tilingMaker->setCurrentPlacedTile(sel->getPlacedTile());
                mouse_interaction = make_shared<MovePolygon>(sel, spt);
                break;
            case ARC_POINT:
                break;
            case SCREEN_POINT:
                break;
            }
            break;

        case Qt::MiddleButton:
            mouse_interaction = make_shared<DrawTranslation>(sel, spt, layerPen);
            break;

        case Qt::RightButton:
            qDebug().noquote() << "right button selection:" << sel->getTypeString();
            switch (sel->getType())
            {
            case VERTEX:
            case TILE_CENTER:
                mouse_interaction = make_shared<CopyJoinPoint>(sel, spt);
                break;
            case MID_POINT:
                mouse_interaction = make_shared<CopyJoinMidPoint>(sel, spt);
                break;
            case EDGE:
                mouse_interaction = make_shared<CopyJoinEdge>(sel, spt);
                break;
            case INTERIOR:
            {
                tilingMaker->setClickedSelector(sel);   // save
                tilingMaker->setClickedPoint(spt);      // save
                PlacedTilePtr tile = sel->getPlacedTile();
                QMenu myMenu;
                myMenu.addSection("Options");
                myMenu.addSeparator();
                myMenu.addAction("Copy/Move",           tilingMaker, &TilingMaker::slot_copyMoveTile);
                myMenu.addAction("Edit Tile",           tilingMaker, &TilingMaker::slot_editTile);
                if (tile->show())
                    myMenu.addAction("Hide Tile",       tilingMaker, &TilingMaker::slot_hideTile);
                else
                    myMenu.addAction("Show Tile",       tilingMaker, &TilingMaker::slot_showTile);
                if (tilingMaker->isIncluded(tile))
                    myMenu.addAction("Exclude",         tilingMaker, &TilingMaker::slot_excludeTile);
                else
                    myMenu.addAction("Include",         tilingMaker, &TilingMaker::slot_includeTile);
                myMenu.addAction("Delete",              tilingMaker, &TilingMaker::slot_deleteTile);
                myMenu.addAction("Uniquify",            tilingMaker, &TilingMaker::slot_uniquifyTile);
                if (tile->getTile()->isRegular())
                    myMenu.addAction("Make irregular",  tilingMaker, &TilingMaker::slot_convertTile);
                else
                    myMenu.addAction("Make regular",    tilingMaker, &TilingMaker::slot_convertTile);
                myMenu.exec(view->mapToGlobal(spt.toPoint()));
            }
            break;

            case ARC_POINT:
            case SCREEN_POINT:
                break;
            }
            break;
        default:
            break;
        }
    }
    else
    {
        tilingMaker->resetCurrentPlacedTile();
    }
}

void TilingMakerView::updateUnderMouse(QPointF spt)
{
    switch (tilingMaker->getTilingMakerMouseMode())
    {
    case TM_NO_MOUSE_MODE:
    case TM_MEASURE_MODE:
    case TM_COPY_MODE:
    case TM_DELETE_MODE:
    case TM_INCLUSION_MODE:
    case TM_EDIT_TILE_MODE:
    case TM_MIRROR_X_MODE:
    case TM_MIRROR_Y_MODE:
    case TM_REFLECT_EDGE:
        tileSelector = findSelection(spt);
        if (tileSelector)
            forceRedraw();  // NOTE this triggers a lot of repainting
        break;

    case TM_TRANSLATION_VECTOR_MODE:
    case TM_CONSTRUCTION_LINES:
        tileSelector = findSelection(spt);
        if (tileSelector)
            forceRedraw();
        break;

    case TM_DRAW_POLY_MODE:
        tileSelector = findVertex(spt);
        if (!tileSelector)
        {
            tileSelector = findMidPoint(spt);
        }
        if (!tileSelector)
        {
            tileSelector = findNearGridPoint(spt);
        }
        forceRedraw();
        break;

    case TM_POSITION_MODE:
        if (!mouse_interaction)
        {
            mouse_interaction = make_shared<Position>(spt);
        }
        else
        {
            // unusual use case
            mouse_interaction->updateDragging(spt);
        }
        break;

    case TM_EDGE_CURVE_MODE:
        tileSelector = findArcPoint(spt);
        if (tileSelector)
        {
            qDebug() << "updateUnderMouse: found arc center";
        }
        else
        {
            tileSelector = findEdge(spt);
            if (tileSelector)
            {
                qDebug() << "updateUnderMouse: found edge";
            }
            else
            {
                tileSelector.reset();
            }
        }
        forceRedraw();
        break;
    }
}

void  TilingMakerView::drawMouseInteraction(GeoGraphics * g2d)
{
    if (mouse_interaction)
    {
        mouse_interaction->draw(g2d);
    }
}


//////////////////////////////////////////////////////////////////
///
/// Layer slots
///
//////////////////////////////////////////////////////////////////

void TilingMakerView::slot_setCenter(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        setCenterScreenUnits(spt);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        // TODO: implement me
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_UNIQUE_TILE))
    {
        // TODO: implement me
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
        // TODO: implement me
    }
}

void TilingMakerView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    if (!view->isActiveLayer(this)) return;

    sMousePos = spt;

    if (debugMouse) qDebug() << "slot_mousePressed:" << sMousePos << screenToWorld(sMousePos);

    TileSelectorPtr sel;

    switch (tilingMaker->getTilingMakerMouseMode())
    {
    case TM_NO_MOUSE_MODE:
        if (view->getMouseMode(MOUSE_MODE_NONE))
        {
            startMouseInteraction(sMousePos,btn);
        }
        break;

    case TM_COPY_MODE:
        sel = findTileUnderMouse();
        if (sel)
            mouse_interaction = make_shared<CopyMovePolygon>(sel, sMousePos);
        break;

    case TM_DELETE_MODE:
        tilingMaker->deleteTile( findTileUnderMouse() );
        tilingMaker->setTilingMakerMouseMode(TM_NO_MOUSE_MODE);
        break;

    case TM_TRANSLATION_VECTOR_MODE:
        sel = findSelection(sMousePos);
        if (sel)
            mouse_interaction = make_shared<DrawTranslation>(sel, sMousePos, layerPen);
        break;

    case TM_DRAW_POLY_MODE:
        mouse_interaction = make_shared<CreatePolygon>(sMousePos);
        break;

    case TM_INCLUSION_MODE:
        tilingMaker->toggleInclusion(findTileUnderMouse());
        break;

    case TM_MEASURE_MODE:
    {
        Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
        if (kms == (Qt::CTRL | Qt::SHIFT))
        {
            TileSelectorPtr nothingToIgnore;
            sel = findEdge(spt,nothingToIgnore);
            if (sel)
            {
                // FIXME  - this looks like incomplete code
                QLineF line = sel->getPlacedLine();
                qreal dist = Point::distToLine(screenToWorld(spt),line);
                qDebug() << "dist" << dist << "line" << line;
                mouse_interaction = make_shared<Measure>(sMousePos,sel);
            }
        }
        else
        {
            mouse_interaction = make_shared<Measure>(sMousePos,sel);
        }
        break;
    }

    case TM_POSITION_MODE:
        setCenterScreenUnits(spt);
        break;

    case TM_EDIT_TILE_MODE:
        sel = findSelection(spt);
        if (sel)
        {
            if (!editPlacedTile && sel->getType() == INTERIOR)
            {
                editPlacedTile = sel->getPlacedTile();
            }
            else if (editPlacedTile && sel->getType() == VERTEX )
            {
                mouse_interaction = make_shared<EditTile>(sel, editPlacedTile, sMousePos);
            }
        }
        break;
    case TM_EDGE_CURVE_MODE:
        sel = findArcPoint(spt);
        if (sel)
        {
            tileSelector = sel;
            QMenu myMenu;
            myMenu.addAction("Use Cursor to change curve", tilingMaker, SLOT(slot_moveArcCenter()));
            myMenu.addAction("Edit Magnitude", tilingMaker, SLOT(slot_editMagnitude()));
            myMenu.exec(view->mapToGlobal(spt.toPoint()));
        }
        else
        {
            sel = findEdge(spt);
            if (sel)
            {
                EdgePtr edge =  sel->getModelEdge();
                if (edge->getType() == EDGETYPE_LINE)
                {
                    PlacedTilePtr ptp = sel->getPlacedTile();
                    TilePtr tile = ptp->getTile();
                    if (tile->isRegular())
                    {
                        tilingMaker->flipTileRegularity(tile);
                    }
                    edge->calcArcCenter(true,false);  // default to convex
                    qDebug() << "edge converted to curve";
                }
                else if (edge->getType() == EDGETYPE_CURVE)
                {
                    tileSelector = sel;
                    QMenu myMenu;
                    myMenu.addAction("Flatten Edge",  tilingMaker, SLOT(slot_flatenCurve()));
                    if (edge->isConvex())
                        myMenu.addAction("Make Concave",  tilingMaker, SLOT(slot_makeConcave()));
                    else
                        myMenu.addAction("Make Convex",  tilingMaker, SLOT(slot_makeConvex()));
                    myMenu.exec(view->mapToGlobal(spt.toPoint()));
                }
                // Methinks tilings cannot have chords only curves(arcs)
            }
        }
        break;

    case TM_MIRROR_X_MODE:
        tilingMaker->mirrorPolygonX(findTileUnderMouse());
        break;

    case TM_MIRROR_Y_MODE:
        tilingMaker->mirrorPolygonY(findTileUnderMouse());
        break;

    case TM_CONSTRUCTION_LINES:
        mouse_interaction = make_shared<TilingConstructionLine>(sel, sMousePos);
        break;

    case TM_REFLECT_EDGE:
        auto fsel = findTileUnderMouse();
        if (fsel)
        {
            if (tilingMaker->reflectPolygon(findTileUnderMouse()))
            {
                tilingMaker->setTilingMakerMouseMode(TM_NO_MOUSE_MODE);
            }
        }
        break;
    }

    if (debugMouse)
    {
        if (mouse_interaction)
            qDebug().noquote() << "press end:"  << mouse_interaction->desc;
        else
            qDebug() << "press end: no mouse_interaction";
    }
}

void TilingMakerView::slot_mouseDragged(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << sMousePos << screenToWorld(sMousePos)  << sTilingMakerMouseMode[tilingMaker->getTilingMakerMouseMode()];

    updateUnderMouse(sMousePos);

    switch (tilingMaker->getTilingMakerMouseMode())
    {
    case TM_NO_MOUSE_MODE:
    case TM_COPY_MODE:
    case TM_TRANSLATION_VECTOR_MODE:
    case TM_DRAW_POLY_MODE:
    case TM_MEASURE_MODE:
    case TM_POSITION_MODE:
    case TM_EDIT_TILE_MODE:
    case TM_EDGE_CURVE_MODE:
    case TM_CONSTRUCTION_LINES:
        if (mouse_interaction)
            mouse_interaction->updateDragging(sMousePos);
        break;
    case TM_DELETE_MODE:
    case TM_MIRROR_X_MODE:
    case TM_MIRROR_Y_MODE:
    case TM_INCLUSION_MODE:
    case TM_REFLECT_EDGE:
        break;
    }
}

void TilingMakerView::slot_mouseTranslate(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        QTransform T = getFrameTransform();
        qreal scale = Transform::scalex(T);
        QPointF mpt = spt/scale;
        QTransform tt = QTransform::fromTranslate(mpt.x(),mpt.y());
        for (const auto & placedTile : allPlacedTiles)
        {
            QTransform t = placedTile->getTransform();
            t *= tt;
            placedTile->setTransform(t);
        }

        tilingMaker->pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
        forceRedraw();
        emit tilingMaker->sig_refreshMenu();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
        auto currentTile = tilingMaker->getCurrentPlacedTile();
        if (!currentTile)
            return;

        QTransform T = getFrameTransform();
        qreal scale = Transform::scalex(T);
        QPointF mpt = spt/scale;
        QTransform tt = QTransform::fromTranslate(mpt.x(),mpt.y());

        QTransform t = currentTile->getTransform();
        t *= tt;
        currentTile->setTransform(t);

        forceRedraw();
        emit tilingMaker->sig_refreshMenu();
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

void TilingMakerView::slot_mouseMoved(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << sMousePos;

    updateUnderMouse(sMousePos);
}

void TilingMakerView::slot_mouseReleased(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "release" << sMousePos << screenToWorld(sMousePos);

    if (mouse_interaction)
    {
        mouse_interaction->endDragging(spt);
        mouse_interaction.reset();
    }
    forceRedraw();
}

void TilingMakerView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

void TilingMakerView::slot_wheel_scale(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        qreal sc = 1.0 + delta;
        QTransform ts;
        ts.scale(sc,sc);
        for (const auto & pplacedTile : allPlacedTiles)
        {
            QTransform t = pplacedTile->getTransform();
            t *= ts;
            pplacedTile->setTransform(t);
        }

        tilingMaker->pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
        forceRedraw();
        emit tilingMaker->sig_refreshMenu();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
        tilingMaker->placedTileDeltaScale(1.0 + delta);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_UNIQUE_TILE))
    {
        tilingMaker->uniqueTileDeltaScale(delta);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setCanvasXform(xf);
        emit tilingMaker->sig_refreshMenu();
    }
}

void TilingMakerView::slot_wheel_rotate(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        QTransform tr;
        tr.rotate(delta);
        Xform xf(tr);
        xf.setModelCenter(getCenterModelUnits());
        QTransform tr2 = xf.toQTransform(QTransform());

        for (const auto & pfp : allPlacedTiles)
        {
            QTransform t = pfp->getTransform();
            t *= tr2;
            pfp->setTransform(t);
        }

        tilingMaker->pushTilingToPrototypeMaker(PROM_TILING_CHANGED);
        forceRedraw();
        emit tilingMaker->sig_refreshMenu();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
       tilingMaker-> placedTileDeltaRotate(0.5 * delta);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_UNIQUE_TILE))
    {
        tilingMaker->uniqueTileDeltaRotate(0.5 * delta);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW))
    {
        Xform xf = getCanvasXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setCanvasXform(xf);
        emit tilingMaker->sig_refreshMenu();
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_SELECTED))
    {
        if (isSelected())
        {
            Xform xf = getCanvasXform();
            xf.setRotateDegrees(xf.getRotateDegrees() + delta);
            setCanvasXform(xf);
            emit tilingMaker->sig_refreshMenu();
        }
    }
}

void TilingMakerView::slot_scale(int amount)
{
    if (!view->isActiveLayer(this)) return;

    //qDebug() << "TilingMaker::slot_scale" << amount;

    if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
        tilingMaker->placedTileDeltaScale(amount);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_UNIQUE_TILE))
    {
        tilingMaker->uniqueTileDeltaScale(amount);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        tilingMaker->tilingDeltaScale(amount);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_VIEW) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1.0 + static_cast<qreal>(amount)/100.0));
        setCanvasXform(xf);
        emit tilingMaker->sig_refreshMenu();
    }
}

void TilingMakerView::slot_rotate(int amount)
{
    if (!view->isActiveLayer(this)) return;

    //qDebug() << "TilingMaker::slot_rotate" << amount;

    if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
        tilingMaker->placedTileDeltaRotate(amount);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_UNIQUE_TILE))
    {
        tilingMaker->uniqueTileDeltaRotate(amount);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        tilingMaker->tilingDeltaRotate(amount);
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
            emit tilingMaker->sig_refreshMenu();
        }
    }
}

void TilingMakerView::slot_moveX(int amount)
{
    if (!view->isActiveLayer(this)) return;

    //qDebug() << "TilingMaker::slot_moveX" << amount;

    if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
        tilingMaker->placedTileDeltaX(amount);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
        tilingMaker->tilingDeltaX(amount);
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
            emit tilingMaker->sig_refreshMenu();
        }
    }
}

void TilingMakerView::slot_moveY(int amount)
{
    if (!view->isActiveLayer(this)) return;

    //qDebug() << "TilingMaker::slot_moveY" << amount;

    if (view->getKbdMode(KBD_MODE_XFORM_PLACED_TILE))
    {
        tilingMaker->placedTileDeltaY(amount);
    }
    else if (view->getKbdMode(KBD_MODE_XFORM_TILING))
    {
       tilingMaker->tilingDeltaY(amount);
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
            emit tilingMaker->sig_refreshMenu();
        }
    }
}

void TilingMakerView::slot_setTileEditPoint(QPointF pt)
{
    tileEditPoint = pt;
    qDebug() << "tile edit point =" << pt;
    forceRedraw();
}

