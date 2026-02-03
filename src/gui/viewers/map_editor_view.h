#pragma once
#ifndef MAP_EDITOR_VIEW_H
#define MAP_EDITOR_VIEW_H

#include "gui/viewers/layer_controller.h"
#include "sys/enums/emapeditor.h"

typedef std::shared_ptr<class DesignElement>    DELPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Tile>             TilePtr;

typedef std::weak_ptr<class DesignElement>      WeakDELPtr;
typedef std::weak_ptr<class Map>                WeakMapPtr;

class MapEditorDb;
class MapEditorSelection;

class MapEditorView : public LayerController
{
public:
    MapEditorView();
    ~MapEditorView();

    void                init(MapEditorDb * maped_db, MapEditorSelection * selector);

    void                paint(QPainter * painter) override;

    void                draw(QPainter * painter);
    void                drawMap(QPainter * painter, eMapedLayer layer, QColor color);
    void                drawDCEL(QPainter * painter);
    void                drawTile(QPainter * painter, DELPtr del);
    void                drawBoundaries(QPainter * painter, DELPtr del);
    void                drawPoints(QPainter * painter, QVector<class PointInfo> & points);
    void                drawConstructionLines(QPainter * painter);
    void                drawConstructionCircles(QPainter * painter);

    void                startMouseInteraction(QPointF spt, Qt::MouseButton mouseButton);

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

    MapEditorDb        * db;
    MapEditorSelection * selector;
};

#endif
