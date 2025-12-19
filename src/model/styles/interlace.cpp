#include "gui/viewers/geo_graphics.h"
#include "model/makers/mosaic_maker.h"
#include "model/styles/casing_neighbours.h"
#include "model/styles/interlace.h"
#include "sys/geometry/arcdata.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/map.h"
#include "sys/geometry/neighbour_map.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"
#include "sys/sys/debugflags.h"

#ifdef DEBUG_THREADS
#include "sys/sys/load_unit.h"
#endif

////////////////////////////////////////////////////////////////////////////
//
// Interlace.java
//
// Probably the most important rendering style from an historical point
// of view.  RenderInterlace assigns an over-under rule to the edges
// of the map and renders a weave the follows that assignment.  Getting
// the over-under rule is conceptually simple but difficult in practice,
// especially since we want to have some robust against degenerate maps
// being produced by other parts of the program (*sigh*).
//
// Basically, if a diagram can be interlaced, you can just choose an
// over-under relationship at one vertex and propagate it to all other
// vertices using a depth-first search.
//
// Drawing the interlacing takes a bit of trig, but it's doable.  It's
// just a pain when crossing edges don't cross in a perfect X.  I
// might get this wrong.

#ifdef QT_DEBUG
uint Interlace::dbgDump2 = 0x0; //0x3240;
#else
uint Interlace::dbgDump2 = 0x00;
#endif
uint Interlace::iCount   = 0;
uint Interlace::iTrigger = 0;

Interlace::Interlace(ProtoPtr proto) : Thick(proto)
{
    outline_width         = 0.03;
    join_style            = Qt::BevelJoin;
    cap_style             = Qt::SquareCap;
    gap                   = 0.0;
    shadow                = 0.0;
    includeTipVertices    = false;
    interlace_start_under = false;
    iTrigger              = 0;  // for debug

    connect(Sys::flags, &DebugFlags::sig_dbgChanged, this, &Interlace::slot_dbgChanged, Qt::QueuedConnection);
    connect(Sys::flags, &DebugFlags::sig_dbgTrigger, this, &Interlace::slot_dbgTrigger);

#ifdef THREAD_LIMITS
    threads.chainLimit  = 0;
#endif
}

Interlace::Interlace(StylePtr other) : Thick(other)
{
    std::shared_ptr<Interlace> intl = std::dynamic_pointer_cast<Interlace>(other);
    if (intl)
    {
        gap                   = intl->gap;
        shadow                = intl->shadow;
        includeTipVertices    = intl->includeTipVertices;
        interlace_start_under = intl->interlace_start_under;
        iTrigger              = 0;
    }
    else
    {
        std::shared_ptr<Thick> thick  = std::dynamic_pointer_cast<Thick>(other);
        if (!thick)
        {
            outline_width  = 0.03;
            join_style     = Qt::BevelJoin;
            cap_style      = Qt::SquareCap;
        }
        gap                   = 0.0;
        shadow                = 0.0;
        includeTipVertices    = false;
        interlace_start_under = false;
        iTrigger              = 0;  // for debug
    }

    connect(Sys::flags, &DebugFlags::sig_dbgChanged, this, &Interlace::slot_dbgChanged, Qt::QueuedConnection);
    connect(Sys::flags, &DebugFlags::sig_dbgTrigger, this, &Interlace::slot_dbgTrigger);
}

Interlace:: ~Interlace()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting interlace";
    pts.clear();
    todo.clear();
    threads.clear();
#endif
}

