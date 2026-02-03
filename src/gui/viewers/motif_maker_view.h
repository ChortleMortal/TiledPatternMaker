#pragma once
#ifndef MOTIF_MAKER_VIEW_H
#define MOTIF_MAKER_VIEW_H

#include "gui/viewers/layer_controller.h"

typedef std::shared_ptr<class Motif>           MotifPtr;
typedef std::shared_ptr<class RadialMotif>     RadialPtr;
typedef std::shared_ptr<class DesignElement>   DELPtr;
typedef std::shared_ptr<class Map>             MapPtr;
typedef std::shared_ptr<class Tile>            TilePtr;
typedef std::shared_ptr<class Contact>         ContactPtr;

typedef QVector<DELPtr>   DesignUnit;

class MotifMakerView : public LayerController
{
public:
    MotifMakerView();
    virtual ~MotifMakerView() override;

    void paint(QPainter *painter) override;

    void iamaLayerController() override {}

    QTransform getLayerTransform() override;

public slots:
    void slot_mousePressed(QPointF spt, Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;

protected:
    void paintExplicitMotifMap( QPainter *painter, MotifPtr motif, QTransform & tr);
    void paintRadialMotifMap(   QPainter *painter, RadialPtr rp,   QTransform & tr);
    void paintMotifBoundary(    QPainter *painter, MotifPtr motif, QTransform & tr);
    void paintExtendedBoundary( QPainter *painter, MotifPtr motif, QTransform & tr);
    void paintTileBoundary(     QPainter *painter, TilePtr tile,   QTransform & tr);
    void paintMap(              QPainter *painter, MapPtr map,     QTransform & tr);

private:
};

#endif
