#include "gui/top/system_view_controller.h"
#include "gui/viewers/geo_graphics.h"
#include "gui/viewers/grid_view.h"
#include "model/makers/tiling_maker.h"
#include "model/settings/configuration.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/fill_region.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_verifier.h"
#include "sys/geometry/vertex.h"

using std::make_shared;

typedef std::shared_ptr<class Vertex> VertexPtr;

GridView::GridView() : LayerController(VIEW_GRID,DERIVED,"Grid")
{
    config  = Sys::config;
    genMap  = false;
    setZLevel(config->gridZLevel);
}

GridView::~GridView()
{}

void GridView::paint(QPainter * painter)
{
    draw(painter);
}

void GridView::draw(QPainter * painter)
{
    if (genMap)
    {
        gridMap = make_shared<Map>("Grid");
    }

    switch (config->gridUnits)
    {
    case GRID_UNITS_TILE:
        if (config->gridTilingAlgo == FLOOD)
            drawFromTilingFlood(painter);
        else
            drawFromTilingRegion(painter);
        break;

    case GRID_UNITS_MODEL:
        if (config->gridModelCenter)
            drawModelUnitsCentered(painter);
        else
            drawModelUnits(painter);
        break;

    case GRID_UNITS_SCREEN:
        if (config->gridScreenCenter)
            drawScreenUnitsCentered(painter);
        else
            drawScreenUnits(painter);
        break;

    case GRID_UNITS_OFF:
        break;
    }

    drawCenters(painter);

    if (genMap)
    {
        MapVerifier mv(gridMap);
        mv.verify();
    }
}

void GridView::drawCenters(QPainter * painter)
{
    if (config->showGridModelCenter)
    {
        auto gg = new GeoGraphics(painter,getLayerTransform());
        corners[0] = QLineF(QPointF(-10,10),QPointF(10,-10));
        corners[1] = QLineF(QPointF(-10,-10),QPointF(10,10));
        ggdrawLine(gg,corners[0],QPen(Qt::green,3));
        ggdrawLine(gg,corners[1],QPen(Qt::green,3));
        delete gg;
    }

    if (config->showGridViewCenter)
    {
        auto r = Sys::viewController->viewRect();
        corners[0] = QLineF(r.topLeft(),r.bottomRight());
        corners[1] = QLineF(r.bottomLeft(),r.topRight());

        QPen pen(Qt::blue,3);
        painter->setPen(pen);
        ppdrawLine(painter,corners[0]);
        ppdrawLine(painter,corners[1]);
    }
}

void GridView::drawFromTilingFlood(QPainter *painter)
{
    qDebug() << "Grid::drawFromTilingFlood()";

    auto r = QRectF(-10,10,20,20);
    QPen pen(QColor(config->gridColorTiling),config->gridTilingWidth);

    QPointF center = getCenterModelUnits();
    r.moveCenter(center);
    //qDebug() << "grid model center" << center;

    auto maker  = Sys::tilingMaker;
    auto tiling = maker->getSelected();
    auto header = tiling->hdr();

    const PlacedTiles tilingUnit = maker->getSelected()->unit().getIncluded();
    if (tilingUnit.size())
    {
        auto placedTile = tilingUnit.first();
        QTransform T    = placedTile->getPlacement();
        center          = T.map(placedTile->getTile()->getCenter());
    }
    QLineF line1(center, header.getTrans1());
    QLineF line2(center, header.getTrans2());
    if (line1.isNull() || line2.isNull())
    {
        return;
    }
    qDebug() << "len1:" << line1.length() << "len2:" << line2.length();

    qreal angle1    = qDegreesToRadians(line1.angle());
    qreal angle2    = qDegreesToRadians(line2.angle());
    qreal gridAngle = qAbs(angle1 - angle2);
    qDebug() << "Line angles:" << angle1 << angle2 << "Grid angle" << gridAngle << "sin" << qAbs(qSin(gridAngle));
    
    auto gg = new GeoGraphics(painter,getLayerTransform());

    QLineF l1 = Geo::extendAsRay(line1,5.0);
    qreal offset = line2.length() * qAbs(qSin(gridAngle));
    l1 = Geo::shiftParallel(l1,offset * -10.0);
    for (int i=0; i <20; i++)
    {
        l1 = Geo::shiftParallel(l1,offset);
        ggdrawLine(gg,l1,pen);
    }
    
    QLineF l2 = Geo::extendAsRay(line2,5.0);
    qreal offset2 = line1.length() * qAbs(qSin(gridAngle));
    l2 = Geo::shiftParallel(l2,offset2 * -10.0);
    for (int i=0; i <20; i++)
    {
        l2 = Geo::shiftParallel(l2,offset2);
        ggdrawLine(gg,l2,pen);
    }

    delete gg;
}

