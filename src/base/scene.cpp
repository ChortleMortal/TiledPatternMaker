#include "scene.h"
#include "base/configuration.h"
#include "geometry/Transform.h"
#include "viewers/workspaceviewer.h"
#include "base/view.h"

Scene::Scene(View * view)
{
    viewer = WorkspaceViewer::getInstance();

    gridPen.setColor(QColor(Qt::red));
    gridPen.setWidth(1.0);

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
    Configuration * config = Configuration::getInstance();
    if (!config->sceneGrid) return;

    painter->setPen(gridPen);

    // draw a grid
    if (!config->fgdGridModel)
    {
        // using screen units
        int step   = config->fgdGridStepScreen;
        if (step < 10)
        {
            qDebug() << "grid step too small" << step;
            return;
        }

        int height = static_cast<int>(r.height());
        int width  = static_cast<int>(r.width());

        // draw horizontal lines
        int h=0;
        while (h < height)
        {
            painter->drawLine(QPoint(0,h),QPoint(width,h));
            h += step;
        }

        // draw vertical lines
        int v=0;
        while (v < width)
        {
            painter->drawLine(QPoint(v,0),QPoint(v,height));
            v += step;
        }
    }
    else
    {
        // using model units
        qreal stepm  = config->fgdGridStepModel;
        Layer * l    = viewer->getActiveLayers().first();
        QTransform T = l->getLayerTransform();
        qreal scale  = Transform::scalex(T);
        qreal step   = stepm * scale;
        if (step < 10.0)
        {
            qDebug() << "grid step too small" << step << stepm;
            return;
        }

        qreal width  = r.width();
        qreal height = r.height();

        // draw horizontal lines
        qreal h=0;
        while (h < height)
        {
            painter->drawLine(QPointF(0,h),QPointF(width,h));
            h += step;
        }

        // draw vertical lines
        int v=0;
        while (v < width)
        {
            painter->drawLine(QPointF(v,0),QPointF(v,height));
            v += step;
        }
    }
}
