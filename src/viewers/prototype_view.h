#ifndef PRTOTOYPE_VIEW
#define PRTOTOYPE_VIEW

#include "misc/layer_controller.h"
#include "geometry/edgepoly.h"
#include "mosaic/design_element.h"

typedef std::shared_ptr<class PrototypeView>        PrototypeViewPtr;
typedef std::shared_ptr<class Prototype>            PrototypePtr;
typedef std::shared_ptr<class PlacedDesignElement>  PlacedDesignElementPtr;

typedef std::weak_ptr<class DesignElement>    WeakDesignElementPtr;
typedef std::weak_ptr<class Prototype>        WeakPrototypePtr;
typedef std::weak_ptr<class Motif>            WeakMotifPtr;
typedef std::weak_ptr<class Tile>             WeakTilePtr;

class sColData
{
public:
    void reset() { wpp.reset(); wdel.reset(); wtilep.reset(); wmotifp.reset(); }
    WeakPrototypePtr     wpp;
    WeakDesignElementPtr wdel;
    WeakTilePtr          wtilep;
    WeakMotifPtr         wmotifp;
};

class ProtoViewColors
{
public:
    QStringList     getColors();
    void            setColors(QStringList & colors);

    QColor          mapColor;
    QColor          tileColor;
    QColor          motifColor;
    QColor          delMotifColor;
    QColor          delTileColor;
    QColor          tileBrushColor;
};

class PlacedDesignElement : public DesignElement
{
public:
    PlacedDesignElement(const TilePtr & featp, const MotifPtr & figp, QTransform T)
    { tile = featp; motif  = figp; trans   = T; }

    QTransform  getTransform() const { return trans; }

protected:
    QTransform  trans;
};

class PrototypeView : public LayerController
{
public:
    static PrototypeViewPtr getSharedInstance();
    PrototypeView();        // don't use this

    void setPrototype(PrototypePtr proto) { this->proto = proto; }
    PrototypePtr getPrototype() {return proto; }

    void selectDEL(sColData sdata) { selectedDEL = sdata; }
    sColData & getSelectedDEL() { return selectedDEL; }

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

    void   drawPlacedDesignElement(GeoGraphics * gg, const PlacedDesignElement &pde, QPen motifPen, QBrush tileBrush, QPen tilePen, bool selected);

    PrototypePtr    proto;
    EdgePoly        edges;              // this is not really an EdgePoly it is a vector of Edges
    ProtoViewColors colors;
    qreal           lineWidth;

    sColData        selectedDEL;

private:
    static PrototypeViewPtr spThis;
};
#endif