void GridView::drawFromTilingRegion(QPainter *painter)
{
    qDebug() << "Grid::drawFromTilingRegion()";

    auto r = QRectF(-10,10,20,20);
    QPen pen(QColor(config->gridColorTiling),config->gridTilingWidth);

    QPointF center = getCenterModelUnits();
    r.moveCenter(center);
    //qDebug() << "grid model center" << center;

    auto maker  = Sys::tilingMaker;
    auto tiling = maker->getSelected();
    auto header  = tiling->hdr();

    const PlacedTiles tilingUnit = maker->getSelected()->unit().getIncluded();
    if (tilingUnit.size())
    {
        auto placedTile = tilingUnit.first();
        QTransform T    = placedTile->getPlacement();
        center          = T.map(placedTile->getTile()->getCenter());
    }
    QLineF lineR(center, header.getTrans1());
    QLineF lineG(center, header.getTrans2());
    if (lineR.isNull() || lineG.isNull())
    {
        return;
    }

    qDebug() << "lenR:" << lineR.length() << "len:" << lineG.length();

    qreal angle1    = qDegreesToRadians(lineR.angle());
    qreal angle2    = qDegreesToRadians(lineG.angle());
    qreal gridAngle = angle1 - angle2;

    qDebug().noquote()  << "Line angles:"
                       << angle1    << Circle::getQuadrantString(angle1)
                       << angle2    << Circle::getQuadrantString(angle2)
                       <<"Grid angle"
                       << gridAngle << Circle::getQuadrantString(qAbs(gridAngle))
                       << "sin" << qAbs(qSin(gridAngle));


    qreal offsetG  = lineG.length() * qAbs(qSin(qAbs(gridAngle)));
    qreal offsetR  = lineR.length() * qAbs(qSin(qAbs(gridAngle)));

    QLineF lr;
    QLineF lg;
    if (gridAngle  >= 0)
    {
        lr = Geo::shiftParallel(lineR,offsetR);
        lg = Geo::shiftParallel(lineG,-offsetG );
    }
    else
    {
        lr = Geo::shiftParallel(lineR,-offsetR);
        lg = Geo::shiftParallel(lineG,offsetG );
    }

    lr.translate(lineG.p2()-lr.p1());
    lg.translate(lineR.p2()-lg.p1());

#if 0
    qDebug() << "lineR" << lineR;
    qDebug() << "lg   " << lg;
    qDebug() << "lr   " << lr;
    qDebug() << "lineG" << lineG;
#endif

    // Use this when sides are correct
    QPolygonF p;
    p << lineR.p1();
    Q_ASSERT(Loose::equalsPt(lineR.p2(),lg.p1()));
    p << lg.p1();
    Q_ASSERT(Loose::equalsPt(lg.p2(),lr.p2()));
    p << lr.p2();
    Q_ASSERT(Loose::equalsPt(lr.p1(),lineG.p2()));
    p << lineG.p2();
    Q_ASSERT(Loose::equalsPt(lineG.p1(),lineR.p1()));

    auto gg = new GeoGraphics(painter,getLayerTransform());

    FillRegion flood(tiling.get(),tiling->hdr().getCanvasSettings().getFillData());
    const Placements & placements = flood.getPlacements(config->repeatMode);
    for (const auto & t : std::as_const(placements))
    {
        auto poly = t.map(p);
        ggdrawPoly(gg,poly,pen);
    }

    static constexpr QColor normal_color = QColor(217,217,255,128);
    gg->fillPolygon(p,normal_color);

    delete gg;
}

