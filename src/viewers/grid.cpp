#include "viewers/grid.h"
#include "settings/configuration.h"
#include "viewers/viewcontrol.h"
#include "geometry/transform.h"
#include "misc/geo_graphics.h"
#include "geometry/map.h"
#include "geometry/vertex.h"
#include "geometry/point.h"

using std::make_shared;

typedef std::shared_ptr<class Vertex>       VertexPtr;

GridPtr Grid::spThis;

GridPtr Grid::getSharedInstance()
{
    if (!spThis)
    {
        PrototypePtr pp;
        spThis = make_shared<Grid>(pp);
    }
    return spThis;
}

Grid::Grid(PrototypePtr pp) : Thick(pp)
{
    config  = Configuration::getInstance();
    view    = ViewControl::getInstance();

    setColor(Qt::red);
    setZValue(-5);

    gridMap = make_shared<Map>("Grid");
    setMap(gridMap);

    create();
}

void Grid::draw(GeoGraphics * gg)
{
    switch (config->gridUnits)
    {
    case GRID_UNITS_MODEL:
        Thick::draw(gg);
        gg->drawLine(corners[0],QPen(Qt::green,3));
        gg->drawLine(corners[1],QPen(Qt::green,3));
        break;

    case GRID_UNITS_SCREEN:
        QPainter * painter = gg->getPainter();
        painter->save();
        painter->setPen(QPen(Qt::red,config->gridScreenWidth));
        gridMap->draw(painter);
        painter->restore();

        painter->setPen(QPen(Qt::blue,3));
        painter->drawLine(corners[0]);
        painter->drawLine(corners[1]);
        break;
    }
}

void Grid::create()
{
    gridMap->wipeout();

    if (!config->showGrid)
    {
        return;
    }

    QRectF rect;

    // draw a grid
    switch (config->gridUnits)
    {
    case GRID_UNITS_SCREEN:
        rect = view->rect();

        setLineWidth(config->gridScreenWidth);
        if (config->gridScreenCenter)
        {
            createGridSceneUnitsCentered(rect);
        }
        else
        {
            createGridSceneUnits(rect);
        }

        corners[0] = QLineF(rect.topLeft(),rect.bottomRight());
        corners[1] = QLineF(rect.bottomLeft(),rect.topRight());
        break;

    case GRID_UNITS_MODEL:;
        rect = QRectF(-10,10,20,20);
        qreal lwidth = screenToWorld(qreal(config->gridModelWidth)) / 2.0; // for some reason Thick multiplies by 2
        setLineWidth(lwidth);

        if (config->gridModelCenter)
        {
            createGridModelUnitsCentered(rect);
        }
        else
        {
            createGridModelUnits(rect);
        }
        corners[0] = QLineF(QPointF(-10,10),QPointF(10,-10));
        corners[1] = QLineF(QPointF(-10,-10),QPointF(10,10));
        break;
    }

    if (config->getViewerType() == VIEW_TILING_MAKER)
    {
        // makes each intersection a vertex for alignment
        gridMap->cleanse(divideupIntersectingEdges);
    }
}


// this is relative to model(0,0)
void Grid::createGridModelUnits(QRectF r)
{
    // this centers on scene
    qreal step   = config->gridModelSpacing;
    qreal min    = -20.0 * step;
    qreal max    =  20.0 * step;

    // vertical
    if (config->gridType != GRID_RHOMBIC)
    {
        qreal j = min;
        while (j < max)
        {
            VertexPtr v1 = gridMap->insertVertex(QPointF(j, -r.height()/2));
            VertexPtr v2 = gridMap->insertVertex(QPointF(j,  r.height()/2));
            gridMap->insertEdge(v1,v2);
            j += step;
        }
    }

    // horizontal
    switch (config->gridType)
    {
    case GRID_ORTHOGONAL:
    {
        qreal i = min;
        while (i < max)
        {
            VertexPtr v1 = gridMap->insertVertex(QPointF(-r.width()/2,i));
            VertexPtr v2 = gridMap->insertVertex(QPointF( r.width()/2,i));
            gridMap->insertEdge(v1,v2);
            i += step;
        }
    }
        break;

    case GRID_ISOMETRIC:
    {
        QTransform t;
        t.rotate(30);
        qreal j = min;
        while (j < max)
        {
            VertexPtr v1 = gridMap->insertVertex(t.map(QPointF(-r.width()/2,j)));
            VertexPtr v2 = gridMap->insertVertex(t.map(QPointF( r.width()/2,j)));
            gridMap->insertEdge(v1,v2);
            j += step;
        }

        QTransform t2;
        t2.rotate(-30);
        j = min;
        while (j < max)
        {
            VertexPtr v1 = gridMap->insertVertex(t2.map(QPointF(-r.width()/2,j)));
            VertexPtr v2 = gridMap->insertVertex(t2.map(QPointF( r.width()/2,j)));
            gridMap->insertEdge(v1,v2);
            j += step;
        }
    }
        break;

    case GRID_RHOMBIC:
    {
        qreal angle = config->gridAngle;

        QTransform t;
        t.rotate(angle);
        qreal j = min;
        while (j < max)
        {
            VertexPtr v1 = gridMap->insertVertex(t.map(QPointF(-r.width()/2,j)));
            VertexPtr v2 = gridMap->insertVertex(t.map(QPointF( r.width()/2,j)));
            gridMap->insertEdge(v1,v2);
            j += step;
        }

        QTransform t2;
        t2.rotate(-angle);
        j = min;
        while (j < max)
        {
            VertexPtr v1 = gridMap->insertVertex(t2.map(QPointF(-r.width()/2,j)));
            VertexPtr v2 = gridMap->insertVertex(t2.map(QPointF( r.width()/2,j)));
            gridMap->insertEdge(v1,v2);
            j += step;
        }
    }
        break;
    }
}

