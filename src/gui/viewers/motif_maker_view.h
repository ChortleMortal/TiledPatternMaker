#pragma once

#include "gui/viewers/layer_controller.h"

typedef std::shared_ptr<class Motif>           MotifPtr;
typedef std::shared_ptr<class RadialMotif>     RadialPtr;
typedef std::shared_ptr<class TileMotif>       TileMotifPtr;
typedef std::shared_ptr<class Map>             MapPtr;
typedef std::shared_ptr<class Tile>            TilePtr;
typedef std::shared_ptr<class Contact>         ContactPtr;

typedef QVector<TileMotifPtr>   TileMotifSet;

class MotifMakerView : public LayerController
{
    Q_OBJECT

public:
    enum class ViewType
    {
        Single      = 0,
        Multiple    = 1,
        All         = 2,  // the whole design unit
        Undefined   = 3
    };

    MotifMakerView();
    virtual ~MotifMakerView() override;

    void paint(QPainter *painter) override;

    void iamaLayerController() override {}

    QTransform getUsedTransform() { return usedTransform; }

    void     setViewMode(ViewType newMode);
    ViewType getViewMode()                  { return viewMode;}

    void     enlarge(TileMotifPtr timo);

    void     setDontAutoFit(bool checked);
    bool     getDontAutoFit()                   { return dontAutoFit; }

    void     setLayerEnabled(bool enb);

public slots:
    void slot_mousePressed(QPointF spt, Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;

signals:
    void sig_delegateTileMotif(TileMotifPtr timo);

protected:
    void paintExplicitMotifMap( QPainter *painter, MotifPtr motif, QTransform & tr);
    void paintRadialMotifMap(   QPainter *painter, RadialPtr rp,   QTransform & tr);
    void paintMotifBoundary(    QPainter *painter, MotifPtr motif, QTransform & tr);
    void paintExtendedBoundary( QPainter *painter, MotifPtr motif, QTransform & tr);
    void paintTileBoundary(     QPainter *painter, TilePtr tile,   QTransform & tr);
    void paintMap(              QPainter *painter, MapPtr map,     QTransform & tr);

private:
    bool getFitXform(Xform &xf, TileMotifPtr timo, qreal margin = 1.20);
    void updateSessionState();     // single choke point for prepare/restore transitions

    QTransform  usedTransform;
    QTransform  baseT;
    Xform       lastFit;                  // most recent Xform broadcast by enlarge()
    bool        dontAutoFit   = false;    // when set, suppresses recalculating the fit as content changes
    bool        layerEnabled  = true;     // mirrors the Layers panel enable state for this layer
    bool        sessionActive = false;    // true between a prepare and its matching restore
    ViewType    viewMode      = ViewType::Undefined;
};
