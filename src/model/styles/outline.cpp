#include <QDebug>
#include "gui/viewers/geo_graphics.h"
#include "model/makers/mosaic_maker.h"
#include "model/styles/casing_neighbours.h"
#include "model/styles/outline.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/map.h"
#include "sys/geometry/neighbour_map.h"
#include "sys/geometry/transform.h"
#include "sys/sys/debugflags.h"


////////////////////////////////////////////////////////////////////////////
//
// Outline.java
//
// The simplest non-trivial rendering style.  Outline just uses
// some trig to fatten up the map's edges, also drawing a line-based
// outline for the resulting fat figure.
//
// The same code that computes the draw elements for Outline can
// be inherited by other "fat" styles, such as Emboss.

Outline::Outline(const ProtoPtr &proto) : Thick (proto)
{
    outline_width   = 0.03;
    join_style      = Qt::BevelJoin;
    cap_style       = Qt::SquareCap;
    created         = false;

        connect(Sys::flags, &DebugFlags::sig_dbgChanged, this, &Outline::slot_dbgChanged);
        connect(Sys::flags, &DebugFlags::sig_dbgTrigger, this, &Outline::slot_dbgTrigger);
}

Outline::Outline(const StylePtr & other) : Thick(other)
{
    std::shared_ptr<Thick> thick  = std::dynamic_pointer_cast<Thick>(other);
    if (!thick)
    {
        outline_width   = 0.03;
        join_style      = Qt::BevelJoin;
        cap_style       = Qt::SquareCap;
    }
    created = false;

        connect(Sys::flags, &DebugFlags::sig_dbgChanged, this, &Outline::slot_dbgChanged,Qt::QueuedConnection);
        connect(Sys::flags, &DebugFlags::sig_dbgTrigger, this, &Outline::slot_dbgTrigger);
}

Outline::~Outline()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting outline";
    pts4.clear();
#endif
}

void Outline::resetStyleRepresentation()
{
    casings.clear();
    created = false;
}

void Outline::createStyleRepresentation()
{
    qDebug() << "Outline::createStyleRepresentation()";

    if (created)
        return;

    Sys::debugMapCreate->wipeout();

    bool oldSetting = Edge::curvesAsLines;
    bool newSetting = Sys::flags->flagged(CURVES_AS_LINES);
    if (oldSetting != newSetting)
    {
        Edge::curvesAsLines = newSetting;
        Sys::mosaicMaker->reload();
        return;
    }

    MapPtr map = getProtoMap();
    NeighbourMap nmap(map);

    for (auto & vertex : map->getVertices())
    {
        NeighboursPtr np   = nmap.getNeighbours(vertex);
        auto cneighbours   = std::make_shared<CasingNeighbours>(*np.get());
        casings.weavings[vertex] = cneighbours;
    }

    uint index = 0;
    for (const EdgePtr & edge : map->getEdges())
    {
        OutlineCasingPtr casing = std::make_shared<OutlineCasing>(&casings,edge,width);
        casing->init();
        casings.push_back(casing);
        casing->edgeIndex = index;
        edge->casingIndex = index;
        index++;
    }

    for (CasingPtr & casing : casings)
    {
        auto edge = casing->getEdge();
        if (edge && edge->isCurve())
        {
            if (!Sys::flags->flagged(NO_ALIGN_CURVES))
            {
                OutlineCasingPtr ocp = std::dynamic_pointer_cast<OutlineCasing>(casing);
                ocp->alignCurvedEdgeSide1(casings);
                ocp->alignCurvedEdgeSide2(casings);
            }
        }
    }

    if (Sys::flags->flagged(VALIDATE))
        casings.validate();

    if (Sys::flags->flagged(DUMP_CASINGS))
        casings.dump("Complete");

    created = true;
}

void Outline::draw(GeoGraphics *gg)
{
    //qDebug() << "Outline::draw";

    if (!isVisible())
        return;

    if (!created)
        return;

    for (auto & casing : std::as_const(casings))
    {
        EdgePtr edge = casing->getEdge();
        if (!edge)
            continue;

        if (Sys::flags->flagged(OUTLINE_DBG))
        {
            if (edge->getType() == EDGETYPE_LINE && Sys::flags->flagged(DRAW_CURVES_ONLY))
                    continue;

            if (Sys::flags->flagged(EXCLUDE_ODDS) && (casing->edgeIndex & 1))
                    continue;

            if (Sys::flags->flagged(EXCLUDE_EVENS) && !(casing->edgeIndex & 1))
                    continue;
        }

        QColor color  = colors.getNextTPColor().color;
        QPen pen(color, 1, Qt::SolidLine, cap_style, join_style);

        casing->setPainterPath();

        casing->fillCasing(gg,pen);

        //QPainterPathStroker ps;
        //ps.setCapStyle(cap_style);
        //ps.setJoinStyle(join_style);
        //gg->fillStrokedPath(path, pen, ps);

        if (drawOutline != OUTLINE_NONE)
        {
            QPen pen;
            if (drawOutline == OUTLINE_SET)
            {
                pen = QPen(outline_color,Transform::scalex(gg->getTransform() * outline_width * 0.5));
            }
            else
            {
                Q_ASSERT(drawOutline == OUTLINE_DEFAULT);
                pen = QPen(Qt::black,1);
            }
            pen.setJoinStyle(join_style);
            pen.setCapStyle(cap_style);
            casing->drawOutline(gg,pen);
        }
    }

    if (Sys::flags->flagged(OUTLINE_DBG))
    {
        //Sys::debugMapPaint->wipeout();

        int colorIndex = 0;
        static const QColor colors[6] = {  Qt::red, Qt::green, Qt::blue, Qt::black, Qt::magenta, Qt::cyan };

        for (auto & casing : std::as_const(casings))
        {
            auto edge = casing->getEdge();
            if (!edge)
                continue;

            if (edge->isCurve() &&  Sys::flags->flagged(INDEX_CURVES))
            {
                Sys::debugMapPaint->insertDebugMark(edge->getMidPoint(), QString::number(casing->edgeIndex),Qt::red);
            }
            else if (Sys::flags->flagged(INDEX_LINES))
            {
                Sys::debugMapPaint->insertDebugMark(edge->getMidPoint(), QString::number(casing->edgeIndex),Qt::red);
            }

            int idx = colorIndex++ % 6;
            QColor color = colors[idx];
            casing->debugDraw(color,width);
        }
    }
}

void Outline::slot_dbgChanged(eDbgType type)
{
    switch(type)
    {
    case FLAG_REPAINT:
        Sys::viewController->slot_updateView();
        break;

    case FLAG_CREATE_STYLE:
        resetStyleRepresentation();
        createStyleRepresentation();
        Sys::viewController->slot_updateView();
        break;

    default:
    case FLAG_CREATE_MOTIF:
        // not handled here
        break;
    }
}

void Outline::slot_dbgTrigger(int val)
{
    qDebug() << "slot_dbgTrigger" << val;
    auto map     = prototype->getProtoMap();
    auto & edges = map->getEdges();
    if (val >= 0 && val < edges.size())
    {
        auto edge = edges[val];
        if(edge->isCurve())
        {
            CasingPtr casing = casings.find(edge);
            OutlineCasingPtr ocp = std::dynamic_pointer_cast<OutlineCasing>(casing);
            ocp->alignCurvedEdgeSide1(casings);
            ocp->alignCurvedEdgeSide2(casings);
            Sys::viewController->slot_updateView();
        }
    }
}

MapPtr Outline::getStyleMap()
{
    casings.buildMap();
    return casings.map;
}
