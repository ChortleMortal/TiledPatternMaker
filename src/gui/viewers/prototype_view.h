#ifndef PRTOTOYPE_VIEW
#define PRTOTOYPE_VIEW

#include "gui/viewers/layer_controller.h"
#include "model/prototypes/design_element.h"
#include "sys/geometry/edge_poly.h"

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

     void iamaLayerController() override {}

public slots:
     void slot_mousePressed(QPointF spt, Qt::MouseButton btn) override;
     void slot_mouseDragged(QPointF spt)       override;
     void slot_mouseMoved(QPointF spt)         override;
     void slot_mouseReleased(QPointF spt)      override;
     void slot_mouseDoublePressed(QPointF spt) override;

protected:
    void   paint(QPainter *painter) override;

private:
    void   draw();
    void   drawProto(const int mode, ProtoPtr proto);
    void   drawDEL(  const int mode, ProtoPtr proto, DELPtr del);

    GeoGraphics    * gg;

    ProtoViewColors colors;
    qreal           lineWidth;
};

#endif
