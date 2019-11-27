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
#include "panels/panel.h"
#include "viewers/workspaceviewer.h"
#include "makers/mapeditor.h"
#include "makers/tilingmaker.h"

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

    setFrameStyle(0);

    QSettings s;
    move(s.value("viewPos").toPoint());


    expectedResize = false;
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
    config  = Configuration::getInstance();
    canvas  = Canvas::getInstance();

    WorkspaceViewer * vw = WorkspaceViewer::getInstance();

    connect(vw,   &WorkspaceViewer::sig_title, this,   &View::setWindowTitle, Qt::QueuedConnection);
    connect(this, &View::sig_procKeyEvent,     canvas, &Canvas::slot_procKeyEvent);
}

void View::slot_sceneRectChanged(const QRectF &rect)
{
    qDebug() << "Canvas rect:"  << rect;
    matchSizeToCanvas();
}

void View::matchSizeToCanvas()
{
    qDebug() << "View::matchSizeToCanvas:" << canvas->scene->width() << canvas->scene->height();
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

    if (config->viewerType == VIEW_TILIING_MAKER)
    {
        TilingMaker * td = TilingMaker::getInstance();
        td->viewRectChanged();

    }
    else if (config->viewerType == VIEW_MAP_EDITOR)
    {
        MapEditor * me = MapEditor::getInstance();
        me->viewRectChanged();
    }

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
        if (config->viewerType == VIEW_MAP_EDITOR)
        {
            MapEditor * me = MapEditor::getInstance();
            me->forceUpdateLayer();
        }
    }
    emit sig_resize();
}

#ifdef DEBUG_PAINT

void View::paintEvent(QPaintEvent *event)
{
    qDebug() << "++++START PAINT";
    canvas->dumpGraphicsInfo();
    QTransform t1 = transform();
    QTransform t2 = viewportTransform();
    qDebug() << "view: items=" << items().size() << "sceneRect" << sceneRect() << "transform" << t1 << "view transform" << t2;

    QGraphicsView::paintEvent(event);

    qDebug() << "++++END PAINT";
}
#endif

void View::keyPressEvent( QKeyEvent *k )
{
    emit sig_procKeyEvent(k);
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
