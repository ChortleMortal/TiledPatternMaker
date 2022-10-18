#ifndef MOTIFVIEW_H
#define MOTIFVIEW_H

#include "misc/layer_controller.h"

typedef std::shared_ptr<class MotifView>       MotifViewPtr;
typedef std::shared_ptr<class Motif>           MotifPtr;
typedef std::shared_ptr<class DesignElement>   DesignElementPtr;
typedef std::shared_ptr<class Map>             MapPtr;
typedef std::shared_ptr<class Tile>            TilePtr;
typedef std::shared_ptr<class Contact>         ContactPtr;


class MotifView : public LayerController
{
public:
    static MotifViewPtr getSharedInstance();
    MotifView();
    virtual ~MotifView() override;

    virtual void paint(QPainter *painter) override;

    void    setDebugContacts(bool enb, QPolygonF pts, QVector<ContactPtr> contacts);    // not currently used

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
    void paintExplicitMotifMap( QPainter *painter, MotifPtr fig);
    void paintRadialMotifMap(   QPainter *painter, MotifPtr fig);
    void paintMotifBoundary(    QPainter *painter, MotifPtr fig);
    void paintExtendedBoundary( QPainter *painter, MotifPtr fig);
    void paintTileBoundary(     QPainter *painter, TilePtr feat);

private:
    static MotifViewPtr spThis;

    void paintMap(QPainter * painter, MapPtr map);

    class MotifMaker * motifMaker;

    QTransform           _T;

    bool                debugContacts;
    QPolygonF           debugPts;
    QVector<ContactPtr> debugContactPts;
};

#endif
