#pragma once
#ifndef TPM_LAYER_H
#define TPM_LAYER_H

#include <QObject>
#include <QPen>
#include "sys/enums/eviewtype.h"
#include "sys/geometry/xform.h"
#include "sys/geometry/circle.h"
#include "gui/top/system_view_controller.h"

typedef std::shared_ptr<class Layer> LayerPtr;

#define DEBUG_XFORM 0x01
#define DEBUG_TFORM 0x02
#define DEBUG_LAYER 0x04

class SystemViewController;
class Configuration;

#define USER_LOCK 0x01
#define SOLO_LOCK 0x02

class Layer : public QObject
{
    Q_OBJECT

public:
    Layer(eViewType viewType,eModelType modelType, QString name);
    Layer(const Layer & other);
    Layer(LayerPtr other);
    virtual ~Layer();

    virtual void paint(QPainter * painter);

    void        initLayer();

    void        forceLayerRecalc(bool update);
    void        forceRedraw();

    bool        isSelected();

    QPointF     screenToModel(QPointF pt);
    QPoint      screenToModel(QPoint pt);
    QPointF     screenToModel(int x, int y);
    QRectF      screenToModel(QRectF rect);
    qreal       screenToModel(qreal val);
    QPolygonF   screenToModel(QPolygonF poly);
    Circle      screenToModel(Circle c);

    QPointF     modelToScreen(QPointF pt);
    QLineF      modelToScreen(QLineF line);
    QRectF      modelToScreen(QRectF rect);
    QPolygonF   modelToScreen(QPolygonF poly);
    Circle      modelToScreen(Circle c);

    QPointF     getCenterScreenUnits();
    QPointF     getCenterModelUnits();
    
    virtual void unloadLayerContent() {}

    virtual const Xform &   getModelXform();
    virtual void            setModelXform(const Xform & xf, bool update, uint sigid);
    
    virtual QTransform  getCanvasTransform();
    virtual QTransform  getModelTransform();
    virtual QTransform  getLayerTransform();
            QTransform  getLayerInvertedTransform()  { return invertedLayer; }

    virtual QString layerName()         { return _name; }

    bool        correctLegacyLayer(QPointF mpt);

    void        setZValue(int z);
    int         zValue()                { return zlevel; }

    void        setVisible(bool enb)    { visible = enb; }
    bool        isVisible()             { return visible; }

    eViewType   viewType()              { return _viewType;  }
    eModelType  modelType()             { return _modelType; }

    void        breakaway(bool set);
    void        lock(bool set);
    void        solo(Layer * l, bool set);

    bool        isPrimary()             { return _modelType == PRIMARY; }
    bool        isBreakaway()           { return !_useSMX; }
    bool        isLocked()              { return (_lockState > 0); }
    bool        isSolo()                { return _solo; }

    void        addSubLayer(LayerPtr item);
    void        removeSubLayer(LayerPtr item);
    void        clearSubLayers() { subLayers.clear(); }

    LayerPtr    firstSubLayer()         { return subLayers.first(); }
    LayerPtr    geSubLayer(int index)   { return subLayers.at(index); }
    QVector<LayerPtr> & getSubLayers()  { return subLayers; }
    int         numSubLayers()          { return subLayers.size(); }

    void        resetPos() { pos = QPointF(); }
    void        setLoc(QPointF loc);
    QPointF     getLoc() { return pos; }

    void        setViewController(SystemViewController * vc) { _viewControl = vc; }
    SystemViewController * viewControl() { return _viewControl; }

    static bool sortByZlevel(LayerPtr s1, LayerPtr s2);
    static bool sortByZlevelP(Layer *  s1, Layer  * s2);

    static int  refs;

signals:
    void        sig_reconstructView();
    void        sig_updateView();

public slots:
    void        id_layer();

protected:
    virtual void drawLayerModelCenter(QPainter * painter);
    void         drawCenterSymbol(QPainter * painter, QPointF spt, QColor circleColor, QColor xColor);

    SystemViewController  * _viewControl;

    int         debug;

    eViewType   _viewType;
    Xform       xf_model;           // used when PRIMARY

private:
    void        _setUserLock(bool set);
    void        _setSoloLock(bool set);
    void        _setSolo(bool set)       { _solo = set;  }
    void        _setBreakaway(bool set)  { _useSMX = !set;  }

    void        connectSignals();
    void        computeLayerTransform();

    QTransform  layerTransform;     // calculated
    QTransform  invertedLayer;      // calculated

    bool        visible;
    int         zlevel;

    eModelType  _modelType;     // PRIMARY or DERIVED
    bool        _useSMX;        // System Model Xform true: use xf_model, false: use local xform;
    bool        _solo;
    uint        _lockState;

    QString     _name;

    QVector<LayerPtr>  subLayers;   // for legacy code
    QPointF     pos;                // for legacy code

};


#endif
