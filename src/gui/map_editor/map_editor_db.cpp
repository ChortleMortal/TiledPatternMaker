#include <QDebug>

#include "gui/map_editor/map_editor_db.h"
#include "gui/top/view_controller.h"
#include "model/makers/motif_maker.h"
#include "model/prototypes/tile_motif.h"
#include "model/settings/configuration.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/dcel.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/map.h"
#include "sys/tiledpatternmaker.h"

MapEditorDb::MapEditorDb(MapEditor * me)
{
    editor = me;

    showMap               = true;
    showPoints            = true;
    showBoundaries        = false;
    showConstructionLines = false;
    showMidPoints         = false;
    showDirnPoints        = false;
    showArcCentre         = false;
    showTileCentre        = false;
}


// everything that is visible is put into the DB
// - so it can be made visible with annotations/embelishment
// - so it can be selected
void MapEditorDb::buildEditorDB()
{
    points.clear();
    lines.clear();
    circles.clear();

    if (Sys::config->mapEditorMode == MapEditor::EditorMode::Map)
    {
        for (MapPtr & map : editor->getDrawMaps())
        {
            // add points from map vertices
            for (const VertexPtr & vert : std::as_const(map->getVertices()))
            {
                PointInfo pi(PointInfo::Type::Vertex,vert);
                points.push_back(pi);
            }

            for (const EdgePtr & edge : std::as_const(map->getEdges()))
            {
                // add lines from map edges
                LineInfo li(LineInfo::Type::LINE_EDGE,edge);
                lines.push_back(li);

                // add points from map edges mid-points
                QPointF midPt = edge->getLine().pointAt(0.5);
                PointInfo pi(PointInfo::Type::VertexMid,midPt);
            }
        }

        MapEditorLayer & editMap = editor->getEditLayer();
        MapedMapPtr layerMap     = editMap.getWorking();
        MapType mapType          = editMap.getLayerMapType();
        switch (mapType)
        {
        case MapType::None:
            break;

        case MapType::TilingUnit:
        case MapType::TilingFull:
        {
            TilingMap * tm = static_cast<TilingMap*>(layerMap.get());
            Q_ASSERT(tm);
            TilingPtr tp = tm->getTiling();
            if (tp)
            {
                TilingUnitController & tu = tp->unit();
                PlacedTileSet set = tu.getIncluded();
                for (auto & placedTile : set)
                {
                    EdgePoly ep = placedTile->getWorldEdgePoly();
                    QPointF pt  = ep.calcCenter();
                    PointInfo pi(PointInfo::Type::Center,pt);
                    points.push_back(pi);
                }
            }
        }   break;

        case MapType::MotifUnit:
        case MapType::Motif:
        case MapType::FileMotif:
        case MapType::CompositeMotif:
        {
            QVector<TimoMap *> vec = editor->getDrawTimoMaps();
            for (TimoMap * tmap : std::as_const(vec))
            {
                TileMotifSet timos = tmap->getTileMotifs();
                for (auto & timo : timos)
                {
                    buildMotifDB(timo,mapType);
                }
            }
        }   break;

        case MapType::Style:
        case MapType::Prototype:
            break;

        case MapType::File:
        case MapType::Created:
        case MapType::Composite:
        case MapType::Crop:
            break;
        }
    }
    else
    {
        Q_ASSERT(Sys::config->mapEditorMode == MapEditor::EditorMode::DCEL);
        DCELPtr dcel = getActiveDCEL();
        if  (dcel)
        {
            // add points from map vertices
            const QVector<VertexPtr>  & vertices = dcel->getVertices();
            for (const VertexPtr & v : std::as_const(vertices))
            {
                PointInfo pi(PointInfo::Type::Vertex,v);
                points.push_back(pi);
            }

            for (const EdgePtr & e : std::as_const(dcel->getEdges()))
            {
                // add lines from map edges
                LineInfo li(LineInfo::Type::LINE_EDGE,e);
                lines.push_back(li);

                // add points from map edges mid-points
                QPointF midPt = e->getLine().pointAt(0.5);
                PointInfo pi(PointInfo::Type::VertexMid,midPt);
                points.push_back(pi);
            }
        }
    }

    QVector<QLineF> clines;
    for (int i = 0; i < constructionLines.size(); i++)
    {
        if (i != liftedCLine)
            clines.push_back(constructionLines[i]);
    }

    // add construction lines
    for (const QLineF & line : std::as_const(clines))
    {
        LineInfo li(LineInfo::Type::LINE_CONSTRUCTION,line);
        lines.push_back(li);

        QPointF a = line.p1();
        PointInfo pi(PointInfo::Type::Line,a);
        points.push_back(pi);

        QPointF b = line.p2();
        PointInfo pi2(PointInfo::Type::Line,b);
        points.push_back(pi2);

        QPointF midPt = line.pointAt(0.5);
        PointInfo pi3(PointInfo::Type::LineMid,midPt);
        points.push_back(pi3);
    }

    // add construction circles
    for (const CirclePtr & circle :std::as_const(constructionCircles))
    {
        circles.push_back(circle);
    }

    // add crop circle
    CropPtr crop = getCrop();
    if (crop && crop->getCropType() == CROP_CIRCLE)
    {
        CirclePtr circle = std::make_shared<Circle>(crop->getCircle());
        circles.push_back(circle);
    }

    // build circle-line intersects
    for (const CirclePtr & c : std::as_const(circles))
    {
        for (const LineInfo & linfo : std::as_const(lines))
        {
            if (linfo.type == LineInfo::Type::LINE_EDGE && !showMap)
            {
                continue;
            }

            QLineF line  = linfo.line;

            // try for circle
            QPointF a;
            QPointF b;
            int count = Geo::findLineCircleIntersections(c->centre, c->radius, line, a, b);
            if (count == 1)
            {
                // this is a tangent line
                PointInfo pi(PointInfo::Type::Circle,a);
                points.push_back(pi);
            }
            else if (count == 2)
            {
                if (Geo::pointOnLine(line,a))
                {
                    PointInfo pi(PointInfo::Type::Circle,a);
                    points.push_back(pi);
                }

                if (Geo::pointOnLine(line,b))
                {
                    PointInfo pi2(PointInfo::Type::Circle,b);
                    points.push_back(pi2);
                }
            }
        }
    }

    // build circle-circle intersects
    for (int i=0; i < circles.size(); i++)
    {
        CirclePtr c1 = circles[i];
        for (int j=i+1; j < circles.size(); j++)
        {
            CirclePtr c2 = circles[j];
            QPointF p1;
            QPointF p2;
            int count = Geo::circleCircleIntersectionPoints(*c1.get(),*c2.get(),p1,p2);
            if (count == 1)
            {
                //qDebug() << "circle-circle touch point=" << p1;
                PointInfo pi(PointInfo::Type::Circle1,p1);
                points.push_back(pi);
            }
            else if (count == 2)
            {
                //qDebug() << "circle-circle touch points=" << p1 << p2;
                PointInfo pi1(PointInfo::Type::Circle1,p1);
                PointInfo pi2(PointInfo::Type::Circle2,p2);
                points.push_back(pi1);
                points.push_back(pi2);
            }
        }
    }

    // build construction line intersect points
    for (const QLineF & cline : std::as_const(clines))
    {
        for (const LineInfo & linfo : std::as_const(lines))
        {
            if (linfo.type == LineInfo::Type::LINE_CONSTRUCTION && cline == linfo.line)
            {
                continue;
            }
            if (linfo.type == LineInfo::Type::LINE_EDGE && !showMap)
            {
                continue;
            }
            if (linfo.type == LineInfo::Type::LINE_EDGE && linfo.edge->isCurve())
            {
                QPointF isect1;
                QPointF isect2;
                int count = Geo::findLineCircleIntersections(linfo.edge->getArcCenter(),linfo.edge->getRadius(),cline,isect1,isect2);

                if (count && linfo.edge->pointWithinArc(isect1))
                {
                    PointInfo pi(PointInfo::Type::Line,isect1);
                    points.push_back(pi);
                }

                if (count == 2 && linfo.edge->pointWithinArc(isect2))
                {
                    PointInfo pi(PointInfo::Type::Line,isect2);
                    points.push_back(pi);
                }
            }
            else
            {
                QLineF line  = linfo.line;
                QPointF intersect;
                QLineF::IntersectType itype = cline.intersects(line);
                if (itype == QLineF::BoundedIntersection)
                {
                    // hapily this does into include intersecting ends (which previously I thought was a Qt bug)
                    PointInfo pi(PointInfo::Type::Line,intersect);
                    points.push_back(pi);
                }
            }
        }
    }
}

