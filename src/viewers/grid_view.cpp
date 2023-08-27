#include "viewers/grid_view.h"
#include "settings/configuration.h"
#include "viewers/viewcontrol.h"
#include "misc/geo_graphics.h"
#include "geometry/fill_region.h"
#include "geometry/map.h"
#include "geometry/vertex.h"
#include "geometry/point.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "tile/placed_tile.h"
#include "tile/tile.h"
#include "tile/tiling.h"

using std::make_shared;

typedef std::shared_ptr<class Vertex> VertexPtr;

GridView * GridView::mpThis = nullptr;

GridView * GridView::getInstance()
{
    if (!mpThis)
    {
        mpThis = new GridView();
    }
    return mpThis;
}

void GridView::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

GridView::GridView() : LayerController("Grid")
{
    config  = Configuration::getInstance();
    view    = ViewControl::getInstance();
    genMap  = false;
    setZValue(config->gridZLevel);
}

GridView::~GridView()
{}

void GridView::paint(QPainter * pp)
{
    draw(pp);
}

void GridView::draw(QPainter * pp)
{
    if (genMap)
    {
        gridMap = make_shared<Map>("Grid");
    }

    this->pp = pp;
    gg = new GeoGraphics(pp,getLayerTransform());

    switch (config->gridUnits)
    {
    case GRID_UNITS_TILE:
        if (config->gridTilingAlgo == FLOOD)
        {
            drawFromTilingFlood();
        }
        else
        {
            drawFromTilingRegion();
        }
        break;

    case GRID_UNITS_MODEL:
        if (config->gridModelCenter)
        {
            drawModelUnitsCanvasCentered();
        }
        else
        {
            drawModelUnitsModelCentered();
        }
        break;

    case GRID_UNITS_SCREEN:
        if (config->gridScreenCenter)
        {
            drawScreenUnitsModelCentered();
        }
        else
        {
            drawScreenUnitsCanvasCentered();
        }
        break;
    }

    if (config->showGridModelCenter)
    {
        corners[0] = QLineF(QPointF(-10,10),QPointF(10,-10));
        corners[1] = QLineF(QPointF(-10,-10),QPointF(10,10));
        ggdrawLine(corners[0],QPen(Qt::green,3));
        ggdrawLine(corners[1],QPen(Qt::green,3));
    }

    if (config->showGridScreenCenter)
    {
        auto r = view->rect();
        corners[0] = QLineF(r.topLeft(),r.bottomRight());
        corners[1] = QLineF(r.bottomLeft(),r.topRight());

        QPen pen(Qt::blue,3);
        pp->setPen(pen);
        ppdrawLine(corners[0]);
        ppdrawLine(corners[1]);
    }

    if (config->showGridLayerCenter)
    {
        QPointF center = view->getCurrentXform().getModelCenter();
        corners[0] = QLineF(QPointF(-10,10),QPointF(10,-10));
        corners[1] = QLineF(QPointF(-10,-10),QPointF(10,10));
        corners[0].translate(center);
        corners[1].translate(center);
        ggdrawLine(corners[0],QPen(Qt::red,3));
        ggdrawLine(corners[1],QPen(Qt::red,3));
    }

    if (genMap)
    {
        gridMap->verify();
    }
    delete gg;
}

void GridView::drawFromTilingFlood()
{
    qDebug() << "Grid::drawFromTilingFlood()";

    auto r = QRectF(-10,10,20,20);
    QPen pen(QColor(config->gridColorTiling),config->gridTilingWidth);

    QPointF center = getCenterModelUnits();
    r.moveCenter(center);
    //qDebug() << "grid model center" << center;

    auto maker = TilingMaker::getInstance();
    auto tiling = maker->getSelected();
    auto tdata = tiling->getData();

    const auto & placedTiles = maker->getInTiling();
    if (placedTiles.size())
    {
        auto placedTile = placedTiles.first();
        QTransform T    = placedTile->getTransform();
        center          = T.map(placedTile->getTile()->getCenter());
    }
    QLineF line1(center, tdata.getTrans1());
    QLineF line2(center, tdata.getTrans2());
    if (line1.isNull() || line2.isNull())
    {
        return;
    }
    qDebug() << "len1:" << line1.length() << "len2:" << line2.length();

    qreal angle1    = qDegreesToRadians(line1.angle());
    qreal angle2    = qDegreesToRadians(line2.angle());
    qreal gridAngle = qAbs(angle1 - angle2);
    qDebug() << "Line angles:" << angle1 << angle2 << "Grid angle" << gridAngle << "sin" << qAbs(qSin(gridAngle));

    QLineF l1 = Point::extendAsRay(line1,5.0);
    qreal offset = line2.length() * qAbs(qSin(gridAngle));
    l1 = Point::shiftParallel(l1,offset * -10.0);
    for (int i=0; i <20; i++)
    {
        l1 = Point::shiftParallel(l1,offset);
        ggdrawLine(l1,pen);
    }

    QLineF l2 = Point::extendAsRay(line2,5.0);
    qreal offset2 = line1.length() * qAbs(qSin(gridAngle));
    l2 = Point::shiftParallel(l2,offset2 * -10.0);
    for (int i=0; i <20; i++)
    {
        l2 = Point::shiftParallel(l2,offset2);
        ggdrawLine(l2,pen);
    }
}

