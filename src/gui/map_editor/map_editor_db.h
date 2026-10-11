#pragma once

#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_layer.h"
#include "gui/map_editor/map_editor_mouseactions.h"
#include "gui/map_editor/map_editor_stash.h"
#include "sys/geometry/circle.h"
#include "model/makers/crop_maker.h"
#include "sys/qt/unique_qvector.h"

class MapEditor;

class MapEditorDb  : public MosaicCropMaker
{
    using MapType   = MapEditorMap::MapType;
    using MouseMode = MapEditor::MouseMode;
    using LayerId   = MapEditorLayer::LayerId;

public:
    MapEditorDb(MapEditor * me);
    virtual ~MapEditorDb() {}

    void                reset();
    void                buildEditorDB();
    void                buildMotifDB(TileMotifPtr timo, MapType mapType);

    QVector<LineInfo>  & getLines()   { return lines; }
    QVector<PointInfo> & getPoints()  { return points; }
    QVector<CirclePtr> & getCircles() { return circles; }

    void                setActiveDCEL(DCELPtr dcel)     { activeDcel = dcel; localDcel.reset();}
    DCELPtr             getActiveDCEL()                 { return activeDcel.lock(); }
    void                setLocalDCEL(DCELPtr dcel)      { activeDcel = dcel; localDcel = dcel; }
    DCELPtr             getLocaldDCEL()                 { return localDcel; }

    void                setCrop(CropPtr acrop);
    CropPtr             getCrop()                       { return crop; }
    void                removeCrop()                    { crop.reset(); }


    static bool         isMotif(MapType type);

    void                setMouseMode(MouseMode emm)     { mouseMode = emm; }
    MouseMode           getMouseMode()                  { return mouseMode;}

    void                setMouseInteraction(MapMouseActionPtr action) { mouse_interaction = action; }
    void                resetMouseInteraction()         { mouse_interaction.reset(); }
    MapMouseActionPtr   getMouseInteraction()           { return mouse_interaction; }

    bool                hasConstructionLines();

    UniqueQVector<QLineF>    constructionLines;
    UniqueQVector<CirclePtr> constructionCircles;
    int                      selectedCLine = -1;
    int                      liftedCLine   = -1;    // being dragged - left out of the DB
    bool                     isCLine(int index) { return index >= 0 && index < constructionLines.size(); }

    bool                showMap;
    bool                showBoundaries;
    bool                showConstructionLines;
    bool                showPoints;
    bool                showMidPoints;
    bool                showDirnPoints;
    bool                showArcCentre;
    bool                showTileCentre;

private:
    MapEditor *         editor;

    MouseMode           mouseMode; // Mouse mode, triggered by the toolbar.

    WeakDCELPtr         activeDcel;
    DCELPtr             localDcel;
    CropPtr             crop;
    MapMouseActionPtr   mouse_interaction;    // used by menu

    QVector<LineInfo>  lines;   // generated
    QVector<PointInfo> points;  // generated
    QVector<CirclePtr> circles; // generated
};
