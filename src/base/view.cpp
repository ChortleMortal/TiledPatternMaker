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
    if (!config->screenIsSplit)
    {
        QSettings s;
        s.setValue("viewPos",pos());
    }
}

void View::init()
{
    config   = Configuration::getInstance();
    canvas   = Canvas::getInstance();
    maped    = MapEditor::getInstance();
    tmaker   = TilingMaker::getInstance();
    wsViewer = WorkspaceViewer::getInstance();

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

#if 0
    QSize oldSize   = wsViewer->getViewSize(config->viewerType);
    QSize viewSize  = size();
    if (viewSize == oldSize)
    {
        //return;
    }

    qDebug() << "View::resizeEvent: was=" << event->oldSize() << "becomes=" << event->size();
    qDebug() << "View::             was=" << oldSize          << "becomes=" << viewSize;
#endif

    if (config->scaleSceneToView)
    {
        QSize viewSize  = size();
        wsViewer->setViewSize(config->viewerType,viewSize);  // only change view size if scaling - don't move this line
    }

    for (auto layer : layers)
    {
        layer->forceUpdateLayer();
    }

    if (config->viewerType == VIEW_MAP_EDITOR)
    {
        MapEditorPtr me = MapEditor::getInstance();
        me->forceUpdateLayer();
    }

    emit sig_reconstructBorder();
}

void View::keyPressEvent( QKeyEvent *k )
{
    if (tmaker->procKeyEvent(k))        // tiling maker
    {
        return;
    }
    else if (maped->procKeyEvent(k))    // map editor
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

void View::drawGridModelUnits(QPainter *painter, const QRectF &r)
{
    // this centers on scene
    QTransform T;
    qreal step   = config->gridStepModel;
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

void View::drawGridSceneUnits(QPainter *painter, const QRectF &r)
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

void View::drawGridSceneUnitsCentered(QPainter *painter, QRectF & r)
{
    QPointF center;
    if (layers.size())
    {
        LayerPtr l = layers.first();
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

void View::dump(bool force)
{
    qDebug() << "View Layers:" << numLayers();
#if 0
    for (auto layer : layers)
    {
        qDebug() << "Layer:" << layer->getName() << Utils::addr(layer);
    }
#endif
    if (force)
    {
        qDebug() << "Tilings:" << Tiling::refs << "Layers:" << Layer::refs  << "Styles:" << Style::refs << "Maps:" << Map::refs << "Protos:" << Prototype::refs << "DELs:" << DesignElement::refs  << "PDELs:" << PlacedDesignElement::refs2 << "Figures:" << Figure::refs << "Features:" << Feature::refs << "Edges:" << Edge::refs << "Vertices:"  << Vertex::refs;
    }
}
