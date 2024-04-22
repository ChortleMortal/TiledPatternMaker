#include "viewers/motif_view.h"
#include "geometry/geo.h"
#include "geometry/map.h"
#include "geometry/debug_map.h"
#include "geometry/transform.h"
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

///////////////////////////////////////////////////////////////////////////
///
///     This view paints all selected DELs in a selected Prototyp
///
///////////////////////////////////////////////////////////////////////////

MotifView::MotifView() : LayerController("Motif View",false)
{
    protoMakerData = Sys::prototypeMaker->getProtoMakerData();
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

    qreal lineWidth = config->motifViewWidth;

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

        QTransform T = placement * baseT;
        QTransform T2 = motif->getDELTransform() * T;
        //qDebug().noquote() << "MotifView::transform" << Transform::toInfoString(_T);

        if (motif->getDebugMap())
        {
            QColor viewColor = viewControl->getCanvas().getBkgdColor();
            QColor color = (viewColor == QColor(Qt::white)) ? Qt::black : Qt::white;
            painter->setPen(QPen(color,lineWidth));
            paintDebugMap(painter,motif,T2);
        }

        // paint boundaries
        if (config->showTileBoundary)
        {
            painter->setPen(QPen(Qt::magenta,lineWidth));
            paintTileBoundary(painter,tile,T);
        }

        if (config->showMotifBoundary)
        {
            painter->setPen(QPen(Qt::red,lineWidth));
            paintMotifBoundary(painter,motif,T);
        }

        if (config->showExtendedBoundary)
        {
            painter->setPen(QPen(Qt::yellow,lineWidth));
            paintExtendedBoundary(painter,motif,T2);
        }

        drawLayerModelCenter(painter);

        if (config->showTileCenter)
        {
            QPointF pt = tile->getCenter();
            pt = T.map(pt);
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
                pt = T.map(pt);
                drawCenterSymbol(painter,pt,QColor(Qt::green),QColor(Qt::red));
            }
        }

        // paint motif
        if (config->showMotif)
        {
            painter->setPen(QPen(Qt::blue,lineWidth));
            if (motif->isRadial())
            {
                paintRadialMotifMap(painter,motif,T);
            }
            else
            {
                paintExplicitMotifMap(painter, motif,T);
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

void MotifView::paintExplicitMotifMap(QPainter *painter, MotifPtr motif, QTransform & tr)
{
    //qDebug() << "paintExplicitMotifMap" << motif->getMotifDesc();

    MapPtr map = motif->getMotifMap();
    if (map)
    {
        //map->verifyMap( "paintExplicitFigure");
        paintMap(painter,map,tr);
    }
}

void MotifView::paintRadialMotifMap(QPainter *painter, MotifPtr motif, QTransform & tr)
{
    qDebug() << "paintRadialMotifMap" << motif->getMotifDesc();

    // Optimize for the case of a RadialFigure.
    RadialPtr rp = std::dynamic_pointer_cast<RadialMotif>(motif);

    MapPtr map = rp->getMotifMap();
    paintMap(painter,map,tr);

    if (Sys::highlightUnit)
    {
        map = rp->getUnitMap();
        painter->save();
        painter->setPen(QPen(Qt::red, config->motifViewWidth +1.0));
        paintMap(painter,map,tr);
        painter->restore();
    }
}

void MotifView::paintDebugMap(QPainter *painter, MotifPtr motif, QTransform & tr)
{
    DebugMapPtr dmap = motif->getDebugMap();
    dmap->paint(painter,tr);
}

void MotifView::paintTileBoundary(QPainter *painter,TilePtr tile, QTransform & tr)
{
    // draw tile
    // qDebug() << "scale" << feat->getScale();
    //qDebug().noquote() << feat->toBaseString();
    //qDebug().noquote() << feat->toString();
    const EdgePoly & ep = tile->getEdgePoly();
    ep.paint(painter,tr,true);
}

void MotifView::paintMotifBoundary(QPainter *painter, MotifPtr motif, QTransform &tr)
{
    // show boundaries
    QPolygonF p = motif->getMotifBoundary();
    p = tr.map(p);
    painter->drawPolygon(p);
    painter->setPen(QPen(Qt::blue,1));
    painter->setBrush(Qt::blue);
    for (QPointF pt : p )
    {
        painter->drawEllipse(pt,4.0,4.0);
    }
    painter->setBrush(Qt::NoBrush);
}

void MotifView::paintExtendedBoundary(QPainter *painter, MotifPtr motif, QTransform & tr)
{
    const ExtendedBoundary & eb = motif->getExtendedBoundary();

    if (!eb.isCircle())
    {
        const QPolygonF & p = eb.getPoly();
        painter->drawPolygon(tr.map(p));
    }
    else
    {
        qreal radius = eb.getScale() * Transform::scalex(tr);
        painter->drawEllipse(tr.map(QPointF(0,0)),radius,radius);
    }
}

void MotifView::paintMap(QPainter * painter, MapPtr map, QTransform &tr)
{
    //map->verify("figure", true, true, true);
    //qDebug() << "MotifView::paintMap" <<  map->namedSummary();
    map->paint(painter,tr);
}

void MotifView::setModelXform(const Xform & xf, bool update)
{
    Q_ASSERT(!_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << xf.info() << (isUnique() ? "unique" : "common");
    viewControl->setCurrentModelXform(xf,update);
}

const Xform & MotifView::getModelXform()
{
    Q_ASSERT(!_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << viewControl->getCurrentModelXform().info() << (isUnique() ? "unique" : "common");
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

    QPointF mpt = screenToModel(spt);

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

