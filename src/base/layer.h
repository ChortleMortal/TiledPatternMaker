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

typedef std::shared_ptr<class Layer> LayerPtr;

class Layer : public QObject
{
    Q_OBJECT

public:
    Layer(QString name);
    Layer(const Layer & other);
    Layer(LayerPtr other);
    ~Layer();

    virtual void paint(QPainter * painter);

    void    addSubLayer(LayerPtr item);
    void    removeSubLayer(LayerPtr item);
    void    clearSubLayers() { subLayers.clear(); }

    void    forceLayerRecalc(bool update = true);
    void    forceRedraw() ;

    bool    isSelected();

    QPointF screenToWorld(QPointF pt) ;
    QPointF screenToWorld(int x, int y);
    QRectF  screenToWorld(QRectF rect) ;
    qreal   screenToWorld(qreal val);

    QPointF worldToScreen(QPointF pt);
    QLineF  worldToScreen(QLineF line);
    QRectF  worldToScreen(QRectF rect);

    void        setCenterScreenUnits(QPointF spt);
    QPointF     getCenterScreenUnits();
    QPointF     getCenterModelUnits();

    const Xform & getCanvasXform();
    void        setCanvasXform(const Xform & xf);

    QTransform  getCanvasTransform();
    QTransform  getFrameTransform();
    QTransform  getLayerTransform();

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

signals:
    void sig_center();
    void sig_refreshView();

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) = 0;
    virtual void slot_mouseDragged(QPointF spt)       = 0;
    virtual void slot_mouseTranslate(QPointF pt)      = 0;
    virtual void slot_mouseMoved(QPointF spt)         = 0;
    virtual void slot_mouseReleased(QPointF spt)      = 0;
    virtual void slot_mouseDoublePressed(QPointF spt) = 0;

    virtual void slot_wheel_scale(qreal delta)  = 0;
    virtual void slot_wheel_rotate(qreal delta) = 0;

    virtual void slot_scale(int amount)  = 0;
    virtual void slot_rotate(int amount) = 0;
    virtual void slot_moveX(int amount)  = 0;
    virtual void slot_moveY(int amount)  = 0;

protected:
    virtual void drawCenter(QPainter * painter);

    Xform      xf_canvas;
    QPen       layerPen;

    class Configuration   * config;
    class View            * view;

private:
    void connectSignals();
    void computeLayerTransform();
    void deltaLoc(QPointF loc);

    QTransform qtr_layer;       // calculated
    QTransform qtr_invert;      // calculated

    bool    visible;
    QVector<LayerPtr>  subLayers;

    QString name;
    int     zlevel;
    QPointF pos;
};


#endif
