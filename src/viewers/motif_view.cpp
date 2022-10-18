#include "viewers/motif_view.h"
#include "motifs/extended_rosette.h"
#include "motifs/extended_star.h"
#include "motifs/motif.h"
#include "motifs/inference_engine.h"
#include "motifs/radial_motif.h"
#include "geometry/arcdata.h"
#include "geometry/edge.h"
#include "geometry/map.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "makers/motif_maker/motif_button.h"
#include "makers/motif_maker/motif_maker.h"
#include "mosaic/design_element.h"
#include "mosaic/prototype.h"
#include "settings/configuration.h"
#include "tile/tile.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"
#include "viewers/viewcontrol.h"

using std::make_shared;

typedef std::shared_ptr<RadialMotif>    RadialPtr;

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

#ifdef INVERT_MAP
    // invert
    painter->translate(0,view->height());
    painter->scale(1.0, -1.0);
#endif

    QTransform baseT;

    if (config->motifEnlarge)
    {
        ViewControl * view = ViewControl::getInstance();
        baseT = MotifButton::resetViewport(-2,del,view->rect()) * getCanvasTransform();
    }
    else
    {
        baseT  = getLayerTransform();
    }

    auto proto  = motifMaker->getSelectedPrototype();
    auto tiling = proto->getTiling();
    for (auto del : motifMaker->getSelectedDesignElements())
    {
        auto motif  = del->getMotif();     // for now just a single motif
        auto tile   = del->getTile();
        //qDebug() << "MotifView  this=" << this << "del:" << _dep.get() << "fig:" << _fig.get();

        QTransform placement;
        if (!config->motifEnlarge)
        {
            placement = tiling->getPlacement(tile,0);
        }

        _T = placement * baseT;
        //qDebug().noquote() << "MotifView::transform" << Transform::toInfoString(_T);

        //fig->buildExtBoundary();

        // paint boundaries
        if (config->showExtendedBoundary)
        {
            painter->setPen(QPen(Qt::yellow,3.0));
            paintExtendedBoundary(painter,motif);
        }

        if (config->showTileBoundary)
        {
            painter->setPen(QPen(Qt::magenta,2.0));
            paintTileBoundary(painter,tile);
        }

        if (config->showMotifBoundary)
        {
            painter->setPen(QPen(Qt::red,1.0));
            paintMotifBoundary(painter,motif);
        }

        // paint figure
        if (config->showMotif)
        {
            painter->setPen(QPen(Qt::blue,2.0));
            if (motif->isRadial())
            {
                paintRadialMotifMap(painter,motif);
            }
            else
            {
                paintExplicitMotifMap(painter, motif);
            }
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
                ContactPtr c = debugContactPts.at(idx);
                painter->drawLine(_T.map(c->other), _T.map(c->position));
            }
        }
    }
}

void MotifView::paintExplicitMotifMap(QPainter *painter, MotifPtr fig)
{
    qDebug() << "paintExplicitMotif<ap" << fig->getMotifDesc();

    MapPtr map = fig->getMap();
    if (map)
    {
        //map->verifyMap( "paintExplicitFigure");
        paintMap(painter,map);
    }

    DebugMapPtr dmap = fig->getDebugMap();
    {
        map = std::dynamic_pointer_cast<Map>(dmap);
        if (map)
        {
            QColor color = (config->motifBkgdWhite) ? Qt::black : Qt::white;
            painter->setPen(QPen(color,1.0));
            paintMap(painter,map);
        }
    }
}

void MotifView::paintRadialMotifMap(QPainter *painter, MotifPtr fig)
{
    qDebug() << "paintRadialMotifMap" << fig->getMotifDesc();

    // Optimize for the case of a RadialFigure.
    RadialPtr rp = std::dynamic_pointer_cast<RadialMotif>(fig);

    MapPtr map = rp->getMap();
    paintMap(painter,map);

    DebugMapPtr dmap = rp->getDebugMap();
    if (dmap)
    {
        map = std::dynamic_pointer_cast<Map>(dmap);
        if (map)
        {
            QColor color = (config->motifBkgdWhite) ? Qt::black : Qt::white;
            painter->setPen(QPen(color,1.0));
            paintMap(painter,map);
        }
    }

    if (config->highlightUnit)
    {
        map = rp->getUnitMap();
        painter->setPen(QPen(Qt::red,3.0));
        paintMap(painter,map);
    }
}

void MotifView::paintTileBoundary(QPainter *painter,TilePtr feat)
{
    // draw feature
    // qDebug() << "scale" << feat->getScale();
    //qDebug().noquote() << feat->toBaseString();
    //qDebug().noquote() << feat->toString();
    EdgePoly  ep = feat->getEdgePoly();
    ep.paint(painter,_T);
}

void MotifView::paintMotifBoundary(QPainter *painter, MotifPtr fig)
{
    // show boundaries
    QPolygonF p = fig->getMotifBoundary();
    painter->drawPolygon(_T.map(p));
}

void MotifView::paintExtendedBoundary(QPainter *painter,MotifPtr fig)
{
    const ExtendedBoundary & eb = fig->getExtendedBoundary();

    if (!eb.isCircle())
    {
        const QPolygonF & p = eb.get();
        painter->drawPolygon(_T.map(p));
    }
    else
    {
        qreal radius = eb.scale * Transform::scalex(_T);
        painter->drawEllipse(_T.map(QPointF(0,0)),radius,radius);
    }
}

void MotifView::paintMap(QPainter * painter, MapPtr map)
{
    //map->verify("figure", true, true, true);
    qDebug() << "MotifView::paintMap" <<  map->namedSummary();
    for (auto & edge : qAsConst(map->getEdges()))
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

    QFont font = painter->font();
    font.setPixelSize(14);
    painter->setFont(font);
    const QVector<QPair<QPointF,QString>> & texts = map->getTexts();
    for (auto & pair : texts)
    {

        QPointF pt  = pair.first;
        QString txt = pair.second;
#ifdef INVERT_VIEW
        painter->save();
        painter->scale(1.0, -1.0);
        QTransform t2 = _T * QTransform().scale(1.0,-1.0);
        pt = t2.map(pt);
        painter->drawText(QPointF(pt.x()+7,pt.y()+13),txt);
        painter->restore();
#else
        pt = _T.map(pt);
        painter->drawText(QPointF(pt.x()+7,pt.y()+13),txt);
#endif
    }
}

void MotifView::setDebugContacts(bool enb, QPolygonF pts, QVector<ContactPtr> contacts)
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

