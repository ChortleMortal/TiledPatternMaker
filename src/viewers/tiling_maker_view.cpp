////////////////////////////////////////////////////////////////////////////
//
// FeatureView.java
//
// It's unlikely this file will get used in the applet.  A FeatureView
// is a special kind of GeoView that assumes a subclass will maintain
// a collection of Features.  It knows how to draw Features quickly,
// and provides a bunch of services to subclasses for mouse-based
// interaction with features.

#include <QDebug>

#include "viewers/tiling_maker_view.h"
#include "misc/geo_graphics.h"
#include "geometry/transform.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/tiling_maker/tiling_mouseactions.h"
#include "makers/tiling_maker/feature_selection.h"
#include "tile/placed_feature.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "settings/configuration.h"
#include "tile/feature.h"
#include "viewers/grid.h"

using std::make_shared;

TilingMakerView::TilingMakerView(TilingMaker * maker) : LayerController("TilingMakerView")
{
    tilingMaker         = maker;
    config              = Configuration::getInstance();

    _hideTiling         = false;
}

TilingMakerView::~TilingMakerView()
{
#ifdef EXPLICIT_DESTRUCTOR
    allPlacedFeatures.clear();
    in_tiling.clear();
#endif
}

void TilingMakerView::paint(QPainter *painter)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    layerPen = QPen(Qt::black,3);

    QTransform tr = getLayerTransform();
    //qDebug() << "TilingMakerView::paint viewT="  << Transform::toInfoString(tr);

    GeoGraphics gg(painter,tr);
    draw(&gg);

    drawCenter(painter);
}

