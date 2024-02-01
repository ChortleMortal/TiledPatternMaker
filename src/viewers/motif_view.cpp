#include "viewers/motif_view.h"
#include "geometry/arcdata.h"
#include "geometry/edge.h"
#include "geometry/geo.h"
#include "geometry/map.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "makers/motif_maker/design_element_button.h"
#include "makers/motif_maker/motif_maker_widget.h"
#include "makers/prototype_maker/prototype.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "mosaic/design_element.h"
#include "motifs/inferred_motif.h"
#include "motifs/irregular_tools.h"
#include "motifs/motif.h"
#include "motifs/radial_motif.h"
#include "settings/configuration.h"
#include "tile/placed_tile.h"
#include "tile/tile.h"
#include "tile/tiling.h"
#include "viewers/view_controller.h"

using std::make_shared;

typedef std::shared_ptr<RadialMotif>    RadialPtr;

MotifView * MotifView::mpThis = nullptr;


///////////////////////////////////////////////////////////////////////////
///
///     This view paints all selected DELs in a selected Prototyp
///
///////////////////////////////////////////////////////////////////////////

MotifView * MotifView::getInstance()
{
    if (!mpThis)
    {
        mpThis = new MotifView();
    }
    return mpThis;
}

void MotifView::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

MotifView::MotifView() : LayerController("Motif View",false)
{
    protoMakerData = PrototypeMaker::getInstance()->getProtoMakerData();
    lineWidth     = config->motifViewWidth;

}

MotifView::~MotifView()
{
    //qDebug() << "MotifView destructor";
}

void MotifView::paint(QPainter *painter)
{
    qDebug() << "MotifView::paint";

#ifdef INVERT_MAP
    // invert
    painter->translate(0,view->height());
    painter->scale(1.0, -1.0);
#endif
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    lineWidth = config->motifViewWidth;

    QTransform baseT;
    DesignElementPtr del = protoMakerData->getSelectedDEL();
    if (config->motifEnlarge && del)
    {
        baseT = DesignElementButton::resetViewport(-2,del,view->rect()) * getModelTransform();
    }
    else
    {
        baseT  = getLayerTransform();
    }

    auto proto  = protoMakerData->getSelectedPrototype();
    if (!proto) return;

    auto tiling = proto->getTiling();
    for (const auto & del : protoMakerData->getSelectedDELs(MVD_DELEM))
    {
        auto motif  = del->getMotif();     // for now just a single motif
        auto tile   = del->getTile();
        //qDebug() << "MotifView  this=" << this << "del:" << _dep.get() << "fig:" << _fig.get();

        QTransform placement;
        if (!config->motifEnlarge)
        {
            placement = tiling->getFirstPlacement(tile);
        }

        _T = placement * baseT;
        //qDebug().noquote() << "MotifView::transform" << Transform::toInfoString(_T);

        // paint boundaries
        if (config->showTileBoundary)
        {
            painter->setPen(QPen(Qt::magenta,lineWidth));
            paintTileBoundary(painter,tile);
        }

        if (config->showMotifBoundary)
        {
            painter->setPen(QPen(Qt::red,lineWidth));
            paintMotifBoundary(painter,motif);
        }

        if (config->showExtendedBoundary)
        {
            painter->setPen(QPen(Qt::yellow,lineWidth));
            paintExtendedBoundary(painter,motif);
        }

        drawLayerModelCenter(painter);

        if (config->showTileCenter)
        {
            QPointF pt = tile->getCenter();
            pt = _T.map(pt);
            drawCenterSymbol(painter,pt,QColor(Qt::red),QColor(Qt::green));
        }

        if (config->showMotifCenter)
        {
            auto map  = motif->getMotifMap();
            if (map)
            {
                auto pts  = map->getPoints();

                QPointF pt;
                if (motif->isRadial())
                {
                    QPolygonF poly(pts);
                    pt = Geo::center(poly);
                }
                else
                {
                    pt = tile->getCenter();
                }
                pt = _T.map(pt);
                drawCenterSymbol(painter,pt,QColor(Qt::green),QColor(Qt::red));
            }
        }

        // paint motif
        if (config->showMotif)
        {
            painter->setPen(QPen(Qt::blue,lineWidth));
            if (motif->isRadial())
            {
                paintRadialMotifMap(painter,motif);
            }
            else
            {
                paintExplicitMotifMap(painter, motif);
            }
        }

        DebugMapPtr dmap = motif->getDebugMap();
        if (dmap)
        {
            auto map = std::dynamic_pointer_cast<Map>(dmap);
            if (map)
            {
                QColor viewColor = viewControl->getCanvas().getBkgdColor();
                QColor color = (viewColor == QColor(Qt::white)) ? Qt::black : Qt::white;
                painter->setPen(QPen(color,lineWidth));
                paintMap(painter,map);
            }
        }

        if (motif->getMotifType() == MOTIF_TYPE_INFERRED)
        {
            auto exp = std::dynamic_pointer_cast<InferredMotif>(motif);
            if (exp->hasDebugContacts())
            {
                painter->save();
                painter->setPen(QPen(Qt::yellow,lineWidth));
                for (auto & c : exp->getDebugContacts())
                {
                    painter->drawLine(baseT.map(c->other), baseT.map(c->position));
                }
                painter->restore();
            }
        }
    }
}

