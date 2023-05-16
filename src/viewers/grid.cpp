#include "viewers/grid.h"
#include "settings/configuration.h"
#include "viewers/viewcontrol.h"
#include "misc/geo_graphics.h"
#include "geometry/fill_region.h"
#include "geometry/map.h"
#include "geometry/vertex.h"
#include "geometry/point.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "tile/tiling.h"

using std::make_shared;

typedef std::shared_ptr<class Vertex>       VertexPtr;

GridPtr Grid::spThis;

GridPtr Grid::getSharedInstance()
{
    if (!spThis)
    {
        spThis = make_shared<Grid>();
    }
    return spThis;
}

Grid::Grid() : LayerController("Grid")
{
    config  = Configuration::getInstance();
    view    = ViewControl::getInstance();
    genMap  = false;
    setZValue(config->gridZLevel);
}

void Grid::paint(QPainter * pp)
{
    draw(pp);
}

void Grid::draw(QPainter * pp)
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
            drawModelUnitsCentered();
        }
        else
        {
            drawModelUnits();
        }
        break;

    case GRID_UNITS_SCREEN:
        if (config->gridScreenCenter)
        {
            drawScreenUnitsCentered();
        }
        else
        {
            drawScreenUnits();
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

    if (config->showGridViewCenter)
    {
        auto r = view->rect();
        corners[0] = QLineF(r.topLeft(),r.bottomRight());
        corners[1] = QLineF(r.bottomLeft(),r.topRight());

        QPen pen(Qt::blue,3);
        pp->setPen(pen);
        ppdrawLine(corners[0]);
        ppdrawLine(corners[1]);
    }

    if (genMap)
    {
        gridMap->verify();
    }
    delete gg;
}

