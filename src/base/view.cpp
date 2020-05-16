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
#include "makers/map_editor/map_editor.h"
#include "makers/tiling_maker/tiling_maker.h"

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
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QSettings s;
    move(s.value("viewPos").toPoint());
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
}

void View::matchViewSizeToScene(const QRectF & sceneRect)
{
    qDebug() << "View::matchSizeToCanvas (old):" << size().width() << size().height();
    qDebug() << "View::matchSizeToCanvas (new):" << sceneRect.width() << sceneRect.height();
    QRect  vrect = sceneRect.toAlignedRect();
    resize(vrect.width(),vrect.height());
    centerOn(sceneRect.width()/2,sceneRect.height()/2);
}

bool View::scaleSceneSizeToView(const QSize &viewSize)
{
    if (!config->scaleToView)
    {
        return false;
    }

    QRectF scene_rectF = sceneRect();
    QRect  scene_rect  = scene_rectF.toAlignedRect();
    QSize  scene_size  = scene_rect.size();
    if (scene_size != viewSize)
    {
        qDebug() << "Setting scene size because scene size=" << scene_size<< "and now view size=" << viewSize;
        QRectF scRect(0.0,0.0,viewSize.width(),viewSize.height());
        wsViewer->setSceneSize(viewSize);
        scene()->setSceneRect(scRect);
        return true;
    }
    return false;
}


void View::setSceneRect(const QRectF & rect)
{
    Q_UNUSED(rect)
    qFatal("Do not call this");
}

void View::setSceneRect(qreal x, qreal y, qreal w, qreal h)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(w);
    Q_UNUSED(h);
    qFatal("Do not call this");
}

void View::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);

    QSize oldSize   = wsViewer->getViewSize(config->viewerType);
    QSize viewSize  = size();
    if (viewSize == oldSize)
    {
        return;
    }

    qDebug() << "View::resizeEvent: was=" << event->oldSize() << "becomes=" << event->size();
    qDebug() << "View::             was=" << oldSize          << "becomes=" << viewSize;
    wsViewer->setViewSize(config->viewerType,viewSize);

    if (scaleSceneSizeToView(viewSize))
    {
        QVector<Layer*> layers = wsViewer->getActiveLayers();
        for (auto layer : layers)
        {
            layer->forceUpdateLayer();
        }

        if (config->viewerType == VIEW_MAP_EDITOR)
        {
            MapEditor * me = MapEditor::getInstance();
            me->forceUpdateLayer();
        }

        emit sig_resize();
    }
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