void MotifView::paintExplicitMotifMap(QPainter *painter, MotifPtr motif)
{
    //qDebug() << "paintExplicitMotifMap" << motif->getMotifDesc();

    MapPtr map = motif->getMotifMap();
    if (map)
    {
        //map->verifyMap( "paintExplicitFigure");
        paintMap(painter,map);
    }

    DebugMapPtr dmap = motif->getDebugMap();
    {
        if (dmap && !dmap->isEmpty())
        {
            map = std::dynamic_pointer_cast<Map>(dmap);
            if (map)
            {
                QColor viewColor = viewControl->getCanvas().getBkgdColor();
                QColor color = (viewColor == QColor(Qt::white)) ? Qt::black : Qt::white;
                painter->setPen(QPen(color,lineWidth));
                paintMap(painter,map);
            }
        }
    }
}

void MotifView::paintRadialMotifMap(QPainter *painter, MotifPtr motif)
{
    qDebug() << "paintRadialMotifMap" << motif->getMotifDesc();

    // Optimize for the case of a RadialFigure.
    RadialPtr rp = std::dynamic_pointer_cast<RadialMotif>(motif);

    MapPtr map = rp->getMotifMap();
    paintMap(painter,map);

    DebugMapPtr dmap = rp->getDebugMap();
    if (dmap && !dmap->isEmpty())
    {
        map = std::dynamic_pointer_cast<Map>(dmap);
        if (map)
        {
            QColor viewColor = viewControl->getCanvas().getBkgdColor();
            QColor color = (viewColor == QColor(Qt::white)) ? Qt::black : Qt::white;
            painter->setPen(QPen(color,lineWidth));
            paintMap(painter,map);
        }
    }

    if (Sys::highlightUnit)
    {
        map = rp->getUnitMap();
        painter->setPen(QPen(Qt::red,lineWidth+1.0));
        paintMap(painter,map);
    }
}

void MotifView::paintTileBoundary(QPainter *painter,TilePtr tile)
{
    // draw tile
    // qDebug() << "scale" << feat->getScale();
    //qDebug().noquote() << feat->toBaseString();
    //qDebug().noquote() << feat->toString();
    EdgePoly  ep = tile->getEdgePoly();
    ep.paint(painter,_T,true);
}

void MotifView::paintMotifBoundary(QPainter *painter, MotifPtr motif)
{
    // show boundaries
    QPolygonF p = motif->getMotifBoundary();
    p = _T.map(p);
    painter->drawPolygon(p);
    painter->setPen(QPen(Qt::blue,1));
    painter->setBrush(Qt::blue);
    for (QPointF pt : p )
    {
        painter->drawEllipse(pt,4.0,4.0);
    }
    painter->setBrush(Qt::NoBrush);
}

void MotifView::paintExtendedBoundary(QPainter *painter, MotifPtr motif)
{
    const ExtendedBoundary & eb = motif->getExtendedBoundary();

    if (!eb.isCircle())
    {
        const QPolygonF & p = eb.getPoly();
        painter->drawPolygon(_T.map(p));
    }
    else
    {
        qreal radius = eb.getScale() * Transform::scalex(_T);
        painter->drawEllipse(_T.map(QPointF(0,0)),radius,radius);
    }
}

void MotifView::paintMap(QPainter * painter, MapPtr map)
{
    //map->verify("figure", true, true, true);
    //qDebug() << "MotifView::paintMap" <<  map->namedSummary();
    for (auto & edge : std::as_const(map->getEdges()))
    {
        QPointF p1 = _T.map(edge->v1->pt);
        QPointF p2 = _T.map(edge->v2->pt);

        //qDebug() << "edge" << p1 << p2;

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
    for (auto & pair : std::as_const(texts))
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

void MotifView::setModelXform(const Xform & xf, bool update)
{
    Q_ASSERT(!_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << xf.toInfoString() << (isUnique() ? "unique" : "common");
    viewControl->setCurrentModelXform(xf,update);
}

const Xform & MotifView::getModelXform()
{
    Q_ASSERT(!_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << viewControl->getCurrentModelXform().toInfoString() << (isUnique() ? "unique" : "common");
    return viewControl->getCurrentModelXform();
}

void MotifView::slot_setCenter(QPointF spt)
{
    if (view->isActiveLayer(this))
    {
        setCenterScreenUnits(spt);
    }
}

void MotifView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    Q_UNUSED(spt);
    Q_UNUSED(btn);

    // in multiView delegates the clicked motif

    if (config->motifEnlarge)
        return;

    if (!config->motifMultiView)
        return;

    QPointF mpt = screenToWorld(spt);

    auto proto  = protoMakerData->getSelectedPrototype();
    if (!proto) return;

    auto tiling = proto->getTiling();
    for (const auto & del : protoMakerData->getSelectedDELs(MVD_DELEM))
    {
        auto tile   = del->getTile();
        QPolygonF poly = tile->getPolygon();

        QTransform placement = tiling->getFirstPlacement(tile);
        poly = placement.map(poly);

        if (poly.containsPoint(mpt,Qt::OddEvenFill))
        {
            auto widget   = protoMakerData->getWidget();
            auto btn      = widget->getButton(del);
            if (btn)
            {
                widget->delegate(btn,config->motifMultiView,true);
            }
            return;
        }
    }
}

void MotifView::slot_mouseDragged(QPointF spt)
{ Q_UNUSED(spt); }
void MotifView::slot_mouseMoved(QPointF spt)
{ Q_UNUSED(spt); }
void MotifView::slot_mouseReleased(QPointF spt)
{ Q_UNUSED(spt); }
void MotifView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

