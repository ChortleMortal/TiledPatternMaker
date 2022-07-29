#ifndef MEASURE_VIEW_H
#define MEASURE_VIEW_H

#include "style/thick.h"
#include "geometry/measurement.h"

typedef std::shared_ptr<class MeasureView>      MeasViewPtr;
typedef std::shared_ptr<class Measure2>         Measure2Ptr;

class MeasureView : public Thick
{
public:
    static MeasViewPtr getSharedInstance();
    MeasureView(PrototypePtr pp);  // don't use this

    void setMeasureMode(bool mode);

    void draw(GeoGraphics * gg ) override;

    void resetStyleRepresentation()  override {};
    void createStyleRepresentation() override {};

    virtual QString getStyleDesc() const override { return "Measurer"; }

    virtual void  setCanvasXform(const Xform & xf) override { Q_UNUSED(xf);}

    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseReleased(QPointF spt)      override;

protected:

private:
    static MeasViewPtr spThis;

    Configuration * config;
    ViewControl   * view;

    bool                 measureMode;
    Measure2Ptr          measurement;
    QVector<MeasurementPtr> measurements;
};

class Measure2
{
public:
    Measure2(Layer * layer, QPointF spt, MeasurementPtr m);
    void updateDragging(QPointF spt );
    void draw( GeoGraphics * g2d );
    void endDragging(QPointF spt );

protected:
    QLineF normalVectorA(QLineF line);
    QLineF normalVectorB(QLineF line);

private:
    MeasurementPtr m;
    QLineF       sPerpLine; // perpendicular line
    Layer *      layer;
};

#endif // MEASURE_VIEW_H
