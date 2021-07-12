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

#include "geometry/fill_region.h"
#include "base/layer.h"
#include <QColor>

typedef std::shared_ptr<class TilingView>      TilingViewPtr;
typedef std::shared_ptr<class Tiling>          TilingPtr;
typedef std::shared_ptr<class PlacedFeature>   PlacedFeaturePtr;

class TilingView : public FillRegion, public Layer
{
public:
    static TilingViewPtr getSharedInstance();
    TilingView();   // dont_use this

    void    setTiling(TilingPtr tiling) { this->tiling = tiling; }

    void    paint(QPainter *painter) override;
    void    draw(GeoGraphics * g2d);
    void    receive(GeoGraphics *gg, int h, int v ) override;

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseTranslate(QPointF spt)      override;
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

