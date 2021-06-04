#include "viewers/grid.h"
#include "base/configuration.h"
#include "viewers/view.h"
#include "geometry/transform.h"
#include "base/geo_graphics.h"

Grid::Grid() : Layer("Grid",LTYPE_GRID)
{
    config  = Configuration::getInstance();
    view    = View::getInstance();

    gridPen.setColor(QColor(Qt::red));
}

void Grid::paint(QPainter * painter)
{
    if (!config->showGrid)
    {
        return;
    }

    QRectF rect = view->rect();

    painter->save();

    // draw a grid
    if (config->gridUnits == GRID_UNITS_SCREEN)
    {
        gridPen.setWidth(config->gridScreenWidth);
        painter->setPen(gridPen);
        if (config->gridScreenCenter)
        {
            drawGridSceneUnitsCentered(painter,rect);
        }
        else
        {
            drawGridSceneUnits(painter,rect);
        }
    }
    else
    {
        Q_ASSERT(config->gridUnits == GRID_UNITS_MODEL);
        gridPen.setWidth(config->gridModelWidth);
        painter->setPen(gridPen);
        if (config->gridModelCenter)
        {
            drawGridModelUnitsCentered(painter,rect);
        }
        else
        {
            drawGridModelUnits(painter,rect);
        }
    }

    painter->restore();
    rect = view->rect();   // restores rect

#if 0
    // draw X and center
    painter->drawLine(rect.topLeft(),   rect.bottomRight());
    painter->drawLine(rect.bottomLeft(),rect.topRight());
#endif

    QPointF center = rect.center();
    painter->drawEllipse(center,10,10);
}


// this is relative to model(0,0)
void Grid::drawGridModelUnits(QPainter *painter, const QRectF &r)
{
    // this centers on scene
    qreal step   = config->gridModelSpacing;
    QTransform T = getLayerTransform();

    GeoGraphics gg(painter,T);

    if (config->gridType != GRID_RHOMBIC)
    {
        // vertical
        for (qreal j = (-20.0 * step); j < (20 * step); j += step)
        {
            gg.drawLine(j, -r.height()/2, j, r.height()/2,gridPen);
        }
    }

    if (config->gridType == GRID_ORTHOGONAL)
    {
        // horizontal
        for (qreal i = (-20.0 * step); i < (20 * step); i += step)
        {
            gg.drawLine(-r.width()/2, i, r.width()/2, i,gridPen);
        }
    }
    else if (config->gridType ==  GRID_ISOMETRIC)
    {
        QTransform t;
        t.rotate(30);
        gg.pushAndCompose(t);
        for (qreal i = (-20.0 * step); i < (20 * step); i += step)
        {
            gg.drawLine(-r.width()/2, i, r.width()/2, i,gridPen);
        }

        gg.pop();

        QTransform t2;
        t2.rotate(-30);
        gg.pushAndCompose(t2);

        for (qreal i = (-20.0 * step); i < (20 * step); i += step)
        {
            gg.drawLine(-r.width()/2, i, r.width()/2, i,gridPen);
        }
    }
    else
    {
        Q_ASSERT(config->gridType == GRID_RHOMBIC);

        qreal angle = config->gridAngle;

        QTransform t;
        t.rotate(angle);
        gg.pushAndCompose(t);
        for (qreal i = (-20.0 * step); i < (20 * step); i += step)
        {
            gg.drawLine(-r.width()/2, i, r.width()/2, i,gridPen);
        }

        gg.pop();

        QTransform t2;
        t2.rotate(-angle);
        gg.pushAndCompose(t2);

        for (qreal i = (-20.0 * step); i < (20 * step); i += step)
        {
            gg.drawLine(-r.width()/2, i, r.width()/2, i,gridPen);
        }
    }
}


// this is relative to layer center
void Grid::drawGridModelUnitsCentered(QPainter *painter, QRectF &r)
{
    // this centers on layer center
    QTransform T    = getLayerTransform();
    QPointF center  = getCenterModel();
    qreal step      = config->gridModelSpacing;

    r.moveCenter(center);

    GeoGraphics gg(painter,T);

    if (config->gridType != GRID_RHOMBIC)
    {
        // vertical
        for (qreal x = center.x() + (-20.0 * step); x <  (center.x() +(20 * step)); x += step)
        {
            gg.drawLine(QPointF(x, r.topLeft().y()),QPointF(x,r.bottomLeft().y()), gridPen);
        }
    }

    if (config->gridType == GRID_ORTHOGONAL)
    {
        // horizontal
        for (qreal y = center.y() + (-20.0 * step); y < (center.y() + (20 * step)); y += step)
        {
            gg.drawLine( QPointF(r.topLeft().x(), y),  QPointF(r.topRight().x(), y), gridPen);
        }
    }
    else if (config->gridType ==  GRID_ISOMETRIC)
    {
        QTransform t;
        t.rotate(30);
        gg.pushAndCompose(t);
        for (qreal y = center.y() + (-20.0 * step); y < (center.y() + (20 * step)); y += step)
        {
            gg.drawLine( QPointF(r.topLeft().x(), y),  QPointF(r.topRight().x(), y), gridPen);
        }
        gg.pop();

        QTransform t2;
        t2.rotate(-30);
        gg.pushAndCompose(t2);
        for (qreal y = center.y() + (-20.0 * step); y < (center.y() + (20 * step)); y += step)
        {
            gg.drawLine( QPointF(r.topLeft().x(), y),  QPointF(r.topRight().x(), y), gridPen);
        }
    }
    else
    {
        Q_ASSERT(config->gridType == GRID_RHOMBIC);

        qreal angle = config->gridAngle;

        QTransform t;
        t.rotate(angle);
        gg.pushAndCompose(t);
        for (qreal y = center.y() + (-20.0 * step); y < (center.y() + (20 * step)); y += step)
        {
            gg.drawLine( QPointF(r.topLeft().x(), y),  QPointF(r.topRight().x(), y), gridPen);
        }
        gg.pop();

        QTransform t2;
        t2.rotate(-angle);
        gg.pushAndCompose(t2);
        for (qreal y = center.y() + (-20.0 * step); y < (center.y() + (20 * step)); y += step)
        {
            gg.drawLine( QPointF(r.topLeft().x(), y),  QPointF(r.topRight().x(), y), gridPen);
        }
    }
}

