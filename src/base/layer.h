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

#ifndef DACPATS_LAYER_H
#define DACPATS_LAYER_H

#include <QGraphicsItemGroup>
#include "geometry/bounds.h"
#include "geometry/Transform.h"
#include "base/shared.h"

class Design;
class Canvas;

#define TRANS   painter->translate(getLoc())
#define UNTRANS painter->translate(-getLoc());

// Notes:
// Rotation is handled by QGraphicsItemGroup
// ZValue   is handled by QGraphicsItemGroup
class Layer : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT

public:
    Layer(QString name);
    Layer(const Layer & layer);
    ~Layer()  { refs--; }

    void    addToGroup(QGraphicsItem *item);
    void    removeFromGroup(QGraphicsItem *item);

    void    forceUpdateLayer();

    void    setBounds(Bounds & bounds) { this->bounds = bounds; }
    void    setDeltas(Bounds & deltas) { this->deltas = deltas; }
    void    setRotateCenter (QPointF pt);

    Bounds  getBounds() { return bounds; }
    Bounds  getDeltas() { return deltas; }
    Bounds  getAdjustedBounds();

    QPointF getRotateCenter() const  { return rotateCenter; }

    void    forceRedraw() ;

    QPointF screenToWorld(QPointF pt) ;
    QPointF screenToWorld(int x, int y) ;
    QPointF worldToScreen(QPointF pt);

    TransformPtr getLayerTransform();
    QPolygonF    getBoundary();

    QString getName() { return name; }

    static int refs;

public slots:
    void slot_moveX(int amount);
    void slot_moveY(int amount);
    void slot_rotate(int amount);
    void slot_scale(int amount);

protected:
    void computeLayerTransform();

    TransformPtr layerTransform;

    QPointF _loc;
    qreal   rotateAngle;

private:
    Canvas * canvas;

    TransformPtr inverse;

    Bounds  bounds;
    Bounds  deltas;

    QPointF rotateCenter;

    QString name;
};

#endif // LAYER_H