// this is relative to model(0,0)
void GridView::drawModelUnits(QPainter *painter)
{
    qreal step = config->gridModelSpacing;

    QPoint pt(0,0);
    QRect vrect(pt,Sys::viewController->viewSize());
    QRect mrect = screenToModel(vrect);

    int x_steps = qCeil(mrect.width()  / step);
    int y_steps = qCeil(mrect.height() / step);

    auto gg = new GeoGraphics(painter,getLayerTransform());
    drawModelUnits(gg,mrect,mrect,pt,x_steps,y_steps,step);
    delete gg;
}

void GridView::drawModelUnitsCentered(QPainter *painter)
{
    qreal step = config->gridModelSpacing;

    QTransform t = getCanvasTransform();     // implies unity model transform
    QRectF viewRect(Sys::viewController->viewRect());
    viewRect = t.inverted().mapRect(viewRect);
    QPointF center = viewRect.center();

    QRectF modelRect(-10,-10,20,20);
    int x_steps = qCeil(viewRect.width()  / step);
    int y_steps = qCeil(viewRect.height() / step);

    auto gg = new GeoGraphics(painter,t);
    drawModelUnits(gg,modelRect,viewRect,center,x_steps,y_steps,step);
    delete gg;
}

void GridView::drawModelUnits(GeoGraphics *gg, QRectF r, QRectF r1, QPointF center, int x_steps, int y_steps, qreal step)
{
    QPen pen(QColor(config->gridColorModel),config->gridModelWidth);
    gg->getPainter()->setPen(pen);

    ggdrawLine(gg,r.topLeft(),r.topRight(),pen);
    ggdrawLine(gg,r.topRight(),r.bottomRight(),pen);
    ggdrawLine(gg,r.bottomRight(),r.bottomLeft(),pen);
    ggdrawLine(gg,r.bottomLeft(),r.topLeft(),pen);

    QVector<QLineF> edges = toEdges(r1);

    eGridType type = config->gridType;
    if (type == GRID_ORTHOGONAL || type == GRID_ISOMETRIC)
    {
        // draw vertical lines
        qreal x = center.x() - ((x_steps/2) * step);
        for (int i = 0; i < x_steps; i++)
        {
            QLineF l(QPointF(x,r.top()),QPointF(x,r.bottom()));
            if (intersects(edges,l))
                ggdrawLine(gg,l,pen);
            x += step;
        }
    }

    // horizontal
    if (type == GRID_ORTHOGONAL)
    {
        // draw horizontal lines
        qreal y = center.y() - ((y_steps/2) * step);
        for (int i = 0; i < y_steps; i++)
        {
            QLineF l(QPointF(r.left(),y),QPointF(r.right(),y));
            if (intersects(edges,l))
                ggdrawLine(gg,l,pen);
            y += step;
        }
    }
    else if (type == GRID_ISOMETRIC || type == GRID_RHOMBIC)
    {
        qreal angle = (type == GRID_ISOMETRIC) ? 30.0 : config->gridAngle;
        y_steps *=2;
        qreal y = center.y() - ((y_steps/2) * step);
        for (int i = 0; i < y_steps; i++)
        {
            QLineF line(QPointF(r1.left(),y),QPointF(r1.right(),y));
            QPointF mid = line.center();

            QTransform t;
            t.translate(mid.x(),mid.y());
            t.rotate(angle);
            t.translate(-mid.x(),-mid.y());

            QLineF line2 = t.map(line);
            if (intersects(edges,line2))
                ggdrawLine(gg,line2,pen);

            QTransform t2;
            t2.translate(mid.x(),mid.y());
            t2.rotate(-angle);
            t2.translate(-mid.x(),-mid.y());

            QLineF line3 = t2.map(line);
            if (intersects(edges,line3))
                ggdrawLine(gg,line3,pen);

            y += step;
        }
    }
}

