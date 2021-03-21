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

#include "makers/map_editor/map_editor.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "base/configuration.h"
#include "base/mosaic.h"
#include "base/shortcuts.h"
#include "geometry/transform.h"
#include "geometry/map_cleanser.h"
#include "tapp/design_element.h"
#include "tapp/prototype.h"
#include "viewers/placed_designelement_view.h"
#include "style/style.h"
#include "viewers/view.h"
#include "panels/panel.h"

MapEditor *  MapEditor::mpThis = nullptr;     // once initialised the destructor is never called
MapEditorPtr MapEditor::spThis;

const bool debugMouse = false;

MapEditorPtr MapEditor::getSharedInstance()
{
    if (!mpThis)
    {
        spThis = make_shared<MapEditor>();
        mpThis = spThis.get();
    }
    return spThis;
}

MapEditor * MapEditor::getInstance()
{
    if (!mpThis)
    {
        spThis = make_shared<MapEditor>();
        mpThis = spThis.get();
    }
    return mpThis;
}

MapEditor::MapEditor() : MapEditorSelection(), stash(this)
{
    qDebug() << "MapEditor::MapEditor";

    config          = Configuration::getInstance();
    decorationMaker = DecorationMaker::getInstance();
    motifMaker      = MotifMaker::getInstance();
    tilingMaker     = TilingMaker::getInstance();
    view            = View::getInstance();
    cpanel          = ControlPanel::getInstance();

    connect(view, &View::sig_mouseDragged,  this, &MapEditor::slot_mouseDragged);
    connect(view, &View::sig_mouseReleased, this, &MapEditor::slot_mouseReleased);
    connect(view, &View::sig_mouseMoved,    this, &MapEditor::slot_mouseMoved);

    connect(cpanel, &ControlPanel::sig_view_synch, this, &MapEditor::slot_view_synch);

    map_mouse_mode  = MAP_MODE_NONE;
    mapType         = MAP_TYPE_UNDEFINED;
    newCircleRadius = 0.25;

    localMap  =  make_shared<Map>(QString("New local map"));
}

void MapEditor::setDesignElement(DesignElementPtr delpptr)
{
    mapType = MAP_TYPE_FIGURE;
    delp    = delpptr;
    figp    = delp->getFigure();
    feap    = delp->getFeature();

    map     = figp->getFigureMap();

    buildEditorDB();

    if (config->getViewerType() == VIEW_MAP_EDITOR)
    {
        forceRedraw();
    }
}

void  MapEditor::setPrototype(PrototypePtr proptr)
{
    mapType = MAP_TYPE_PROTO;
    prop    = proptr;

    if (config->getViewerType() == VIEW_MAP_EDITOR)
    {
        map = prop->getProtoMap();
        buildEditorDB();
        forceRedraw();
    }
    else
    {
        map     = prop->getProtoMap();
        buildEditorDB();
    }
}

void MapEditor::setStyle(StylePtr styptr)
{
    mapType = MAP_TYPE_STYLE;
    styp    = styptr;
    prop    = styp->getPrototype();

    map     = prop->getProtoMap();

    buildEditorDB();

    if (config->getViewerType() == VIEW_MAP_EDITOR)
    {
        forceRedraw();
    }
}

void MapEditor::setLocal()
{
    mapType   = MAP_TYPE_LOCAL;

    map       = localMap;

    buildEditorDB();

    if (config->getViewerType() == VIEW_MAP_EDITOR)
    {
        forceRedraw();
    }
}

void  MapEditor::setTiling(TilingPtr tiling)
{
    mapType      = MAP_TYPE_TILING;
    this->tiling = tiling;
    map          = createFromTiling();

    buildEditorDB();

    if (config->getViewerType() == VIEW_MAP_EDITOR)
    {
        forceRedraw();
    }
}

void MapEditor::setDCEL(WeakDCELPtr dcel)
{
    mapType = MAP_TYPE_DCEL;
    map.reset();
    this->dcel = dcel;

    buildEditorDB();

    if (config->getViewerType() == VIEW_MAP_EDITOR)
    {
        forceRedraw();
    }
}

void  MapEditor::reload()
{
    qDebug() << "MapEditor::reload";

    switch (config->mapEditorMode)
    {
    case MAP_MODE_FIGURE:
    {
        // this is set by the figure editor
        DesignElementPtr dep  = motifMaker->getSelectedDesignElement();
        if (dep)
            setDesignElement(dep);
        else
            unload();
        break;
    }
    case MAP_MODE_PROTO:
    {
        PrototypePtr proto = motifMaker->getSelectedPrototype();
        if (proto)
        {
            setPrototype(proto);
        }
        else
            unload();
        break;
    }
    case MAP_MODE_MOSAIC:
    {
        MosaicPtr mosaic = decorationMaker->getMosaic();
        if (!mosaic)
        {
            unload();
        }
        else
        {
            const StyleSet & sset = mosaic->getStyleSet();
            if (sset.size())
            {
                StylePtr sp  = sset.first();
                setStyle(sp);
            }
            else
            {
                unload();
            }
        }
        break;
    }

    case MAP_MODE_LOCAL:
        setLocal();
        break;

    case MAP_MODE_TILING:
    {
        TilingPtr tp = tilingMaker->getSelected();
        setTiling(tp);
    }
        break;

    case MAP_MODE_DCEL:
    {
        WeakDCELPtr wdp = config->dcel;
        setDCEL(wdp);
    }
        break;
    }

    forceRedraw();
}

