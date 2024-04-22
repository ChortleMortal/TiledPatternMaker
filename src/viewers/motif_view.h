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
    MotifView();
    virtual ~MotifView() override;

    void paint(QPainter *painter) override;

    const Xform &   getModelXform() override;
    void            setModelXform(const Xform & xf, bool update) override;

    eViewType iamaLayer() override { return VIEW_MOTIF_MAKER; }
    void      iamaLayerController() override {}

public slots:
    void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;
    void slot_setCenter(QPointF spt)          override;

protected:
    void paintExplicitMotifMap( QPainter *painter, MotifPtr motif, QTransform & tr);
    void paintRadialMotifMap(   QPainter *painter, MotifPtr motif, QTransform & tr);
    void paintMotifBoundary(    QPainter *painter, MotifPtr motif, QTransform & tr);
    void paintExtendedBoundary( QPainter *painter, MotifPtr motif, QTransform & tr);
    void paintDebugMap(         QPainter *painter, MotifPtr motif, QTransform & tr);
    void paintTileBoundary(     QPainter *painter, TilePtr tile,   QTransform & tr);
    void paintMap(              QPainter *painter, MapPtr map,     QTransform & tr);

private:
    PrototypeData    * protoMakerData;
};

#endif
