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

#include "makers/mapeditor.h"
#include "base/configuration.h"
#include "base/canvas.h"
#include "base/shortcuts.h"
#include "tapp/DesignElement.h"
#include "tapp/Prototype.h"
#include "viewers/placeddesignelementview.h"
#include "viewers/workspaceviewer.h"
#include "style/Style.h"

MapEditor * MapEditor::mpThis = nullptr;

const bool debugMouse = false;

MapEditor * MapEditor::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new MapEditor;
    }
    return mpThis;
}

MapEditor::MapEditor() : MapEditorSelection(), stash(this)
{
    qDebug() << "MapEditor::MapEditor";

    canvas = Canvas::getInstance();
    config = Configuration::getInstance();
    view   = View::getInstance();

    connect(view, &View::sig_mousePressed,  this, &MapEditor::slot_mousePressed);
    connect(view, &View::sig_mouseDragged,  this, &MapEditor::slot_mouseDragged);
    connect(view, &View::sig_mouseReleased, this, &MapEditor::slot_mouseReleased);
    connect(view, &View::sig_mouseMoved,    this, &MapEditor::slot_mouseMoved);
    connect(view, &View::sig_procKeyEvent,  this, &MapEditor::slot_procKeyEvent);

    map_mouse_mode  = MAP_MODE_NONE;
    inputMode       = ME_INPUT_UNDEFINED;
    newCircleRadius = 0.25;
}

MapEditor::~MapEditor()
{
}

void MapEditor::setDesignElement(DesignElementPtr delpptr)
{
    inputMode = ME_INPUT_FIGURE;
    delp = delpptr;

    figp = delp->getFigure();
    feap = delp->getFeature();
    map  = figp->getFigureMap();

    buildEditorDB();

    if (config->viewerType == VIEW_MAP_EDITOR)
    {
        forceRedraw();
    }
}

void  MapEditor::setPrototype(PrototypePtr proptr)
{
    inputMode = ME_INPUT_PROTO;
    prop = proptr;

    map = prop->getProtoMap();

    buildEditorDB();
    if (config->viewerType == VIEW_MAP_EDITOR)
    {
        forceRedraw();
    }
}

void MapEditor::setStyle(StylePtr styptr)
{
    inputMode = ME_INPUT_STYLE;
    styp = styptr;

    prop = styp->getPrototype();
    map = prop->getProtoMap();

    buildEditorDB();
    if (config->viewerType == VIEW_MAP_EDITOR)
    {
        forceRedraw();
    }
}

void MapEditor::draw(QPainter *painter )
{
    if (inputMode == ME_INPUT_UNDEFINED)
    {
        return;
    }

    if (inputMode == ME_INPUT_FIGURE)
    {
        drawFeature(painter);
        drawBoundaries(painter);
    }

    drawConstructionLines(painter);
    drawConstructionCircles(painter);

    drawMap(painter);

    drawPoints(painter,points);

    for (auto it = currentSelections.begin(); it != currentSelections.end(); it++)
    {
        auto sel = *it;
        eMapSelection type = sel->getType();
        if (type == MAP_EDGE && !hideMap)
        {
            QLineF l1 = sel->getLine();
            painter->setPen(QPen(Qt::red,selectionWidth));
            painter->drawLine(viewT.map(l1));
        }
        else if (type == MAP_LINE  && !hideConstructionLines)
        {
            QLineF l1 = sel->getLine();
            painter->setPen(QPen(Qt::red,selectionWidth));
            painter->drawLine(viewT.map(l1));
        }
        else if ((type == MAP_VERTEX || type == MAP_POINT) && !hidePoints)
        {
            qreal radius = 8.0;
            painter->setPen(QPen(Qt::blue,selectionWidth));
            painter->setBrush(Qt::red);
            painter->drawEllipse(viewT.map(sel->getPoint()), radius, radius);
        }
        else if (type == MAP_CIRCLE)
        {
            CirclePtr c = sel->getCircle();
            if (!sel->isConstructionLine() || (sel->isConstructionLine() && !hideConstructionLines))
            {
                qreal radius   = c->radius;
                QPointF center = c->centre;
                radius = Transform::scalex(viewT) * radius;
                painter->setPen(QPen(Qt::red,selectionWidth));
                painter->setBrush(QBrush());
                painter->drawEllipse(viewT.map(center), radius, radius);

                if (sel->hasCircleIntersect())
                {
                    QPointF pt = sel->getPoint();
                    radius = 8.0;
                    painter->setPen(QPen(Qt::red,selectionWidth));
                    painter->setBrush(QBrush());
                    painter->drawEllipse(viewT.map(pt), radius, radius);
                }
            }
        }
    }

    if (mouse_interaction)
    {
        mouse_interaction->draw(painter);
    }
}

