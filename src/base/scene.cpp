#include "scene.h"
#include "base/configuration.h"
#include "geometry/Transform.h"
#include "viewers/workspaceviewer.h"
#include "base/view.h"
#include "viewers/GeoGraphics.h"

Scene::Scene()
{
	viewer = WorkspaceViewer::getInstance();
    config = Configuration::getInstance();

    gridPen.setColor(QColor(Qt::red));

    paintBackground(true);

    view = View::getInstance();
}

void Scene::setSceneRect(const QRectF & rect)
{
    QRectF old = sceneRect();
    if (rect != old)
    {
        QGraphicsScene::setSceneRect(rect);
    }
    view->matchViewSizeToScene(rect);
}

void Scene::setSceneRect(qreal x, qreal y, qreal w, qreal h)
{
    setSceneRect(QRectF(x,y,w,h));
}

void Scene::drawBackground(QPainter *painter, const QRectF &rect)
{
    if (_paintBackground)
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
    QTransform T;
    qreal step   = config->gridStepModel;
    if (viewer->hasLayers())
    {
        Layer * l = viewer->getActiveLayers().first();
        T = l->getLayerTransform();
    }
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
    QTransform T;
    QPointF center;
    if (viewer->hasLayers())
    {
        Layer * l   = viewer->getActiveLayers().first();
        T           = l->getLayerTransform();
        center      = l->getCenter();
    }
    qreal step      = config->gridStepModel;
    qreal scale     = Transform::scalex(T);
    step *= scale;
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
    QPointF center;
    if (viewer->hasLayers())
    {
        Layer * l  = viewer->getActiveLayers().first();
        center = l->getCenter();
    }
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
