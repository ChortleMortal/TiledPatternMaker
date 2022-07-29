#ifndef MOTIFVIEW_H
#define MOTIFVIEW_H

#include "misc/layer_controller.h"

typedef std::shared_ptr<class MotifView>        MotifViewPtr;
typedef std::shared_ptr<class Figure>           FigurePtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Feature>          FeaturePtr;
typedef std::weak_ptr<class DesignElement>      WeakDesignElementPtr;
typedef std::weak_ptr<class Figure>             WeakFigurePtr;
typedef std::weak_ptr<class Feature>            WeakFeaturePtr;

class MotifView : public LayerController
{
public:
    static MotifViewPtr getSharedInstance();
    MotifView();
    virtual ~MotifView() override;

    virtual void paint(QPainter *painter) override;

    void    setDebugContacts(bool enb, QPolygonF pts, QVector<class contact*> contacts);

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
    void paintExplicitFigureMap(QPainter *painter, FigurePtr fig, QPen pen);
    void paintRadialFigureMap(QPainter *painter, FigurePtr fig, QPen pen);
    void paintFeatureBoundary(QPainter *painter, FeaturePtr feat);
    void paintRadialFigureBoundary(QPainter *painter,FigurePtr fig);
    void paintExtendedBoundary(QPainter *painter, FigurePtr fig, FeaturePtr feat);

private:
    static MotifViewPtr spThis;

    void paintMap(QPainter * painter, MapPtr map, QPen pen);

    class MotifMaker * motifMaker;

    QTransform           _T;

    QTransform          lt;
    QPointF             pt_lt;
    qreal               scale_lt;
    qreal               rot_lt;

    bool                debugContacts;
    QPolygonF           debugPts;
    QVector<contact *>  debugContactPts;
};

#endif // FIGUREVIEW_H
