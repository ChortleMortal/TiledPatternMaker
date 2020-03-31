#include "scene.h"
#include "base/configuration.h"
#include "geometry/Transform.h"
#include "viewers/workspaceviewer.h"
#include "base/view.h"
#include "viewers/GeoGraphics.h"

Scene::Scene(View * view)
{
	viewer = WorkspaceViewer::getInstance();
    config = Configuration::getInstance();

    gridPen.setColor(QColor(Qt::red));

    paintBackground = true;

    connect(this, &Scene::sceneRectChanged, view, &View::slot_sceneRectChanged);
}

void Scene::drawBackground(QPainter *painter, const QRectF &rect)
{
    if (paintBackground)
    {
        QGraphicsScene::drawBackground(painter,rect);
    }
}

void Scene::drawForeground(QPainter *painter, const QRectF & r)
{
    QRectF rect = r;    // rect is modifiable;

    Configuration * config = Configuration::getInstance();
    if (!config->sceneGrid)
    {
        return;
    }

    gridPen.setWidth(config->gridWidth);
    painter->setPen(gridPen);

    // draw a grid
    switch (config->gridModel)
    {
    case GRID_SCREEN:
        if (config->gridCenter)
            drawGridSceneUnitsCentered(painter,rect);
        else
            drawGridSceneUnits(painter,rect);
        break;
    case GRID_MODEL:
        if (config->gridCenter)
            drawGridModelUnitsCentered(painter,rect);
        else
            drawGridModelUnits(painter,rect);
        break;
    }

    // draw X and center
    painter->drawLine(rect.topLeft(),   rect.bottomRight());
    painter->drawLine(rect.bottomLeft(),rect.topRight());
    QPointF center = rect.center();
    painter->drawEllipse(center,10,10);
}

void Scene::drawGridModelUnits(QPainter *painter, const QRectF &r)
{
    // this centers on scene
    qreal step   = config->gridStepModel;
    Layer * l    = viewer->getActiveLayers().first();
    QTransform T = l->getLayerTransform();
    GeoGraphics gg(painter,T);

    // horizontal
    for (qreal i = (-10.0 * step); i < (10 * step); i += step)
    {
        gg.drawLine(-r.width()/2, i, r.width()/2, i,gridPen);
    }

    // vertical
    for (qreal j = (-10.0 * step); j < (10 * step); j += step)
    {
        gg.drawLine(j, -r.height()/2, j, r.height()/2,gridPen);
    }
}

void Scene::drawGridModelUnitsCentered(QPainter *painter, QRectF &r)
{
    // this centers on layer center
    Layer * l       = viewer->getActiveLayers().first();
    QTransform T    = l->getLayerTransform();
    qreal step      = config->gridStepModel;
    qreal scale     = Transform::scalex(T);
    step *= scale;
    QPointF center  = l->getCenter();
    r.moveCenter(center);

    painter->setPen(gridPen);

    // horizontal
    for (qreal y = center.y() + (-10.0 * step); y < (center.y() + (10 * step)); y += step)
    {
        painter->drawLine( QPointF(r.topLeft().x(), y),  QPointF(r.topRight().x(), y));
    }

    // vertical
    for (qreal x = center.x() + (-10.0 * step); x <  (center.x() +(10 * step)); x += step)
    {
        painter->drawLine(QPointF(x, r.topLeft().y()),QPointF(x,r.bottomLeft().y()));
    }
}

void Scene::drawGridSceneUnits(QPainter *painter, const QRectF &r)
{
    QPointF center = r.center();
    qreal step = config->gridStepScreen;
    if (step < 10.0)
    {
        qDebug() << "grid step too small" << step;
        return;
    }

    // draw horizontal lines
    qreal y = center.y() - ((r.height()/2) * step);
    while (y < r.height())
    {
        painter->drawLine(QPointF(r.topLeft().x(),y),QPointF(r.topRight().x(),y));
        y += step;
    }

    // draw vertical lines
    qreal x = center.x() - ((r.width()/2) * step);
    while (x < r.width())
    {
        painter->drawLine(QPointF(x,r.topLeft().y()),QPointF(x,r.bottomLeft().y()));
        x += step;
    }
}

void Scene::drawGridSceneUnitsCentered(QPainter *painter, QRectF & r)
{
    Layer * l       = viewer->getActiveLayers().first();
    QPointF center = l->getCenter();
    r.moveCenter(center);

    qreal step = config->gridStepScreen;
    if (step < 10.0)
    {
        qDebug() << "grid step too small" << step;
        return;
    }

    // draw horizontal lines
    qreal y = center.y() - ((r.height()/2) * step);
    while (y < r.height())
    {
        painter->drawLine(QPointF(r.topLeft().x(),y),QPointF(r.topRight().x(),y));
        y += step;
    }

    // draw vertical lines
    qreal x = center.x() - ((r.width()/2) * step);
    while (x < r.width())
    {
        painter->drawLine(QPointF(x,r.topLeft().y()),QPointF(x,r.bottomLeft().y()));
        x += step;
    }
}
