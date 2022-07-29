#include "viewers/motif_view.h"
#include "figures/extended_rosette.h"
#include "figures/extended_star.h"
#include "figures/figure.h"
#include "figures/infer.h"
#include "figures/radial_figure.h"
#include "geometry/arcdata.h"
#include "geometry/edge.h"
#include "geometry/map.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "makers/motif_maker/feature_button.h"
#include "makers/motif_maker/motif_maker.h"
#include "mosaic/design_element.h"
#include "mosaic/prototype.h"
#include "settings/configuration.h"
#include "tile/feature.h"
#include "tile/placed_feature.h"
#include "tile/tiling.h"
#include "viewers/viewcontrol.h"

using std::make_shared;

typedef std::shared_ptr<RadialFigure>    RadialPtr;

MotifViewPtr MotifView::spThis;

MotifViewPtr MotifView::getSharedInstance()
{
    if (!spThis)
    {
        spThis = make_shared<MotifView>();
    }
    return spThis;
}

MotifView::MotifView() : LayerController("Motif View")
{
    motifMaker = MotifMaker::getInstance();

    debugContacts = false;
}

MotifView::~MotifView()
{
    //qDebug() << "MotifView destructor";
}

void MotifView::paint(QPainter *painter)
{
    qDebug() << "MotifView::paint";

    DesignElementPtr del = motifMaker->getSelectedDesignElement();
    if (!del)
    {
        qDebug() << "MotifView - no design element";
        return;
    }

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    //painter->translate(0,view->height());
    //painter->scale(1.0, -1.0);

    QTransform baseT;

    if (config->motifEnlarge)
    {
        ViewControl * view = ViewControl::getInstance();
        baseT = FeatureButton::resetViewport(-2,del,view->rect());
    }
    else
    {
        baseT  = getLayerTransform();
    }

    auto proto  = motifMaker->getSelectedPrototype();
    auto tiling = proto->getTiling();
    for (auto del : motifMaker->getSelectedDesignElements())
    {
        auto fig    = del->getFigure();     // for now just a single figure
        auto feat   = del->getFeature();
        //qDebug() << "MotifView  this=" << this << "del:" << _dep.get() << "fig:" << _fig.get();

        QTransform placement;
        if (!config->motifEnlarge)
        {
            placement = tiling->getPlacement(feat,0);
        }

        _T = placement * baseT;
        //qDebug().noquote() << "MotifView::transform" << Transform::toInfoString(_T);

        //fig->buildExtBoundary();

        // paint boundaries
        if (config->showExtendedBoundary)
        {
            painter->setPen(QPen(Qt::yellow,3.0));
            paintExtendedBoundary(painter,fig,feat);
        }

        if (config->showFeatureBoundary)
        {
            painter->setPen(QPen(Qt::magenta,2.0));
            paintFeatureBoundary(painter,feat);
        }

        if (config->showFigureBoundary)
        {
            painter->setPen(QPen(Qt::red,1.0));
            paintRadialFigureBoundary(painter,fig);
        }

        // paint figure
        if (fig->isRadial())
        {
            paintRadialFigureMap(painter,fig, QPen(Qt::blue, 2.0));
        }
        else
        {
            paintExplicitFigureMap(painter, fig, QPen(Qt::blue, 2.0));
        }

        drawCenter(painter);

        if (debugContacts)
        {
            painter->setPen(Qt::white);
            for( int idx = 0; idx < debugPts.size(); ++idx )
            {
                painter->drawLine( _T.map(debugPts[idx]), _T.map(debugPts[ (idx+1) % debugPts.size()]) );
            }

            painter->setPen(Qt::blue);
            for( int idx = 0; idx < debugContactPts.size(); ++idx )
            {
                contact * c = debugContactPts.at(idx);
                painter->drawLine(_T.map(c->other), _T.map(c->position));
            }
        }
    }
}

void MotifView::paintExplicitFigureMap(QPainter *painter, FigurePtr fig, QPen pen)
{
    qDebug() << "paintExplicitFigure" << fig->getFigureDesc();

    MapPtr map = fig->getFigureMap();
    if (map)
    {
        //map->verifyMap( "paintExplicitFigure");
        paintMap(painter,map,pen);
    }
}

