#pragma once

#include "gui/viewers/layer_controller.h"
#include "gui/viewers/geo_graphics.h"
#include "model/settings/configuration.h"

typedef std::shared_ptr<class PlacedTileSelector>   PlacedTileSelectorPtr;

class Tiling;

class GridView : public LayerController
{
public:
    GridView();
    ~GridView();

    void paint(QPainter * painter ) override;

    QTransform getCanvasTransform() override;
    void       setModelXform(const Xform & xf, bool update, uint sigid) override;

    void       setPosition(eGridUnits units, eGridPosition pos);
    void       setMoving(bool move);
    void       restorePosition();
    void       syncPosition();

    bool nearGridPoint(QPointF spt, QPointF & foundGridPoint);  // used by tiling maker

protected:
    void draw(QPainter * painter);

    void drawLattice(QPainter * painter);
    void drawFromTilingRegion(QPainter * painter);

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
    bool    getLattice(QTransform & S, bool & third);
    bool    getBasis(qreal step, QPointF & u, QPointF & v, bool & third);
    QPointF getTilingOrigin(Tiling * tiling);

    QTransform getDesignTransform();

    void            storeXform();
    eGridPosition * positionOf(eGridUnits units);
    Xform         * xformOf(eGridUnits units);

    Configuration   * config;
};
