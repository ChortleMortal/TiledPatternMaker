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
#include "geometry/Transform.h"
#include "panels/panel.h"
#include "viewers/workspaceviewer.h"
#include "makers/mapeditor.h"

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

View::View() : QGraphicsView()
{
    dragging = false;
    setMouseTracking(true);

    config  = Configuration::getInstance();
    canvas  = Canvas::getInstance();

    setFrameStyle(0);
    //setAlignment(Qt::AlignLeft | Qt::AlignTop);
    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QSettings s;
    move(s.value("viewPos").toPoint());

    WorkspaceViewer * vw = WorkspaceViewer::getInstance();
    connect(vw, &WorkspaceViewer::sig_title, this, &View::setWindowTitle);

    expectedResize = false;
    setScene(canvas);
    matchSizeToCanvas();

    connect(canvas, &Canvas::sceneRectChanged, this, &View::slot_sceneRectChanged);

    show();
}

View::~View()
{
    if (!config->screenIsSplit)
    {
        QSettings s;
        s.setValue("viewPos",pos());
    }
}

void View::slot_sceneRectChanged(const QRectF &rect)
{
    qDebug() << "Canvas rect:"  << rect;
    matchSizeToCanvas();
}

void View::matchSizeToCanvas()
{
    qDebug() << "View::matchSizeToCanvas:" << canvas->width() << canvas->height();
    QRectF crect = sceneRect();
    QRect  vrect = crect.toAlignedRect();
    expectedResize = true;
    resize(vrect.width(),vrect.height());
    centerOn(crect.width()/2,crect.height()/2);
}

void View::resizeEvent(QResizeEvent *event)
{
    qDebug() << "View::resizeEvent: was=" << size() << "becomes=" << event->size();

    QGraphicsView::resizeEvent(event);

    if (expectedResize)
    {
        expectedResize = false;
    }
    else
    {
        QRectF crect = sceneRect();
        QRect  vrect = crect.toAlignedRect();
        if (vrect.size() != size())
        {
            qDebug() << "Setting canvas size because canvas size=" << vrect.size() << "and now view size=" << size();
            QRectF scRect(0.0,0.0,size().width(),size().height());
            canvas->setSceneRect(scRect);
        }

        WorkspaceViewer * viewer = WorkspaceViewer::getInstance();
        QVector<Layer*>   layers = viewer->getActiveLayers();

        if (layers.size())
        {
            for (auto it= layers.begin(); it != layers.end(); it++)
            {
                Layer * layer = *it;
                layer->forceUpdateLayer();
            }
        }
        else if (config->viewerType == VIEW_MAP_EDITOR)
        {
            MapEditor * me = MapEditor::getInstance();
            me->forceUpdateLayer();
        }
    }
}


#ifdef DEBUG_PAINT

void View::paintEvent(QPaintEvent *event)
{
    qDebug() << "++++START PAINT";
    canvas->dumpGraphicsInfo();
    QList<QGraphicsItem*> qgl = items();
    Transform t1(transform());
    Transform t2(viewportTransform());
    qDebug() << "view: items=" << qgl.size() << "sceneRect" << sceneRect() << "transform" << t1.toString() << "view transform" << t2.toString();

    QGraphicsView::paintEvent(event);

    qDebug() << "++++END PAINT";
}
#endif

void View::keyPressEvent( QKeyEvent *k )
{
    switch (config->viewerType)
    {
    case VIEW_TILIING_MAKER:
    case VIEW_MAP_EDITOR:
        emit keyEvent(k);
        break;
    default:
        canvas->procKeyEvent(k);
        break;
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
    if (dragging)
        emit sig_mouseDragged(event->localPos());
    else
        emit sig_mouseMoved(event->localPos());
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << event->scenePos();
    emit sig_mouseReleased(event->localPos());
    dragging = false;
}