void MapEditor::viewRectChanged()
{
    QRect rect = view->rect();
    QPointF pt = view->sceneRect().center();
    setRotateCenter(pt);

    qDebug() << "Map editor rect=" << rect;
    int height = rect.height();
    int width  = rect.width();
    SizeAndBounds sab = WorkspaceViewer::viewDimensions[VIEW_MAP_EDITOR];
    sab.viewSize = QSize(width,height);
    QTransform t0 = WorkspaceViewer::calculateViewTransform(sab);
    //qDebug().noquote() << Transform::toInfoString(layerT) << "-" << Transform::toInfoString(t0);

    deltaTrans = Transform::trans(t0) - Transform::trans(baseT);
    qDebug() << "deltaTrans=" << deltaTrans;
}

void MapEditor::setMouseMode(eMapMouseMode mode)
{
    map_mouse_mode = mode;
    forceRedraw();
}

eMapMouseMode MapEditor::getMouseMode()
{
    return map_mouse_mode;
}

void MapEditor::startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton)
{
    SelectionSet set = findSelectionsUsingDB(spt);
    for (auto it = set.begin(); it != set.end(); it++)
    {
        auto sel = *it;
        switch (mouseButton)
        {
        case Qt::LeftButton:
            switch (sel->getType())
            {
            case MAP_VERTEX:
                mouse_interaction = make_shared<MoveVertex>(this, sel->getVertex(), spt);
                return;
            case MAP_EDGE:
                mouse_interaction = make_shared<MoveEdge>(this, sel->getEdge(), spt);
                return;
            default:
                break;
            }
            break;

        case Qt::MiddleButton:
        case Qt::RightButton:
        default:
            break;
        }
    }

    mouse_interaction.reset();
}


void MapEditor::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    if (config->viewerType != VIEW_MAP_EDITOR)
        return;

    spt -= deltaTrans;
    setMousePos(spt);

    if (debugMouse)
    {
        if (mouse_interaction)
            qDebug() << "press start:" << mousePos << screenToWorld(mousePos) << mouse_interaction->desc;
        else
            qDebug() << "press start:" << mousePos << screenToWorld(mousePos) << "no mouse interaction";
    }

    SelectionSet set;

    switch (map_mouse_mode)
    {
    case MAP_MODE_NONE:
        startMouseInteraction(mousePos,btn);
        break;

    case MAP_MODE_DRAW_LINE:
    {
        set = findSelectionsUsingDB(spt);
        mouse_interaction = make_shared<DrawLine>(this,set,mousePos);
        break;
    }

    case MAP_MODE_CONSTRUCTION_LINES:
    {
        set = findSelectionsUsingDB(spt);
        mouse_interaction = make_shared<ConstructionLine>(this,set,mousePos);
        break;
    }

    case MAP_MODE_DELETE:
        set = findSelectionsUsingDB(spt);
        // only delete one line
        for (auto it = set.begin(); it != set.end(); it++)
        {
            auto sel = *it;
            if (sel->getType() == MAP_EDGE)
            {
                map->removeEdge(sel->getEdge());
                map->verifyMap("delete edge");
                break;
            }
            else if (sel->getType() == MAP_LINE && sel->isConstructionLine())
            {
                QLineF line = sel->getLine();
                constructionLines.removeAll(line);
                stash.stash();
                break;
            }
            else if (sel->getType() == MAP_CIRCLE)
            {
                CirclePtr c = sel->getCircle();
                constructionCircles.removeOne(c);
            }
        }
        buildEditorDB();
        forceRedraw();
        setMouseMode(MAP_MODE_NONE);
        break;

    case MAP_MODE_SPLIT_LINE:
    {
        QVector<EdgePtr> qvep;
        set = findEdges(spt, qvep);
        for (auto it = set.begin(); it != set.end(); it++)
        {
            auto sel = *it;
            map->splitEdge(sel->getEdge());
            map->verifyMap("split edge");
        }
        buildEditorDB();
        forceRedraw();
        setMouseMode(MAP_MODE_NONE);
    }
        break;

    case MAP_MODE_EXTEND_LINE:
        set = findSelectionsUsingDB(spt);
        mouse_interaction = make_shared<ExtendLine>(this,set,mousePos);
        break;

    case MAP_MODE_CONSTRUCTION_CIRCLES:
        if (btn == Qt::RightButton)
        {
            // add circle
            SelectionSet endset    = findSelectionsUsingDB(spt);
            MapSelectionPtr endsel = findAPoint(endset);
            QPointF center;
            if (endsel)
            {
                center = QPointF(endsel->getPoint());
            }
            else
            {
                center = viewTinv.map(spt);
            }
            constructionCircles.push_back(make_shared<Circle>(center, newCircleRadius));
            saveStash();
            buildEditorDB();
            forceRedraw();
        }
        else if (btn == Qt::LeftButton)
        {
            MapSelectionPtr sel = findConstructionCircle(spt);
            if (sel)
            {
                CirclePtr c         = sel->getCircle();
                if (c)
                {
                    Qt::KeyboardModifiers kms =  QApplication::keyboardModifiers();
                    if (kms == Qt::SHIFT)
                    {
                        // resize circle
                        c->radius = newCircleRadius;
                        saveStash();
                        buildEditorDB();
                        forceRedraw();
                    }
                    else
                    {
                        // move circle
                        mouse_interaction = make_shared<MoveConstructionCircle>(this,c,mousePos);
                    }
                }
            }
        }
        break;
    }

    if (mouse_interaction)
    {
        qDebug().noquote() << "press end:"  << mouse_interaction->desc;
    }
    else
    {
        qDebug() << "press end: no mouse_interaction";
    }
}

