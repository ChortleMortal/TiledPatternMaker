#ifndef PRTOTOYPE_VIEW
#define PRTOTOYPE_VIEW

#include "misc/layer_controller.h"
#include "geometry/edgepoly.h"

typedef std::shared_ptr<class PrototypeView>        PrototypeViewPtr;
typedef std::shared_ptr<class Prototype>            PrototypePtr;
typedef std::shared_ptr<class PlacedDesignElement>  PlacedDesignElementPtr;

class ProtoViewColors
{
public:
    QStringList     getColors();
    void            setColors(QStringList & colors);

    QColor          mapColor;
    QColor          featureColor;
    QColor          figureColor;
    QColor          delFigureColor;
    QColor          delFeatureColor;
    QColor          featureBrushColor;
};

class PrototypeView : public LayerController
{
public:
    static PrototypeViewPtr getSharedInstance();
    PrototypeView();        // don't use this

    void setPrototype(PrototypePtr proto) { this->proto = proto; }
    PrototypePtr getPrototype() {return proto; }

    ProtoViewColors & getColors() { return colors; }

    virtual void iamaLayer() override {}
    virtual void iamaLayerController() override {}

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;

protected:
    void   paint(QPainter *painter) override;
    void   draw(GeoGraphics * gg);

    void   drawPlacedDesignElement(GeoGraphics * gg, const PlacedDesignElement &pde, QPen figurePen, QBrush featureBrush, QPen featurePen, bool selected);

    PrototypePtr    proto;
    EdgePoly        edges;              // this is not really an EdgePoly it is a vector of Edges
    ProtoViewColors colors;

private:
    static PrototypeViewPtr spThis;
};
#endif
