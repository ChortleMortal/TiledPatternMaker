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

#include "gui/model_editors/tiling_edit/tile_selection.h"
#include "gui/model_editors/tiling_edit/tiling_mouseactions.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/construction_lines_view.h"
#include "gui/viewers/geo_graphics.h"
#include "gui/viewers/grid_view.h"
#include "gui/viewers/tiling_maker_view.h"
#include "model/makers/tiling_maker.h"
#include "model/makers/motif_maker.h"
#include "model/settings/configuration.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/enums/etilingmaker.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"
#include "sys/sys.h"

using std::make_shared;

Q_DECLARE_METATYPE(PlacedTilePtr)

TilingMakerView::TilingMakerView() : LayerController(Type::TilingMaker,Model::Derived,ZLevel::Standard_Z0,"Tiling Maker")
{
    debugMouse   = false;
    _hideVectors = false;
    _viewMode     = ViewMode::showTiles;
}

TilingMakerView::~TilingMakerView()
{}

void TilingMakerView::setTiling(TilingPtr tiling)
{
    wTiling = tiling;
}

void TilingMakerView::setViewPlacedTileSelector(PlacedTileSelectorPtr tsp, bool toggle)
{
    if (toggle)
    {
        if (_viewPlacedTileSelector != tsp)
        {
            _viewPlacedTileSelector = tsp;
        }
        else
        {
            _viewPlacedTileSelector = nullptr;  // 04AUG26 pressing again deselects
        }
    }
    else
    {
        _viewPlacedTileSelector = tsp;
    }
}

void TilingMakerView::setFill(bool enb)
{
    Sys::tm_fill = enb;
    for (auto & tiling : Sys::tilingMaker->getTilings())
    {
        tiling->setTilingViewChanged();
    }
    emit sig_updateView();
}

void TilingMakerView::clearViewData()
{
    for (auto & m : wMeasurements)
    {
        delete m;
    }
    wMeasurements.clear();
    setViewPlacedTileSelector(nullptr);
    resetEditPlacedTile();
    mouse_interaction.reset();
}

void TilingMakerView::paint(QPainter *painter)
{
    //qDebug() << "TilingMakerView::paint - START";

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    
    QColor bcolor = viewControl()->getViewBackgroundColor();

    lineColor = (bcolor == QColor(Qt::white) ? QColor(Qt::black) : QColor(Qt::white));

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);
    
    drawLayerModelCenter(painter);

    //qDebug() << "TilingMakerView::paint - END";
}

