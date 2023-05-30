#ifndef PRTOTOYPE_VIEW
#define PRTOTOYPE_VIEW

#include "misc/layer_controller.h"
#include "geometry/edgepoly.h"
#include "mosaic/design_element.h"
#include "makers/prototype_maker/prototype_maker.h"

typedef std::shared_ptr<class Prototype>            ProtoPtr;

enum eProtoViewMode
{
    PROTO_DRAW_MAP      =  0x01,
    PROTO_ALL_TILES     =  0x02,
    PROTO_ALL_MOTIFS    =  0x04,
    PROTO_DEL_TILES     =  0x08,
    PROTO_DEL_MOTIFS    =  0x10,
    PROTO_DRAW_PROTO    =  0x20,
    PROTO_ALL_VISIBLE   =  0x40,
    PROTO_VISIBLE_TILE  =  0x80,
    PROTO_VISIBLE_MOTIF =  0x100
};

class ProtoViewColors
{
public:
    ProtoViewColors();

    QStringList     getColors();
    void            setColors(QStringList & colors);

    QColor          mapColor;
    QColor          tileColor;
    QColor          motifColor;
    QColor          delMotifColor;
    QColor          delTileColor;
    QColor          tileBrushColor;
    QColor          visibleTileColor;
    QColor          visibleMotifColor;
};

class PrototypeView : public LayerController
{
public:
    static PrototypeView * getInstance();
    static void            releaseInstance();

    ProtoViewColors & getColors()                    { return colors; }

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

private:
    PrototypeView();
    ~PrototypeView();

    static PrototypeView * mpThis;

    void   draw();
    void   drawProto(ProtoPtr proto);


    PrototypeMaker * protoMaker;
    GeoGraphics    * gg;

    EdgePoly        edges;              // this is not really an EdgePoly it is a vector of Edges
    ProtoViewColors colors;
    qreal           lineWidth;
    int             mode;
};

#endif