void Interlace::draw(GeoGraphics * gg)
{
    if (!isVisible())
        return;

    if (casings.size() == 0)
        return;

    int index = Sys::flags->getDbgIndex();
    bool solo = Sys::flags->flagged(SOLO_EDGE);

    //Sys::debugMapPaint->wipeout();

    QPen pen(Qt::black,1, Qt::SolidLine, cap_style, join_style);      // OUTLINE_DEFAULT;
    if (drawOutline == OUTLINE_SET)
    {
        pen = QPen(outline_color,Transform::scalex(gg->getTransform() * outline_width * 0.5));
    }
    pen.setJoinStyle(join_style);
    pen.setCapStyle(cap_style);

    for (CasingPtr & casing : casings)
    {
        if (solo&& index != casing->edgeIndex)
            continue;

        if (!casing->created())
            continue;

        casing->setPainterPath();

        InterlaceCasingPtr icp = std::static_pointer_cast<InterlaceCasing>(casing);
        QPen pen2(icp->color, 1, Qt::SolidLine, cap_style, join_style);
        casing->fillCasing(gg,pen2);
    }

    if (!Sys::flags->flagged(NO_SHADOW))
    {
        for (CasingPtr & casing : casings)
        {
            if (shadow > 0.0)
            {
                InterlaceCasingPtr icp = std::static_pointer_cast<InterlaceCasing>(casing);
                QPen & spen = icp->getShadowPen();
                spen.setJoinStyle(join_style);
                spen.setCapStyle(cap_style);
                icp->drawShadows(gg,shadow);
            }
        }
    }

    if (drawOutline != OUTLINE_NONE)
    {
        for (CasingPtr & casing : casings)
        {
            casing->drawOutline(gg,pen);
        }
    }

    if (!Sys::flags->flagged(ILACE_DBG))
        return;

    // DEBUG CODE BELOW

    int colorIndex = 0;
    static const QColor colors[6] = {  Qt::red, Qt::green, Qt::blue, Qt::yellow, Qt::magenta, Qt::cyan };

    for (auto & casing : casings)
    {
        auto edge = casing->getEdge();
        if (!edge)
            continue;

        if (!casing->created())
            continue;

        if (solo && casing->edgeIndex != index)
            continue;

        if (edge->isLine() && Sys::flags->flagged(DRAW_CURVES_ONLY))
            continue;

        if ((casing->edgeIndex == index) && Sys::flags->flagged(HIGHLIGHT_SELECTED))
        {
            QPen p2(Qt::red,3,Qt::SolidLine, cap_style, join_style);
            casing->fillCasing(gg,p2);
            casing->drawOutline(gg,p2);
        }

        if (Sys:: flags->flagged(OVER_UNDER))
        {
            auto is1 = static_cast<InterlaceSide*>(casing->s1);
            auto is2 = static_cast<InterlaceSide*>(casing->s2);
            QString str = QString::number(casing->edgeIndex);
            str  += (is1->under()) ? "U" : "O";
            Sys::debugMapPaint->insertDebugMark(casing->s1->mid,str,Qt::blue);
            QString str2 = QString::number(casing->edgeIndex);
            str2 += (is2->under()) ? "U" : "O";
            Sys::debugMapPaint->insertDebugMark(casing->s2->mid,str2,Qt::blue);
        }

        if (Sys::flags->flagged(SIDE_1_LINE))
        {
            auto is1 = static_cast<InterlaceSide*>(casing->s1);
            if (is1->under())
                Sys::debugMapPaint->insertDebugLine(casing->s1Line(),Qt::green,3);
            else
                Sys::debugMapPaint->insertDebugLine(casing->s1Line(),Qt::red,3);
        }

        if (Sys::flags->flagged(SIDE_2_LINE))
        {
            auto is2 = static_cast<InterlaceSide*>(casing->s2);
            if (is2->under())
                Sys::debugMapPaint->insertDebugLine(casing->s2Line(),Qt::green,3);
            else
                Sys::debugMapPaint->insertDebugLine(casing->s2Line(),Qt::red,3);
        }

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

void Interlace::resetStyleRepresentation()
{
    Thick::resetStyleRepresentation();
    casings.reset();
    threads.clear();
    created = false;
}

void Interlace::createStyleRepresentation()
{
    qDebug() << "Interlace::createStyleRepresentation";

    if (created)
        return;

    MapPtr map = getProtoMap();
    if (!map)
        return;

    Q_ASSERT(!map->isEmpty());
    Q_ASSERT(casings.isEmpty());
    Q_ASSERT(threads.isEmpty());
    Q_ASSERT(todo.isEmpty());

    Sys::debugMapCreate->wipeout();

    iCount   = 0;

    bool oldSetting = Edge::curvesAsLines;
    bool newSetting = Sys::flags->flagged(CURVES_AS_LINES);
    if (oldSetting != newSetting)
    {
        Edge::curvesAsLines = newSetting;
        Sys::mosaicMaker->reload();
        return;
    }

    NeighbourMap nmap(map);
    for (auto & vertex : map->getVertices())
    {
        NeighboursPtr np   = nmap.getNeighbours(vertex);
        auto cneighbours   = std::make_shared<CasingNeighbours>(*np.get());
        casings.weavings[vertex] = cneighbours;
    }

    int index = 0;
    for (auto & edge  : map->getEdges())
    {
        auto casing = std::make_shared<InterlaceCasing>(&casings,edge,width);
        casing->init();
        casings.push_back(casing);
        casing->edgeIndex = index;
        edge->casingIndex = index;
        index++;
    }

    for (auto it = casings.weavings.begin(); it != casings.weavings.end(); it++)
    {
        CNeighboursPtr cneighbours   = it.value();
        cneighbours->findNeighbouringCasings(&casings);
    }

    // setup interlacing
#ifdef DEBUG_THREADS
    defaultColor = Qt::white;
#else
    defaultColor = colors.getLastTPColor().color;
#endif
    // assign thread to each casing
    if (colors.size() > 1)
    {
        threads.createThreads(casings);
        threads.assignColors(colors);
#ifdef DEBUG_THREADS
        qInfo() << "LOG2: " << Sys::mosaicMaker->getLoadUnit()->getLoadFile().getVersionedName().get() << "Threads " << threads.size();
#endif
    }

    // define under/over for casing
    auto & vertices = getProtoMap()->getVertices();
    for (auto & vert : vertices)
    {
        vert->visited = false;
    }

    // Stack of edge to be processed.
    todo.clear();

    for (auto & casing : casings)
    {
        auto is1 = static_cast<InterlaceSide*>(casing->s1);
        if (!is1->visited())
        {
            InterlaceCasingPtr icp = std::static_pointer_cast<InterlaceCasing>(casing);

            icp->setUnder(!interlace_start_under);

            todo.push(icp);
            buildFrom();
            //map->dumpMap(false);
        }
    }

    // create casing colors
    for (auto & casing  : casings)
    {
        InterlaceCasingPtr icp = std::static_pointer_cast<InterlaceCasing>(casing);
        icp->createColors(defaultColor);
    }

    // re-align curved edges
    if (!Sys::flags->flagged(NO_ALIGN_CURVES))
    {
        for (auto & casing : casings)
        {
            auto edge = casing->getEdge();
            if (edge && edge->isCurve())
            {
                casing->innerCircle = Circle(edge->getArcCenter(),edge->getRadius()-width);
                casing->outerCircle = Circle(edge->getArcCenter(),edge->getRadius()+width);

                casing->alignCurvedEdgeSide1(casings);
                casing->alignCurvedEdgeSide2(casings);
            }
        }
    }

    if (Sys::flags->flagged(DUMP_CASINGS)) casings.dump("aligned");

    casings.weave();

    if (dbgDump2 & 0x800) casings.dumpWeaveStatus();

    if (!Sys::flags->flagged(NO_GAP))
    {
        if (gap > 0.0)
        {
            // set gaps
            for (auto & casing : casings)
            {
                InterlaceCasingPtr icp = std::static_pointer_cast<InterlaceCasing>(casing);
                icp->setGap(gap);
            }
        }
    }

    if (Sys::flags->flagged(VALIDATE)) casings.validate();
    if (Sys::flags->flagged(DUMP_CASINGS)) casings.dump("Complete");

    created = true;
}

#if 0 // FIXME - delete unused
void Interlace::createUnders()
{
    Q_ASSERT(Sys::flags->flagged(APPROACH_6));

    for (auto & casing  : casings)
    {
        CasingData cd1 = casing->getCasingData();

        InterlaceCasingPtr icp = std::static_pointer_cast<InterlaceCasing>(casing);
        icp->createUnder();

        if (Interlace::dbgDump2  & 0x80)
        {
            CasingData cd2 = casing->getCasingData();
            if (cd1 != cd2)
                cd1.dumpDiffs(cd2);
        }

        if (Sys::flags->flagged(USE_TRIGGER_2) && (casing->edgeIndex == iTrigger))
            break;
    }
}
#endif
#if 0 // FIXME - delete unused
void Interlace::alignCurvedEdges(eDbgFlag flag)
{

    if (Sys::flags->flagged(NO_ALIGN_CURVES))
        return;

    Q_UNUSED(flag);
    for (CasingPtr & casing : casings)
    {
        auto edge = casing->getEdge();
        if (edge && edge->isCurve())
        {
            qDebug() << "aligning edge:" << casing->edgeIndex;
            auto is1 = static_cast<InterlaceSide*>(casing->s1);
            is1->alignS1Inner(casing);
            is1->alignS1Outer(casing);

            auto is2 = static_cast<InterlaceSide*>(casing->s2);
            is2->alignS2Inner(casing);
            is2->alignS2Outer(casing);
        }
    }
}
#endif

// Propagate the over-under relation from an edge to its incident vertices.
void Interlace::buildFrom()
{
    //qDebug() << "Interlace::buildFrom";

    while (!todo.empty())
    {
        InterlaceCasingPtr casing = todo.pop();

        if (!casing->getEdge()->v1->visited)
        {
            casing->getEdge()->v1->visited = true;
            auto is1 = static_cast<InterlaceSide*>(casing->s1);
            propagate(casing, is1, is1->under());
        }
        if (!casing->getEdge()->v2->visited)
        {
            casing->getEdge()->v2->visited = true;
            auto is2 = static_cast<InterlaceSide*>(casing->s2);
            propagate(casing, is2, is2->under());
        }
    }
}

// Propagate the over-under relationship from a vertices to its
// adjacent edges.  The relationship is encapsulated in the
// "edge_under_at_vert" variable, which says whether the
// edge passed in is in the under state at this vertex.
// The whole trick is to manage how neighbours receive modifications
// of edge_under_at_vert.

void Interlace::propagate(InterlaceCasingPtr &cp, InterlaceSide *s, bool edge_under_at_vert)
{
    VertexPtr v = s->vertex();
    NeighboursPtr neighbours = casings.weavings.value(v);

    EdgePtr edge = cp->getEdge();

    int nn = neighbours->numNeighbours();
    if (nn == 1)
    {
        if ( includeTipVertices)
        {
            EdgePtr oe = neighbours->getNeighbour(0);
            CasingPtr cp = casings.find(oe);
            auto is1 = static_cast<InterlaceSide*>(cp->s1);
            if (!is1->visited())
            {
                // With a bend, we don't want to change the underness
                // of the edge we're propagating to.
                InterlaceCasingPtr icp = std::static_pointer_cast<InterlaceCasing>(cp);
                if (cp->s1->vertex() == s->vertex())
                {
                    // The new edge starts at the current vertex.
                    icp->setUnder(!edge_under_at_vert);
                }
                else
                {
                    // The new edge ends at the current vertex.
                    icp->setUnder(edge_under_at_vert);
                }
                todo.push(icp);
            }
        }
    }
    else if (nn == 2 || nn == 3)
    {
        BeforeAndAfter  ba  = neighbours->getBeforeAndAfter(edge);
        EdgePtr oe          = ba.before;
        CasingPtr cp = casings.find(oe);
        auto is1 = static_cast<InterlaceSide*>(cp->s1);
        if (!is1->visited())
        {
            // With a bend, we don't want to change the underness
            // of the edge we're propagating to.
            InterlaceCasingPtr icp = std::static_pointer_cast<InterlaceCasing>(cp);
            if( cp->s1->vertex() == s->vertex())
            {
                // The new edge starts at the current vertex.
                icp->setUnder(!edge_under_at_vert);
            }
            else
            {
                // The new edge ends at the current vertex.
                icp->setUnder(edge_under_at_vert);
            }
            todo.push(icp);
        }
    }
    else
    {
        QVector<WeakEdgePtr> ns;
        int index    = 0;
        int edge_idx = -1;  // index of edge in ns

        for (auto & wedge : *neighbours)
        {
            ns << wedge;
            if (wedge.lock() == edge)
            {
                edge_idx = index;
            }
            index++;
        }
        Q_ASSERT(edge_idx != -1);

        // this assumes edges are sorted by angle, so the +1 edge has the reversed
        // v1_under, and the +2 edge is the colinear continuation of the first edge, etc.
        // Seems reasonable for nn=4.
        // I am wondering if when nn=3 this works if +1 is from left but +1 could be colinear
        for (int idx = 1; idx < nn; ++idx )
        {
            int cur = (edge_idx + idx) % nn;
            EdgePtr oe      = ns[cur].lock();
            Q_ASSERT(oe);
            CasingPtr cp   = casings.find(oe);
            auto is1 = static_cast<InterlaceSide*>(cp->s1);
            if (!is1->visited())
            {
                InterlaceCasingPtr icp = std::static_pointer_cast<InterlaceCasing>(cp);
                if( cp->s1->vertex() == s->vertex())
                {
                    icp->setUnder(!edge_under_at_vert);
                }
                else
                {
                    icp->setUnder(edge_under_at_vert);
                }
                todo.push(icp);
            }
            edge_under_at_vert = !edge_under_at_vert;
        }
    }
}

void Interlace::slot_dbgChanged(eDbgType type)
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

    case FLAG_CREATE_MOTIF:
        // not handled here
        break;
    }
}

