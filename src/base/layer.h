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

enum eLayerType
{
    LTYPE_VIEW,
    LTYPE_BACKGROUND,
    LTYPE_MARK,
    LTYPE_BORDER,
    LTYPE_MAP_EDITOR,
    LTYPE_TILING_MAKER,
    LTYPE_GRID
};

class Layer : public QObject
{
    Q_OBJECT

public:
    Layer(QString name, eLayerType ltype);
    Layer(const Layer & other);
    Layer(LayerPtr other);
    ~Layer();

    virtual void paint(QPainter * painter);

    void    addSubLayer(LayerPtr item);
    void    removeSubLayer(LayerPtr item);
    void    clearSubLayers() { subLayers.clear(); }

    void    forceLayerRecalc(bool update = true);
    void    forceRedraw() ;

    QPointF screenToWorld(QPointF pt) ;
    QPointF screenToWorld(int x, int y);
    QRectF  screenToWorld(QRectF rect) ;

    QPointF worldToScreen(QPointF pt);
    QLineF  worldToScreen(QLineF line);
    QRectF  worldToScreen(QRectF rect);

    void        setCenterScreen(QPointF spt);
    void        setCenterModel(QPointF mpt);
    QPointF     getCenterScreen();
    QPointF     getCenterModel();

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

public slots:
    virtual void slot_moveX(int amount);
    virtual void slot_moveY(int amount);
    virtual void slot_rotate(int amount);
    virtual void slot_scale(int amount);

    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn);

    virtual void slot_mouseTranslate(QPointF pt);
    virtual void slot_wheel_rotate(qreal delta);
    virtual void slot_wheel_scale(qreal delta);
    virtual void slot_setCenterScreen(QPointF spt);

protected:
    virtual void drawCenter(QPainter * painter);

    QPen              layerPen;

    class Configuration   * config;
    class View            * view;

private:
    void computeLayerTransform();
    void deltaLoc(QPointF loc);

    eLayerType  layerType;

    Xform      xf_canvas;
    QTransform qtr_layer;       // calculated
    QTransform qtr_invert;      // calculated

    bool    visible;
    QVector<LayerPtr>  subLayers;

    QString name;
    int     zlevel;
    QPointF pos;
};

#endif
