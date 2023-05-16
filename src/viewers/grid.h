#pragma once
#ifndef GRID_H
#define GRID_H

#include "misc/layer_controller.h"
#include "misc/geo_graphics.h"

typedef std::shared_ptr<class Grid>           GridPtr;
typedef std::shared_ptr<class TileSelector>   TileSelectorPtr;
typedef std::shared_ptr<class Map>            MapPtr;

class Grid : public LayerController
{
public:
    static GridPtr getSharedInstance();
    Grid();  // don't use this

    void paint(QPainter * pp ) override;

    bool nearGridPoint(QPointF spt, QPointF & foundGridPoint);

    virtual const Xform  & getCanvasXform() override;
    virtual void           setCanvasXform(const Xform & xf) override;

protected:
    void draw(QPainter * pp);

    void drawFromTilingFlood();
    void drawFromTilingRegion();
    void drawModelUnits();
    void drawModelUnitsCentered();
    void drawScreenUnits();
    void drawScreenUnitsCentered();

    void iamaLayer() override {}
    void iamaLayerController() override {}

    static int refs;

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;

private:
    void ggdrawLine(QLineF line, QPen pen);
    void ggdrawLine(QPointF p1, QPointF p2, QPen  pen);
    void ggdrawPoly(QPolygonF & poly, QPen  pen);
    void ppdrawLine(QLineF line);
    void ppdrawLine(QPointF p1, QPointF p2);

    static GridPtr spThis;

    Configuration * config;
    ViewControl   * view;

    QPainter    * pp;
    GeoGraphics * gg;

    Xform       xf_canvas;

    MapPtr      gridMap;

    QLineF      corners[2];

    bool        genMap;
};

#endif // GRID_H
