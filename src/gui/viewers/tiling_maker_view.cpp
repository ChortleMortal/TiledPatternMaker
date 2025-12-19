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
#include "gui/top/system_view_controller.h"
#include "gui/viewers/geo_graphics.h"
#include "gui/viewers/grid_view.h"
#include "gui/viewers/gui_modes.h"
#include "gui/viewers/tiling_maker_view.h"
#include "model/makers/tiling_maker.h"
#include "model/makers/prototype_maker.h"
#include "model/settings/configuration.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/enums/etilingmaker.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"

using std::make_shared;

Q_DECLARE_METATYPE(PlacedTilePtr)

TilingMakerView::TilingMakerView() : LayerController(VIEW_TILING_MAKER,DERIVED,"Tiling Maker")
{
    debugMouse   = false;
    _hideVectors = false;
}

TilingMakerView::~TilingMakerView()
{}

void TilingMakerView::setTiling(TilingPtr tiling)
{
    wTiling = tiling;
}

bool TilingMakerView::setTileSelector(PlacedTileSelectorPtr tsp)
{
    if (_tileSelector != tsp)
    {
        _tileSelector = tsp;
        return true;
    }
    else
    {
        return false;
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
    resetTileSelector();
    resetEditPlacedTile();
    mouse_interaction.reset();
}

void TilingMakerView::paint(QPainter *painter)
{
    //qDebug() << "TilingMakerView::paint - START";

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    
    QColor bcolor = viewControl()->getCanvas().getBkgdColor();

    lineColor = (bcolor == QColor(Qt::white) ? QColor(Qt::black) : QColor(Qt::white));

    QTransform tr = getLayerTransform();
    //qDebug() << "TilingMakerView::paint viewT="  << Transform::info(tr);

    GeoGraphics gg(painter,tr);
    draw(&gg);
    
    drawLayerModelCenter(painter);

    //qDebug() << "TilingMakerView::paint - END";
}

void TilingMakerView::draw(GeoGraphics * g2d)
{
    //qDebug() << "TilingMakerView::draw";
    const QVector<TilingPtr> & tilings = tilingMaker->getTilings();
    for (auto & tiling : tilings)
    {
        if (tiling && tiling->isViewable())
        {
            drawTiling(g2d,tiling);

            drawTranslationVectors(g2d,tiling);

            QPen apen(Qt::red,3);
            for (auto & line : constructionLines)
            {
                g2d->drawLine(line,apen);
            }
        }
    }

    if (tileSelector())
    {
        //qDebug() << "current selection:"  << strTiliingSelection[currentSelection->getType()];
        switch (tileSelector()->getType())
        {
        case INTERIOR:
            // nothing - handled by currentTile
            break;

        case EDGE:
        {
            auto edge = tileSelector()->getPlacedEdge();
            g2d->drawEdge(edge, QPen(QColor(Qt::green),3));
            if (edge->isCurve())
            {
                g2d->drawLine(edge->getLine(),QPen(QColor(0x5000ff00),1));
            }
        }    break;

        case VERTEX:
        case MID_POINT:
        case TILE_CENTER:
            g2d->drawCircle(tileSelector()->getPlacedPoint(), 12, QPen(circle_color),QBrush(circle_color));
            break;

        case ARC_POINT:
            g2d->drawCircle(tileSelector()->getPlacedPoint(), 12, QPen(circle_color), QBrush(circle_color));
            g2d->drawEdge(tileSelector()->getPlacedEdge(), QPen(QColor(Qt::red),3));
            break;

        case SCREEN_POINT:
            g2d->drawCircle(tileSelector()->getModelPoint(), 14, QPen(Qt::red), QBrush(Qt::red));
            break;
        }
    }

    if (tilingMaker->getTilingMakerMouseMode() == TM_EDIT_TILE_MODE)
    {
        QPolygonF p;
        if (editPlacedTile())
        {
            p = editPlacedTile()->getPlacedPoints();
            drawTile(g2d, editPlacedTile(), true, normal_color);
        }
        else if (tileSelector() && tileSelector()->getType() == INTERIOR)
        {
            p = tileSelector()->getPlacedPolygon();
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

bool TilingMakerView::tileUnderMouse(const PlacedTilePtr & tile)
{
    return (tile->show()
            && tileSelector()
            && ((tileSelector()->getType() == INTERIOR) || (tileSelector()->getType() == TILE_CENTER))
            && tileSelector()->getPlacedTile() == tile);
}

bool TilingMakerView::tileIsSelected(const PlacedTilePtr & tile)
{
    return (tile->show() && (tile == tilingMaker->selectedTile()));
}

void TilingMakerView::drawTiling(GeoGraphics * g2d, TilingPtr tiling)
{
    if (tiling->isTilingViewChanged())
    {
        tiling->createViewablePlacedTiles();
        determineOverlapsAndTouching(tiling);
        tiling->resetTilingViewChanged();
    }

    const PlacedTiles & viewable = tiling->getViewablePlacements();

    // pass 1
    for (const PlacedTilePtr & ptp : std::as_const(viewable))
    {
        //qDebug() << ptp->getTile()->info();
        if (tileIsSelected(ptp) || tileUnderMouse(ptp))
        {
            continue;
        }

        if (ptp->show())
        {
            QColor color = Qt::black;   // default
            switch (Sys::config->tm_tileColorMode)
            {
            case TILE_COLOR_TOUCHING:
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
                    color = normal_color;
                }
                break;

            case TILE_COLOR_INCLUDES:
                if (ptp->isIncluded())
                {
                    color = in_tiling_color;
                }
                else
                {
                    color = normal_color;
                }
                break;

            case TILE_COLOR_UNIQUE:
                color = normal_color;
                break;
            }
            drawTile(g2d, ptp, true, color);
        }
    }

    // pass 2 - color selected tile
    PlacedTilePtr ptp = tilingMaker->selectedTile();
    if (ptp)
    {
        if (ptp->show())
        {
            drawTile(g2d, ptp, true, selected_color);
        }
    }

    // pass 3 - color tile under mouse
    for (const auto & tile : std::as_const(viewable))
    {
        if (tile->show())
        {
            if (tileUnderMouse(tile))
            {
                drawTile(g2d, tile, true, under_mouse_color);
            }
        }
    }
}

void TilingMakerView::drawTranslationVectors(GeoGraphics * g2d, TilingPtr tiling)
{
    if (!_hideVectors)
    {
        QPointF tOrigin =  tiling->hdr().getTranslateOrigin();
        if (tOrigin.isNull() && tiling->numViewable() > 0)
        {
            PlacedTilePtr ptp = tiling->getViewablePlacements().first();
            if (ptp)
            {
                QTransform T = ptp->getPlacement();
                tOrigin      = T.map(ptp->getTile()->getCenter());
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
        g2d->drawText(modelToScreen(t1_end) + QPointF(10,0),"T1");
    }

    if (t2_start != t2_end)
    {
        g2d->drawLine(t2_start, t2_end, pen);
        g2d->drawArrow(t2_start, t2_end, arrow_length, arrow_width, construction_color);
        g2d->drawText(modelToScreen(t2_end) + QPointF(10,0),"T2");
    }
}

void TilingMakerView::drawTile(GeoGraphics * g2d, PlacedTilePtr pf, bool drawCenter, QColor color)
{
    // fill the polygon
    EdgePoly ep = pf->getPlacedEdgePoly();

    if (pf->show())
    {
        QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        g2d->fillEdgePoly(ep,pen);
    }

    QPen pen2;
    if (pf->getTile()->isRegular())
        pen2 = QPen(lineColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    else
        pen2 = QPen(Qt::blue, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    g2d->drawEdgePoly(ep,pen2);

    if (tilingMaker->getTilingMakerMouseMode() == TM_EDGE_CURVE_MODE)
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
        QPolygonF pts = pf->getPlacedPoints();
        QPointF pt    = pf->getTile()->getCenter();
        pt            = pf->getPlacement().map(pt);

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

    PlacedTiles viewable = wTiling.lock()->getViewablePlacements();
    // reverse the list so that newest additions are 'on top'
    std::reverse(viewable.begin(), viewable.end());

    for (auto placedTile : std::as_const(viewable))
    {
        if (ignore && (ignore->getPlacedTile() == placedTile))
            continue;

        QPolygonF pgon = placedTile->getPlacedPoints();
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
    const PlacedTiles & viewable = wTiling.lock()->getViewablePlacements();
    for(auto & pf : std::as_const(viewable))
    {
        if (ignore && (ignore->getPlacedTile() == pf))
            continue;

        TilePtr tile = pf->getTile();
        if (!tile)
            continue;

        QPolygonF pgon = tile->getPoints();
        QTransform   T = pf->getPlacement();
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

    const PlacedTiles & viewable = wTiling.lock()->getViewablePlacements();
    for(auto & placed : std::as_const(viewable))
    {
        if (ignore && (ignore->getPlacedTile() == placed))
            continue;

        TilePtr tile = placed->getTile();
        if (!tile)
            continue;

        const EdgePoly & ep  = tile->getEdgePoly();
        QTransform T = placed->getPlacement();
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
    const PlacedTiles & viewable = wTiling.lock()->getViewablePlacements();
    for(const auto & pf : std::as_const(viewable))
    {
        TilePtr tile = pf->getTile();
        if (!tile)
            continue;

        const EdgePoly & epoly = tile->getEdgePoly();
        QTransform T   = pf->getPlacement();

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
    const PlacedTiles & viewable = wTiling.lock()->getViewablePlacements();
    for(const auto & pf : std::as_const(viewable))
    {
        if (ignore && (ignore->getPlacedTile() == pf))
            continue;

        TilePtr tile = pf->getTile();
        if (!tile)
            continue;

        const EdgePoly & epoly = tile->getEdgePoly();
        QTransform T   = pf->getPlacement();
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
    if (sel)
        qDebug().noquote() << "findPoint: found -" << sel->getTypeString();
    return sel;
}

QPointF TilingMakerView::findSelectionPointOrPoint(QPointF spt)
{
    PlacedTileSelectorPtr sel = findPoint(spt);
    if (!sel)
    {
        return screenToModel(spt);
    }

    return sel->getPlacedPoint();
}

PlacedTileSelectorPtr TilingMakerView::findCenter(PlacedTilePtr pf, QPointF spt)
{
    EdgePoly  epoly = pf->getPlacedEdgePoly();
    QPointF    wpt  = epoly.calcCenter();
    QPointF    spt2 = modelToScreen(wpt);
    
    if (Geo::isNear(spt,spt2))
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
    Q_ASSERT(tilingMaker->getTilingMakerMouseMode() == TM_NO_MOUSE_MODE);

    PlacedTileSelectorPtr sel = findSelection(spt);
    if(sel)
    {
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
                mouse_interaction = make_shared<DrawTranslation>(sel, spt, QPen(lineColor,3));
        }
        else if (mouseButton == Qt::RightButton)
        {
            qDebug().noquote() << "right button selection:" << sel->getTypeString();
            auto type = sel->getType();
            switch (type)
            {
            case VERTEX:
            case TILE_CENTER:
            case MID_POINT:
            case EDGE:
            case INTERIOR:
            {
                tilingMaker->setClickedSelector(sel);   // save
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
                    myMenu.addAction("Copy/Join Mid",   tilingMaker, &TilingMaker::slot_copyJoinMidPoint);
                else if (type == EDGE)
                    myMenu.addAction("Copy/Move Edge",  tilingMaker, &TilingMaker::slot_copyJoinEdge);

                if (type == INTERIOR || type == TILE_CENTER)
                    myMenu.addAction("Move to corner",  this,        &TilingMakerView::slot_subMenu1);

                myMenu.addAction("Edit Tile",           tilingMaker, &TilingMaker::slot_view_menu_editTile);

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

                myMenu.exec(Sys::viewController->mapToGlobal(spt.toPoint()));
            }   break;

            case ARC_POINT:
            case SCREEN_POINT:
                break;
            }
        }
    }
    else
    {
        tilingMaker->deselectTile();
        forceRedraw();      // FIXME - does not trigger the menu
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
    QPoint pt   = Sys::viewController->viewRect().topLeft();
    QPointF pt2 = screenToModel(QPointF(pt));
    tilingMaker->moveTileTo(pt2);
}

void TilingMakerView::slot_TR()
{
    QPoint pt   = Sys::viewController->viewRect().topRight();
    QPointF pt2 = screenToModel(QPointF(pt));
    tilingMaker->moveTileTo(pt2);
}

void TilingMakerView::slot_BL()
{
    QPoint pt   = Sys::viewController->viewRect().bottomLeft();
    QPointF pt2 = screenToModel(QPointF(pt));
    tilingMaker->moveTileTo(pt2);
}

void TilingMakerView::slot_BR()
{
    QPoint pt   = Sys::viewController->viewRect().bottomRight();
    QPointF pt2 = screenToModel(QPointF(pt));
    tilingMaker->moveTileTo(pt2);
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
    case TM_UNIQUIFY_MODE:
    case TM_UNIFY_MODE:
    case TM_DECOMPOSE_MODE:
        if (setTileSelector(findSelection(spt)))
            forceRedraw();  // NOTE this triggers a lot of repainting
        break;

    case TM_TRANSLATION_VECTOR_MODE:
    case TM_CONSTRUCTION_LINES:
        if (setTileSelector(findSelection(spt)))
            forceRedraw();
        break;

    case TM_DRAW_POLY_MODE:
    {
        bool rv = setTileSelector(findVertex(spt));
        if (!tileSelector())
        {
            rv = setTileSelector(findMidPoint(spt));
        }
        if (!tileSelector())
        {
            rv = setTileSelector(findNearGridPoint(spt));
        }
        if (rv)
            forceRedraw();
    }    break;

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
    {
        bool rv = setTileSelector(findArcPoint(spt));
        if (tileSelector())
        {
            qDebug() << "updateUnderMouse: found arc center";
        }
        else
        {
            rv = setTileSelector(findEdge(spt));
            if (tileSelector())
            {
                qDebug() << "updateUnderMouse: found edge";
            }
            else
            {
                resetTileSelector();
            }
        }
        if (rv)
            forceRedraw();
    }    break;
    }
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
    if (!viewControl()->isEnabled(viewType())) return;

    sMousePos = spt;

    if (debugMouse) qDebug() << "slot_mousePressed:" << sMousePos << screenToModel(sMousePos);

    PlacedTileSelectorPtr sel;

    switch (tilingMaker->getTilingMakerMouseMode())
    {
    case TM_NO_MOUSE_MODE:
        if (Sys::guiModes->getMouseMode(MOUSE_MODE_NONE))
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
        if (sel && sel->isPoint())
            mouse_interaction = make_shared<DrawTranslation>(sel, sMousePos, QPen(lineColor,3));
        break;

    case TM_DRAW_POLY_MODE:
        mouse_interaction = make_shared<CreatePolygon>(sMousePos);
        break;

    case TM_POSITION_MODE:
        break;

    case TM_INCLUSION_MODE:
        tilingMaker->toggleInclusion(findTileUnderMouse());
        break;

    case TM_MEASURE_MODE:
    {
        Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
        if (kms == (Qt::CTRL | Qt::SHIFT))
        {
            PlacedTileSelectorPtr nothingToIgnore;
            sel = findEdge(spt,nothingToIgnore);
            if (sel)
            {
                // TODO  - this looks like incomplete code
                QLineF line = sel->getPlacedLine();
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

    case TM_EDIT_TILE_MODE:
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

    case TM_EDGE_CURVE_MODE:
        sel = findArcPoint(spt);
        if (sel)
        {
            setTileSelector(sel);
            QMenu myMenu;
            myMenu.addAction("Use Cursor to change curve", tilingMaker, SLOT(slot_moveArcCenter()));
            myMenu.addAction("Edit Magnitude", tilingMaker, SLOT(slot_editMagnitude()));
            myMenu.exec(Sys::viewController->mapToGlobal(spt.toPoint()));
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
                        setTileSelector(sel);
                        QMenu myMenu;
                        myMenu.addAction("Make Convex",  tilingMaker, SLOT(slot_createConvex()));
                        myMenu.addAction("Make Concave", tilingMaker, SLOT(slot_createConcave()));
                        myMenu.exec(Sys::viewController->mapToGlobal(spt.toPoint()));
                    }
                    else if (edge->getType() == EDGETYPE_CURVE)
                    {
                        setTileSelector(sel);
                        QMenu myMenu;
                        myMenu.addAction("Make Flat",  tilingMaker, SLOT(slot_flatenCurve()));
                        if (edge->getCurveType() == CURVE_CONVEX)
                            myMenu.addAction("Make Concave",  tilingMaker, SLOT(slot_makeConcave()));
                        else
                            myMenu.addAction("Make Convex",  tilingMaker, SLOT(slot_makeConvex()));
                        myMenu.addAction("Use Cursor to change curve", tilingMaker, SLOT(slot_moveArcCenter()));
                        myMenu.addAction("Edit Magnitude", tilingMaker, SLOT(slot_editMagnitude()));
                        myMenu.exec(Sys::viewController->mapToGlobal(spt.toPoint()));
                    }
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
    {
        auto fsel = findTileUnderMouse();
        if (fsel)
        {
            if (tilingMaker->reflectPolygon(findTileUnderMouse()))
            {
                tilingMaker->setTilingMakerMouseMode(TM_NO_MOUSE_MODE);
            }
        }
    }   break;

    case TM_UNIFY_MODE:
    {
        sel = findTileUnderMouse();
        if (sel && sel->getType() != SCREEN_POINT )
        {
            tilingMaker->unifyTile(sel->getPlacedTile());
        }
    }   break;

    case TM_UNIQUIFY_MODE:
    {
        sel = findTileUnderMouse();
        if (sel && sel->getType() != SCREEN_POINT )
        {
            tilingMaker->uniquifyTile(sel->getPlacedTile());
        }
    }   break;

    case TM_DECOMPOSE_MODE:
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
    if (!viewControl()->isEnabled(viewType())) return;

    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << sMousePos << screenToModel(sMousePos)  << sTilingMakerMouseMode[tilingMaker->getTilingMakerMouseMode()];

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
    case TM_UNIQUIFY_MODE:
    case TM_UNIFY_MODE:
    case TM_DECOMPOSE_MODE:
        break;
    }
}

void TilingMakerView::slot_mouseMoved(QPointF spt)
{
    if (!viewControl()->isEnabled(viewType())) return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << sMousePos;

    updateUnderMouse(sMousePos);
}

void TilingMakerView::slot_mouseReleased(QPointF spt)
{
    if (!viewControl()->isEnabled(viewType())) return;

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

    switch (Sys::guiModes->TMKbdMode())
    {
    case TM_MODE_XFORM_TILING:
    {
        qreal sc = 1.0 + delta;
        QTransform ts;
        ts.scale(sc,sc);
        const PlacedTiles & viewable = wTiling.lock()->getViewablePlacements();
        for (const auto & pplacedTile : std::as_const(viewable))
        {
            QTransform t = pplacedTile->getPlacement();
            t *= ts;
            pplacedTile->setPlacement(t);
        }

        if (tilingMaker->getPropagate())
        {
            ProtoEvent protoEvent;
            protoEvent.event = PROM_TILING_CHANGED;
            protoEvent.tiling = wTiling.lock();
            Sys::prototypeMaker->sm_takeUp(protoEvent);
        }

        forceRedraw();
        emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
    }   break;

    case TM_MODE_XFORM_PLACED_TILE:
        tilingMaker->placedTileDeltaScale(delta);
        break;

    case TM_MODE_XFORM_UNIQUE_TILE:
        tilingMaker->uniqueTileDeltaScale(delta);
        break;

    case TM_MODE_XFORM_ALL:
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

    switch (Sys::guiModes->TMKbdMode())
    {
    case TM_MODE_XFORM_TILING:
    {
        QTransform tr;
        tr.rotate(delta);

        const PlacedTiles & viewable = wTiling.lock()->getViewablePlacements();
        for (const auto & pfp : std::as_const(viewable))
        {
            QTransform t = pfp->getPlacement();
            t *= tr;
            pfp->setPlacement(t);
        }

        if (tilingMaker->getPropagate())
        {
            ProtoEvent protoEvent;
            protoEvent.event = PROM_TILING_CHANGED;
            protoEvent.tiling = wTiling.lock();
            Sys::prototypeMaker->sm_takeUp(protoEvent);
        }

        forceRedraw();
        emit tilingMaker->sig_menuRefresh(TMR_PLACED_TILE);
    }   break;

    case TM_MODE_XFORM_PLACED_TILE:
        tilingMaker-> placedTileDeltaRotate(0.5 * delta);
        break;

    case TM_MODE_XFORM_UNIQUE_TILE:
        tilingMaker->uniqueTileDeltaRotate(0.5 * delta);
        break;

    case TM_MODE_XFORM_ALL:
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

    switch (Sys::guiModes->TMKbdMode())
    {
    case TM_MODE_XFORM_TILING:
    {
        QTransform T  = getCanvasTransform();
        qreal scale   = Transform::scalex(T);
        QPointF mpt   = spt/scale;
        QTransform tt = QTransform::fromTranslate(mpt.x(),mpt.y());

        const PlacedTiles & viewable = wTiling.lock()->getViewablePlacements();
        for (const auto & placedTile : std::as_const(viewable))
        {
            QTransform t = placedTile->getPlacement();
            t *= tt;
            placedTile->setPlacement(t);
        }

        if (tilingMaker->getPropagate())
        {
            ProtoEvent protoEvent;
            protoEvent.event = PROM_TILING_CHANGED;
            protoEvent.tiling = wTiling.lock();
            Sys::prototypeMaker->sm_takeUp(protoEvent);
        }
    }   break;

    case TM_MODE_XFORM_PLACED_TILE:
    {
        auto currentTile = tilingMaker->selectedTile();
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
            ProtoEvent protoEvent;
            protoEvent.event  = PROM_TILE_PLACED_TRANS_CHANGED;
            protoEvent.tiling = tilingMaker->getSelected();
            protoEvent.ptile  = currentTile;
            Sys::prototypeMaker->sm_takeUp(protoEvent);
        }
    }   break;

    case TM_MODE_XFORM_UNIQUE_TILE:
        break;  // tiles only have scale and rotation

    case TM_MODE_XFORM_ALL:
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

    switch (Sys::guiModes->TMKbdMode())
    {
    case TM_MODE_XFORM_TILING:
        tilingMaker->tilingDeltaScale(amount);
        break;

    case TM_MODE_XFORM_PLACED_TILE:
        tilingMaker->placedTileDeltaScale(amount);
        break;

    case TM_MODE_XFORM_UNIQUE_TILE:
        tilingMaker->uniqueTileDeltaScale(amount);
        break;

    case TM_MODE_XFORM_ALL:
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

    switch (Sys::guiModes->TMKbdMode())
    {
    case TM_MODE_XFORM_TILING:
        tilingMaker->tilingDeltaRotate(amount);
        break;

    case TM_MODE_XFORM_PLACED_TILE:
        tilingMaker->placedTileDeltaRotate(amount);
        break;

    case TM_MODE_XFORM_UNIQUE_TILE:
        tilingMaker->uniqueTileDeltaRotate(amount);
        break;

    case TM_MODE_XFORM_ALL:
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

    switch (Sys::guiModes->TMKbdMode())
    {
    case TM_MODE_XFORM_TILING:
        tilingMaker->tilingDeltaX(amount);
        break;

    case TM_MODE_XFORM_PLACED_TILE:
        tilingMaker->placedTileDeltaX(amount);
        break;

    case TM_MODE_XFORM_UNIQUE_TILE:
        break;  // tiles only have scale and rotation

    case TM_MODE_XFORM_ALL:
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

    switch (Sys::guiModes->TMKbdMode())
    {
    case TM_MODE_XFORM_TILING:
        tilingMaker->tilingDeltaY(amount);
        break;

    case TM_MODE_XFORM_PLACED_TILE:
        tilingMaker->placedTileDeltaY(amount);
        break;

    case TM_MODE_XFORM_UNIQUE_TILE:
        break;  // tiles only have scale and rotation

    case TM_MODE_XFORM_ALL:
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

void TilingMakerView::determineOverlapsAndTouching(TilingPtr tiling)
{
    PlacedTiles allPlacedTiles = tiling->unit().getAll();  // makes a local copy

    for (const auto & tile : std::as_const(allPlacedTiles))
    {
        tile->clearViewState();
    }

    for (const auto & tile1 : std::as_const(allPlacedTiles))
    {
        if (!tile1->show())  continue;

        QPolygonF poly1 = tile1->getPlacedPoints();
        for (const auto & tile2 : std::as_const(allPlacedTiles))
        {
            if (!tile2->show())  continue;
            if (tile2 ==  tile1) continue;

            QPolygonF poly2 = tile2->getPlacedPoints();

            if (poly1.intersects(poly2))
            {
                QPolygonF p3 = poly1.intersected(poly2);
                qreal area = Geo::calcArea(p3);
                if (Loose::zero(area))
                {
                    tile1->setTouching();
                    tile2->setTouching();
                }
                else
                {
                    tile1->setOverlapping();
                    tile2->setOverlapping();
                }
            }
        }
    }
}