void GridView::drawScreenUnitsCentered(QPainter *painter)
{
    qreal step     = config->gridScreenSpacing;
    if (step < 10.0)
    {
        qDebug() << "grid step too small" << step;
        return;
    }

    QRectF viewRect  = Sys::viewController->viewRect();
    QPointF center   = viewRect.center();
    {
        int x_steps = qCeil(viewRect.width()  / step);
        int y_steps = qCeil(viewRect.height() / step);

        drawScreenUnits(painter,viewRect, viewRect, center, x_steps, y_steps, step);
    }
}
void GridView::drawScreenUnits(QPainter *painter)
{
    qreal step     = config->gridScreenSpacing;
    if (step < 10.0)
    {
        qDebug() << "grid step too small" << step;
        return;
    }

    QRectF viewRect(Sys::viewController->viewRect());
    QPointF center = viewRect.center();

    int x_steps = qCeil(viewRect.width()  / step);
    int y_steps = qCeil(viewRect.height() / step);

    QPointF pt = getModelXform().getTranslate();
    QTransform t = QTransform::fromTranslate(pt.x(),pt.y());

    painter->save();
    painter->setWorldTransform(t);
    drawScreenUnits(painter,viewRect, viewRect, center, x_steps, y_steps, step);
    painter->restore();
}

void GridView::drawScreenUnits(QPainter * painter, QRectF r, QRectF r1, QPointF center,int x_steps, int y_steps, qreal step)
{
    QPen pen(QColor(config->gridColorScreen),config->gridScreenWidth);
    painter->setPen(pen);

    QVector<QLineF> edges = toEdges(r);

    eGridType type = config->gridType;
    if (type == GRID_ORTHOGONAL || type == GRID_ISOMETRIC)
    {
        // draw vertical lines
        qreal x = center.x() - ((x_steps/2) * step);
        for (int i = 0; i < x_steps; i++)
        {
            QLineF l(QPointF(x,r.top()),QPointF(x,r.bottom()));
            if (intersects(edges,l))
                ppdrawLine(painter,l);
            x += step;
        }
    }

    if (type == GRID_ORTHOGONAL)
    {
        // draw horizontal lines
        qreal y = center.y() - ((y_steps/2) * step);
        for (int i = 0; i < y_steps; i++)
        {
            QLineF l(QPointF(r.left(),y),QPointF(r.right(),y));
            if (intersects(edges,l))
                ppdrawLine(painter,l);
            y += step;
        }
    }
    else if (type == GRID_ISOMETRIC || type == GRID_RHOMBIC)
    {
        qreal angle = (type == GRID_ISOMETRIC) ? 30.0 : config->gridAngle;
        y_steps *=2;
        qreal y = center.y() - ((y_steps/2) * step);
        for (int i = 0; i < y_steps; i++)
        {
            QLineF line(QPointF(r1.left(),y),QPointF(r1.right(),y));
            QPointF mid = line.center();

            QTransform t;
            t.translate(mid.x(),mid.y());
            t.rotate(angle);
            t.translate(-mid.x(),-mid.y());

            QLineF line2 = t.map(line);
            if (intersects(edges,line2))
                ppdrawLine(painter,line2);

            QTransform t2;
            t2.translate(mid.x(),mid.y());
            t2.rotate(-angle);
            t2.translate(-mid.x(),-mid.y());

            QLineF line3 = t2.map(line);
            if (intersects(edges,line3))
                ppdrawLine(painter,line3);

            y += step;
        }
    }
}

bool GridView::intersects(QVector<QLineF> &edges, QLineF & line)
{
    QPointF p1;
    for (const auto & edge : std::as_const(edges))
    {
        QPointF intersect;
        if (line.intersects(edge,&intersect) != QLineF::NoIntersection && Geo::isOnLine(intersect,edge))
        {
            if (p1.isNull())
                p1 = intersect;
            else
            {
                line.setP1(p1);
                line.setP2(intersect);
                return true;
            }
        }
    }
    return false;
}

