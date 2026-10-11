#pragma once

#include "gui/map_editor/map_editor_db.h"
#include "gui/viewers/layer_controller.h"

typedef std::shared_ptr<class TileMotif>    TileMotifPtr;
typedef std::shared_ptr<class Map>          MapPtr;
typedef std::shared_ptr<class Tile>         TilePtr;

typedef std::weak_ptr<class TileMotif>      WeakTileMotifPtr;
typedef std::weak_ptr<class Map>            WeakMapPtr;

class MapEditorDb;
class MapEditorSelection;

class MapEditorView : public LayerController
{
    using LayerId = MapEditorLayer::LayerId;
    using MapType = MapEditorMap::MapType;

public:
    MapEditorView();
    ~MapEditorView();

    void                init(MapEditor * maped, MapEditorSelection * selector);

    void                paint(QPainter * painter) override;

    void                draw(QPainter * painter);
    void                drawMap(QPainter * painter, QTransform t, MapPtr map, QColor color);
    void                drawDCEL(QPainter * painter);
    void                drawTile(QPainter * painter, TileMotifPtr timo);
    void                drawBoundaries(QPainter * painter, TileMotifPtr timo);
    void                drawPoints(QPainter * painter, QVector<class PointInfo> & points);
    void                drawConstructionLines(QPainter * painter);
    void                drawSelectedConstructionLine(QPainter * painter);
    void                drawConstructionCircles(QPainter * painter);

    void                startMouseInteraction(QPointF spt, Qt::MouseButton mouseButton);
    void                startConstructionLineInteraction(QPointF spt, Qt::MouseButton mouseButton);

    MapEditorSelection * getSelector() { return selector; }
    MapEditorDb        * getDb() { return db; }

    uint                mapLineWidth;
    uint                constructionLineWidth;
    qreal               selectionWidth;

public slots:
    void slot_mousePressed(QPointF spt, Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;

    void iamaLayerController() override {}

    void    setMousePos(QPointF pt);
    QPointF getMousePos() { return mousePos; }

    QTransform  viewT;
    QTransform  viewTinv;  // inverted

protected:

private:
    Configuration      * config;

    QPointF             mousePos;             // used by menu
    bool                debugMouse;

    MapEditor          * editor;
    MapEditorDb        * db;
    MapEditorSelection * selector;
};