void TilingMakerView::draw( GeoGraphics * g2d )
{
    qDebug() << "TilingMakerView::draw";

    if (!_hideTiling && !editPlacedFeature)
    {
        drawTiling(g2d);

        drawTranslationVectors(g2d,visibleT1.p1(),visibleT1.p2(),visibleT2.p1(),visibleT2.p2());

        QPen apen(Qt::red,3);
        for (auto line : qAsConst(constructionLines))
        {
            g2d->drawLine(line,apen);
        }
    }

    if (featureSelector && !editPlacedFeature)
    {
        //qDebug() << "current selection:"  << strTiliingSelection[currentSelection->getType()];
        switch (featureSelector->getType())
        {
        case INTERIOR:
            // nothing - handled by currentFeature
            break;

        case EDGE:
            g2d->drawEdge(featureSelector->getPlacedEdge(), QPen(QColor(Qt::green),3));
            break;

        case VERTEX:
        case MID_POINT:
        case FEAT_CENTER:
            g2d->drawCircle(featureSelector->getPlacedPoint(), 12, QPen(circle_color),QBrush(circle_color));
            break;

        case ARC_POINT:
            g2d->drawCircle(featureSelector->getPlacedPoint(), 12, QPen(circle_color), QBrush(circle_color));
            g2d->drawEdge(featureSelector->getPlacedEdge(), QPen(QColor(Qt::red),3));
            break;

        case SCREEN_POINT:
            g2d->drawCircle(featureSelector->getModelPoint(), 14, QPen(Qt::red), QBrush(Qt::red));
            break;
        }
    }

    if (tilingMakerMouseMode == TM_EDIT_FEATURE_MODE)
    {
        QPolygonF p;
        if (editPlacedFeature)
        {
            p = editPlacedFeature->getPlacedPolygon();
            drawFeature(g2d, editPlacedFeature, true, normal_color);
        }
        else if (featureSelector && featureSelector->getType() == INTERIOR)
        {
            p = featureSelector->getPlacedPolygon();
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

    tilingMaker->drawMouseInteraction(g2d);

    if (!featureEditPoint.isNull())
    {
        g2d->drawCircle(featureEditPoint,10,QPen(Qt::red),QBrush());
    }
}

void TilingMakerView::drawTiling( GeoGraphics * g2d )
{
    determineOverlapsAndTouching();

    for (auto& pf : allPlacedFeatures)
    {
        if (pf == currentPlacedFeature)
        {
            drawFeature(g2d, pf, true, under_mouse_color);
        }
        else if (featureSelector
             && ((featureSelector->getType() == INTERIOR) || (featureSelector->getType() == FEAT_CENTER))
             && featureSelector->getPlacedFeature() == pf)
        {
            drawFeature(g2d, pf, true, selected_color);
        }
        else if (overlapping.contains(pf))
        {
            drawFeature(g2d, pf, true, overlapping_color);
        }
        else if (touching.contains(pf))
        {
            drawFeature(g2d, pf, true, touching_color);
        }
        else if (in_tiling.contains(pf))
        {
            drawFeature(g2d, pf, true, in_tiling_color);
        }
        else
        {
            drawFeature(g2d, pf, true, normal_color);
        }
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

void TilingMakerView::drawFeature(GeoGraphics * g2d, PlacedFeaturePtr pf, bool draw_c, QColor icol )
{
    if (!pf->show()) return;

    // fill the polygon
    EdgePoly ep = pf->getPlacedEdgePoly();
    g2d->fillEdgePoly(ep,icol);
    g2d->drawEdgePoly(ep,Qt::black,3);

    if (tilingMakerMouseMode == TM_EDGE_CURVE_MODE)
    {
        for (const auto & edge : qAsConst(ep))
        {
            QPen apen = layerPen;
            apen.setColor(Qt::blue);
            g2d->drawCircle(edge->getArcCenter(),5,apen,QBrush());
        }
    }

    // draw center circle
    if( draw_c )
    {
        QPolygonF pts = pf->getPlacedPolygon();
        QPointF pt    = Point::center(pts);

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
            qDebug() << "draw accum edge";
            QPointF p1 = edge->v1->pt;
            QPointF p2 = edge->v2->pt;
            g2d->drawCircle(p1,6,layerPen,brush);
            g2d->drawCircle(p2,6,layerPen,brush);
            g2d->drawLine(p1, p2,layerPen);
        }
        else if (edge->getType() == EDGETYPE_POINT)
        {
            qDebug() << "draw accum point";
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
// Feature, edge and vertex finding.
//
TilingSelectorPtr TilingMakerView::findFeature(QPointF spt)
{
    TilingSelectorPtr nothingToIgnore;
    return findFeature(spt, nothingToIgnore);
}

TilingSelectorPtr TilingMakerView::findFeature(QPointF spt, TilingSelectorPtr ignore)
{
    QPointF wpt = screenToWorld(spt);

    for(auto placedFeature : qAsConst(allPlacedFeatures))
    {
        if (ignore && (ignore->getPlacedFeature() == placedFeature))
            continue;

        QPolygonF pgon = placedFeature->getPlacedPolygon();
        if (pgon.containsPoint(wpt,Qt::OddEvenFill))
        {
            return make_shared<InteriorTilingSelector>(placedFeature);
        }
    }

    TilingSelectorPtr sel;
    return sel;
}

TilingSelectorPtr TilingMakerView::findVertex(QPointF spt)
{
    TilingSelectorPtr nothingToIgnore;
    return findVertex(spt,nothingToIgnore);
}

TilingSelectorPtr TilingMakerView::findVertex(QPointF spt,TilingSelectorPtr ignore)
{
    for(auto it = allPlacedFeatures.begin(); it != allPlacedFeatures.end(); it++ )
    {
        PlacedFeaturePtr pf = *it;

        if (ignore && (ignore->getPlacedFeature() == pf))
            continue;

        QPolygonF pgon = pf->getFeaturePolygon();
        QTransform   T = pf->getTransform();
        for( int v = 0; v < pgon.size(); ++v )
        {
            QPointF a = pgon[v];
            QPointF b = T.map(a);
            QPointF c = worldToScreen(b);
            if (Point::dist2(spt,c) < 49.0 )
            {
                return make_shared<VertexTilingSelector>(pf,a);
            }
        }
    }

    TilingSelectorPtr sel;
    return sel;
}

TilingSelectorPtr TilingMakerView::findMidPoint(QPointF spt)
{
    TilingSelectorPtr nothingToIgnore;
    return findMidPoint(spt,nothingToIgnore);
}

TilingSelectorPtr TilingMakerView::findMidPoint(QPointF spt, TilingSelectorPtr ignore)
{
    TilingSelectorPtr sel;

    for(auto it = allPlacedFeatures.begin(); it != allPlacedFeatures.end(); it++ )
    {
        PlacedFeaturePtr pf = *it;

        if (ignore && (ignore->getPlacedFeature() == pf))
            continue;

        QTransform T = pf->getTransform();
        EdgePoly ep  = pf->getFeatureEdgePoly();
        for (auto edge : ep)
        {
            QPointF a    = edge->v1->pt;
            QPointF b    = edge->v2->pt;
            QPointF mid  = edge->getMidPoint();
            QPointF aa   = T.map(a);
            QPointF bb   = T.map(b);
            QPointF midd = T.map(mid);
            QPointF smid = worldToScreen(midd);
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
                return make_shared<MidPointTilingSelector>(pf, edge, mid);
            }
        }
    }

    return sel;
}

TilingSelectorPtr TilingMakerView::findArcPoint(QPointF spt)
{
    TilingSelectorPtr sel;

    for(auto& pf : allPlacedFeatures)
    {
        QTransform T   = pf->getTransform();
        EdgePoly epoly = pf->getFeatureEdgePoly();

        for(auto ep : epoly)
        {
            if (ep->getType() == EDGETYPE_CURVE || ep->getType() == EDGETYPE_CHORD)
            {
                QPointF a    = ep->getArcCenter();
                QPointF aa   = T.map(a);
                QPointF aad  = worldToScreen(aa);
                if (Point::dist2(spt,aad) < 49.0)
                {
                    return make_shared<ArcPointTilingSelector>(pf, ep, a);
                }
            }
        }
    }

    return sel;
}

TilingSelectorPtr TilingMakerView::findEdge(QPointF spt)
{
    TilingSelectorPtr nothingToIgnore;
    return findEdge(spt, nothingToIgnore);
}

TilingSelectorPtr TilingMakerView::findEdge(QPointF spt, TilingSelectorPtr ignore )
{
    for(auto it = allPlacedFeatures.begin(); it != allPlacedFeatures.end(); it++ )
    {
        PlacedFeaturePtr pf = *it;

        if (ignore && (ignore->getPlacedFeature() == pf))
            continue;

        EdgePoly epoly = pf->getFeatureEdgePoly();
        QTransform T   = pf->getTransform();

        for( int v = 0; v < epoly.size(); ++v)
        {
            EdgePtr edge  = epoly[v];
            QLineF  line  = edge->getLine();
            QLineF  lineW = T.map(line);
            QLineF  LineS = worldToScreen(lineW);

            if (Point::distToLine(spt, LineS) < 7.0)
            {
                return make_shared<EdgeTilingSelector>(pf,edge);
            }
        }
    }

    TilingSelectorPtr sel;
    return sel;
}

TilingSelectorPtr TilingMakerView::findSelection(QPointF spt)
{
    TilingSelectorPtr sel;

    if (      (sel = findVertex(spt)) )
        return sel;
    else if ( (sel = findMidPoint(spt)) )
        return sel;
    else if ( (sel = findEdge(spt)) )
        return sel;
    else
    {
        TilingSelectorPtr sel2 = findFeature(spt);
        if (sel2)
        {
            sel = findCenter(sel2->getPlacedFeature(),spt);
            if (!sel)
                sel = sel2;
        }
    }
    return sel;
}

TilingSelectorPtr TilingMakerView::findPoint(QPointF spt)
{
    TilingSelectorPtr nothingToIgnore;
    return findPoint(spt,nothingToIgnore);
}

TilingSelectorPtr TilingMakerView::findPoint(QPointF spt, TilingSelectorPtr ignore)
{
    TilingSelectorPtr sel = findVertex(spt,ignore);
    if (!sel)
    {
        sel = findMidPoint(spt,ignore);
    }
    if (!sel)
    {
        TilingSelectorPtr sel2 = findFeature(spt,ignore);
        if (sel2)
        {
            sel = findCenter(sel2->getPlacedFeature(),spt);
        }
    }
    if (sel)
        qDebug() << "findPoint:" << sel->getTypeString();
    return sel;
}

QPointF TilingMakerView::findSelectionPointOrPoint(QPointF spt)
{
    TilingSelectorPtr sel = findPoint(spt);
    if (!sel)
    {
        return screenToWorld(spt);
    }

    return sel->getPlacedPoint();
}

TilingSelectorPtr TilingMakerView::findCenter(PlacedFeaturePtr pf, QPointF spt)
{
    EdgePoly  epoly = pf->getPlacedEdgePoly();
    QPointF    wpt  = epoly.calcCenter();
    QPointF    spt2 = worldToScreen(wpt);

    if (Point::isNear(spt,spt2))
    {
        FeaturePtr feature = pf->getFeature();
        QPointF mpt = feature->getCenter();
        return make_shared<CenterTilingSelector>(pf, mpt);
    }

    TilingSelectorPtr sel;
    return sel;
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
    overlapping.clear();
    touching.clear();

    if (!config->tm_showOverlaps)
    {
        return;
    }

    for (auto pf : qAsConst(allPlacedFeatures))
    {
        if (!pf->show()) continue;

        QPolygonF poly = pf->getPlacedPolygon();

        for (auto pf2 : qAsConst(allPlacedFeatures))
        {
            if  (!pf2->show()) continue;
            if (pf2 == pf) continue;

            QPolygonF poly2 = pf2->getPlacedPolygon();
            if (poly2.intersects(poly))
            {
                QPolygonF p3 = poly2.intersected(poly);
                if (!p3.isEmpty())
                {
                    //qDebug() << "overlapping";
                    overlapping.push_back(pf);
                    overlapping.push_back(pf2);
                }
                else
                {
                    //qDebug() << "touching";
                    touching.push_back(pf);
                    touching.push_back(pf2);
                }
            }
        }
    }
}

TilingSelectorPtr TilingMakerView::findNearGridPoint(QPointF spt)
{
    TilingSelectorPtr tsp;
    QPointF p;

    if (Grid::getSharedInstance()->nearGridPoint(spt,p))
    {
        tsp = make_shared<ScreenTilingSelector>(p);  // not really a vertex, but good enough
    }
    return tsp;
}
