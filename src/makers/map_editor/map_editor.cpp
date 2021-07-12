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
#include "base/border.h"
#include "base/mosaic.h"
#include "base/shortcuts.h"
#include "geometry/transform.h"
#include "panels/panel.h"
#include "geometry/crop.h"
#include "tapp/figure.h"
#include "viewers/view.h"
#include "tapp/prototype.h"
#include "geometry/map.h"
#include "tapp/design_element.h"
#include "style/style.h"

using std::make_shared;

MapEditorPtr MapEditor::spThis;

const bool debugMouse = false;

MapEditorPtr MapEditor::getSharedInstance()
{
    if (!spThis)
    {
        spThis = make_shared<MapEditor>();
    }
    return spThis;
}

MapEditor::MapEditor() : MapEditorSelection(), stash(this)
{
    qDebug() << "MapEditor::MapEditor";

    config          = Configuration::getInstance();
    decorationMaker = DecorationMaker::getInstance();
    motifMaker      = MotifMaker::getInstance();
    tilingMaker     = TilingMaker::getSharedInstance();
    view            = View::getInstance();
    cpanel          = ControlPanel::getInstance();

    connect(cpanel, &ControlPanel::sig_view_synch, this, &MapEditor::slot_view_synch);

    map_mouse_mode  = MAPED_MOUSE_NONE;
    newCircleRadius = 0.25;

    // there is always a local map but the local map may be empty
    localMap  =  make_shared<Map>(QString("Local map"));
}

void  MapEditor::reload()
{
    qDebug() << "MapEditor::reload";

    unload();

    switch (config->mapEditorMode)
    {
    case MAPED_MODE_FIGURE:
    {
        // this is set by the figure editor
        delp  = motifMaker->getSelectedDesignElement();
        if (delp)
        {
            figp    = delp->getFigure();
            feap    = delp->getFeature();
        }
        else
        {
            unload();
        }
        break;
    }
    case MAPED_MODE_PROTO:
    {
        prototype = motifMaker->getSelectedPrototype();
        if (prototype)
        {
            border = prototype->getBorder();
        }
        else
        {
            unload();
        }
        break;
    }
    case MAPED_MODE_MOSAIC:
    {
        MosaicPtr mosaic = decorationMaker->getMosaic();
        if (mosaic)
        {
            const StyleSet & sset = mosaic->getStyleSet();
            if (sset.size())
            {
                styp  = sset.first();
                prototype  = styp->getPrototype();
                border = prototype->getBorder();
            }
            else
            {
                qDebug() << "mosaic has no style set";
                unload();
            }
        }
        else
        {
            qDebug() << "no mosaic";
            unload();
        }
        break;
    }

    case MAPED_MODE_LOCAL:
        break;

    case MAPED_MODE_TILING:
        tiling = tilingMaker->getSelected();
        if (!tiling)
            qDebug() << "no tiling";
        break;

    case MAPED_MODE_FILLED_TILING:
        tiling = tilingMaker->getSelected();
        if (!tiling)
            qDebug() << "no tiling";
        break;

    case MAPED_MODE_DCEL:
        if (map)
        {
            dcel = map->getDCEL();
            if (!dcel)
                qDebug() << "no DCEL";
        }
        else
            qDebug() << "No map so no DCEL: either";
        break;

    case MAPED_MODE_NONE:
        break;
    }

    if (config->getViewerType() == VIEW_MAP_EDITOR)
    {
        forceRedraw();
    }
}

void MapEditor::slot_view_synch(int id, int enb)
{
    if (enb && id == VIEW_MAP_EDITOR)
    {
        reload();
    }
    else if (enb && id != VIEW_MAP_EDITOR)
    {
        mouse_interaction.reset();
    }
}

