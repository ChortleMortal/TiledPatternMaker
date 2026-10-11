#pragma once

#include <QObject>
#include <QPen>
#include "sys/geometry/xform.h"
#include "sys/geometry/circle.h"

typedef std::shared_ptr<class Layer> LayerPtr;

#define DEBUG_XFORM 0x01
#define DEBUG_TFORM 0x02
#define DEBUG_LAYER 0x04

class ViewController;
class Configuration;

#define USER_LOCK 0x01
#define SOLO_LOCK 0x02

class Layer : public QObject
{
    Q_OBJECT

public:
    enum class Type
    {
        LegacyDesign,
        Mosaic,
        Prototype,
        MotifMaker,
        Tiling,
        TilingMaker,
        MapEditor,
        BkgdImage,
        Grid,
        ConstructionLines,
        Crop,
        DebugLayer,
        BMPImage
    };

    enum class Model
    {
        Primary,        // VIEW_MOSAIC,VIEW_TILING,VIEW_BKGD_IMAGE & VIEW_BMP_IMAGE
        Derived         // all other followers
    };

    enum class ZLevel
    {
        Bkgd_minus20    = -20,
        Standard_Z0     = 0,
        Image_Z1        = 1,
        Z1              = 1,
        Z2              = 2,
        Z3              = 3,
        Z4              = 4,
        Z5              = 5,
        Grid_Z5         = 5,
        Measure_Z9      = 9,
        Border_Z10      = 10,
        Crop_Z11        = 11,
        BkgdLifted_Z25  = 25,
    };

    Layer(Type type, Model model, ZLevel zlevel, QString name);
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
    QRect       screenToModel(QRect rect);
    QRectF      screenToModel(QRectF rect);
    qreal       screenToModel(qreal val);
    QPolygonF   screenToModel(QPolygonF poly);
    Circle      screenToModel(Circle c);

    QPointF     modelToScreen(QPointF pt);
    QLineF      modelToScreen(QLineF line);
    QRect       modelToScreen(QRect rect);
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
    QTransform          modelTransformOf(const Xform & xf);
    virtual QTransform  getLayerTransform();
    QTransform          getUndistortedLayerTransform();

    virtual QString layerName()         { return _name; }

    bool        correctLegacyCenter(QPointF mpt);

    void        setZLevel(ZLevel z)     { _zlevel = z; }
    ZLevel      getZLevel()             { return _zlevel; }

    void        setOpacity(qreal o)     { _opacity = o; }
    qreal       getOpacity()            { return _opacity; }

    void        setVisible(bool enb)    { visible = enb; }
    bool        isVisible()             { return visible; }

    Type        type()                  { return _type;  }
    Model       model()                 { return _model; }

    void        breakaway(bool set);
    void        lock(bool set);
    void        solo(Layer * l, bool set);

    bool        isPrimary()             { return _model == Model::Primary; }
    bool        isBreakaway()           { return !_useSMX; }
    bool        isLocked()              { return (_lockState > 0); }
    bool        isUserLocked()          { return (_lockState & USER_LOCK); }
    bool        isSolo()                { return _solo; }

    bool        isClipable()            { return _clipable; }
    void        setClipable(bool set)   { _clipable = set; }

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

    void        setViewController(ViewController * vc) { _viewControl = vc; }
    ViewController * viewControl() { return _viewControl; }

    static bool sortByZlevel(LayerPtr s1, LayerPtr s2);
    static bool sortByZlevelP(Layer *  s1, Layer  * s2);

    void        setLegacyCenter(QPointF p) { _legacyCenter = p; centerConv = true;}
    QPointF     getLegacyCenter()          { return _legacyCenter; }

    static int  refs;

signals:
    void        sig_reconstructView();
    void        sig_updateView();

public slots:
    void        id_layer();

protected:
    virtual void drawLayerModelCenter(QPainter * painter);
    void         drawCenterSymbol(QPainter * painter, QPointF spt, QColor circleColor, QColor xColor);

    ViewController  * _viewControl;

    int         debug;

    Type        _type;
    Xform       xf_model;           // used when PRIMARY
    QPointF     _legacyCenter;
    QColor      bkgdColor;

private:
    void        _setUserLock(bool set);
    void        _setSoloLock(bool set);
    void        _setSolo(bool set)       { _solo = set;  }
    void        _setBreakaway(bool set)  { _useSMX = !set;  }

    void        connectSignals();
    void        computeLayerTransform();

    QTransform  layerTransform;     // calculated

    bool        visible;
    ZLevel      _zlevel;
    qreal       _opacity;           // session only, not persisted

    Model       _model;             // Primary or Derived
    bool        _useSMX;            // System Model Xform true: use xf_model, false: use local xform;
    bool        _solo;
    uint        _lockState;

    bool        _clipable;

    QString     _name;

    QVector<LayerPtr>  subLayers;   // for legacy code
    QPointF     pos;                // for legacy code
    bool        centerConv;         // center convert for legacty code

};

constexpr int NUM_VIEW_TYPES = static_cast<int>(Layer::Type::BMPImage) + 1;

extern const QString sLayerType[];
extern const QString sLayerType2[];




