#ifndef PRTOTOYPE_VIEW
#define PRTOTOYPE_VIEW

#include "misc/layer_controller.h"
#include "geometry/edgepoly.h"
#include "mosaic/design_element.h"
#include "makers/prototype_maker/prototype_maker.h"

typedef std::shared_ptr<class Prototype>            ProtoPtr;

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
    PrototypeView();
    ~PrototypeView();

    ProtoViewColors & getColors()                    { return colors; }

     const Xform &   getModelXform() override;
     void            setModelXform(const Xform & xf, bool update) override;

     eViewType iamaLayer() override { return VIEW_PROTOTYPE; }
     void iamaLayerController() override {}

public slots:
     void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
     void slot_mouseDragged(QPointF spt)       override;
     void slot_mouseMoved(QPointF spt)         override;
     void slot_mouseReleased(QPointF spt)      override;
     void slot_mouseDoublePressed(QPointF spt) override;

protected:
    void   paint(QPainter *painter) override;

private:
    void   draw();
    void   drawProto(ProtoPtr proto);

    PrototypeMaker * protoMaker;
    GeoGraphics    * gg;

    ProtoViewColors colors;
    qreal           lineWidth;
    int             mode;
};

#endif
