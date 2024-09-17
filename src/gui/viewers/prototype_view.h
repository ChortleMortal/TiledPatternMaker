#ifndef PRTOTOYPE_VIEW
#define PRTOTOYPE_VIEW

#include "gui/viewers/layer_controller.h"
#include "model/makers/prototype_maker.h"
#include "model/prototypes/design_element.h"
#include "sys/geometry/edgepoly.h"

typedef std::shared_ptr<class Prototype> ProtoPtr;

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

    GeoGraphics    * gg;

    ProtoViewColors colors;
    qreal           lineWidth;
    int             mode;
};

#endif
