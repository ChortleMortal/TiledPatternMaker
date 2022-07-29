////////////////////////////////////////////////////////////////////////////
//
// TilingViewer.java
//
// TilingViewer gets a Tiling instance and displays the tiling in a
// view window.  It always draws as much of the tiling as necessary to
// fill the whole view area, using the modified polygon-filling algorithm
// in geometry.FillRegion.
//
// TilingViewer also has a test application that accepts the name of
// a built-in tiling on the command line or the specification of a tiling
// on System.in and displays that tiling.  Access it using
// 		java tile.TilingViewer

#ifndef TILING_VIEWER_H
#define TILING_VIEWER_H

#include "misc/layer_controller.h"
#include <QColor>

typedef std::shared_ptr<class TilingView>      TilingViewPtr;
typedef std::shared_ptr<class Tiling>          TilingPtr;
typedef std::shared_ptr<class PlacedFeature>   PlacedFeaturePtr;

class GeoGraphics;

class TilingView : public LayerController
{
public:
    static TilingViewPtr getSharedInstance();
    TilingView();   // dont_use this

    void    setTiling(TilingPtr tiling) { this->tiling = tiling; }

    void    paint(QPainter *painter) override;
    void    draw(GeoGraphics * gg);

    virtual void iamaLayer() override {}
    virtual void iamaLayerController() override {}

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseTranslate(QPointF spt)     override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;

    virtual void slot_wheel_scale(qreal delta)  override;
    virtual void slot_wheel_rotate(qreal delta) override;

    virtual void slot_scale(int amount)  override;
    virtual void slot_rotate(int amount) override;
    virtual void slot_moveX(int amount)  override;
    virtual void slot_moveY(int amount)  override;

protected:
    void drawPlacedFeature(GeoGraphics * g2d, PlacedFeaturePtr pf);

private:

    static TilingViewPtr spThis;

    TilingPtr           tiling;
};

#endif