void GridView::drawFromTilingRegion()
{
    qDebug() << "Grid::drawFromTilingRegion()";

    auto r = QRectF(-10,10,20,20);
    QPen pen(QColor(config->gridColorTiling),config->gridTilingWidth);

    QPointF center = getCenterModelUnits();
    r.moveCenter(center);
    //qDebug() << "grid model center" << center;

    auto maker = TilingMaker::getInstance();
    auto tiling = maker->getSelected();
    auto tdata = tiling->getData();

    const auto & placedTiles = maker->getInTiling();
    if (placedTiles.size())
    {
        auto placedTile = placedTiles.first();
        QTransform T    = placedTile->getTransform();
        center          = T.map(placedTile->getTile()->getCenter());
    }
    QLineF lineR(center, tdata.getTrans1());
    QLineF lineG(center, tdata.getTrans2());
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
        lr = Point::shiftParallel(lineR,offsetR);
        lg = Point::shiftParallel(lineG,-offsetG );
    }
    else
    {
        lr = Point::shiftParallel(lineR,-offsetR);
        lg = Point::shiftParallel(lineG,offsetG );
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

     FillRegion flood(tiling,view->getFillData());
     const Placements & placements = flood.getPlacements(config->repeatMode);
     for (const auto & t : placements)
     {
        auto poly = t.map(p);
        ggdrawPoly(poly,pen);
     }

     static constexpr QColor normal_color = QColor(217,217,255,128);
     gg->fillPolygon(p,normal_color);
}

// this is relative to model(0,0)
void GridView::drawModelUnitsModelCentered()
{
    qreal step = config->gridModelSpacing;

    // this centers on scene
    QRectF modelRect(-10,-10,20,20);
    QPointF center = modelRect.center();

    int x_steps = qCeil(modelRect.width()  / step);
    int y_steps = qCeil(modelRect.height() / step);

    drawModelUnits(modelRect,modelRect,center,x_steps,y_steps,step);
}

void GridView::drawModelUnitsCanvasCentered()
{
    qreal step = config->gridModelSpacing;

    QRectF viewRect = view->rect();
    viewRect = screenToWorld(viewRect);
    QPointF center = viewRect.center();

    QRectF modelRect(-10,-10,20,20);
    int x_steps = qCeil(viewRect.width()  / step);
    int y_steps = qCeil(viewRect.height() / step);

    drawModelUnits(modelRect,viewRect,center,x_steps,y_steps,step);
}

void GridView::drawModelUnits(QRectF r, QRectF r1, QPointF center, int x_steps, int y_steps, qreal step)
{
    QPen pen(QColor(config->gridColorModel),config->gridModelWidth);
    pp->setPen(pen);

    ggdrawLine(r.topLeft(),r.topRight(),pen);
    ggdrawLine(r.topRight(),r.bottomRight(),pen);
    ggdrawLine(r.bottomRight(),r.bottomLeft(),pen);
    ggdrawLine(r.bottomLeft(),r.topLeft(),pen);

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
                ggdrawLine(l,pen);
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
                ggdrawLine(l,pen);
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
                ggdrawLine(line2,pen);

            QTransform t2;
            t2.translate(mid.x(),mid.y());
            t2.rotate(-angle);
            t2.translate(-mid.x(),-mid.y());

            QLineF line3 = t2.map(line);
            if (intersects(edges,line3))
                ggdrawLine(line3,pen);

            y += step;
        }
    }
}

void GridView::drawScreenUnitsCanvasCentered()
{
    qreal step     = config->gridScreenSpacing;
    if (step < 10.0)
    {
        qDebug() << "grid step too small" << step;
        return;
    }

    QRectF viewRect  = view->rect();
    QPointF center   = viewRect.center();

    int x_steps = qCeil(viewRect.width()  / step);
    int y_steps = qCeil(viewRect.height() / step);

    drawScreenUnits(viewRect, viewRect, center, x_steps, y_steps, step);
}

