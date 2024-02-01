#pragma once
#ifndef MEASURE_VIEW_H
#define MEASURE_VIEW_H

#include "style/thick.h"
#include "geometry/measurement.h"

typedef std::shared_ptr<class Measure2> Measure2Ptr;

class MeasureView : public Thick
{
public:
    static MeasureView * getInstance();
    static void          releaseInstance();

    void clear() { measurements.clear(); }

    void draw(GeoGraphics * gg ) override;

    void resetStyleRepresentation()  override {};
    void createStyleRepresentation() override {};

    virtual eViewType iamaLayer() override { return VIEW_MEASURE; };
    virtual QString getStyleDesc() const override { return "Measurer"; }

    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseReleased(QPointF spt)      override;

protected:

private:
    MeasureView(ProtoPtr pp);
    ~MeasureView();

    static MeasureView * mpThis;

    Configuration * config;

    Measure2Ptr           measurement;
    QVector<Measurement*> measurements;
};

class Measure2
{
public:
    Measure2(Layer * layer, QPointF spt, Measurement * m);
    void updateDragging(QPointF spt );
    void draw( GeoGraphics * g2d );
    void endDragging(QPointF spt );

protected:
    QLineF normalVectorA(QLineF line);
    QLineF normalVectorB(QLineF line);

private:
    Measurement * m;
    Layer       * layer;
    QLineF        sPerpLine; // perpendicular line
};

#endif // MEASURE_VIEW_H
