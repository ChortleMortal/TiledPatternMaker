﻿#pragma once
#ifndef MAP_EDITOR_VIEW_H
#define MAP_EDITOR_VIEW_H

#include "misc/layer_controller.h"
#include "enums/emapeditor.h"

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
    static MapEditorView * getInstance();
    static void            releaseInstance();

    void                init(MapEditorDb * maped_db, MapEditorSelection * selector);

    virtual void        paint(QPainter * painter) override;
    virtual void        draw(QPainter *);

    virtual const Xform &   getModelXform() override;
    virtual void            setModelXform(const Xform & xf, bool update) override;

    void                drawMap(QPainter * painter, eMapedLayer layer, QColor color);
    void                drawDCEL(QPainter * painter);
    void                drawTile(QPainter * painter, DesignElementPtr del);
    void                drawBoundaries(QPainter * painter, DesignElementPtr del);
    void                drawPoints(QPainter * painter, QVector<class pointInfo> & points);
    void                drawConstructionLines(QPainter * painter);
    void                drawConstructionCircles(QPainter * painter);

    void                startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton);

    QTransform          getPlacement(TilePtr tile);

    MapEditorSelection * getSelector() { return selector; }
    MapEditorDb        * getDb() { return db; }

    qreal               mapLineWidth;
    qreal               constructionLineWidth;
    qreal               selectionWidth;

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseTranslate(QPointF pt)      override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;
    virtual void slot_setCenter(QPointF spt) override;

    virtual void slot_wheel_scale(qreal delta)  override;
    virtual void slot_wheel_rotate(qreal delta) override;

    virtual void slot_scale(int amount)  override;
    virtual void slot_rotate(int amount) override;
    virtual void slot_moveX(qreal amount)  override;
    virtual void slot_moveY(qreal amount)  override;

    virtual eViewType iamaLayer() override { return VIEW_MAP_EDITOR; }
    virtual void iamaLayerController() override {}

    void    setMousePos(QPointF pt);
    QPointF getMousePos() { return mousePos; }

    QTransform  viewT;
    QTransform  viewTinv;  // inverted

protected:

private:
    MapEditorView();
    ~MapEditorView();

    static MapEditorView * mpThis;

    Configuration      * config;

    QPointF             mousePos;             // used by menu
    bool                debugMouse;

    MapEditorDb        * db;
    MapEditorSelection * selector;
};

#endif