// this is relative to layer center
void Grid::createGridModelUnitsCentered(QRectF r)
{
    // this centers on layer center
    QPointF center  = getCenterModelUnits();
    r.moveCenter(center);
    qDebug() << "grid model center" << center;

    qreal step    = config->gridModelSpacing;
    qreal minmax  = 20.0 * step;

    if (config->gridType != GRID_RHOMBIC)
    {
        // vertical
        qreal  x = center.x() - minmax;
        while (x < center.x() + minmax)
        {
            VertexPtr v1 = gridMap->insertVertex(QPointF(x,r.top()));
            VertexPtr v2 = gridMap->insertVertex(QPointF(x,r.bottom()));
            gridMap->insertEdge(v1,v2);
            x += step;
        }
    }

    // horizontal
    switch (config->gridType)
    {
    case GRID_ORTHOGONAL:
    {
        qreal  y = center.y() - minmax;
        while (y < center.y() + minmax)
        {
            VertexPtr v1 = gridMap->insertVertex(QPointF(r.left(),  y));
            VertexPtr v2 = gridMap->insertVertex(QPointF(r.right(), y));
            gridMap->insertEdge(v1,v2);
            y += step;
        }
    }
        break;

    case GRID_ISOMETRIC:
    {
        QTransform t;
        t.rotate(30);
        qreal  y = center.y() - minmax;
        while (y < center.y() + minmax)
        {
            VertexPtr v1 = gridMap->insertVertex(t.map(QPointF(r.left(), y)));
            VertexPtr v2 = gridMap->insertVertex(t.map(QPointF(r.right(),y)));
            gridMap->insertEdge(v1,v2);
            y += step;
        }

        QTransform t2;
        t2.rotate(-30);
        y = center.y() - minmax;
        while (y < center.y() + minmax)
        {
            VertexPtr v1 = gridMap->insertVertex(t2.map(QPointF(r.left(), y)));
            VertexPtr v2 = gridMap->insertVertex(t2.map(QPointF(r.right(),y)));
            gridMap->insertEdge(v1,v2);
            y += step;
        }
    }
        break;

    case GRID_RHOMBIC:
    {
        qreal angle = config->gridAngle;

        QTransform t;
        t.rotate(angle);
        qreal  y = center.y() - minmax;
        while (y < center.y() + minmax)
        {
            VertexPtr v1 = gridMap->insertVertex(t.map(QPointF(r.left(), y)));
            VertexPtr v2 = gridMap->insertVertex(t.map(QPointF(r.right(),y)));
            gridMap->insertEdge(v1,v2);
            y += step;
        }

        QTransform t2;
        t2.rotate(-angle);
        y = center.y() - minmax;
        while (y < center.y() + minmax)
        {
            VertexPtr v1 = gridMap->insertVertex(t2.map(QPointF(r.left(), y)));
            VertexPtr v2 = gridMap->insertVertex(t2.map(QPointF(r.right(),y)));
            gridMap->insertEdge(v1,v2);
            y += step;
        }
    }
        break;
    }
}

