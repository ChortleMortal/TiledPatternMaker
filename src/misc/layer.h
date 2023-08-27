#pragma once
#ifndef TPM_LAYER_H
#define TPM_LAYER_H

#include <QObject>
#include <QPen>
#include "enums/eviewtype.h"
#include "geometry/xform.h"
#include "geometry/circle.h"

typedef std::shared_ptr<class Layer> LayerPtr;

class Layer : public QObject
{
    Q_OBJECT

public:
    Layer(QString name);
    Layer(const Layer & other);
    Layer(LayerPtr other);
    virtual ~Layer();

    void reset() { pos = QPointF(); }

    virtual void paint(QPainter * painter);

    void    addSubLayer(LayerPtr item);
    void    removeSubLayer(LayerPtr item);
    void    clearSubLayers() { subLayers.clear(); }

    void    forceLayerRecalc(bool update = true);
    void    forceRedraw();

    bool    isSelected();

    QPointF screenToWorld(QPointF pt);
    QPointF screenToWorld(int x, int y);
    QRectF  screenToWorld(QRectF rect);
    qreal   screenToWorld(qreal val);
    QPolygonF screenToWorld(QPolygonF poly);

    QPointF   worldToScreen(QPointF pt);
    QLineF    worldToScreen(QLineF line);
    QRectF    worldToScreen(QRectF rect);
    QPolygonF worldToScreen(QPolygonF poly);
    Circle    worldToScreen(Circle c);

    void    setCenterScreenUnits(QPointF spt);
    QPointF getCenterScreenUnits();
    QPointF getCenterModelUnits();

    virtual const Xform  & getCanvasXform();
    virtual void           setCanvasXform(const Xform & xf);

    QTransform  getCanvasTransform();
    QTransform  getFrameTransform();
    QTransform  getLayerTransform();

    virtual QString getName() { return name; }

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

    virtual eViewType iamaLayer() = 0;

    static bool sortByZlevel(LayerPtr s1, LayerPtr s2);
    static bool sortByZlevelP(Layer *  s1, Layer  * s2);

    static int refs;

signals:
    void sig_refreshView();

protected:
    virtual void drawLayerModelCenter(QPainter * painter);
    void drawCenterSymbol(QPainter * painter, QPointF spt, QColor circleColor, QColor xColor);

    QPen       layerPen;

    class Configuration   * config;
    class ViewControl     * view;

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
