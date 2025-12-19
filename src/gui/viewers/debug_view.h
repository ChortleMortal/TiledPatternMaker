#pragma once
#ifndef DEBUG_VIEW_H
#define DEBU_VIEW_H

#include "gui/viewers/layer_controller.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/measurement.h"

class GeoGraphics;

typedef std::shared_ptr<class DebugViewMeasure> Measure3Ptr;

class DebugView : public LayerController
{
    Q_OBJECT

public:
    DebugView();
    ~DebugView();

    void unloadLayerContent() override   { Sys::debugMapCreate->wipeout(); Sys::debugMapPaint->wipeout(); measurements.clear(); }
    void clearMeasurements() { measurements.clear(); }

    void paint(QPainter * painter ) override;

    bool canMeasure()            { return _canMeasure; }
    void setCanMeasure(bool set) { _canMeasure = set; measurePt = QPointF(); }

    bool getShowLines()     { return _showLines; }
    bool getShowDirection() { return _showDirection; }
    bool getShowArcCentres(){ return _showArcCentres; }
    bool getShowPoints()    { return _showPoints; }
    bool getShowMarks()     { return _showMarks; }
    bool getShowCurves()    { return _showCurves; }
    bool getShowCircles()   { return _showCircles; }

    void showLines(bool show)     { _showLines      = show;  writeConfig(); }
    void showDirection(bool show) { _showDirection  = show;  writeConfig(); }
    void showArcCentres(bool show){ _showArcCentres = show;  writeConfig(); }
    void showPoints(bool show)    { _showPoints     = show;  writeConfig(); }
    void showMarks(bool show)     { _showMarks      = show;  writeConfig(); }
    void showCurves(bool show)    { _showCurves     = show;  writeConfig(); }
    void showCircles(bool show)   { _showCircles    = show;  writeConfig(); }

    void do_testA(bool enb);
    void do_testB(bool enb);

    void execTestA(QPainter * painter, QTransform tr);
    void execTestB(QPainter * painter, QTransform tr);

protected:
    void iamaLayerController() override {}

    void readConfig();
    void writeConfig();

    static int refs;

public slots:
    void slot_mousePressed(QPointF spt, Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;

private:
    Measure3Ptr           measurement;
    QVector<Measurement*> measurements;
    QPointF               measurePt;

    bool _canMeasure;
    bool doTestA;
    bool doTestB;

#define DVlines     0x01
#define DVcentres   0x02
#define DVdirn      0x04
#define DVpoints    0x08
#define DVmarks     0x10
#define DVcircles   0x20
#define DVcurves    0x40

    bool _showLines;
    bool _showDirection;
    bool _showArcCentres;
    bool _showPoints;
    bool _showMarks;
    bool _showCircles;
    bool _showCurves;
};

class DebugViewMeasure
{
public:
    DebugViewMeasure(Layer * layer, QPointF spt, Measurement * m);
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

#endif