void MapEditor::slot_view_synch(int id, int enb)
{
    if (enb && id == VIEW_MAP_EDITOR)
    {
        reload();
    }
}

void MapEditor::draw(QPainter *painter )
{
    if (mapType == MAP_TYPE_UNDEFINED)
    {
        return;
    }

    if (mapType == MAP_TYPE_FIGURE)
    {
        drawFeature(painter);
        drawBoundaries(painter);
    }

    if (mapType == MAP_TYPE_DCEL)
    {
        drawDCEL(painter);
    }

    drawConstructionLines(painter);
    drawConstructionCircles(painter);

    drawMap(painter);

    drawPoints(painter,points);

    drawCropMap(painter);

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

void MapEditor::setMouseMode(eMapMouseMode mode)
{
    map_mouse_mode = mode;
    forceRedraw();
}

MapPtr MapEditor::createFromTiling()
{
    MapPtr map = make_shared<Map>("tiling map");
    const QVector<PlacedFeaturePtr> & qlpf = tiling->getPlacedFeatures();
    for (auto pfp : qlpf)
    {
        EdgePoly poly = pfp->getPlacedEdgePoly();
        MapPtr emap = make_shared<Map>("feature",poly);
        map->mergeMap(emap);
    }

    return map;
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
                mouse_interaction = std::make_shared<MoveVertex>(this, sel->getVertex(), spt);
                return;
            case MAP_EDGE:
                mouse_interaction = std::make_shared<MoveEdge>(this, sel->getEdge(), spt);
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
    if (config->getViewerType() != VIEW_MAP_EDITOR)
        return;

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
        mouse_interaction = std::make_shared<DrawLine>(this,set,mousePos);
        break;
    }

    case MAP_MODE_CONSTRUCTION_LINES:
    {
        set = findSelectionsUsingDB(spt);
        mouse_interaction = std::make_shared<ConstructionLine>(this,set,mousePos);
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
                break;
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
        mouse_interaction = std::make_shared<ExtendLine>(this,set,mousePos);
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
            constructionCircles.push_back(std::make_shared<Circle>(center, newCircleRadius));
            saveStash();
            buildEditorDB();
            forceRedraw();
        }
        else if (btn == Qt::LeftButton)
        {
            MapSelectionPtr sel = findConstructionCircle(spt);
            if (sel)
            {
                CirclePtr c = sel->getCircle();
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
                        mouse_interaction = std::make_shared<MoveConstructionCircle>(this,c,mousePos);
                    }
                }
            }
        }
        break;

    case MAP_MODE_CREATE_CROP:
    {
        mouse_interaction = std::make_shared<CreateCrop>(this,mousePos);
        break;
    }
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
    if (config->getViewerType() != VIEW_MAP_EDITOR)
        return;

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
    if (config->getViewerType() != VIEW_MAP_EDITOR)
        return;

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
    if (config->getViewerType() != VIEW_MAP_EDITOR)
        return;

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

void  MapEditor::applyCrop()
{
    qDebug() << "apply crop";

    if (!cropRect.isEmpty())
    {
        map->crop(cropRect);
        buildEditorDB();
        cropRect = QRectF();    // reset
        forceRedraw();
    }
}

//////////////////////////////////////////////////////////////////
///
/// Keyboard events
///
//////////////////////////////////////////////////////////////////

bool MapEditor::procKeyEvent(QKeyEvent * k)
{
    if (config->getViewerType() != VIEW_MAP_EDITOR)
    {
        return false;
    }

    int key = k->key();

    switch (key)
    {
    // actions
    case 'F': flipLineExtension(); break;
    case 'M': emit view->sig_raiseMenu(); break;
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
    case Qt::Key_Escape: setMouseMode(MAP_MODE_NONE);  return false; // propagate
    case Qt::Key_F3:     setMouseMode(MAP_MODE_DRAW_LINE); break;
    case Qt::Key_F4:     setMouseMode(MAP_MODE_CONSTRUCTION_LINES); break;
    case Qt::Key_F5:     setMouseMode(MAP_MODE_DELETE); break;
    case Qt::Key_F6:     setMouseMode(MAP_MODE_SPLIT_LINE); break;
    case Qt::Key_F7:     setMouseMode(MAP_MODE_EXTEND_LINE); break;
    case Qt::Key_F8:     setMouseMode(MAP_MODE_CREATE_CROP); break;
    case Qt::Key_F9:     setMouseMode(MAP_MODE_CONSTRUCTION_CIRCLES); break;

    default: return false;
    }

    return true;
}
