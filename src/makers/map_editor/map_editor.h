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

#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include "viewers/map_editor_view.h"
#include "tile/tiling.h"
#include "makers/map_editor/map_mouseactions.h"
#include "makers/map_editor/map_editor_selection.h"
#include "makers/map_editor/map_editor_stash.h"

class Canvas;

class MapEditor : public MapEditorSelection, std::enable_shared_from_this<MapEditor>
{
    Q_OBJECT

    friend class MoveVertex;
    friend class MoveEdge;
    friend class DrawLine;
    friend class ConstructionLine;
    friend class ExtendLine;
    friend class MoveConstructionCircle;
    friend class CreateCrop;
    friend class CreateBorder;
    friend class EditBorder;

public:
    static MapEditorPtr getSharedInstance();
    MapEditor();    // dont call this

    void            reload();
    void            setLocalMap(MapPtr map) { localMap = map; }

    DesignElementPtr getDesignElement() { return delp; }
    PrototypePtr     getPrototype() { return prototype; }
    StylePtr         getStyle() { return styp; }
    SelectionSet     getCurrentSelections() { return currentSelections; }

    void            draw(QPainter * painter) override;
    void            updateStatus();

    bool            procKeyEvent(QKeyEvent *k);
    eMapMouseMode   getMouseMode();

    bool            loadCurrentStash();
    bool            loadNamedStash(QString file, bool animate);
    bool            loadPrevStash();
    bool            loadNextStash();
    bool            saveStash();
    bool            keepStash(QString name);
    bool            initStashFrom(QString name);

    void            flipLineExtension();

    void            applyCrop();
    void            applyMask();
    void            redisplayCurrentMap();
    void            completeBorder();

    void            clearAllMaps();

    MapMouseActionPtr mouse_interaction;    // used by menu
    QPointF           mousePos;             // used by menu
    qreal             newCircleRadius;      // used by menu
    MapEditorStash    stash;                // stash of construction lines

public slots:
    void setMouseMode(eMapMouseMode mapType);

    void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt);
    void slot_mouseReleased(QPointF spt);
    void slot_mouseMoved(QPointF spt);
    void slot_view_synch(int id, int enb);

protected:
    void    setMousePos(QPointF pt);

    MapPtr  localMap;       // for new maps created in the editor

private:
    static MapEditorPtr     spThis;

    class View            * view;
    class DecorationMaker * decorationMaker;
    class MotifMaker      * motifMaker;
    TilingMakerPtr          tilingMaker;
    class ControlPanel    * cpanel;

    SelectionSet currentSelections;

    // Mouse tracking.
    MapSelectionPtr findFeatureUnderMouse();

    // Mouse interactions.
    void startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton);

    // Mouse mode, triggered by the toolbar.
    eMapMouseMode     map_mouse_mode;
};
#endif
