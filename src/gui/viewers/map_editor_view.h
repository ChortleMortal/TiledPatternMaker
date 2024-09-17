#pragma once
#ifndef MAP_EDITOR_VIEW_H
#define MAP_EDITOR_VIEW_H

#include "gui/viewers/layer_controller.h"
#include "sys/enums/emapeditor.h"

typedef std::shared_ptr<class DesignElement>    DesignElementPtr;
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

    virtual const Xform &   getModelXform() override;
    virtual void            setModelXform(const Xform & xf, bool update) override;

    void                drawMap(QPainter * painter, eMapedLayer layer, QColor color);
    void                drawDCEL(QPainter * painter);
    void                drawTile(QPainter * painter, DesignElementPtr del);
    void                drawBoundaries(QPainter * painter, DesignElementPtr del);
    void                drawPoints(QPainter * painter, QVector<class PointInfo> & points);
    void                drawConstructionLines(QPainter * painter);
    void                drawConstructionCircles(QPainter * painter);

    void                startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton);

    QTransform          getPlacement(TilePtr tile);

    MapEditorSelection * getSelector() { return selector; }
    MapEditorDb        * getDb() { return db; }

    uint                mapLineWidth;
    uint                constructionLineWidth;
    qreal               selectionWidth;

public slots:
    void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseTranslate(QPointF pt)      override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;
    void slot_setCenter(QPointF spt) override;

    void slot_wheel_scale(qreal delta)  override;
    void slot_wheel_rotate(qreal delta) override;

    void slot_scale(int amount)  override;
    void slot_rotate(int amount) override;
    void slot_moveX(qreal amount)  override;
    void slot_moveY(qreal amount)  override;

    eViewType iamaLayer() override { return VIEW_MAP_EDITOR; }
    void       iamaLayerController() override {}

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
