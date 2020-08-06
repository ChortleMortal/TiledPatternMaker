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

#ifndef TPM_LAYER_H
#define TPM_LAYER_H

#include <QPen>
#include "geometry/xform.h"
#include "base/shared.h"

class Design;
class Canvas;
class WorkspaceViewer;
class Configuration;
class View;

class Layer : public QObject
{
    Q_OBJECT

public:
    Layer(QString name);
    Layer(const Layer & other);
    ~Layer();

    virtual void paint(QPainter * painter);

    void    addSubLayer(LayerPtr item);
    void    removeSubLayer(LayerPtr item);

    void    setCenter (QPointF pt);
    QPointF getCenter();

    void    setCanvasXform(Xform & xf);
    Xform   getCanvasXform();

    void    forceUpdateLayer();
    void    forceRedraw() ;

    QPointF screenToWorld(QPointF pt) ;
    QPointF screenToWorld(int x, int y);

    QPointF worldToScreen(QPointF pt);
    QLineF  worldToScreen(QLineF line);

    QTransform  getLayerTransform();
    QTransform  getViewTransform();

    QString getName() { return name; }

    void    setZValue(int z);
    int     zValue() { return zlevel; }

    void    setVisible(bool visibility) { visible = visibility; }
    bool    isVisible() { return visible; }

    LayerPtr firstSubLayer()           { return subLayers.first(); }
    LayerPtr geSubLayer(int index)     { return subLayers.at(index); }
    QVector<LayerPtr> & getSubLayers() { return subLayers; }
    int     numSubLayers()             { return subLayers.size(); }

    void    setLoc(QPointF loc);
    QPointF getLoc() { return pos; }

    static bool sortByZlevel(LayerPtr s1, LayerPtr s2);

    static int refs;

public slots:
    void slot_moveX(int amount);
    void slot_moveY(int amount);
    void slot_rotate(int amount);
    void slot_scale(int amount);
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn);

protected:
    Xform      xf_canvas;
    QTransform qtr_view;
    QTransform qtr_layer;       // calculated
    QTransform qtr_invert;      // calculated

    QPen       layerPen;

    Configuration   * config;
    Canvas          * canvas;
    WorkspaceViewer * wsViewer;
    View            * view;

private:
    void computeLayerTransform();
    void deltaLoc(QPointF loc);

    bool    visible;
    QVector<LayerPtr>  subLayers;

    QString name;
    int     zlevel;
    QPointF pos;
};

#endif