void TilingMakerView::draw(GeoGraphics * g2d)
{
    const QVector<TilingPtr> & tilings = tilingMaker->getTilings();

    // pass 1 = prepare
    for (auto & tiling : tilings)
    {
        if (tiling && tiling->isViewable())
        {
            const PlacedTileSet & set = tiling->getDrawSet();

            if (set.count() == 0 || tiling->isTilingViewChanged())
            {
                PlacedTileSet newset = _createViewablePlacedTiles(tiling);
                determineOverlapsAndTouching(newset);
                tiling->setDrawSet(newset);
                tiling->resetTilingViewChanged();
            }
        }
    }

    // pass 2 = draw
    for (auto & tiling : tilings)
    {
        if (tiling && tiling->isViewable())
        {
            const PlacedTileSet & set = tiling->getDrawSet();

            if (!Sys::tm_hideTiles)
            {
                drawTiling(g2d,set);
            }

            // T0/T1/T2 are fixed canvas-space numbers, like construction lines - they were
            // practically created relative to a tile point but don't keep tracking it afterward.
            g2d->push(getUndistortedLayerTransform());
            drawTranslationVectors(g2d,tiling,set);
            g2d->pop();

            drawTranslationAnchors(g2d);
        }
    }

    if (viewPlacedTileSelector())
    {
        //qDebug() << "current selection:"  << strTiliingSelection[currentSelection->getType()];
        switch (viewPlacedTileSelector()->getType())
        {
        case INTERIOR:
            // nothing - handled by currentTile
            break;

        case EDGE:
        {
            auto edge = viewPlacedTileSelector()->getPlacedEdge(true);
            g2d->drawEdge(edge, QPen(QColor(Qt::green),3));
            if (edge->isCurve())
            {
                g2d->drawLine(edge->getLine(),QPen(QColor(0x5000ff00),1));
            }
        }    break;

        case VERTEX:
        case MID_POINT:
        case TILE_CENTER:
            g2d->drawCircle(viewPlacedTileSelector()->getPlacedPoint(true), 12, QPen(circle_color),QBrush(circle_color));
            break;

        case ARC_POINT:
            g2d->drawCircle(viewPlacedTileSelector()->getPlacedPoint(true), 12, QPen(circle_color), QBrush(circle_color));
            g2d->drawEdge(viewPlacedTileSelector()->getPlacedEdge(true), QPen(QColor(Qt::red),3));
            break;

        case SCREEN_POINT:
            g2d->drawCircle(viewPlacedTileSelector()->getModelPoint(), 14, QPen(Qt::red), QBrush(Qt::red));
            break;
        }
    }

    if (tilingMaker->getMouseMode() == TilingMaker::MouseMode::Edit)
    {
        QPolygonF p;
        if (editPlacedTile())
        {
            p = editPlacedTile()->getWorldPoints();
            drawTile(g2d, editPlacedTile(), true, tile_color);
        }
        else if (viewPlacedTileSelector() && viewPlacedTileSelector()->getType() == INTERIOR)
        {
            p = viewPlacedTileSelector()->getPlacedPolygon(true);
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

    drawAnchors(g2d);

    drawAccum(g2d);

    drawMouseInteraction(g2d);

    if (!tileEditPoint.isNull())
    {
        g2d->drawCircle(tileEditPoint,10,QPen(Qt::red),QBrush());
    }
}

bool TilingMakerView::tileUnderMouse(const PlacedTilePtr & tile)
{
    return (tile->show()
            && viewPlacedTileSelector()
            && ((viewPlacedTileSelector()->getType() == INTERIOR) || (viewPlacedTileSelector()->getType() == TILE_CENTER))
            && viewPlacedTileSelector()->getPlacedTile() == tile);
}

bool TilingMakerView::tileIsSelected(const PlacedTilePtr & tile)
{
    return (tile->show() && (tile == tilingMaker->selectedTile()));
}

void TilingMakerView::drawTiling(GeoGraphics * g2d, const PlacedTileSet & set)
{
    for (const PlacedTilePtr & ptp : set)
    {
        if (!ptp->show())
            continue;

        QColor color;

        switch (_viewMode)
        {
        case ViewMode::showTiles:
            color = tile_color;
            break;

        case ViewMode::showExcluded:
            switch(ptp->getState().excluded.get())
            {
            case Tristate::eTristate::True:
                color = excluded_color;
                break;
            case Tristate::eTristate::False:
                color = included_color;
                break;
            case Tristate::eTristate::Undefined:
                color = tile_color;
                break;
            }
            break;

        case ViewMode::showAttachment:
            if (ptp->isOverlapping())
            {
                color = overlapping_color;
            }
            else if (ptp->isTouching())
            {
                color = touching_color;
            }
            else
            {
                color = tile_color;
            }
            break;

        case ViewMode::showUnique:
            if (ptp->isUnique())
            {
                color = unique_color;
            }
            else
            {
                color = tile_color;
            }
            break;

        }
        if (ptp == tilingMaker->selectedTile())
        {
            color = selected_color;
        }

        if (tileUnderMouse(ptp))
        {
            color = under_mouse_color;
        }

        drawTile(g2d, ptp, true, color);
    }
}

void TilingMakerView::drawTranslationVectors(GeoGraphics * g2d, TilingPtr tiling, const PlacedTileSet & set)
{
    if (!_hideVectors)
    {
        QPointF tOrigin =  tiling->hdr().getTranslateOrigin();
        if (tOrigin.isNull() && set.count() > 0)
        {
            PlacedTilePtr ptp = set.first();
            if (ptp)
            {
                QTransform T = ptp->getPlacement();
                QPointF trueCenter = T.map(ptp->getTile()->getCenter());
                tOrigin      = undistortedScreenToModel(modelToScreen(trueCenter));
            }
        }

        // draw translation vectors
        QPointF  tp1 =  tiling->hdr().getTrans1();
        QPointF  tp2 =  tiling->hdr().getTrans2();
        QLineF visibleT1;
        QLineF visibleT2;
        if (!tp1.isNull())
        {
            visibleT1.setP1(tOrigin);
            visibleT1.setP2(tOrigin + tp1);
        }
        if (!tp2.isNull())
        {
            visibleT2.setP1(tOrigin);
            visibleT2.setP2(tOrigin + tp2);
        }

        drawTranslationVectors(g2d,visibleT1.p1(),visibleT1.p2(),visibleT2.p1(),visibleT2.p2());
    }
}

void TilingMakerView::drawTranslationVectors(GeoGraphics * g2d, QPointF t1_start, QPointF t1_end, QPointF t2_start, QPointF t2_end)
{
    qreal arrow_length = Transform::distFromInvertedZero(g2d->getTransform(),12.0);
    qreal arrow_width  = Transform::distFromInvertedZero(g2d->getTransform(),6.0);

    QPen pen(construction_color,3);

    if (t1_start != t1_end)
    {
        g2d->drawLine(t1_start, t1_end, pen);
        g2d->drawArrow(t1_start, t1_end, arrow_length, arrow_width, construction_color);
        g2d->drawText(undistortedModelToScreen(t1_end) + QPointF(10,0),"T1");
    }

    if (t2_start != t2_end)
    {
        g2d->drawLine(t2_start, t2_end, pen);
        g2d->drawArrow(t2_start, t2_end, arrow_length, arrow_width, construction_color);
        g2d->drawText(undistortedModelToScreen(t2_end) + QPointF(10,0),"T2");
    }
}

void TilingMakerView::drawTile(GeoGraphics * g2d, PlacedTilePtr pf, bool drawCenter, QColor color)
{
    // fill the polygon
    EdgePoly ep = pf->getWorldEdgePoly(Sys::getActiveDistortion());

    if (pf->show())
    {
        color.setAlpha(qRound(Sys::config->tm_tileOpacity * 255.0 / 100.0));
        QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        g2d->fillEdgePoly(ep,pen);
    }

    QPen pen2;
    if (pf->getTile()->isRegular())
        pen2 = QPen(lineColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    else
        pen2 = QPen(Qt::blue, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    g2d->drawEdgePoly(ep,pen2);

    if (tilingMaker->getMouseMode() == TilingMaker::MouseMode::CurveEdge)
    {
        QPen apen(Qt::blue,3);
        for (const auto & edge : ep.get())
        {
            if (edge->isCurve())
            {
                g2d->drawCircle(edge->getArcCenter(),5,apen,QBrush());
            }
        }
    }

    // draw center circle
    if (drawCenter && pf->show())
    {
        QPointF pt = pf->getTile()->getCenter();
        pt         = pf->getWorldTransform(Sys::getActiveDistortion()).map(pt);

        QPen pen(Qt::red,1);
        g2d->drawCircle(pt,9,pen, QBrush());

        qreal len = screenToModel(9.0);
        g2d->drawLine(QPointF(pt.x()-len,pt.y()),QPointF(pt.x()+len,pt.y()),pen);
        g2d->drawLine(QPointF(pt.x(),pt.y()-len),QPointF(pt.x(),pt.y()+len),pen);
    }
}

void TilingMakerView::drawAccum(GeoGraphics * g2d)
{
    if ( wAccum.size() == 0)
    {
        return;
    }

    QPen pen(construction_color,3);
    QBrush brush(construction_color);
    for (auto it = wAccum.begin(); it != wAccum.end(); it++)
    {
        EdgePtr edge = *it;
        if (edge->getType() == EDGETYPE_LINE)
        {
            //qDebug() << "draw accum edge";
            QPointF p1 = edge->v1->pt;
            QPointF p2 = edge->v2->pt;
            g2d->drawCircle(p1,6,pen,brush);
            g2d->drawCircle(p2,6,pen,brush);
            g2d->drawLine(p1, p2,pen);
        }
        else if (edge->getType() == EDGETYPE_POINT)
        {
            //qDebug() << "draw accum point";
            QPointF p = edge->v1->pt;
            g2d->drawCircle(p,6,pen,brush);
        }
    }
}

const PlacedTileSet & TilingMakerView::getViewablePlacedTiles()
{
    static PlacedTileSet emptySet;

    TilingPtr tp = wTiling.lock();
    if (!tp)
        return emptySet;
    else
        return tp->getDrawSet();
}

// called on demand by paint
PlacedTileSet TilingMakerView::_createViewablePlacedTiles(TilingPtr tiling)
{
    PlacedTileSet ptset;
    PlacedTileSet ptset2;
    if (!tiling)
    {
        return ptset;
    }

    if (Sys::tm_fill == false)
    {
        // viewable consists of tiling unit
        if (_viewMode == ViewMode::showExcluded)
            ptset = tiling->unit().getWithExcluded();
        else
            ptset = tiling->unit().getIncluded();
    }
    else
    {
        if (_viewMode == ViewMode::showExcluded)
            ptset = tiling->unit().getWithExcluded();
        else
            ptset = tiling->unit().getIncluded();

        QPointF t1    = tiling->hdr().getTrans1();
        QPointF t2    = tiling->hdr().getTrans2();
        if (t1.isNull() || t2.isNull())
        {
            return ptset;
        }

        int minX, maxX, minY, maxY;
        bool singleton;
        const FillData fd = tiling->hdr().getTilingCanvas()->getFillData();
        fd.get(singleton,minX, maxX, minY, maxY);

        if (singleton)
        {
            return ptset;
        }

        // Create copies of the tile to help visualise the result in the panel.
        for (const auto & placed : std::as_const(ptset))
        {
            if (!placed->show() || placed->isExcluded())
            {
                continue;
            }


            TilePtr tile = placed->getTile();
            QTransform T = placed->getPlacement();

            for( int y = minY; y <= maxY; ++y )
            {
                for( int x = minX; x <= maxX; ++x )
                {
                    if ( y == 0 && x == 0 )
                    {
                        continue;
                    }
                    QPointF pt    = (t1*x) + (t2 * y);
                    QTransform tt = QTransform::fromTranslate(pt.x(),pt.y());
                    QTransform tr = T * tt;
                    auto ptp = make_shared<PlacedTile>(tile, tr );
                    ptset2.push_back(ptp);
                }
            }
        }
    }
    ptset2 += ptset;     // ptset comes second so overwrites
    return ptset2;
}

void TilingMakerView::drawMeasurements(GeoGraphics *g2d)
{
    QPen pen(construction_color,3);
    for (auto & mm : wMeasurements)
    {
        g2d->drawLineDirect(mm->startS(), mm->endS(),pen);
        QString msg = QString("%1 (%2)").arg(QString::number(mm->lenS(),'f',2)).arg(QString::number(mm->lenW(),'f',8));
        g2d->drawText(mm->endS() + QPointF(10,0),msg);
    }
}

void TilingMakerView::drawAnchors(GeoGraphics * g2d)
{
    QBrush brush(Qt::NoBrush);
    QPen   joinedPen(QColor(Qt::cyan),2);
    QPen   brokenPen(QColor(Qt::red), 2, Qt::DashLine);

    const QVector<TileAnchor> & anchors = tilingMaker->getAnchors();
    for (const auto & anchor : anchors)
    {
        EdgePtr edgeA = TilingMaker::edgeAt(anchor.tileA, anchor.edgeIndexA);
        EdgePtr edgeB = TilingMaker::edgeAt(anchor.tileB, anchor.edgeIndexB);
        if (!edgeA || !edgeB)
            continue;

        QPointF ptA = anchor.tileA->getPlacement().map(edgeA->getMidPoint());
        QPointF ptB = anchor.tileB->getPlacement().map(edgeB->getMidPoint());

        if (Geo::dist2(modelToScreen(ptA), modelToScreen(ptB)) < 49.0)
        {
            // still touching - a single marker at the shared joint
            g2d->drawCircle(ptA, 16, joinedPen, brush);
            g2d->drawAnchorGlyph(ptA, 24, Qt::cyan);
        }
        else
        {
            // pulled apart by a plain drag/edit that didn't propagate - show both ends and the gap
            g2d->drawCircle(ptA, 16, brokenPen, brush);
            g2d->drawCircle(ptB, 16, brokenPen, brush);
            g2d->drawLine(QLineF(ptA,ptB), brokenPen);
            g2d->drawAnchorGlyph(ptA, 24, Qt::red);
            g2d->drawAnchorGlyph(ptB, 24, Qt::red);
        }
    }
}

void TilingMakerView::drawTranslationAnchors(GeoGraphics * g2d)
{
    if (!Sys::config->tm_vectorAnchorMode)
        return;

    QBrush brush(Qt::NoBrush);
    QPen   pen(QColor(Qt::cyan),2);

    const TransAnchor & origin = tilingMaker->getOriginAnchor();
    const TransAnchor & t1     = tilingMaker->getT1Anchor();
    const TransAnchor & t2     = tilingMaker->getT2Anchor();

    if (origin.tile)
        g2d->drawCircle(origin.tile->getPlacement().map(origin.localPt), 12, pen, brush);
    if (t1.tile)
        g2d->drawCircle(t1.tile->getPlacement().map(t1.localPt), 12, pen, brush);
    if (t2.tile)
        g2d->drawCircle(t2.tile->getPlacement().map(t2.localPt), 12, pen, brush);
}

void TilingMakerView::hideVectors(bool state)
{
    _hideVectors = state;
    forceRedraw();
}

//
// Tile, edge and vertex finding.
//

PlacedTileSelectorPtr TilingMakerView::findTile(QPointF spt)
{
    PlacedTileSelectorPtr nothingToIgnore;
    return findTile(spt, nothingToIgnore);
}

PlacedTileSelectorPtr TilingMakerView::findTile(QPointF spt, PlacedTileSelectorPtr ignore)
{
    QPointF wpt = screenToModel(spt);

    PlacedTileSet viewable = getViewablePlacedTiles();  // copies

    // reverse the list so that newest additions are 'on top'
    std::reverse(viewable.begin(), viewable.end());

    for (auto placedTile : std::as_const(viewable))
    {
        if (ignore && (ignore->getPlacedTile() == placedTile))
            continue;

        QPolygonF pgon = placedTile->getWorldPoints(Sys::getActiveDistortion());
        if (pgon.containsPoint(wpt,Qt::OddEvenFill))
        {
            return make_shared<InteriorTilleSelector>(placedTile);
        }
    }
    
    PlacedTileSelectorPtr sel;
    return sel;
}

PlacedTileSelectorPtr TilingMakerView::findVertex(QPointF spt)
{
    PlacedTileSelectorPtr nothingToIgnore;
    return findVertex(spt,nothingToIgnore);
}

PlacedTileSelectorPtr TilingMakerView::findVertex(QPointF spt,PlacedTileSelectorPtr ignore)
{
    const PlacedTileSet & viewable = getViewablePlacedTiles();
    for(auto & pf : std::as_const(viewable))
    {
        if (ignore && (ignore->getPlacedTile() == pf))
            continue;

        TilePtr tile = pf->getTile();
        if (!tile)
            continue;

        QPolygonF pgon = tile->getPoints();
        QTransform   T = pf->getWorldTransform(Sys::getActiveDistortion());
        for( int v = 0; v < pgon.size(); ++v )
        {
            QPointF a = pgon[v];
            QPointF b = T.map(a);
            QPointF c = modelToScreen(b);
            if (Geo::dist2(spt,c) < 49.0 )
            {
                return make_shared<VertexTileSelector>(pf,a);
            }
        }
    }
    
    PlacedTileSelectorPtr sel;
    return sel;
}

PlacedTileSelectorPtr TilingMakerView::findMidPoint(QPointF spt)
{
    PlacedTileSelectorPtr nothingToIgnore;
    return findMidPoint(spt,nothingToIgnore);
}

PlacedTileSelectorPtr TilingMakerView::findMidPoint(QPointF spt, PlacedTileSelectorPtr ignore)
{
    PlacedTileSelectorPtr sel;

    const PlacedTileSet & viewable = getViewablePlacedTiles();
    for(auto & placed : std::as_const(viewable))
    {
        if (ignore && (ignore->getPlacedTile() == placed))
            continue;

        TilePtr tile = placed->getTile();
        if (!tile)
            continue;

        const EdgePoly & ep  = tile->getEdgePoly();
        QTransform T = placed->getWorldTransform(Sys::getActiveDistortion());
        for (const auto & edge : ep.get())
        {
            QPointF a     = edge->v1->pt;
            QPointF b     = edge->v2->pt;
            QPointF mid   = edge->getMidPoint();
            QPointF aa    = T.map(a);
            QPointF bb    = T.map(b);
            QPointF pmid  = T.map(mid);
            QPointF smid  = modelToScreen(pmid);
            if (Geo::dist2(spt,smid) < 49.0)
            {
                // Avoid selecting middle point if end-points are too close together.
                QPointF a2 = modelToScreen(aa);
                QPointF b2 = modelToScreen(bb);
                qreal screenDist = Geo::dist2(a2,b2);
                if ( screenDist < (6.0 * 6.0 * 6.0 * 6.0) )
                {
                    qDebug() << "Screen dist too small = " << screenDist;
                    return sel;
                }
                return make_shared<MidPointTileSelector>(placed, edge, mid);
            }
        }
    }

    return sel;
}

PlacedTileSelectorPtr TilingMakerView::findArcPoint(QPointF spt)
{
    PlacedTileSelectorPtr sel;
    const PlacedTileSet & viewable = getViewablePlacedTiles();
    for(const auto & pf : std::as_const(viewable))
    {
        TilePtr tile = pf->getTile();
        if (!tile)
            continue;

        const EdgePoly & epoly = tile->getEdgePoly();
        QTransform T   = pf->getWorldTransform(Sys::getActiveDistortion());

        for(const auto & ep : epoly.get())
        {
            if (ep->getType() == EDGETYPE_CURVE)
            {
                QPointF a    = ep->getArcCenter();
                QPointF aa   = T.map(a);
                QPointF aad  = modelToScreen(aa);
                if (Geo::dist2(spt,aad) < 49.0)
                {
                    return make_shared<ArcPointTileSelector>(pf, ep, a);
                }
            }
        }
    }

    return sel;
}

PlacedTileSelectorPtr TilingMakerView::findEdge(QPointF spt)
{
    PlacedTileSelectorPtr nothingToIgnore;
    return findEdge(spt, nothingToIgnore);
}

PlacedTileSelectorPtr TilingMakerView::findEdge(QPointF spt, PlacedTileSelectorPtr ignore )
{
    const PlacedTileSet & viewable = getViewablePlacedTiles();
    for(const auto & pf : std::as_const(viewable))
    {
        if (ignore && (ignore->getPlacedTile() == pf))
            continue;

        TilePtr tile = pf->getTile();
        if (!tile)
            continue;

        const EdgePoly & epoly = tile->getEdgePoly();
        QTransform T   = pf->getWorldTransform(Sys::getActiveDistortion());
        const EdgeSet & eset = epoly.get();
        for (uint v = 0; v < eset.size(); ++v)
        {
            EdgePtr edge  = eset[v];
            QLineF  line  = edge->getLine();
            QLineF  lineW = T.map(line);
            QLineF  LineS = modelToScreen(lineW);
            
            if (Geo::distToLine(spt, LineS) < 7.0)
            {
                return make_shared<EdgeTileSelector>(pf,edge);
            }
        }
    }
    
    PlacedTileSelectorPtr sel;
    return sel;
}

QPointF TilingMakerView::undistortedScreenToModel(QPointF spt)
{
    return getUndistortedLayerTransform().inverted().map(spt);
}

QPointF TilingMakerView::undistortedModelToScreen(QPointF wpt)
{
    return getUndistortedLayerTransform().map(wpt);
}

QLineF TilingMakerView::undistortedModelToScreen(QLineF line)
{
    QTransform t = getUndistortedLayerTransform();
    return QLineF(t.map(line.p1()), t.map(line.p2()));
}

int TilingMakerView::findConstructionLineNear(QPointF spt)
{
    return ConstructionLinesView::findLineNear(constructionLines, getUndistortedLayerTransform(), spt);
}

int TilingMakerView::findConstructionEndpointNear(int lineIndex, QPointF spt)
{
    if (lineIndex < 0 || lineIndex >= constructionLines.size())
        return -1;

    return ConstructionLinesView::findEndpointNear(constructionLines[lineIndex], getUndistortedLayerTransform(), spt);
}

// finds the best snap candidate near spt: existing tile vertex/edge-midpoint/center,
// then nearest point on any tile edge, then any construction line's endpoint or midpoint.
// returns false (outPt untouched) if nothing is near enough.
bool TilingMakerView::findConstructionSnapCandidate(QPointF spt, QPointF & outPt, PlacedTileSelectorPtr ignore, int excludeLineIndex)
{
    constexpr qreal tolerance2 = 7.0 * 7.0;

    // a tile candidate is a true (undistorted) model point, but the tile itself is drawn
    // distorted - re-express the candidate as wherever it actually appears on screen right now,
    // so a construction line snapped to it starts out visually touching the tile, then behaves
    // like any other fixed construction-line point from then on (won't keep tracking the tile).
    PlacedTileSelectorPtr sel = findPoint(spt, ignore);
    if (sel)
    {
        qDebug() << "SNAP - tile vertex/edge-midpoint/center";
        outPt = undistortedScreenToModel(modelToScreen(sel->getPlacedPoint(true)));
        return true;
    }

    PlacedTileSelectorPtr esel = findEdge(spt, ignore);
    if (esel)
    {
        qDebug() << "SNAP - tile edge";
        QLineF line = esel->getPlacedLine(true);
        QPointF truePt = Geo::getClosestPoint(line, screenToModel(spt));
        outPt = undistortedScreenToModel(modelToScreen(truePt));
        return true;
    }

    for (int i = 0; i < constructionLines.size(); i++)
    {
        if (i == excludeLineIndex)
            continue;

        const QLineF & line = constructionLines[i];
        if (Geo::dist2(spt, undistortedModelToScreen(line.p1())) < tolerance2)
        {
            qDebug() << "SNAP - construction line endpoint";
            outPt = line.p1();
            return true;
        }
        if (Geo::dist2(spt, undistortedModelToScreen(line.p2())) < tolerance2)
        {
            qDebug() << "SNAP - construction line endpoint";
            outPt = line.p2();
            return true;
        }
        QPointF mid = line.pointAt(0.5);
        if (Geo::dist2(spt, undistortedModelToScreen(mid)) < tolerance2)
        {
            qDebug() << "SNAP - construction line midpoint";
            outPt = mid;
            return true;
        }
    }

    for (int i = 0; i < constructionLines.size(); i++)
    {
        if (i == excludeLineIndex)
            continue;

        for (int j = i+1; j < constructionLines.size(); j++)
        {
            if (j == excludeLineIndex)
                continue;

            QPointF ipt;
            if (constructionLines[i].intersects(constructionLines[j], &ipt) == QLineF::BoundedIntersection
                && Geo::dist2(spt, undistortedModelToScreen(ipt)) < tolerance2)
            {
                qDebug() << "SNAP - construction line intersection";
                outPt = ipt;
                return true;
            }
        }
    }

    return false;
}

// snap target for drawing a new construction line - the found candidate, else the raw model point
QPointF TilingMakerView::findConstructionSnapPoint(QPointF spt)
{
    QPointF pt;
    if (findConstructionSnapCandidate(spt, pt))
        return pt;

    return undistortedScreenToModel(spt);
}

// persistent connectivity check for one endpoint of a construction line (drawn every frame,
// unlike the transient drag-time snap feedback): does it actually touch a tile vertex/edge or
// another construction line's body (covers its endpoints, midpoint, and T-junctions/crossings)?
TilingMakerView::ConstructionEndpointState TilingMakerView::classifyConstructionEndpoint(QPointF pt, int excludeLineIndex)
{
    constexpr qreal matchTolerance = 5.0;
    constexpr qreal nearTolerance  = 20.0;

    // pt is a construction-line endpoint, always actually drawn via the undistorted transform
    // (findConstructionSnapCandidate pre-compensates tile-snapped points for this) - so its real
    // current screen position, for matching against anything else currently on screen, is here.
    QPointF ptSu = undistortedModelToScreen(pt);

    if (findPoint(ptSu))
        return ConstructionEndpointState::Match;

    if (findEdge(ptSu))
        return ConstructionEndpointState::Match;

    bool near = false;
    for (int j = 0; j < constructionLines.size(); j++)
    {
        if (j == excludeLineIndex)
            continue;

        QLineF otherS = undistortedModelToScreen(constructionLines[j]);
        qreal d = Geo::distToLine(ptSu, otherS);
        if (d <= matchTolerance)
            return ConstructionEndpointState::Match;
        if (d <= nearTolerance)
            near = true;
    }

    return near ? ConstructionEndpointState::Near : ConstructionEndpointState::None;
}

PlacedTileSelectorPtr TilingMakerView::findSelection(QPointF spt)
{
    PlacedTileSelectorPtr sel;

    if (      (sel = findVertex(spt)) )
        return sel;
    else if ( (sel = findMidPoint(spt)) )
        return sel;
    else if ( (sel = findEdge(spt)) )
        return sel;
    else
    {
        PlacedTileSelectorPtr sel2 = findTile(spt);
        if (sel2)
        {
            sel = findCenter(sel2->getPlacedTile(),spt);
            if (!sel)
                sel = sel2;
        }
    }
    return sel;
}

PlacedTileSelectorPtr TilingMakerView::findPoint(QPointF spt)
{
    PlacedTileSelectorPtr nothingToIgnore;
    return findPoint(spt,nothingToIgnore);
}

PlacedTileSelectorPtr TilingMakerView::findPoint(QPointF spt, PlacedTileSelectorPtr ignore)
{
    PlacedTileSelectorPtr sel = findVertex(spt,ignore);
    if (!sel)
    {
        sel = findMidPoint(spt,ignore);
    }
    if (!sel)
    {
        PlacedTileSelectorPtr sel2 = findTile(spt,ignore);
        if (sel2)
        {
            sel = findCenter(sel2->getPlacedTile(),spt);
        }
    }
    //if (sel)
    //    qDebug().noquote() << "findPoint: found -" << sel->getTypeString();
    return sel;
}

QPointF TilingMakerView::findSelectionPointOrPoint(QPointF spt)
{
    PlacedTileSelectorPtr sel = findPoint(spt);
    if (sel)
    {
        qDebug() << "SNAP point:" << spt << "->" << sel->getPlacedPoint(true);
        return sel->getPlacedPoint(true);
    }

    QPointF pt;
    if (findConstructionSnapCandidate(spt, pt))
    {
        qDebug() << "SNAP construction:" << spt << "->" << pt;
        return pt;
    }

    return screenToModel(spt);
}

PlacedTileSelectorPtr TilingMakerView::findCenter(PlacedTilePtr pf, QPointF spt)
{
    EdgePoly  epoly = pf->getWorldEdgePoly(Sys::getActiveDistortion());
    QPointF    wpt  = epoly.calcCenter();
    QPointF    spt2 = modelToScreen(wpt);

    constexpr qreal tolerance2 = 20.0 * 20.0;
    if (Geo::dist2(spt,spt2) < tolerance2)
    {
        TilePtr tile = pf->getTile();
        QPointF mpt  = tile->getCenter();
        return make_shared<CenterTileSelector>(pf, mpt);
    }
    
    PlacedTileSelectorPtr sel;
    return sel;
}

bool TilingMakerView::accumHasPoint(QPointF wpt)
{
    QPointF newpoint = modelToScreen(wpt);
    for (auto it = wAccum.begin(); it != wAccum.end(); it++)
    {
        EdgePtr edge = *it;
        QPointF existing = modelToScreen(edge->v1->pt);
        if (Geo::isNear(newpoint,existing))
        {
            return true;
        }
    }
    return false;
}

PlacedTileSelectorPtr TilingMakerView::findTileUnderMouse()
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

void TilingMakerView::startMouseInteraction(QPointF spt, Qt::MouseButton mouseButton)
{
    Q_ASSERT(tilingMaker->getMouseMode() == TilingMaker::MouseMode::None);

    PlacedTileSelectorPtr sel = findSelection(spt);
    if(sel)
    {
        qDebug() << "placed tile" << sel->getPlacedTile().get();
        // do this first
        wAccum.clear();
        mouse_interaction.reset();

        if (mouseButton == Qt::LeftButton)
        {
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
                tilingMaker->selectTile(sel->getPlacedTile());
                mouse_interaction = make_shared<MovePolygon>(sel, spt);
                break;
            case ARC_POINT:
                break;
            case SCREEN_POINT:
                break;
            }
        }
        else if (mouseButton == Qt::MiddleButton)
        {
            if (sel->isPoint())
                mouse_interaction = make_shared<DrawTranslation>(sel, sel->getPlacedPoint(true), spt, QPen(lineColor,3));
        }
        else if (mouseButton == Qt::RightButton)
        {
            qDebug().noquote() << "right button selection:" << sel->getTypeString();
            eSelection type = sel->getType();
            switch (type)
            {
            case VERTEX:
            case TILE_CENTER:
            case MID_POINT:
            case EDGE:
            case INTERIOR:
            {
                setViewPlacedTileSelector(sel,true);    // toggles save

                tilingMaker->setClickedPoint(spt);      // save
                PlacedTilePtr tile = sel->getPlacedTile();

                QMenu myMenu;
                myMenu.addSection("Options");
                myMenu.addSeparator();

                if (type == INTERIOR)
                    myMenu.addAction("Copy/Move Tile",  tilingMaker, &TilingMaker::slot_copyMoveTile);
                else if (type == VERTEX || type == TILE_CENTER)
                    myMenu.addAction("Copy/Join Corner",tilingMaker, &TilingMaker::slot_copyJoinPoint);
                else if (type == MID_POINT)
                {
                    myMenu.addAction("Copy/Join Mid",   tilingMaker, &TilingMaker::slot_copyJoinMidPoint);
                    myMenu.addAction("Create Anchor Here", tilingMaker, &TilingMaker::slot_createAnchor);
                    myMenu.addAction("Remove Anchor",      tilingMaker, &TilingMaker::slot_removeAnchor);
                }
                else if (type == EDGE)
                    myMenu.addAction("Copy/Move Edge",  tilingMaker, &TilingMaker::slot_copyJoinEdge);

                if (type == INTERIOR || type == TILE_CENTER)
                    myMenu.addAction("Move to corner",  this,        &TilingMakerView::slot_subMenu1);

                myMenu.addAction("Edit Tile",           tilingMaker, &TilingMaker::slot_view_menu_editTile);

                if (tile == tilingMaker->selectedTile())
                    myMenu.addAction("De-Select",       tilingMaker, &TilingMaker::slot_deselectTile);

                if (tile->show())
                    myMenu.addAction("Hide Tile",       tilingMaker, &TilingMaker::slot_hideTile);
                else
                    myMenu.addAction("Show Tile",       tilingMaker, &TilingMaker::slot_showTile);

                if (tile->isIncluded())
                    myMenu.addAction("Exclude",         tilingMaker, &TilingMaker::slot_excludeTile);
                else
                    myMenu.addAction("Include",         tilingMaker, &TilingMaker::slot_includeTile);

                myMenu.addAction("Delete",              tilingMaker, &TilingMaker::slot_deleteTile);

                myMenu.addAction("Uniquify",            tilingMaker, &TilingMaker::slot_uniquifyTile);

                if (tile->getTile()->isRegular())
                    myMenu.addAction("Make irregular",  tilingMaker, &TilingMaker::slot_convertTile);
                else
                    myMenu.addAction("Make regular",    tilingMaker, &TilingMaker::slot_convertTile);

                myMenu.exec(Sys::systemViewController->mapToGlobal(spt.toPoint()));
            }   break;

            case ARC_POINT:
            case SCREEN_POINT:
                break;
            }
        }
    }
    else
    {
        tilingMaker->selectTile(nullptr);
        forceRedraw();
        emit tilingMaker->sig_menuRefresh(TMR_TILING_UNIT);
    }
}

void TilingMakerView::slot_subMenu1()
{
    QMenu myMenu;
    myMenu.addAction("Move to Top Left",      this, &TilingMakerView::slot_TL);
    myMenu.addAction("Move to Top Right",    this, &TilingMakerView::slot_TR);
    myMenu.addAction("Move to Bottom Left",  this, &TilingMakerView::slot_BL);
    myMenu.addAction("Move to Bottom Right", this, &TilingMakerView::slot_BR);
    myMenu.exec(QCursor::pos() - QPoint(30,10));
}

void TilingMakerView::slot_TL()
{
    QPoint pt   = Sys::systemViewController->getViewRect().topLeft();
    QPointF pt2 = screenToModel(QPointF(pt));
    tilingMaker->moveTileTo(pt2);
}

void TilingMakerView::slot_TR()
{
    QPoint pt   = Sys::systemViewController->getViewRect().topRight();
    QPointF pt2 = screenToModel(QPointF(pt));
    tilingMaker->moveTileTo(pt2);
}

void TilingMakerView::slot_BL()
{
    QPoint pt   = Sys::systemViewController->getViewRect().bottomLeft();
    QPointF pt2 = screenToModel(QPointF(pt));
    tilingMaker->moveTileTo(pt2);
}

void TilingMakerView::slot_BR()
{
    QPoint pt   = Sys::systemViewController->getViewRect().bottomRight();
    QPointF pt2 = screenToModel(QPointF(pt));
    tilingMaker->moveTileTo(pt2);
}

// called bymouse_moved and by mouse_dragged
void TilingMakerView::updateUnderMouse(QPointF spt)
{
    PlacedTileSelectorPtr sel;
    switch (tilingMaker->getMouseMode())
    {
    case TilingMaker::MouseMode::None:
    case TilingMaker::MouseMode::Measure:
    case TilingMaker::MouseMode::Copy:
    case TilingMaker::MouseMode::Delete:
    case TilingMaker::MouseMode::Inclusion:
    case TilingMaker::MouseMode::Edit:
    case TilingMaker::MouseMode::MirrorX:
    case TilingMaker::MouseMode::MirrorY:
    case TilingMaker::MouseMode::ReflectEdge:
    case TilingMaker::MouseMode::Uniquify:
    case TilingMaker::MouseMode::Unify:
    case TilingMaker::MouseMode::Decompose:
        sel = findSelection(spt);
        break;

    case TilingMaker::MouseMode::TranslationVectors:
        sel = findSelection(spt);
        break;

    case TilingMaker::MouseMode::ConstructionLines:
        sel = findSelection(spt);
        break;

    case TilingMaker::MouseMode::DrawPolygon:
    {
        sel = findVertex(spt);
        if (!sel)
        {
            sel = findMidPoint(spt);
        }
        if (!sel)
        {
            sel = findNearGridPoint(spt);
        }
    }    break;

    case TilingMaker::MouseMode::Position:
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

    case TilingMaker::MouseMode::CurveEdge:
    {
        sel = findArcPoint(spt);
        if (sel)
        {
            qDebug() << "updateUnderMouse: found arc center";
        }
        else
        {
            sel = findEdge(spt);
            if (sel)
            {
                qDebug() << "updateUnderMouse: found edge";
            }
            else
            {
                setViewPlacedTileSelector(nullptr);
            }
        }
    }    break;
    }

    setViewPlacedTileSelector(sel);
    forceRedraw();
}

void  TilingMakerView::drawMouseInteraction(GeoGraphics * g2d)
{
    if (mouse_interaction)
    {
        mouse_interaction->draw(g2d);
    }
}

// used when creating a polygon
PlacedTileSelectorPtr TilingMakerView::findNearGridPoint(QPointF spt)
{
    PlacedTileSelectorPtr tsp;
    QPointF p;

    if (Sys::gridViewer->nearGridPoint(spt,p))
    {
        tsp = make_shared<ScreenTileSelector>(p);  // not really a vertex, but good enough
    }
    return tsp;
}

//////////////////////////////////////////////////////////////////
///
/// Layer slots
///
//////////////////////////////////////////////////////////////////

void TilingMakerView::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    if (!viewControl()->isEnabled(type())) return;

    sMousePos = spt;

    if (debugMouse) qDebug() << "slot_mousePressed:" << sMousePos << screenToModel(sMousePos);

    PlacedTileSelectorPtr sel;

    switch (tilingMaker->getMouseMode())
    {
    case TilingMaker::MouseMode::None:
        if (Sys::getSysMouseMode(MOUSE_MODE_NONE))
        {
            startMouseInteraction(sMousePos,btn);
        }
        break;

    case TilingMaker::MouseMode::Copy:
        sel = findTileUnderMouse();
        if (sel)
            mouse_interaction = make_shared<CopyMovePolygon>(sel, sMousePos);
        break;

    case TilingMaker::MouseMode::Delete:
        tilingMaker->deleteTile( findTileUnderMouse() );
        tilingMaker->setMouseMode(TilingMaker::MouseMode::None);
        break;

    case TilingMaker::MouseMode::TranslationVectors:
    {
        sel = findSelection(sMousePos);
        QPointF originPt;
        bool ok = false;
        if (sel && sel->isPoint())
        {
            originPt = sel->getPlacedPoint(true);
            ok = true;
        }
        else
        {
            sel = PlacedTileSelectorPtr();     // not a tile point - try a construction-line snap instead
            ok  = findConstructionSnapCandidate(sMousePos, originPt);
        }
        if (ok)
            mouse_interaction = make_shared<DrawTranslation>(sel, originPt, sMousePos, QPen(lineColor,3));
        break;
    }

    case TilingMaker::MouseMode::DrawPolygon:
        mouse_interaction = make_shared<CreatePolygon>(sMousePos);
        break;

    case TilingMaker::MouseMode::Position:
        break;

    case TilingMaker::MouseMode::Inclusion:
        tilingMaker->toggleInclusion(findTileUnderMouse());
        break;

    case TilingMaker::MouseMode::Measure:
    {
        Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
        if (kms == (Qt::CTRL | Qt::SHIFT))
        {
            PlacedTileSelectorPtr nothingToIgnore;
            sel = findEdge(spt,nothingToIgnore);
            if (sel)
            {
                // TODO  - this looks like incomplete code
                QLineF line = sel->getPlacedLine(true);
                qreal dist = Geo::distToLine(screenToModel(spt),line);
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

    case TilingMaker::MouseMode::Edit:
        sel = findSelection(spt);
        if (sel)
        {
            if (sel->getType() == INTERIOR)
            {
                // this could be a different tile than before
                setEditPlacedTile(sel->getPlacedTile());
                tilingMaker->editTile(editPlacedTile());
            }
            else if (editPlacedTile() && sel->getType() == VERTEX )
            {
                mouse_interaction = make_shared<EditTilePoint>(sel, editPlacedTile(), sMousePos);
            }
        }
        break;

    case TilingMaker::MouseMode::CurveEdge:
        sel = findArcPoint(spt);
        if (sel)
        {
            setViewPlacedTileSelector(sel);
            QMenu myMenu;
            myMenu.addAction("Use Cursor to change curve", tilingMaker, SLOT(slot_moveArcCenter()));
            myMenu.addAction("Edit Magnitude", tilingMaker, SLOT(slot_editMagnitude()));
            myMenu.exec(Sys::systemViewController->mapToGlobal(spt.toPoint()));
        }
        else
        {
            sel = findEdge(spt);
            if (sel)
            {
                auto tile = sel->getPlacedTile()->getTile();
                if (tile->isRegular())
                {
                    tilingMaker->flipTileRegularity(tile);
                }
                else
                {
                    EdgePtr edge =  sel->getModelEdge();
                    if (edge->getType() == EDGETYPE_LINE)
                    {
                        setViewPlacedTileSelector(sel);
                        QMenu myMenu;
                        myMenu.addAction("Make Convex",  tilingMaker, SLOT(slot_createConvex()));
                        myMenu.addAction("Make Concave", tilingMaker, SLOT(slot_createConcave()));
                        myMenu.exec(Sys::systemViewController->mapToGlobal(spt.toPoint()));
                    }
                    else if (edge->getType() == EDGETYPE_CURVE)
                    {
                        setViewPlacedTileSelector(sel);
                        QMenu myMenu;
                        myMenu.addAction("Make Flat",  tilingMaker, SLOT(slot_flatenCurve()));
                        if (edge->getCurveType() == CURVE_CONVEX)
                            myMenu.addAction("Make Concave",  tilingMaker, SLOT(slot_makeConcave()));
                        else
                            myMenu.addAction("Make Convex",  tilingMaker, SLOT(slot_makeConvex()));
                        myMenu.addAction("Use Cursor to change curve", tilingMaker, SLOT(slot_moveArcCenter()));
                        myMenu.addAction("Edit Magnitude", tilingMaker, SLOT(slot_editMagnitude()));
                        myMenu.exec(Sys::systemViewController->mapToGlobal(spt.toPoint()));
                    }
                }
                // Methinks tilings cannot have chords only curves(arcs)
            }
        }
        break;

    case TilingMaker::MouseMode::MirrorX:
        tilingMaker->mirrorPolygonX(findTileUnderMouse());
        break;

    case TilingMaker::MouseMode::MirrorY:
        tilingMaker->mirrorPolygonY(findTileUnderMouse());
        break;

    case TilingMaker::MouseMode::ConstructionLines:
        break;     // handled by ConstructionLinesView

    case TilingMaker::MouseMode::ReflectEdge:
    {
        auto fsel = findTileUnderMouse();
        if (fsel)
        {
            if (tilingMaker->reflectPolygon(findTileUnderMouse()))
            {
                tilingMaker->setMouseMode(TilingMaker::MouseMode::None);
            }
        }
    }   break;

    case TilingMaker::MouseMode::Unify:
    {
        sel = findTileUnderMouse();
        if (sel && sel->getType() != SCREEN_POINT )
        {
            tilingMaker->unifyTile(sel->getPlacedTile());
        }
    }   break;

    case TilingMaker::MouseMode::Uniquify:
    {
        sel = findTileUnderMouse();
        if (sel && sel->getType() != SCREEN_POINT )
        {
            tilingMaker->uniquifyTile(sel->getPlacedTile());
        }
    }   break;

    case TilingMaker::MouseMode::Decompose:
    {
        sel = findTileUnderMouse();
        if (sel && sel->getType() != SCREEN_POINT )
        {
            tilingMaker->decomposeTile(sel->getPlacedTile());
        }
    }   break;

    }  //end of case statement

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
    if (!viewControl()->isEnabled(type())) return;

    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << sMousePos << screenToModel(sMousePos)  << sTilingMakerMouseMode[(int)tilingMaker->getMouseMode()];

    updateUnderMouse(sMousePos);

    switch (tilingMaker->getMouseMode())
    {
    case TilingMaker::MouseMode::None:
    case TilingMaker::MouseMode::Copy:
    case TilingMaker::MouseMode::TranslationVectors:
    case TilingMaker::MouseMode::DrawPolygon:
    case TilingMaker::MouseMode::Measure:
    case TilingMaker::MouseMode::Position:
    case TilingMaker::MouseMode::Edit:
    case TilingMaker::MouseMode::CurveEdge:
        if (mouse_interaction)
            mouse_interaction->updateDragging(sMousePos);
        break;
    case TilingMaker::MouseMode::ConstructionLines:
    case TilingMaker::MouseMode::Delete:
    case TilingMaker::MouseMode::MirrorX:
    case TilingMaker::MouseMode::MirrorY:
    case TilingMaker::MouseMode::Inclusion:
    case TilingMaker::MouseMode::ReflectEdge:
    case TilingMaker::MouseMode::Uniquify:
    case TilingMaker::MouseMode::Unify:
    case TilingMaker::MouseMode::Decompose:
        break;
    }
}

void TilingMakerView::slot_mouseMoved(QPointF spt)
{
    if (!viewControl()->isEnabled(type())) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << sMousePos;

    updateUnderMouse(sMousePos);
}

void TilingMakerView::slot_mouseReleased(QPointF spt)
{
    if (!viewControl()->isEnabled(type())) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "release" << sMousePos << screenToModel(sMousePos);

    if (mouse_interaction)
    {
        mouse_interaction->endDragging(spt);
        mouse_interaction.reset();
    }
    forceRedraw();
}

void TilingMakerView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

void TilingMakerView::slot_wheel_scale(uint sigid, qreal delta)
{
    if (!validateSignal()) return;

    switch (tilingMaker->getKeybdMode())
    {
    case TilingMaker::KeybdMode::XformTiling:
    {
        qreal sc = 1.0 + delta;
        QTransform ts;
        ts.scale(sc,sc);
        const PlacedTileSet & viewable = getViewablePlacedTiles();
        for (const auto & pplacedTile : std::as_const(viewable))
        {
            QTransform t = pplacedTile->getPlacement();
            t *= ts;
            pplacedTile->setPlacement(t);
        }

        if (tilingMaker->getPropagate())
        {
            MotifEvent motifEvent;
            motifEvent.event = MotifMaker::Event::TilingChanged;
            motifEvent.tiling = wTiling.lock();
            Sys::motifMaker->sm_takeUp(motifEvent);
        }

        forceRedraw();
        emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
    }   break;

    case TilingMaker::KeybdMode::XFormPlacedTile:
        tilingMaker->placedTileDeltaScale(delta);
        break;

    case TilingMaker::KeybdMode::XFormUniqueTile:
        tilingMaker->uniqueTileDeltaScale(delta);
        break;

    case TilingMaker::KeybdMode::XformAll:
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setModelXform(xf,true,sigid);
        emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
    }   break;
    }
}

void TilingMakerView::slot_wheel_rotate(uint sigid, qreal delta)
{
    if (!validateSignal()) return;

    switch (tilingMaker->getKeybdMode())
    {
    case TilingMaker::KeybdMode::XformTiling:
    {
        QTransform tr;
        tr.rotate(delta);

        const PlacedTileSet & viewable = getViewablePlacedTiles();
        for (const auto & pfp : std::as_const(viewable))
        {
            QTransform t = pfp->getPlacement();
            t *= tr;
            pfp->setPlacement(t);
        }

        if (tilingMaker->getPropagate())
        {
            MotifEvent motifEvent;
            motifEvent.event  = MotifMaker::Event::TilingChanged;
            motifEvent.tiling = wTiling.lock();
            Sys::motifMaker->sm_takeUp(motifEvent);
        }

        forceRedraw();
        emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
    }   break;

    case TilingMaker::KeybdMode::XFormPlacedTile:
        tilingMaker-> placedTileDeltaRotate(0.5 * delta);
        break;

    case TilingMaker::KeybdMode::XFormUniqueTile:
        tilingMaker->uniqueTileDeltaRotate(0.5 * delta);
        break;

    case TilingMaker::KeybdMode::XformAll:
    {
        Xform xf = getModelXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setModelXform(xf,true,sigid);
        emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
    }   break;
    }
}

void TilingMakerView::slot_mouseTranslate(uint sigid, QPointF spt)
{
    if (!validateSignal()) return;

    switch (tilingMaker->getKeybdMode())
    {
    case TilingMaker::KeybdMode::XformTiling:
    {
        QTransform T  = getCanvasTransform();
        qreal scale   = Transform::scalex(T);
        QPointF mpt   = spt/scale;
        QTransform tt = QTransform::fromTranslate(mpt.x(),mpt.y());

        const PlacedTileSet & viewable = getViewablePlacedTiles();
        for (const auto & placedTile : std::as_const(viewable))
        {
            QTransform t = placedTile->getPlacement();
            t *= tt;
            placedTile->setPlacement(t);
        }

        if (tilingMaker->getPropagate())
        {
            MotifEvent motifEvent;
            motifEvent.event = MotifMaker::Event::TilingChanged;
            motifEvent.tiling = wTiling.lock();
            Sys::motifMaker->sm_takeUp(motifEvent);
        }
    }   break;

    case TilingMaker::KeybdMode::XFormPlacedTile:
    {
        PlacedTilePtr currentTile = tilingMaker->selectedTile();
        if (!currentTile)
            return;

        QTransform T = getCanvasTransform();
        qreal scale = Transform::scalex(T);
        QPointF mpt = spt/scale;
        QTransform tt = QTransform::fromTranslate(mpt.x(),mpt.y());

        QTransform t = currentTile->getPlacement();
        t *= tt;
        currentTile->setPlacement(t);

        if (tilingMaker->getPropagate())
        {
            MotifEvent motifEvent;
            motifEvent.event  = MotifMaker::Event::PlaceTileTransformChanged;
            motifEvent.tiling = tilingMaker->getSelectedTiling();
            motifEvent.ptile  = currentTile;
            Sys::motifMaker->sm_takeUp(motifEvent);
        }
    }   break;

    case TilingMaker::KeybdMode::XFormUniqueTile:
        break;  // tiles only have scale and rotation

    case TilingMaker::KeybdMode::XformAll:
    {
        Xform xf = getModelXform();
        spt /= xf.getScale();
        xf.setTranslate(xf.getTranslate() + spt);
        setModelXform(xf,true,sigid);
    }   break;
    }

    forceRedraw();
    emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
}

void TilingMakerView::slot_scale(uint sigid, int amount)
{
    //qDebug() << "TilingMaker::slot_scale" << amount;

    if (!validateSignal()) return;

    switch (tilingMaker->getKeybdMode())
    {
    case TilingMaker::KeybdMode::XformTiling:
        tilingMaker->tilingDeltaScale(amount);
        break;

    case TilingMaker::KeybdMode::XFormPlacedTile:
        tilingMaker->placedTileDeltaScale(amount);
        break;

    case TilingMaker::KeybdMode::XFormUniqueTile:
        tilingMaker->uniqueTileDeltaScale(amount);
        break;

    case TilingMaker::KeybdMode::XformAll:
    {
        Xform xf = getModelXform();
        xf.setScale(xf.getScale() * (1.0 + static_cast<qreal>(amount)/100.0));
        setModelXform(xf,true,sigid);
        emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
    }   break;
    }
}

void TilingMakerView::slot_rotate(uint sigid,int amount)
{
    //qDebug() << "TilingMaker::slot_rotate" << amount;

    if (!validateSignal()) return;

    switch (tilingMaker->getKeybdMode())
    {
    case TilingMaker::KeybdMode::XformTiling:
        tilingMaker->tilingDeltaRotate(amount);
        break;

    case TilingMaker::KeybdMode::XFormPlacedTile:
        tilingMaker->placedTileDeltaRotate(amount);
        break;

    case TilingMaker::KeybdMode::XFormUniqueTile:
        tilingMaker->uniqueTileDeltaRotate(amount);
        break;

    case TilingMaker::KeybdMode::XformAll:
    {
        Xform xf = getModelXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setModelXform(xf,true,sigid);
        emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
    }   break;
    }
}

void TilingMakerView::slot_moveX(uint sigid, qreal amount)
{
    //qDebug() << "TilingMaker::slot_moveX" << amount;

    if (!validateSignal()) return;

    switch (tilingMaker->getKeybdMode())
    {
    case TilingMaker::KeybdMode::XformTiling:
        tilingMaker->tilingDeltaX(amount);
        break;

    case TilingMaker::KeybdMode::XFormPlacedTile:
        tilingMaker->placedTileDeltaX(amount);
        break;

    case TilingMaker::KeybdMode::XFormUniqueTile:
        break;  // tiles only have scale and rotation

    case TilingMaker::KeybdMode::XformAll:
    {
        Xform xf = getModelXform();
        amount /= xf.getScale();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setModelXform(xf,true,sigid);
        emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
    }   break;
    }
}

void TilingMakerView::slot_moveY(uint sigid, qreal amount)
{
    //qDebug() << "TilingMaker::slot_moveY" << amount;

    if (!validateSignal()) return;

    switch (tilingMaker->getKeybdMode())
    {
    case TilingMaker::KeybdMode::XformTiling:
        tilingMaker->tilingDeltaY(amount);
        break;

    case TilingMaker::KeybdMode::XFormPlacedTile:
        tilingMaker->placedTileDeltaY(amount);
        break;

    case TilingMaker::KeybdMode::XFormUniqueTile:
        break;  // tiles only have scale and rotation

    case TilingMaker::KeybdMode::XformAll:
    {
        Xform xf = getModelXform();
        amount /= xf.getScale();
        xf.setTranslateY(xf.getTranslateY() + amount);
        setModelXform(xf,true,sigid);
        emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
    }   break;
    }
}

void TilingMakerView::slot_setTileEditPoint(QPointF pt)
{
    tileEditPoint = pt;
    //qDebug() << "tile edit point =" << pt;
    forceRedraw();
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

void TilingMakerView::determineOverlapsAndTouching(const PlacedTileSet & placedTiles)
{
    TilingPtr tp = wTiling.lock();
    if (!tp)
        return;

    PlacedTileSet uniques = tp->unit().getUniquePlacedTiles();

    //qDebug() << "determineOverlapsAndTouching - BEGIN" << "count = " << placedTiles.size();
    for (const auto & ptile : std::as_const(placedTiles))
    {
        ptile->getState().touching    = false;
        ptile->getState().overlapping = false;
        ptile->getState().unique      = false;
        if (uniques.contains(ptile))
            ptile->getState().unique = true;
    }

    // touching-vs-overlapping is classified on the actual distorted (as-drawn) geometry,
    // since with in-situ per-tile distortion two tiles that touched before distortion
    // genuinely do separate or overlap once it's non-identity - that's real geometry,
    // not a display artifact. The threshold is relative to tile area rather than a fixed
    // epsilon, because the intersection-area magnitude scales with distortion too; a fixed
    // absolute epsilon (calibrated for undistorted-space noise) misclassifies real touching
    // as overlap as soon as distortion scales that noise up.
    constexpr qreal touchingAreaFraction = 1e-4;   // intersection under this fraction of tile area counts as touching

    for (const auto & tile : std::as_const(placedTiles))
    {
        QPolygonF poly1 = tile->getWorldPoints(Sys::getActiveDistortion());
        qreal     area1 = qAbs(Geo::calcArea(poly1));

        for (const auto & tile2 : std::as_const(placedTiles))
        {
            if (tile2 ==  tile) continue;

            QPolygonF poly2 = tile2->getWorldPoints(Sys::getActiveDistortion());

            if (poly1.intersects(poly2))
            {
                QPolygonF p3      = poly1.intersected(poly2);
                qreal     area    = qAbs(Geo::calcArea(p3));
                qreal     area2   = qAbs(Geo::calcArea(poly2));
                qreal     refArea = qMin(area1,area2);

                if (refArea > 0 && area < (touchingAreaFraction * refArea))
                {
                    tile->getState().touching = true;
                    tile2->getState().touching = true;
                }
                else
                {
                    tile->getState().overlapping = true;;
                    tile2->getState().overlapping = true;;
                }
            }
        }
    }
    //qDebug() << "determineOverlapsAndTouching - END";
}