void MapEditor::draw(QPainter *painter )
{
    eMapEditorMode mode = config->mapEditorMode;
    switch (mode)
    {
    case MAPED_MODE_FIGURE:
        if (figp)
        {
            map = figp->getFigureMap();
            buildEditorDB();
            drawFeature(painter);
            drawBoundaries(painter);
        }
        break;

    case MAPED_MODE_PROTO:
        if (prototype)
        {
            map = prototype->getProtoMap();
            buildEditorDB();
        }
        break;

    case MAPED_MODE_MOSAIC:
        if (prototype)
        {
            map = prototype->getProtoMap();
            buildEditorDB();
        }
        break;

    case MAPED_MODE_LOCAL:
        map = localMap;
        buildEditorDB();
        break;

    case MAPED_MODE_TILING:
        map.reset();
        if (tiling)
        {
            map = tiling->createMap();
            buildEditorDB();
        }
        break;

    case MAPED_MODE_FILLED_TILING:
        map.reset();
        if (tiling)
        {
            map = tiling->createFilledMap();
            buildEditorDB();
        }
        break;

    case MAPED_MODE_DCEL:
        buildEditorDB();
        drawDCEL(painter);
        break;

    case MAPED_MODE_NONE:
        return;
    }

    drawConstructionLines(painter);
    drawConstructionCircles(painter);

    if (mode != MAPED_MODE_DCEL)
    {
        drawMap(painter);
    }

    drawCropMap(painter);

    drawPoints(painter,points);

    for (auto sel : currentSelections)
    {
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

void MapEditor::setMapedMouseMode(eMapMouseMode mode)
{
    map_mouse_mode = mode;

    mouse_interaction.reset();      // only needed by border edit

    switch (mode)
    {
    case MAPED_MOUSE_CREATE_CROP:
        cropRect = make_shared<Crop>();
        break;

    case MAPED_MOUSE_CREATE_BORDER:
    {
        bool finished = false;
        MosaicPtr mosaic = decorationMaker->getMosaic();
        if (mosaic)
        {
            BorderPtr bp = mosaic->getBorder();
            if (!bp)
            {
                bp = make_shared<InnerBorder>();
                mosaic->setBorder(bp);
            }
            InnerBorder * ib = dynamic_cast<InnerBorder*>(bp.get());
            if (ib)
            {
                cropRect = ib->getInnerBoundary();
                finished = true;
            }
        }
        if (!finished)
        {
            cropRect.reset();
        }
        mosaic->resetPrototypeMaps();
    }
        break;

    case MAPED_MOUSE_EDIT_BORDER:
    {
        MosaicPtr mosaic = decorationMaker->getMosaic();
        if (mosaic)
        {
            BorderPtr bp = mosaic->getBorder();
            if (!bp)
            {
                bp = make_shared<InnerBorder>();
                mosaic->setBorder(bp);
            }
            InnerBorder * ib = dynamic_cast<InnerBorder*>(bp.get());
            if (ib)
            {
                cropRect = ib->getInnerBoundary();
            }
            mosaic->resetPrototypeMaps();
        }
    }
        break;

    default:
        break;
    }

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
    if (cropRect)
    {
        if (cropRect->getState() == CROP_BORDER_PREPARED)
        {
            qDebug() << "apply crop to border";
            map->addCropBorder(cropRect->getRect());
            cropRect->setState(CROP_APPLIED);
            setMapedMouseMode(MAPED_MOUSE_NONE);
            buildEditorDB();
            forceRedraw();
        }
        else
        {
            qDebug() << "apply only supported on borders";
        }
    }
}

void  MapEditor::applyMask()
{
    qDebug() << "apply mask";

    if (cropRect)
    {
        switch(cropRect->getState())
        {
            case CROP_APPLIED:
            case CROP_DEFINED:
            case CROP_PREPARED:
            case CROP_BORDER_DEFINED:
            case CROP_BORDER_PREPARED:
                map->removeOutisde(cropRect->getRect());
                cropRect->setState(CROP_MASKED);
                buildEditorDB();
                break;

            default:
                break;
        }

        setMapedMouseMode(MAPED_MOUSE_NONE);
        forceRedraw();
    }
}

// called by CreateBorder destructor
void MapEditor::completeBorder()
{
    MosaicPtr mosaic = decorationMaker->getMosaic();
    if (mosaic)
    {
        mosaic->resetPrototypeMaps();
    }
    if (cropRect && cropRect->getState() != CROP_NONE)
    {
        cropRect->setState(CROP_COMPLETE);
    }
    redisplayCurrentMap();
}

// called by CreateBorder::endDragging()
void  MapEditor::redisplayCurrentMap()
{
    emit view->sig_refreshView();

    buildEditorDB();
    forceRedraw();
}

void MapEditor::clearAllMaps()
{
    view->dump(true);
    localMap->wipeout();
    view->dump(true);
    if (map)
    {
        map->wipeout();
        view->dump(true);
    }
    cropRect.reset();

    buildEditorDB();
    forceRedraw();
}

//////////////////////////////////////////////////////////////////
///
/// Layer slotsevents
///
//////////////////////////////////////////////////////////////////

void MapEditor::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    if (    config->kbdMode == KBD_MODE_XFORM_VIEW
        || (config->kbdMode == KBD_MODE_XFORM_SELECTED && isSelected()))
    {
        qDebug() << getName() << config->kbdMode;
        if (view->getMouseMode() == MOUSE_MODE_CENTER && btn == Qt::LeftButton)
        {
            setCenterScreenUnits(spt);
            forceLayerRecalc();
            emit sig_refreshView();
            return;
        }
    }

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
    case MAPED_MOUSE_NONE:
        startMouseInteraction(mousePos,btn);
        break;

    case MAPED_MOUSE_DRAW_LINE:
    {
        set = findSelectionsUsingDB(spt);
        mouse_interaction = std::make_shared<DrawLine>(this,set,mousePos);
        break;
    }

    case MAPED_MOUSE_CONSTRUCTION_LINES:
    {
        set = findSelectionsUsingDB(spt);
        mouse_interaction = std::make_shared<ConstructionLine>(this,set,mousePos);
        break;
    }

    case MAPED_MOUSE_DELETE:
        set = findSelectionsUsingDB(spt);
        // only delete one line
        for (auto it = set.begin(); it != set.end(); it++)
        {
            auto sel = *it;
            if (sel->getType() == MAP_EDGE)
            {
                map->removeEdge(sel->getEdge());
                map->verify();
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
        setMapedMouseMode(MAPED_MOUSE_NONE);
        break;

    case MAPED_MOUSE_SPLIT_LINE:
    {
        QVector<EdgePtr> qvep;
        set = findEdges(spt, qvep);
        for (auto it = set.begin(); it != set.end(); it++)
        {
            auto sel = *it;
            map->splitEdge(sel->getEdge());
            map->verify();
        }
        buildEditorDB();
        forceRedraw();
        setMapedMouseMode(MAPED_MOUSE_NONE);
    }
    break;

    case MAPED_MOUSE_EXTEND_LINE:
        set = findSelectionsUsingDB(spt);
        mouse_interaction = std::make_shared<ExtendLine>(this,set,mousePos);
        break;

    case MAPED_MOUSE_CONSTRUCTION_CIRCLES:
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

    case MAPED_MOUSE_CREATE_CROP:
        cropRect->setState(CROP_NONE);
        mouse_interaction = std::make_shared<CreateCrop>(this,mousePos);
        break;

    case MAPED_MOUSE_CREATE_BORDER:
        mouse_interaction = std::make_shared<CreateBorder>(this,mousePos);
        mouse_interaction->updateDragging(spt);
        break;

    case MAPED_MOUSE_EDIT_BORDER:
        mouse_interaction = std::make_shared<EditBorder>(this,mousePos);
        mouse_interaction->updateDragging(spt);
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

    if (config->getViewerType() == VIEW_MAP_EDITOR)
    {
        setCenterScreenUnits(spt);
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

void MapEditor::slot_mouseTranslate(QPointF pt)
{
    if (config->getViewerType() != VIEW_MAP_EDITOR && Layer::config->kbdMode == KBD_MODE_XFORM_VIEW)
    {
        xf_canvas.setTranslateX(xf_canvas.getTranslateX() + pt.x());
        xf_canvas.setTranslateY(xf_canvas.getTranslateY() + pt.y());
        forceLayerRecalc();
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

void MapEditor::slot_mouseReleased(QPointF spt)
{
    if (config->getViewerType() != VIEW_MAP_EDITOR)
        return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "release" << mousePos << screenToWorld(mousePos);

    if (mouse_interaction)
    {
        mouse_interaction->endDragging(mousePos);
        if (map_mouse_mode != MAPED_MOUSE_EDIT_BORDER && map_mouse_mode != MAPED_MOUSE_CREATE_BORDER)
        {
            mouse_interaction.reset();
        }
    }
}

void MapEditor::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

void MapEditor::slot_wheel_scale(qreal delta)
{
    if (config->getViewerType() == VIEW_MAP_EDITOR && Layer::config->kbdMode == KBD_MODE_XFORM_VIEW)
    {
        xf_canvas.setScale(xf_canvas.getScale() + delta);
        forceLayerRecalc();
    }
}

void MapEditor::slot_wheel_rotate(qreal delta)
{
    if (config->getViewerType() == VIEW_MAP_EDITOR && Layer::config->kbdMode == KBD_MODE_XFORM_VIEW)
    {
        xf_canvas.setRotateDegrees(xf_canvas.getRotateDegrees() + delta);
        forceLayerRecalc();
    }
}

void MapEditor::slot_scale(int amount)
{
    if (config->getViewerType() == VIEW_MAP_EDITOR && Layer::config->kbdMode == KBD_MODE_XFORM_VIEW)
    {
        xf_canvas.setScale(xf_canvas.getScale() + static_cast<qreal>(amount)/100.0);
        forceLayerRecalc();
    }
}

void MapEditor::slot_rotate(int amount)
{
    if (config->getViewerType() == VIEW_MAP_EDITOR && Layer::config->kbdMode == KBD_MODE_XFORM_VIEW)
    {
        xf_canvas.setRotateRadians(xf_canvas.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        forceLayerRecalc();
    }
}

void MapEditor:: slot_moveX(int amount)
{
    if (config->getViewerType() == VIEW_MAP_EDITOR && Layer::config->kbdMode == KBD_MODE_XFORM_VIEW)
    {
        xf_canvas.setTranslateX(xf_canvas.getTranslateX() + amount);
        forceLayerRecalc();
    }
}

void MapEditor::slot_moveY(int amount)
{
    if (config->getViewerType() == VIEW_MAP_EDITOR && Layer::config->kbdMode == KBD_MODE_XFORM_VIEW)
    {
        xf_canvas.setTranslateY(xf_canvas.getTranslateY() + amount);
        forceLayerRecalc();
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
    case Qt::Key_Escape: setMapedMouseMode(MAPED_MOUSE_NONE);  return false; // propagate
    case Qt::Key_F3:     setMapedMouseMode(MAPED_MOUSE_DRAW_LINE); break;
    case Qt::Key_F4:     setMapedMouseMode(MAPED_MOUSE_CONSTRUCTION_LINES); break;
    case Qt::Key_F5:     setMapedMouseMode(MAPED_MOUSE_DELETE); break;
    case Qt::Key_F6:     setMapedMouseMode(MAPED_MOUSE_SPLIT_LINE); break;
    case Qt::Key_F7:     setMapedMouseMode(MAPED_MOUSE_EXTEND_LINE); break;
    case Qt::Key_F8:     setMapedMouseMode(MAPED_MOUSE_CREATE_CROP); break;
    case Qt::Key_F9:     setMapedMouseMode(MAPED_MOUSE_CONSTRUCTION_CIRCLES); break;

    default: return false;
    }

    return true;
}
