/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/canvas.h"
#include "base/cycler.h"
#include "base/view.h"
#include "base/utilities.h"
#include "panels/panel.h"
#include "viewers/workspace_viewer.h"
#include "makers/map_editor/map_editor.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "style/style.h"

View * View::mpThis = nullptr;

View * View::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new View();
    }
    return mpThis;
}

void View::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

View::View()
{
    dragging = false;
    setMouseTracking(true);

    gridPen.setColor(QColor(Qt::red));

    QSettings s;
    move(s.value("viewPos").toPoint());

    QGridLayout * grid = new QGridLayout();
    setLayout(grid);
}

View::~View()
{
    if (!config->splitScreen)
    {
        QSettings s;
        s.setValue("viewPos",pos());
    }
}

void View::init()
{
    config      = Configuration::getInstance();
    canvas      = Canvas::getInstance();
    mapEditor   = MapEditor::getInstance();
    tilingMaker = TilingMaker::getInstance();
    wsViewer    = WorkspaceViewer::getInstance();

    connect(wsViewer, &WorkspaceViewer::sig_title, this, &View::setWindowTitle, Qt::QueuedConnection);

    //resize(QSize(1500,1000));
    show();
}

QSize View::sizeHint() const
{
    return QSize(1500,1000);    // default
}

void View::addLayer(LayerPtr layer)
{
    layers.push_back(layer);
}

void View::clearView()
{
    clearLayers();
    clearLayout();
}

QVector<LayerPtr> View::getActiveLayers()
{
    return layers;
}

void View::paintEvent(QPaintEvent *event)
{
    //qDebug() << "++++START VIEW PAINT - Scene: items=" << layers.size() << "viewRect" << rect();

    QWidget::paintEvent(event);

    QPainter painter(this);

    std::stable_sort(layers.begin(),layers.end(),Layer::sortByZlevel);  // tempting to move this to addLayer, but if zlevel changed would not be picked up

    for (auto layer : layers)
    {
        layer->paint(&painter);
    }

    QRectF r = rect();
    drawForeground(&painter,r);

    //qDebug() << "++++END PAINT";
}

void View::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!config->scaleToView)
    {
        return;
    }

    QSize oldSize   = wsViewer->getViewSize(config->viewerType);
    QSize newSize  = size();
    qDebug() << "View::resizeEvent: old" << oldSize << "new" << newSize;

    if (oldSize == newSize)
    {
        return;
    }

    wsViewer->setViewSize(config->viewerType,newSize);

    for (auto layer : layers)
    {
       // set scale based on widht only - height is not affecting scale
        Xform xf        = layer->getCanvasXform();

        qreal oldWidth  = oldSize.width();
        qreal newWidth  = newSize.width();

        // recenter scaled image
        qreal old_dx = xf.getTranslateX();
        qreal old_dy = xf.getTranslateY();
        qreal new_dx = (newWidth/oldWidth) * old_dx;
        xf.setTranslateX(new_dx);

        qreal oldHeight = oldSize.height();
        qreal newHeight = newSize.height();
        qreal new_dy    = (newHeight/oldHeight) * old_dy;
        xf.setTranslateY(new_dy);

        layer->setCanvasXform(xf);

        layer->forceUpdateLayer();
    }

    if (config->viewerType == VIEW_MAP_EDITOR)
    {
        MapEditor * me = MapEditor::getInstance();
        me->forceUpdateLayer();
    }

    emit sig_reconstructBorder();
}

void View::keyPressEvent( QKeyEvent *k )
{
    if (tilingMaker->procKeyEvent(k))        // tiling maker
    {
        return;
    }
    else if (mapEditor->procKeyEvent(k))    // map editor
    {
        return;
    }
    else
    {
        canvas->procKeyEvent(k);
    }
}

void View::mousePressEvent(QMouseEvent *event)
{
    emit sig_mousePressed(event->localPos(),event->button());
    dragging = true;
}


void View::mouseDoubleClickEvent(QMouseEvent * event)
{
    emit sig_mouseDoublePressed(event->localPos(),event->button());
    dragging = false;
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    QPointF spt  = event->localPos();
    if (dragging)
        emit sig_mouseDragged(spt);
    else
        emit sig_mouseMoved(spt);
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << event->scenePos();
    emit sig_mouseReleased(event->localPos());
    dragging = false;
}