void MapEditor::slot_mouseDragged(QPointF spt)
{
    if (config->viewerType != VIEW_MAP_EDITOR)
        return;

    spt -= deltaTrans;
    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << mousePos << screenToWorld(mousePos)  << sMapMouseMode[map_mouse_mode];

    currentSelections = findSelectionsUsingDB(spt);

    if (mouse_interaction)
    {
        mouse_interaction->updateDragging(mousePos);
    }

    forceRedraw();
}

void MapEditor::slot_mouseReleased(QPointF spt)
{
    if (config->viewerType != VIEW_MAP_EDITOR)
        return;

    spt -= deltaTrans;
    setMousePos(spt);

    if (debugMouse) qDebug() << "release" << mousePos << screenToWorld(mousePos);

    if (mouse_interaction)
    {
        mouse_interaction->endDragging(mousePos);
        mouse_interaction.reset();
    }
}

void MapEditor::slot_mouseMoved(QPointF spt)
{
    if (config->viewerType != VIEW_MAP_EDITOR)
        return;

    qDebug() << "mousemove: deltaTrans=" << deltaTrans;
    spt -= deltaTrans;
    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << mousePos;

    currentSelections = findSelectionsUsingDB(spt);

    forceRedraw();
}


void MapEditor::setMousePos(QPointF pt)
{
    Qt::KeyboardModifiers km = QApplication::keyboardModifiers();
    if (km & Qt::ControlModifier)
    {
        mousePos.setY(pt.y());
    }
    else if (km & Qt::ShiftModifier)
    {
        mousePos.setX(pt.x());
    }
    else
    {
        mousePos = pt;
    }
}


void MapEditor::flipLineExtension()
{
    if (mouse_interaction)
    {
        MapMouseAction * mma = mouse_interaction.get();
        ExtendLine * el = dynamic_cast<ExtendLine*>(mma);
        if (el)
        {
            el->flipDirection();
        }
    }
}


void MapEditor::updateStatus()
{
    QString s("Map Editor:: ");
    s += sMapMouseMode[map_mouse_mode];
    if (mouse_interaction)
    {
        s += "  ";
        s += mouse_interaction->desc;
    }
    view->setWindowTitle(s);
}


bool MapEditor::loadCurrentStash()
{
    bool rv = stash.destash();
    if (rv)
    {
        buildEditorDB();
        forceRedraw();
    }
    return rv;
}

bool MapEditor::loadNamedStash(QString file, bool animate)
{
    bool rv;
    if (animate)
    {
        rv = stash.animateReadStash(file);
    }
    else
    {
        rv = stash.readStash(file);
    }
    if (!rv) return false;

    buildEditorDB();
    forceRedraw();
    saveStash();

    return true;
}

bool MapEditor::saveStash()
{
    bool rv = stash.stash();
    return rv;
}

bool MapEditor::keepStash(QString name)
{
    bool rv = stash.keepStash(name);
    return rv;
}

bool MapEditor::initStashFrom(QString name)
{
    return stash.initStash(name);
}


//////////////////////////////////////////////////////////////////
///
/// Keyboard events
///
//////////////////////////////////////////////////////////////////

void MapEditor::slot_procKeyEvent(QKeyEvent * k)
{
    if (config->viewerType != VIEW_MAP_EDITOR)
        return;

    bool consumed = true;
    int key = k->key();
    switch (key)
    {
        // actions
        case 'F': flipLineExtension(); break;
        case 'M': emit canvas->sig_raiseMenu(); break;
        case 'Q': QApplication::quit(); break;
        case Qt::Key_F1:
            {
                QMessageBox  * box = new QMessageBox();
                box->setWindowTitle("Map Editor Shortcuts");
                box->setText(Shortcuts::getMapEditorShortcuts());
                box->setModal(false);
                box->show();
                break;
            }

        // modes
        case Qt::Key_Escape: setMouseMode(MAP_MODE_NONE); consumed = false; break;
        case Qt::Key_F3:     setMouseMode(MAP_MODE_DRAW_LINE); break;
        case Qt::Key_F4:     setMouseMode(MAP_MODE_CONSTRUCTION_LINES); break;
        case Qt::Key_F5:     setMouseMode(MAP_MODE_DELETE); break;
        case Qt::Key_F6:     setMouseMode(MAP_MODE_SPLIT_LINE); break;
        case Qt::Key_F7:     setMouseMode(MAP_MODE_EXTEND_LINE); break;
        case Qt::Key_F9:     setMouseMode(MAP_MODE_CONSTRUCTION_CIRCLES); break;
        default: consumed = false; break;
   }

    if (!consumed)
        canvas->procKeyEvent(k);
}