void Grid::drawFromTilingFlood()
{
    qDebug() << "Grid::drawFromTilingFlood()";

    auto r = QRectF(-10,10,20,20);
    QPen pen(QColor(config->gridColorTiling),config->gridTilingWidth);

    QPointF center = getCenterModelUnits();
    r.moveCenter(center);
    //qDebug() << "grid model center" << center;

    auto maker = TilingMaker::getSharedInstance();
    auto tiling = maker->getSelected();
    auto tdata = tiling->getData();

    QLineF line1 = maker->getVisT1();
    QLineF line2 = maker->getVisT2();
    if (line1.isNull() || line2.isNull())
    {
        QPointF p1 = tdata.getTrans1();
        QPointF p2 = tdata.getTrans2();
        if (p1.isNull() || p2.isNull())
        {
            return;
        }
        line1 = QLineF(QPointF(),p1);
        line2 = QLineF(QPointF(),p2);
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

    if (config->showGridTilingCenter)
    {
    }
}

void Grid::drawFromTilingRegion()
{
    qDebug() << "Grid::drawFromTilingRegion()";

    auto r = QRectF(-10,10,20,20);
    QPen pen(QColor(config->gridColorTiling),config->gridTilingWidth);

    QPointF center = getCenterModelUnits();
    r.moveCenter(center);
    //qDebug() << "grid model center" << center;

    auto maker = TilingMaker::getSharedInstance();
    auto tiling = maker->getSelected();
    auto tdata = tiling->getData();

    QLineF lineR = maker->getVisT1();
    QLineF lineG = maker->getVisT2();
    if (lineR.isNull() || lineG.isNull())
    {
        QPointF p1 = tdata.getTrans1();
        QPointF p2 = tdata.getTrans2();
        if (p1.isNull() || p2.isNull())
        {
            return;
        }
        lineR = QLineF(QPointF(),p1);
        lineG = QLineF(QPointF(),p2);
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
     for (auto t : placements)
     {
        auto poly = t.map(p);
        ggdrawPoly(poly,pen);
     }

     static constexpr QColor normal_color = QColor(217,217,255,128);
     gg->fillPolygon(p,normal_color);

    if (config->showGridTilingCenter)
    {
    }
}

// this is relative to model(0,0)
void Grid::drawModelUnits()
{
    auto r = QRectF(-10,10,20,20);
    QPen pen(QColor(config->gridColorModel),config->gridModelWidth);

    // this centers on scene
    eGridType type = config->gridType;
    qreal step     = config->gridModelSpacing;
    qreal min      = -20.0 * step;
    qreal max      =  20.0 * step;

    // vertical
    if (type == GRID_ORTHOGONAL || type == GRID_ISOMETRIC)
    {
        qreal j = min;
        while (j < max)
        {
            ggdrawLine(QPointF(j, -r.height()/2),QPointF(j, r.height()/2),pen);
            j += step;
        }
    }

    // horizontal
    if (type == GRID_ORTHOGONAL)
    {
        qreal i = min;
        while (i < max)
        {
            ggdrawLine(QPointF(-r.width()/2,i),QPointF(r.width()/2,i),pen);
            i += step;
        }
    }
    else if (type == GRID_ISOMETRIC || type == GRID_RHOMBIC)
    {
        qreal gridAngle = (type == GRID_ISOMETRIC) ? 30.0 : config->gridAngle;

        QTransform t;
        t.rotate(gridAngle);
        qreal j = min;
        while (j < max)
        {
            ggdrawLine(t.map(QPointF(-r.width()/2,j)),t.map(QPointF(r.width()/2,j)),pen);
            j += step;
        }

        QTransform t2;
        t2.rotate(-gridAngle);
        j = min;
        while (j < max)
        {
            ggdrawLine(t2.map(QPointF(-r.width()/2,j)),t2.map(QPointF(r.width()/2,j)),pen);
            j += step;
        }
    }
}

// this is relative to layer center
void Grid::drawModelUnitsCentered()
{
    QRectF r = view->rect();
    QPen pen(QColor(config->gridColorModel),config->gridModelWidth);

    // this centers on layer center
    eGridType type = config->gridType;
    QPointF center = getCenterModelUnits();
    r.moveCenter(center);
    qDebug() << "grid model center" << center;

    qreal step    = config->gridModelSpacing;
    qreal minmax  = 20.0 * step;

    if (type == GRID_ORTHOGONAL || type == GRID_ISOMETRIC)
    {
        // vertical line
        qreal  x = center.x() - minmax;
        while (x < center.x() + minmax)
        {
            ggdrawLine(QPointF(x,r.top()),QPointF(x,r.bottom()),pen);
            x += step;
        }
    }

    if (type == GRID_ORTHOGONAL)
    {
        // horizontal line
        qreal  y = center.y() - minmax;
        while (y < center.y() + minmax)
        {
            ggdrawLine(QPointF(r.left(),  y),QPointF(r.right(), y),pen);
            y += step;
        }
    }
    else if (type == GRID_ISOMETRIC || type == GRID_RHOMBIC)
    {
        qreal gridAngle = (type == GRID_ISOMETRIC) ? 30.0 : config->gridAngle;

        QTransform t;
        t.rotate(gridAngle);
        qreal  y = center.y() - minmax;
        while (y < center.y() + minmax)
        {
            ggdrawLine(t.map(QPointF(r.left(),  y)),t.map(QPointF(r.right(), y)),pen);
            y += step;
        }

        QTransform t2;
        t2.rotate(-gridAngle);
        y = center.y() - minmax;
        while (y < center.y() + minmax)
        {
            ggdrawLine(t2.map(QPointF(r.left(),  y)),t2.map(QPointF(r.right(), y)),pen);
            y += step;
        }
    }
}

void Grid::drawScreenUnits()
{
    auto r = view->rect();
    QPen pen(QColor(config->gridColorScreen),config->gridScreenWidth);
    pp->setPen(pen);

    eGridType type = config->gridType;
    QPointF center = r.center();
    qreal step     = config->gridScreenSpacing;
    if (step < 10.0)
    {
        qDebug() << "grid step too small" << step;
        return;
    }

    if (type == GRID_ORTHOGONAL || type == GRID_ISOMETRIC)
    {
        // draw vertical lines
        qreal x = center.x() - (r.width()/2);
        while (x < r.width())
        {
            ppdrawLine(QPointF(x,r.top()),QPointF(x,r.bottom()));
            x += step;
        }
    }

    // draw horizontal lines
    if (type == GRID_ORTHOGONAL)
    {
        qreal y = center.y() - (r.height()/2);
        while (y < r.height())
        {
            ppdrawLine(QPointF(r.left(),y),QPointF(r.right(),y));
            y += step;
        }
    }
    else if (type == GRID_ISOMETRIC || type == GRID_RHOMBIC)
    {
        qreal angle = (type == GRID_ISOMETRIC) ? 30.0 : config->gridAngle;
        qreal y = center.y() - r.height();
        while (y < (center.y() + r.height()))
        {
            QLineF line(QPointF(r.left(),y),QPointF(r.right(),y));
            QPointF mid = line.center();

            QTransform t;
            t.translate(mid.x(),mid.y());
            t.rotate(angle);
            t.translate(-mid.x(),-mid.y());

            QLineF line2 = t.map(line);
            ppdrawLine(line2.p1(),line2.p2());

            QTransform t2;
            t2.translate(mid.x(),mid.y());
            t2.rotate(-angle);
            t2.translate(-mid.x(),-mid.y());

            QLineF line3 = t2.map(line);
            ppdrawLine(line3.p1(),line3.p2());

            y += step;
        }
    }
}

void Grid::drawScreenUnitsCentered()
{
    QRectF r = view->rect();
    QPen pen(QColor(config->gridColorScreen),config->gridScreenWidth);
    pp->setPen(pen);
    eGridType type = config->gridType;

    QPointF center = getCenterScreenUnits();
    r.moveCenter(center);

    qreal step = config->gridScreenSpacing;
    if (step < 10.0)
    {
        qDebug() << "grid step too small" << step;
        return;
    }

    // draw vertical lines
    if (type == GRID_ORTHOGONAL || type == GRID_ISOMETRIC)
    {
        qreal x = center.x() - ((r.width()/2) * step);
        while (x < r.width())
        {
            ppdrawLine(QPointF(x,r.top()),QPointF(x,r.bottom()));
            x += step;
        }
    }

    if (type == GRID_ORTHOGONAL)
    {
        // draw horizontal lines
        qreal y = center.y() - ((r.height()/2) * step);
        while (y < r.height())
        {
            ppdrawLine(QPointF(r.left(),y),QPointF(r.right(),y));
            y += step;
        }
    }
    else if (type == GRID_ISOMETRIC || type == GRID_RHOMBIC)
    {
        qreal angle = (type == GRID_ISOMETRIC) ? 30.0 : config->gridAngle;
        qreal y = center.y() - r.height();
        while (y < (center.y() + r.height()))
        {
            QLineF line(QPointF(r.left(),y),QPointF(r.right(),y));
            QPointF mid = line.center();

            QTransform t;
            t.translate(mid.x(),mid.y());
            t.rotate(angle);
            t.translate(-mid.x(),-mid.y());

            QLineF line2 = t.map(line);
            ppdrawLine(line2.p1(),line2.p2());

            QTransform t2;
            t2.translate(mid.x(),mid.y());
            t2.rotate(-angle);
            t2.translate(-mid.x(),-mid.y());

            QLineF line3 = t2.map(line);
            ppdrawLine(line3.p1(),line3.p2());

            y += step;
        }
    }
}

bool Grid::nearGridPoint(QPointF spt, QPointF & foundGridPoint)
{
    if (!config->showGrid || !config->snapToGrid)
    {
        return false;
    }

    genMap = true;
    view->repaint();
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

void Grid::ggdrawLine(QLineF line, QPen pen)
{
    gg->drawLine(line,pen);
    if (genMap)
        gridMap->insertEdge(line);
}

void Grid::ggdrawLine(QPointF p1, QPointF p2, QPen  pen)
{
    gg->drawLine(p1, p2, pen);
    if (genMap)
        gridMap->insertEdge(p1,p2);
}

void Grid::ggdrawPoly(QPolygonF & poly, QPen  pen)
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

void Grid::ppdrawLine(QLineF line)
{
    pp->drawLine(line);
    if (genMap)
        gridMap->insertEdge(line);
}

void Grid::ppdrawLine(QPointF p1, QPointF p2)
{
    pp->drawLine(p1,p2);
    if (genMap)
        gridMap->insertEdge(p1,p2);
}

void Grid::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    Q_UNUSED(spt);
    Q_UNUSED(btn);
}

void Grid::slot_mouseDragged(QPointF spt)
{
    Q_UNUSED(spt);
}

void Grid::slot_mouseMoved(QPointF spt)
{
    Q_UNUSED(spt);
}

void Grid::slot_mouseReleased(QPointF spt)
{
    Q_UNUSED(spt);
}

void Grid::slot_mouseDoublePressed(QPointF spt)
{
    Q_UNUSED(spt);
}

const Xform  & Grid::getCanvasXform()
{
    return xf_canvas;
}

void Grid::setCanvasXform(const Xform & xf)
{
    xf_canvas = xf;
}
