#pragma once
#ifndef MOTIFVIEW_H
#define MOTIFVIEW_H

#include "misc/layer_controller.h"

typedef std::shared_ptr<class Motif>           MotifPtr;
typedef std::shared_ptr<class DesignElement>   DesignElementPtr;
typedef std::shared_ptr<class Map>             MapPtr;
typedef std::shared_ptr<class Tile>            TilePtr;
typedef std::shared_ptr<class Contact>         ContactPtr;

class PrototypeData;

class MotifView : public LayerController
{
public:
    static MotifView * getInstance();
    static void        releaseInstance();

    virtual void paint(QPainter *painter) override;


    virtual void iamaLayer() override {}
    virtual void iamaLayerController() override {}

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;
    virtual void slot_setCenter(QPointF spt)          override;

protected:
    void paintExplicitMotifMap( QPainter *painter, MotifPtr motif);
    void paintRadialMotifMap(   QPainter *painter, MotifPtr motif);
    void paintMotifBoundary(    QPainter *painter, MotifPtr motif);
    void paintExtendedBoundary( QPainter *painter, MotifPtr motif);
    void paintTileBoundary(     QPainter *painter, TilePtr tile);
    void paintMap(              QPainter *painter, MapPtr map);

private:
    MotifView();
    virtual ~MotifView() override;

    static MotifView * mpThis;

    PrototypeData    * protoMakerData;
    QTransform         _T;
    qreal              lineWidth;


};

#endif