void Grid::createGridSceneUnits(const QRectF r)
{
    QPointF center = r.center();
    qreal step = config->gridScreenSpacing;
    if (step < 10.0)
    {
        qDebug() << "grid step too small" << step;
        return;
    }

    if (config->gridType != GRID_RHOMBIC)
    {
        // draw vertical lines
        qreal x = center.x() - ((r.width()/2) * step);
        while (x < r.width())
        {
            VertexPtr v1 = gridMap->insertVertex(QPointF(x,r.top()));

            VertexPtr v2 = gridMap->insertVertex(QPointF(x,r.bottom()));
            gridMap->insertEdge(v1,v2);
            x += step;
        }
    }

    // draw horizontal lines
    switch (config->gridType)
    {
    case GRID_ORTHOGONAL:
    {
        qreal y = center.y() - ((r.height()/2) * step);
        while (y < r.height())
        {
            VertexPtr v1 = gridMap->insertVertex(QPointF(r.left(),y));
            VertexPtr v2 = gridMap->insertVertex(QPointF(r.right(),y));
            gridMap->insertEdge(v1,v2);
            y += step;
        }
    }
        break;

    case GRID_ISOMETRIC:
    {
        qreal y = center.y() - r.height();
        while (y < (center.y() + r.height()))
        {
            QLineF line(QPointF(r.left(),y),QPointF(r.right(),y));
            QPointF mid = line.center();

            QTransform t;
            t.translate(mid.x(),mid.y());

            t.rotate(30);
            t.translate(-mid.x(),-mid.y());

            QLineF line2 = t.map(line);
            VertexPtr v1 = gridMap->insertVertex(line2.p1());
            VertexPtr v2 = gridMap->insertVertex(line2.p2());
            gridMap->insertEdge(v1,v2);

            QTransform t2;
            t2.translate(mid.x(),mid.y());
            t2.rotate(-30);
            t2.translate(-mid.x(),-mid.y());

            QLineF line3 = t2.map(line);
            v1 = gridMap->insertVertex(line3.p1());
            v2 = gridMap->insertVertex(line3.p2());
            gridMap->insertEdge(v1,v2);

            y += step;
        }
    }
        break;

    case GRID_RHOMBIC:
    {
        qreal angle = config->gridAngle;

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
            VertexPtr v1 = gridMap->insertVertex(line2.p1());
            VertexPtr v2 = gridMap->insertVertex(line2.p2());
            gridMap->insertEdge(v1,v2);

            QTransform t2;
            t2.translate(mid.x(),mid.y());
            t2.rotate(-angle);
            t2.translate(-mid.x(),-mid.y());

            QLineF line3 = t2.map(line);
            v1 = gridMap->insertVertex(line3.p1());
            v2 = gridMap->insertVertex(line3.p2());
            gridMap->insertEdge(v1,v2);

            y += step;
        }
    }
        break;
    }
}

void Grid::createGridSceneUnitsCentered(QRectF r)
{
    QPointF center = getCenterScreenUnits();
    r.moveCenter(center);

    qreal step = config->gridScreenSpacing;
    if (step < 10.0)
    {
        qDebug() << "grid step too small" << step;
        return;
    }

    if (config->gridType != GRID_RHOMBIC)
    {
        // draw vertical lines
        qreal x = center.x() - ((r.width()/2) * step);
        while (x < r.width())
        {
            VertexPtr v1 = gridMap->insertVertex(QPointF(x,r.top()));
            VertexPtr v2 = gridMap->insertVertex(QPointF(x,r.bottom()));
            gridMap->insertEdge(v1,v2);
            x += step;
        }
    }

    switch (config->gridType)
    {
    case GRID_ORTHOGONAL:
    {
        // draw horizontal lines
        qreal y = center.y() - ((r.height()/2) * step);
        while (y < r.height())
        {
            VertexPtr v1 = gridMap->insertVertex(QPointF(r.left(),y));
            VertexPtr v2 = gridMap->insertVertex(QPointF(r.right(),y));
            gridMap->insertEdge(v1,v2);
            y += step;
        }
    }
        break;

    case GRID_ISOMETRIC:
    {
        qreal y = center.y() - r.height();
        while (y < (center.y() + r.height()))
        {
            QLineF line(QPointF(r.left(),y),QPointF(r.right(),y));
            QPointF mid = line.center();

            QTransform t;
            t.translate(mid.x(),mid.y());
            t.rotate(30);
            t.translate(-mid.x(),-mid.y());

            QLineF line2 = t.map(line);
            VertexPtr v1 = gridMap->insertVertex(line2.p1());
            VertexPtr v2 = gridMap->insertVertex(line2.p2());
            gridMap->insertEdge(v1,v2);

            QTransform t2;
            t2.translate(mid.x(),mid.y());
            t2.rotate(-30);
            t2.translate(-mid.x(),-mid.y());

            QLineF line3 = t2.map(line);
            v1 = gridMap->insertVertex(line3.p1());
            v2 = gridMap->insertVertex(line3.p2());
            gridMap->insertEdge(v1,v2);

            y += step;
        }
    }
        break;

    case GRID_RHOMBIC:
    {
        qreal angle = config->gridAngle;

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
            VertexPtr v1 = gridMap->insertVertex(line2.p1());
            VertexPtr v2 = gridMap->insertVertex(line2.p2());
            gridMap->insertEdge(v1,v2);

            QTransform t2;
            t2.translate(mid.x(),mid.y());
            t2.rotate(-angle);
            t2.translate(-mid.x(),-mid.y());

            QLineF line3 = t2.map(line);
            v1 = gridMap->insertVertex(line3.p1());
            v2 = gridMap->insertVertex(line3.p2());
            gridMap->insertEdge(v1,v2);

            y += step;
        }
    }
        break;
    }
}

bool Grid::nearGridPoint(QPointF spt, QPointF & foundGridPoint)
{
    if (!config->showGrid || !config->snapToGrid)
    {
        return false;
    }
    for (const VertexPtr & v : qAsConst(gridMap->getVertices()))
    {
        QPointF a = v->pt;
        QPointF b = a;
        if (config->gridUnits == GRID_UNITS_MODEL)
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
            return true;
        }
    }
    return false;
}