void Interlace::slot_dbgTrigger(int val)
{
    qDebug() << "slot_dbgTrigger" << val;

    iTrigger = val;
    slot_dbgChanged(FLAG_CREATE_STYLE);

#ifdef DEBUG_EDGES
    auto & map   = prototype->getProtoMap();
    auto & edges = map->getEdges();
    if (val >= 0 && val < edges.size())
    {
        auto edge = edges[val];
        if(edge->isCurve())
        {
            auto casing = casings.find(edge);
            casing->alignCurvedEdge(casings);
            Sys::viewController->slot_updateView();
        }
    }
#endif
#ifdef DEBUG_THREADS
    slot_dbgChanged(FLAG_CREATE_STYLE);
    threads.chainLimit++;
#endif
}

MapPtr Interlace::getStyleMap()
{
    casings.buildMap();
    return casings.map;
}

void Interlace::slot_styleMapUpdated(MapPtr map)
{
    if (map != casings.map)
        return;

    casings.useMap();

    Sys::viewController->repaintView();
}

bool Interlace::dbgBreak(InterlaceCasingPtr & casing, QString msg)
{
    switch (casing->edgeIndex)
    {
    case 278:
    case 399:
    case 521:
    case 580:
        qDebug() << "BREAK" << msg << casing->edgeIndex;
        return true;

    default:
        return false;
    }
}
