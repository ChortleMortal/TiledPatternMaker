#pragma once
#ifndef DEBUG_VIEW_H
#define DEBU_VIEW_H

#include "misc/layer_controller.h"

typedef std::shared_ptr<class Map>            MapPtr;
typedef std::shared_ptr<class DebugMap>       DebugMapPtr;

class DebugView : public LayerController
{
public:
    DebugView();
    ~DebugView();

    DebugMapPtr getMap() { return  dMap; }
    void setMap(DebugMapPtr map);

    void clear();

    void paint(QPainter * painter ) override;

    bool getShow()          { return _viewEnable; }
    bool getShowDirection() { return _showDirection; }
    bool getShowVertices()  { return _showVertices; }
    bool getShowArcCentres(){ return _showArcCentres; }

    void showDirection(bool show) { _showDirection  = show;  writeConfig(); }
    void showVertices(bool show)  { _showVertices   = show;  writeConfig(); }
    void showArcCentres(bool show){ _showArcCentres = show;  writeConfig(); }
    void show(bool show)          { _viewEnable     = show;  writeConfig(); }

    const Xform &   getModelXform() override;
     void           setModelXform(const Xform & xf, bool update) override;

protected:
    void draw(QPainter * painter);

    eViewType iamaLayer() override { return VIEW_GRID; }
    void iamaLayerController() override {}

    void readConfig();
    void writeConfig();

    static int refs;

public slots:
     void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
     void slot_mouseDragged(QPointF spt)       override;
     void slot_mouseTranslate(QPointF pt)      override;
     void slot_mouseMoved(QPointF spt)         override;
     void slot_mouseReleased(QPointF spt)      override;
     void slot_mouseDoublePressed(QPointF spt) override;
     void slot_setCenter(QPointF spt)          override;

     void slot_wheel_scale(qreal delta)  override;
     void slot_wheel_rotate(qreal delta) override;

     void slot_scale(int amount)  override;
     void slot_rotate(int amount) override;
     void slot_moveX(qreal amount)  override;
     void slot_moveY(qreal amount)  override;

private:


#define DVenable    0x01
#define DVvertices  0x02
#define DVcentres   0x04
#define DVdirn      0x08

    Configuration    * config;
    DebugMapPtr      dMap;

    bool _showVertices;
    bool _showDirection;
    bool _showArcCentres;
    bool _viewEnable;
};
#endif