void GridView::drawScreenUnitsModelCentered()
{
    qreal step     = config->gridScreenSpacing;
    if (step < 10.0)
    {
        qDebug() << "grid step too small" << step;
        return;
    }

    QTransform t = getLayerTransform();
    QRectF modelRect(-10,-10,20,20);
    modelRect = t.mapRect(modelRect);
    QPointF center = modelRect.center();

    QRectF viewRect = view->rect();

    int x_steps = qCeil(viewRect.width()  / step);
    int y_steps = qCeil(viewRect.height() / step);

    drawScreenUnits(viewRect, modelRect, center, x_steps, y_steps, step);
}

void GridView::drawScreenUnits(QRectF r, QRectF r1, QPointF center,int x_steps, int y_steps, qreal step)
{
    QPen pen(QColor(config->gridColorScreen),config->gridScreenWidth);
    pp->setPen(pen);

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
                ppdrawLine(l);
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
                ppdrawLine(l);
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
                ppdrawLine(line2);

            QTransform t2;
            t2.translate(mid.x(),mid.y());
            t2.rotate(-angle);
            t2.translate(-mid.x(),-mid.y());

            QLineF line3 = t2.map(line);
            if (intersects(edges,line3))
                ppdrawLine(line3);

            y += step;
        }
    }
}

bool GridView::intersects(QVector<QLineF> &edges, QLineF & line)
{
    QPointF p1;
    for (const auto & edge : edges)
    {
        QPointF intersect;
        if (line.intersects(edge,&intersect) != QLineF::NoIntersection && Point::isOnLine(intersect,edge))
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
    if (!config->showGrid || !config->snapToGrid)
    {
        return false;
    }

    genMap = true;
    view->update();
    genMap = false;

    for (const VertexPtr & v : qAsConst(gridMap->getVertices()))
    {
        QPointF a = v->pt;
        QPointF b = a;
        if (config->gridUnits == GRID_UNITS_MODEL || config->gridUnits == GRID_UNITS_TILE)
        {
            b = worldToScreen(a);
        }
        if (Point::isNear(spt,b))
        {
            if (config->gridUnits == GRID_UNITS_MODEL)
            {
                foundGridPoint = a;
            }
            else
            {
                foundGridPoint = screenToWorld(a);
            }
            qDebug() << "nearGridPoint - FOUND";
            return true;
        }
    }
    return false;
}

void GridView::ggdrawLine(QLineF line, QPen pen)
{
    gg->drawLine(line,pen);
    if (genMap)
        gridMap->insertEdge(line);
}

void GridView::ggdrawLine(QPointF p1, QPointF p2, QPen  pen)
{
    gg->drawLine(p1, p2, pen);
    if (genMap)
        gridMap->insertEdge(p1,p2);
}

void GridView::ggdrawPoly(QPolygonF & poly, QPen  pen)
{
    gg->drawPolygon(poly,pen);
    if (genMap)
    {
        EdgePoly ep(poly);
        auto vec = ep.getLines();
        for (auto & line : qAsConst(vec))
        {
            gridMap->insertEdge(line);
        }
    }
}

void GridView::ppdrawLine(QLineF & line)
{
    pp->drawLine(line);
    if (genMap)
        gridMap->insertEdge(line);
}

void GridView::ppdrawLine(QPointF p1, QPointF p2)
{
    pp->drawLine(p1,p2);
    if (genMap)
        gridMap->insertEdge(p1,p2);
}

void GridView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
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

void GridView::slot_mouseTranslate(QPointF pt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateX(xf.getTranslateX() + pt.x());
        xf.setTranslateY(xf.getTranslateY() + pt.y());
        setCanvasXform(xf);
    }
}

void GridView::slot_wheel_scale(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setCanvasXform(xf);
    }
}

void GridView::slot_wheel_rotate(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setCanvasXform(xf);
    }
}

void GridView::slot_scale(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setCanvasXform(xf);
    }
}

void GridView::slot_rotate(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setCanvasXform(xf);
    }
}

void GridView:: slot_moveX(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        qDebug() << "move x" << getName();
        Xform xf = getCanvasXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setCanvasXform(xf);
    }
}

void GridView::slot_moveY(int amount)
{
    qDebug() << getName();

    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        qDebug() << "move y" << getName();
        Xform xf = getCanvasXform();
        xf.setTranslateY(xf.getTranslateY() + amount);
        setCanvasXform(xf);
    }
}

void GridView::slot_setCenter(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (view->getKbdMode(KBD_MODE_XFORM_GRID) || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))

    {
        setCenterScreenUnits(spt);
    }
}
