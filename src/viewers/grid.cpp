#include "viewers/grid.h"
#include "settings/configuration.h"
#include "viewers/viewcontrol.h"
#include "misc/geo_graphics.h"
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
    setZValue(-5);
}

void Grid::paint(QPainter * pp)
{
    draw(pp);
}

void Grid::createMap(QPainter *pp)
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

    if (config->gridUnits == GRID_UNITS_TILE)
    {
        drawFromTiling();
    }
    else if (config->gridUnits == GRID_UNITS_MODEL)
    {
        if (config->gridModelCenter)
        {
            drawModelUnitsCentered();
        }
        else
        {
            drawModelUnits();
        }
    }
    else
    {
        Q_ASSERT(config->gridUnits == GRID_UNITS_SCREEN);
        if (config->gridScreenCenter)
        {
            drawScreenUnitsCentered();
        }
        else
        {
            drawScreenUnits();
        }
    }
    delete gg;
}

void Grid::drawFromTiling()
{
    auto r = QRectF(-10,10,20,20);

    QPointF center = getCenterModelUnits();
    r.moveCenter(center);
    qDebug() << "grid model center" << center;

    auto maker = TilingMaker::getSharedInstance();
    auto tiling = maker->getSelected();
    auto tdata = tiling->getData();

    QPointF p1 = tdata.getTrans1();
    QPointF p2 = tdata.getTrans2();
    if (p1.isNull() || p2.isNull())
    {
        return;
    }

    // calc T1
    QLineF line1(QPointF(),p1);
    qreal angle1 = -line1.angle();
    qDebug() << "angle T1 =" << angle1;
    if  (angle1 > 180.0)
    {
        angle1 -= 180.0;
    }
    if (angle1 < -180.0)
    {
        angle1 += 180.0;
    }
    qDebug() << "angle T1 =" << angle1;

    qreal step1 = line1.length();
    Q_ASSERT(!Loose::zero(step1));

    // calc T2
    QLineF line2(QPointF(),p2);
    qreal angle2 = line2.angle();
    qDebug() << "angle T2 =" << angle2;
    if  (angle2 > 180.0)
    {
        angle2 -= 180.0;
    }
    if (angle2 < -180.0)
    {
        angle2 += 180.0;
    }
    qDebug() << "angle T2 =" << angle2;
    qreal step2 = line2.length();
    qDebug() << "step:" << step2;
    Q_ASSERT(!Loose::zero(step2));

    // T1
    qreal minmax = 20 * step1;
    QTransform t1;
    t1.rotate(angle1);
    QPen apen(Qt::red,2);
    qreal  y1 = center.y() - minmax;
    while (y1 < center.y() + minmax)
    {
        ggdrawLine(t1.map(QPointF(r.left(), y1)),t1.map(QPointF(r.right(), y1)),apen);
        y1 += step1;
    }

    // T2
    minmax = 20 * step2;
    QTransform t2;
    t2.rotate(-angle2);
    QPen bpen(Qt::green,2);
    qreal  y2 = center.y() - minmax;
    while (y2 < center.y() + minmax)
    {
        ggdrawLine(t2.map(QPointF(r.left(), y2)),t2.map(QPointF(r.right(), y2)),bpen);
        y2 += step2;
    }

    corners[0] = QLineF(QPointF(-10,10),QPointF(10,-10));
    corners[1] = QLineF(QPointF(-10,-10),QPointF(10,10));
    ggdrawLine(corners[0],QPen(Qt::green,3));
    ggdrawLine(corners[1],QPen(Qt::green,3));
}

// this is relative to model(0,0)
void Grid::drawModelUnits()
{
    auto r     = QRectF(-10,10,20,20);
    QPen apen(Qt::red,config->gridModelWidth);

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
            ggdrawLine(QPointF(j, -r.height()/2),QPointF(j, r.height()/2),apen);
            j += step;
        }
    }

    // horizontal
    if (type == GRID_ORTHOGONAL)
    {
        qreal i = min;
        while (i < max)
        {
            ggdrawLine(QPointF(-r.width()/2,i),QPointF(r.width()/2,i),apen);
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
            ggdrawLine(t.map(QPointF(-r.width()/2,j)),t.map(QPointF(r.width()/2,j)),apen);
            j += step;
        }

        QTransform t2;
        t2.rotate(-gridAngle);
        j = min;
        while (j < max)
        {
            ggdrawLine(t2.map(QPointF(-r.width()/2,j)),t2.map(QPointF(r.width()/2,j)),apen);
            j += step;
        }
    }

    corners[0] = QLineF(QPointF(-10,10),QPointF(10,-10));
    corners[1] = QLineF(QPointF(-10,-10),QPointF(10,10));
    ggdrawLine(corners[0],QPen(Qt::green,3));
    ggdrawLine(corners[1],QPen(Qt::green,3));
}

// this is relative to layer center
void Grid::drawModelUnitsCentered()
{
    QRectF r = view->rect();
    QPen apen(Qt::red,config->gridModelWidth);

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
            ggdrawLine(QPointF(x,r.top()),QPointF(x,r.bottom()),apen);
            x += step;
        }
    }

    if (type == GRID_ORTHOGONAL)
    {
        // horizontal line
        qreal  y = center.y() - minmax;
        while (y < center.y() + minmax)
        {
            ggdrawLine(QPointF(r.left(),  y),QPointF(r.right(), y),apen);
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
            ggdrawLine(t.map(QPointF(r.left(),  y)),t.map(QPointF(r.right(), y)),apen);
            y += step;
        }

        QTransform t2;
        t2.rotate(-gridAngle);
        y = center.y() - minmax;
        while (y < center.y() + minmax)
        {
            ggdrawLine(t2.map(QPointF(r.left(),  y)),t2.map(QPointF(r.right(), y)),apen);
            y += step;
        }
    }

    corners[0] = QLineF(QPointF(-10,10),QPointF(10,-10));
    corners[1] = QLineF(QPointF(-10,-10),QPointF(10,10));
    ggdrawLine(corners[0],QPen(Qt::green,3));
    ggdrawLine(corners[1],QPen(Qt::green,3));
}

void Grid::drawScreenUnits()
{
    auto r = view->rect();
    QPen apen(Qt::red,config->gridScreenWidth);
    pp->setPen(apen);

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

    corners[0] = QLineF(r.topLeft(),r.bottomRight());
    corners[1] = QLineF(r.bottomLeft(),r.topRight());

    apen = QPen(Qt::blue,3);
    pp->setPen(apen);
    ppdrawLine(corners[0]);
    ppdrawLine(corners[1]);
}

void Grid::drawScreenUnitsCentered()
{
    QRectF r = view->rect();
    QPen apen(Qt::red,config->gridScreenWidth);
    pp->setPen(apen);
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

    corners[0] = QLineF(r.topLeft(),r.bottomRight());
    corners[1] = QLineF(r.bottomLeft(),r.topRight());
    apen = QPen(Qt::blue,3);
    pp->setPen(apen);
    ppdrawLine(corners[0]);
    ppdrawLine(corners[1]);
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