void Grid::drawGridSceneUnits(QPainter *painter, const QRectF &r)
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
            painter->drawLine(QPointF(x,r.topLeft().y()),QPointF(x,r.bottomLeft().y()));
            x += step;
        }
    }

    if (config->gridType == GRID_ORTHOGONAL)
    {
        // draw horizontal lines
        qreal y = center.y() - ((r.height()/2) * step);
        while (y < r.height())
        {
            painter->drawLine(QPointF(r.topLeft().x(),y),QPointF(r.topRight().x(),y));
            y += step;
        }
    }
    else if (config->gridType ==  GRID_ISOMETRIC)
    {
        qreal y = center.y() - r.height();
        while (y < (center.y() + r.height()))
        {
            QLineF line(QPointF(r.topLeft().x(),y),QPointF(r.topRight().x(),y));
            QPointF mid = line.center();

            QTransform t;
            t.translate(mid.x(),mid.y());
            t.rotate(30);
            t.translate(-mid.x(),-mid.y());

            QLineF line2 = t.map(line);
            painter->drawLine(line2);

            QTransform t2;
            t2.translate(mid.x(),mid.y());
            t2.rotate(-30);
            t2.translate(-mid.x(),-mid.y());

            QLineF line3 = t2.map(line);
            painter->drawLine(line3);

            y += step;
        }
    }
    else
    {
        Q_ASSERT(config->gridType == GRID_RHOMBIC);

        qreal angle = config->gridAngle;

        qreal y = center.y() - r.height();
        while (y < (center.y() + r.height()))
        {
            QLineF line(QPointF(r.topLeft().x(),y),QPointF(r.topRight().x(),y));
            QPointF mid = line.center();

            QTransform t;
            t.translate(mid.x(),mid.y());
            t.rotate(angle);
            t.translate(-mid.x(),-mid.y());

            QLineF line2 = t.map(line);
            painter->drawLine(line2);

            QTransform t2;
            t2.translate(mid.x(),mid.y());
            t2.rotate(-angle);
            t2.translate(-mid.x(),-mid.y());

            QLineF line3 = t2.map(line);
            painter->drawLine(line3);

            y += step;
        }
    }
}

void Grid::drawGridSceneUnitsCentered(QPainter *painter, QRectF & r)
{
    QPointF center = getCenterScreen();
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
            painter->drawLine(QPointF(x,r.topLeft().y()),QPointF(x,r.bottomLeft().y()));
            x += step;
        }
    }

    if (config->gridType == GRID_ORTHOGONAL)
    {
        // draw horizontal lines
        qreal y = center.y() - ((r.height()/2) * step);
        while (y < r.height())
        {
            painter->drawLine(QPointF(r.topLeft().x(),y),QPointF(r.topRight().x(),y));
            y += step;
        }
    }
    else if (config->gridType ==  GRID_ISOMETRIC)
    {
        qreal y = center.y() - r.height();
        while (y < (center.y() + r.height()))
        {
            QLineF line(QPointF(r.topLeft().x(),y),QPointF(r.topRight().x(),y));
            QPointF mid = line.center();

            QTransform t;
            t.translate(mid.x(),mid.y());
            t.rotate(30);
            t.translate(-mid.x(),-mid.y());

            QLineF line2 = t.map(line);
            painter->drawLine(line2);

            QTransform t2;
            t2.translate(mid.x(),mid.y());
            t2.rotate(-30);
            t2.translate(-mid.x(),-mid.y());

            QLineF line3 = t2.map(line);
            painter->drawLine(line3);

            y += step;
        }
    }
    else
    {
        Q_ASSERT(config->gridType == GRID_RHOMBIC);

        qreal angle = config->gridAngle;

        qreal y = center.y() - r.height();
        while (y < (center.y() + r.height()))
        {
            QLineF line(QPointF(r.topLeft().x(),y),QPointF(r.topRight().x(),y));
            QPointF mid = line.center();

            QTransform t;
            t.translate(mid.x(),mid.y());
            t.rotate(angle);
            t.translate(-mid.x(),-mid.y());

            QLineF line2 = t.map(line);
            painter->drawLine(line2);

            QTransform t2;
            t2.translate(mid.x(),mid.y());
            t2.rotate(-angle);
            t2.translate(-mid.x(),-mid.y());

            QLineF line3 = t2.map(line);
            painter->drawLine(line3);

            y += step;
        }
    }
}
