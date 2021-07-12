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

#ifndef DESIGN_PREVIEW2
#define DESIGN_PREVIEW2

#include <QtCore>
#include "geometry/fill_region.h"
#include "base/layer.h"
#include "geometry/edgepoly.h"

typedef std::shared_ptr<class PrototypeView>        PrototypeViewPtr;
typedef std::shared_ptr<class Prototype>            PrototypePtr;
typedef std::shared_ptr<class PlacedDesignElement>  PlacedDesignElementPtr;

class PrototypeView : public FillRegion, public Layer
{
public:
    static PrototypeViewPtr getSharedInstance();
    PrototypeView();        // don't use this

    void setPrototype(PrototypePtr proto) { this->proto = proto; }
    PrototypePtr getPrototype() {return proto; }

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseTranslate(QPointF pt)      override;
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
    void   paint(QPainter *painter) override;
    void   receive(GeoGraphics * gg,int h, int v ) override;
    void   draw(GeoGraphics * gg);

    void   drawPlacedDesignElement(GeoGraphics * gg, PlacedDesignElementPtr pde, QPen linePen, QBrush interiorBrush, QPen borderPen,bool selected);

    QPointF         t1;
    QPointF         t2;

    PrototypePtr    proto;

    EdgePoly        edges;              // this is not really an EdgePoly it is a vector of Edges
    QColor          feature_interior;
    QColor          feature_border;

    QVector<PlacedDesignElement> rpfs;

private:

    static PrototypeViewPtr spThis;
};
#endif
