#pragma once
#ifndef GRID_VIEW_H
#define GRID_VIEW_H

#include "gui/viewers/layer_controller.h"
#include "gui/viewers/geo_graphics.h"

typedef std::shared_ptr<class PlacedTileSelector>   PlacedTileSelectorPtr;
typedef std::shared_ptr<class Map>                  MapPtr;

class GridView : public LayerController
{
public:
    GridView();
    ~GridView();

    void paint(QPainter * painter ) override;

    bool nearGridPoint(QPointF spt, QPointF & foundGridPoint);  // used by tiling maker

protected:
    void draw(QPainter * painter);

    void drawFromTilingFlood(QPainter * painter);
    void drawFromTilingRegion(QPainter * painter);

    void drawModelUnits(QPainter * painter);
    void drawModelUnitsCentered(QPainter * painter);

    void drawScreenUnits(QPainter * painter);
    void drawScreenUnitsCentered(QPainter * painter);

    void drawCenters(QPainter *painter);

    void iamaLayerController() override {}

    static int refs;

public slots:
    void slot_mousePressed(QPointF spt, Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;

private:
    void drawScreenUnits(QPainter * painter, QRectF r, QRectF r1, QPointF center, int x_steps, int y_steps, qreal step);
    void drawModelUnits( GeoGraphics * gg, QRectF r, QRectF r1, QPointF center, int x_steps, int y_steps, qreal step);

    void ggdrawLine(GeoGraphics * gg, QLineF line, QPen pen);
    void ggdrawLine(GeoGraphics * gg, QPointF p1, QPointF p2, QPen  pen);
    void ggdrawPoly(GeoGraphics * gg, QPolygonF & poly, QPen  pen);

    void ppdrawLine(QPainter * painter, QLineF &line);
    void ppdrawLine(QPainter * painter, QPointF p1, QPointF p2);

    bool intersects(QVector<QLineF> &edges , QLineF & line);
    QVector<QLineF> toEdges(const QRectF & r);

    Configuration   * config;
    MapPtr          gridMap;

    QLineF      corners[2];
    bool        genMap;
};
#endif