void View::setBackgroundColor(QColor color)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, color);
    //setAutoFillBackground(true);
    setPalette(pal);
}

QColor View::getBackgroundColor()
{
    QPalette pal = palette();
    QColor c = pal.color(QPalette::Window);
    return c;
}

void View::drawForeground(QPainter *painter, const QRectF & r)
{
    QRectF rect = r;    // rect is modifiable;

    Configuration * config = Configuration::getInstance();
    if (!config->showGrid)
    {
        return;
    }

    // draw a grid
    if (config->gridType == GRID_SCREEN)
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
        Q_ASSERT(config->gridType == GRID_MODEL);
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

    // draw X and center
    painter->drawLine(rect.topLeft(),   rect.bottomRight());
    painter->drawLine(rect.bottomLeft(),rect.topRight());
    QPointF center = rect.center();
    painter->drawEllipse(center,10,10);
}

void View::drawGridModelUnits(QPainter *painter, const QRectF &r)
{
    // this centers on scene
    QTransform T;
    qreal step   = config->gridModelSpacing;
    if (layers.size())
    {
        LayerPtr l = layers.first();
        T = l->getLayerTransform();
    }
    GeoGraphics gg(painter,T);

    // horizontal
    for (qreal i = (-20.0 * step); i < (20 * step); i += step)
    {
        gg.drawLine(-r.width()/2, i, r.width()/2, i,gridPen);
    }

    // vertical
    for (qreal j = (-20.0 * step); j < (20 * step); j += step)
    {
        gg.drawLine(j, -r.height()/2, j, r.height()/2,gridPen);
    }
}

void View::drawGridModelUnitsCentered(QPainter *painter, QRectF &r)
{
    // this centers on layer center
    QTransform T;
    QPointF center;
    if (layers.size())
    {
        LayerPtr l  = layers.first();
        T           = l->getLayerTransform();
        center      = l->getCenter();
    }
    qreal step      = config->gridModelSpacing;
    qreal scale     = Transform::scalex(T);
    step *= scale;
    r.moveCenter(center);

    painter->setPen(gridPen);

    // horizontal
    for (qreal y = center.y() + (-20.0 * step); y < (center.y() + (20 * step)); y += step)
    {
        painter->drawLine( QPointF(r.topLeft().x(), y),  QPointF(r.topRight().x(), y));
    }

    // vertical
    for (qreal x = center.x() + (-20.0 * step); x <  (center.x() +(20 * step)); x += step)
    {
        painter->drawLine(QPointF(x, r.topLeft().y()),QPointF(x,r.bottomLeft().y()));
    }
}

void View::drawGridSceneUnits(QPainter *painter, const QRectF &r)
{
    QPointF center = r.center();
    qreal step = config->gridScreenSpacing;
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

void View::drawGridSceneUnitsCentered(QPainter *painter, QRectF & r)
{
    QPointF center;
    if (layers.size())
    {
        LayerPtr l = layers.first();
        center = l->getCenter();
    }
    r.moveCenter(center);

    qreal step = config->gridScreenSpacing;
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

void View::clearLayout()
{
    clearLayout(layout(),true);
}

void View::clearLayout(QLayout* layout, bool deleteWidgets)
{
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (deleteWidgets)
        {
            if (QWidget* widget = item->widget())
                widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout())
            clearLayout(childLayout, deleteWidgets);
        delete item;
    }
}

void View::dump(bool summary)
{
    qDebug() << "View: layers =" << numLayers();

    if (!summary)
    {
        for (auto layer : layers)
        {
            qDebug() << "Layer:" << layer->getName() << Utils::addr(layer.get());
        }
    }

    qDebug() << "Tilings:" << Tiling::refs << "Layers:" << Layer::refs  << "Styles:" << Style::refs << "Maps:" << Map::refs << "Protos:" << Prototype::refs << "DELs:" << DesignElement::refs  << "PDELs:" << PlacedDesignElement::refs2 << "Figures:" << Figure::refs << "Features:" << Feature::refs << "Edges:" << Edge::refs << "Vertices:"  << Vertex::refs;
}