void MotifView::paintRadialFigureMap(QPainter *painter, FigurePtr fig, QPen pen)
{
    qDebug() << "paintRadialFigure" << fig->getFigureDesc();

    // Optimize for the case of a RadialFigure.
    RadialPtr rp = std::dynamic_pointer_cast<RadialFigure>(fig);

    MapPtr map = rp->useBuiltMap();
    if (!map || map->isEmpty())
    {
        map = rp->getFigureMap();
    }

    if (map)
    {
        paintMap(painter,map,pen);
    }

    map = rp->useDebugMap();
    if (map)
    {
        paintMap(painter,map,QPen(Qt::white,1.0));
    }

    if (config->highlightUnit)
    {
        map = rp->useUnitMap();
        paintMap(painter,map,QPen(Qt::red,3.0));
    }
}

void MotifView::paintFeatureBoundary(QPainter *painter,FeaturePtr feat)
{
    // draw feature
    // qDebug() << "scale" << feat->getScale();
    //qDebug().noquote() << feat->toBaseString();
    //qDebug().noquote() << feat->toString();
    EdgePoly  ep = feat->getEdgePoly();
    ep.paint(painter,_T);
}

void MotifView::paintRadialFigureBoundary(QPainter *painter, FigurePtr fig)
{
    // show boundaries
    QPolygonF p1        = fig->getRadialFigBoundary();
    painter->drawPolygon(_T.map(p1));
}

void MotifView::paintExtendedBoundary(QPainter *painter,FigurePtr fig, FeaturePtr feat)
{
    QPolygonF p2        = fig->getExtBoundary();
    bool drawCircle     = fig->hasExtCircleBoundary();
    qreal boundaryScale = fig->getExtBoundaryScale();

    QPointF center      = feat->getCenter();
    QTransform ft       = QTransform::fromTranslate(center.x(), center.y());
    p2                  = ft.map(p2);

    if (!drawCircle)
    {
        painter->drawPolygon(_T.map(p2));
    }
    else
    {
        qreal scale = Transform::scalex(_T);
        painter->drawEllipse(_T.map(QPointF(0,0)),boundaryScale*scale,boundaryScale*scale);
    }
}

void MotifView::paintMap(QPainter * painter, MapPtr map, QPen pen)
{
    //map->verify("figure", true, true, true);

    painter->setPen(pen);

    for (auto edge : map->getEdges())
    {
        QPointF p1 = _T.map(edge->v1->pt);
        QPointF p2 = _T.map(edge->v2->pt);

        if (edge->getType() == EDGETYPE_LINE)
        {
            painter->drawLine(p1,p2);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            QPointF arcCenter = _T.map(edge->getArcCenter());
            ArcData ad(p1,p2,arcCenter,edge->isConvex());
            painter->drawArc(ad.rect, qRound(ad.start * 16.0),qRound(ad.span*16.0));
        }
        else if (edge->getType() == EDGETYPE_CHORD)
        {
            QPointF arcCenter = _T.map(edge->getArcCenter());
            ArcData ad(p1,p2,arcCenter,edge->isConvex());
            painter->drawChord(ad.rect, qRound(ad.start * 16.0),qRound(ad.span*16.0));
        }
    }

    for (const auto & stxt : map->getTexts())
    {
        painter->save();
        painter->scale(1.0, -1.0);
        QTransform t2 = _T * QTransform().scale(1.0,-1.0);
        QPointF pt = t2.map(stxt.pt);
        painter->drawText(QPointF(pt.x()+7,pt.y()+13),stxt.txt);
        painter->restore();
    }
}

void MotifView::setDebugContacts(bool enb, QPolygonF pts, QVector<contact*> contacts)
{
    debugContacts   = enb;
    debugPts        = pts;
    debugContactPts = contacts;
}

void MotifView::slot_setCenter(QPointF spt)
{
    if (config->getViewerType() == VIEW_MOTIF_MAKER)
    {
        setCenterScreenUnits(spt);
    }
}

void MotifView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{ Q_UNUSED(spt); Q_UNUSED(btn); }
void MotifView::slot_mouseDragged(QPointF spt)
{ Q_UNUSED(spt); }
void MotifView::slot_mouseMoved(QPointF spt)
{ Q_UNUSED(spt); }
void MotifView::slot_mouseReleased(QPointF spt)
{ Q_UNUSED(spt); }
void MotifView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

