#pragma once
#ifndef TPM_LAYER_H
#define TPM_LAYER_H

#include <QObject>
#include <QPen>
#include "enums/eviewtype.h"
#include "geometry/xform.h"
#include "geometry/circle.h"
#include "viewers/view_controller.h"

typedef std::shared_ptr<class Layer> LayerPtr;

#define DEBUG_XFORM 0x01
#define DEBUG_TFORM 0x02
#define DEBUG_LAYER 0x04

class ViewController;

class Layer : public QObject
{
    Q_OBJECT

public:
    Layer(QString name, bool unique);
    Layer(const Layer & other);
    Layer(LayerPtr other);
    virtual ~Layer();

    void        reset() { pos = QPointF(); }

    virtual void paint(QPainter * painter);

    void        addSubLayer(LayerPtr item);
    void        removeSubLayer(LayerPtr item);
    void        clearSubLayers() { subLayers.clear(); }

    void        forceLayerRecalc(bool update);
    void        forceRedraw();

    bool        isSelected();

    QPointF     screenToWorld(QPointF pt);
    QPointF     screenToWorld(int x, int y);
    QRectF      screenToWorld(QRectF rect);
    qreal       screenToWorld(qreal val);
    QPolygonF   screenToWorld(QPolygonF poly);
    Circle      screenToWorld(Circle c);

    QPointF     worldToScreen(QPointF pt);
    QLineF      worldToScreen(QLineF line);
    QRectF      worldToScreen(QRectF rect);
    QPolygonF   worldToScreen(QPolygonF poly);
    Circle      worldToScreen(Circle c);

    void        setCenterScreenUnits(QPointF spt);
    QPointF     getCenterScreenUnits();
    QPointF     getCenterModelUnits();
    
    virtual const Xform &   getModelXform() = 0;
    virtual void            setModelXform(const Xform & xf, bool update) = 0;
    
    virtual QTransform  getModelTransform();
    virtual QTransform  getCanvasTransform();
    virtual QTransform  getLayerTransform();
            QTransform  getLayerInvertedTransform()  { return invertedLayer; }

    virtual QString getLayerName()      { return _name; }

    bool        isUnique()              { return _unique; }

    void        setZValue(int z);
    int         zValue()                { return zlevel; }

    void        setVisible(bool enb)    { visible = enb; }
    bool        isVisible()             { return visible; }

    LayerPtr    firstSubLayer()         { return subLayers.first(); }
    LayerPtr    geSubLayer(int index)   { return subLayers.at(index); }
    QVector<LayerPtr> & getSubLayers()  { return subLayers; }
    int         numSubLayers()          { return subLayers.size(); }

    void        setLoc(QPointF loc);
    QPointF     getLoc() { return pos; }

    void        setViewController(ViewController * vc) { viewControl = vc; }

    virtual eViewType iamaLayer() = 0;

    static bool sortByZlevel(LayerPtr s1, LayerPtr s2);
    static bool sortByZlevelP(Layer *  s1, Layer  * s2);

    static int  refs;

signals:
    void        sig_refreshView();

public slots:
    void        id_layer();

protected:
    virtual void drawLayerModelCenter(QPainter * painter);
    void         drawCenterSymbol(QPainter * painter, QPointF spt, QColor circleColor, QColor xColor);

    class Configuration   * config;
    class View            * view;
          ViewController  * viewControl;

    bool        _unique;
    int         debug;

    Xform       xf_model;          // used when unique canvas

private:
    void        connectSignals();
    void        computeLayerTransform();

    QTransform  layerTransform;     // calculated
    QTransform  invertedLayer;      // calculated


    bool        visible;

    QString     _name;
    int         zlevel;

    QVector<LayerPtr>  subLayers;   // for legacy code
    QPointF     pos;                // for legacy code

};


#endif