QVector<QLineF> GridView::toEdges(const QRectF & r)
{
    QVector<QLineF> vec;
    vec.push_back(QLineF(r.topLeft(),r.topRight()));
    vec.push_back(QLineF(r.topRight(),r.bottomRight()));
    vec.push_back(QLineF(r.bottomRight(),r.bottomLeft()));
    vec.push_back(QLineF(r.bottomLeft(),r.topLeft()));
    return vec;
}

// used by tiling maker when creating a polygon
bool GridView::nearGridPoint(QPointF spt, QPointF & foundGridPoint)
{
    if (!viewControl()->isEnabled(VIEW_GRID) || !config->tm_snapToGrid)
    {
        return false;
    }

    genMap = true;
    Sys::viewController->repaintView();  // generates gridMap, used below
    genMap = false;

    for (const VertexPtr & v : std::as_const(gridMap->getVertices()))
    {
        QPointF a = v->pt;
        QPointF b = a;
        if (config->gridUnits == GRID_UNITS_MODEL || config->gridUnits == GRID_UNITS_TILE)
        {
            b = modelToScreen(a);
        }
        if (Geo::isNear(spt,b))
        {
            if (config->gridUnits == GRID_UNITS_MODEL)
            {
                foundGridPoint = a;
            }
            else
            {
                foundGridPoint = screenToModel(a);
            }
            qDebug() << "nearGridPoint - FOUND";
            return true;
        }
    }
    return false;
}

void GridView::ggdrawLine(GeoGraphics * gg, QLineF line, QPen pen)
{
    gg->drawLine(line,pen);
    if (genMap)
    {
        VertexPtr v1 = gridMap->insertVertex(line.p1());
        VertexPtr v2 = gridMap->insertVertex(line.p2());
        gridMap->insertEdge(v1,v2);
    }
}

void GridView::ggdrawLine(GeoGraphics *gg, QPointF p1, QPointF p2, QPen  pen)
{
    gg->drawLine(p1, p2, pen);
    if (genMap)
    {
        VertexPtr v1 = gridMap->insertVertex(p1);
        VertexPtr v2 = gridMap->insertVertex(p2);
        gridMap->insertEdge(v1,v2);
    }
}

void GridView::ggdrawPoly(GeoGraphics *gg, QPolygonF & poly, QPen  pen)
{
    gg->drawPolygon(poly,pen);
    if (genMap)
    {
        EdgePoly ep(poly);
        auto vec = ep.getLines();
        for (auto & line : std::as_const(vec))
        {
            VertexPtr v1 = gridMap->insertVertex(line.p1());
            VertexPtr v2 = gridMap->insertVertex(line.p2());
            gridMap->insertEdge(v1,v2);
        }
    }
}

void GridView::ppdrawLine(QPainter * painter, QLineF & line)
{
    painter->drawLine(line);
    if (genMap)
    {
        VertexPtr v1 = gridMap->insertVertex(line.p1());
        VertexPtr v2 = gridMap->insertVertex(line.p2());
        gridMap->insertEdge(v1,v2);
    }
}

void GridView::ppdrawLine(QPainter *painter, QPointF p1, QPointF p2)
{
    painter->drawLine(p1,p2);
    if (genMap)
    {
        VertexPtr v1 = gridMap->insertVertex(p1);
        VertexPtr v2 = gridMap->insertVertex(p2);
        gridMap->insertEdge(v1,v2);
    };
}
void GridView::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    Q_UNUSED(spt);
    Q_UNUSED(btn);
}

void GridView::slot_mouseDragged(QPointF spt)
{
    Q_UNUSED(spt);
}

void GridView::slot_mouseMoved(QPointF spt)
{
    Q_UNUSED(spt);
}

void GridView::slot_mouseReleased(QPointF spt)
{
    Q_UNUSED(spt);
}

void GridView::slot_mouseDoublePressed(QPointF spt)
{
    Q_UNUSED(spt);
}