// this does all placements of the tile motif
void MapEditorDb::buildMotifDB(TileMotifPtr timo, MapType mapType)
{
    Q_ASSERT(timo);
    MotifPtr  motif  = timo->getMotif();
    Q_ASSERT(motif);
    TilePtr tile     = timo->getTile();
    Q_ASSERT(tile);
    TilingPtr tiling = timo->getTiling();
    Q_ASSERT(tiling);

    Placements placements;
    if (mapType == MapType::Motif)
    {
        //selected
        placements.push_back(tiling->unit().getFirstPlacement(tile));
    }
    else
    {
        placements = tiling->unit().getPlacements(tile);
    }

    for (QTransform & placement : placements)
    {
        //qDebug() << "placement" << Transform::info(placement);

        for (ExtenderPtr ep : motif->getExtenders())
        {
            const ExtendedBoundary & eb = ep->getExtendedBoundary();
            if (showBoundaries)
            {
                // add points from ext boundary
                const QPolygonF & rextBoundary = eb.getPoly();
                QPolygonF extBoundary  = placement.map(rextBoundary);

                if (extBoundary.size())
                {
                    QPointF center = tile->getCenter();
                    QTransform t0  = QTransform::fromTranslate(center.x(),center.y());
                    extBoundary    = t0.map(extBoundary);

                    for (auto & apt : std::as_const(extBoundary))
                    {
                        PointInfo pi(PointInfo::Type::Line,apt);
                        points.push_back(pi);
                    }

                    QVector<QLineF> edges = Geo::polyToLines(extBoundary);
                    for (auto & line : std::as_const(edges))
                    {
                        // add lines from ext boundary
                        LineInfo li(LineInfo::Type::LINE_FIXED,line);
                        lines.push_back(li);

                        // add points from ext bopundary mid-points
                        QPointF midPt = line.pointAt(0.5);
                        PointInfo pi(PointInfo::Type::LineMid,midPt);
                        points.push_back(pi);
                    }

                    if (!extBoundary.isClosed())
                    {
                        QLineF line(extBoundary.last(),extBoundary.first());
                        LineInfo li(LineInfo::Type::LINE_FIXED,line);
                        lines.push_back(li);

                        QPointF midPt = line.pointAt(0.5);
                        PointInfo pi(PointInfo::Type::LineMid,midPt);
                        points.push_back(pi);
                    }
                }
            }

            // add points from radial motif boundary
            QPolygonF motifBoundary = motif->getMotifBoundary();
            motifBoundary            = placement.map(motifBoundary);
            if (motifBoundary.size())
            {
                for (auto & apt : std::as_const(motifBoundary))
                {
                    PointInfo pi(PointInfo::Type::Line,apt);
                    points.push_back(pi);
                }

                QVector<QLineF> edges = Geo::polyToLines(motifBoundary);
                for (auto & line : std::as_const(edges))
                {
                    // add line from radial motif boundary
                    LineInfo li(LineInfo::Type::LINE_FIXED,line);
                    lines.push_back(li);

                    // add point from raidal motif boundary
                    QPointF midPt = line.pointAt(0.5);
                    PointInfo pi(PointInfo::Type::LineMid,midPt);
                    points.push_back(pi);
                }

                if (!motifBoundary.isClosed())
                {
                    QLineF line(motifBoundary.last(),motifBoundary.first());
                    LineInfo li(LineInfo::Type::LINE_FIXED,line);
                    lines.push_back(li);

                    QPointF midPt = line.pointAt(0.5);
                    PointInfo pi(PointInfo::Type::LineMid,midPt);
                    points.push_back(pi);
                }
            }

            // add external boundary circle
            if (eb.isCircle())
            {
                CirclePtr c = std::make_shared<Circle>(QPointF(0,0), eb.getRadius());
                circles.push_back(c);
            }
        }

        // add points from tile
        EdgePoly ep = tile->getEdgePoly();
        ep.mapD(placement);

        QPolygonF poly = ep.getPolygon();
        for (auto & apt : std::as_const(poly))
        {
            PointInfo pi(PointInfo::Type::Line,apt);
            points.push_back(pi);
        }

        auto plines = ep.getLines();
        for (auto & line : plines)
        {
            LineInfo li(LineInfo::Type::LINE_FIXED,line);
            lines.push_back(li);

            // add mid-points of edge
            QPointF midPt = line.pointAt(0.5);
            PointInfo pi(PointInfo::Type::LineMid,midPt);
            points.push_back(pi);
        }

        // Add tile center
        QPointF center;
        if (tile->isRegular())
            center = ep.calcCenter();
        else
            center = ep.calcIrregularCenter();
        PointInfo pi(PointInfo::Type::Center2,center);
        points.push_back(pi);
    }
}

void MapEditorDb::reset()
{
    localDcel.reset();
    activeDcel.reset();
}

void MapEditorDb::setCrop(CropPtr acrop)
{
    // this local crop is unique
    Crop bcrop = *acrop.get();
    crop = std::make_shared<Crop>(bcrop);
}

bool MapEditorDb::isMotif(MapType type)
{
    switch (type)
    {
    case MapType::Motif:
    case MapType::MotifUnit:
    case MapType::FileMotif:
    case MapType::CompositeMotif:
    case MapType::Prototype:
        return true;

    default:
        return false;
    }
}

bool MapEditorDb::hasConstructionLines()
{
    return ( constructionLines.size()|| constructionCircles.size() );
}