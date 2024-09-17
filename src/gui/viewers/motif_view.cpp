#include "gui/viewers/motif_view.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/map.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/transform.h"
#include "gui/model_editors/motif_edit/design_element_button.h"
#include "gui/model_editors/motif_edit/motif_maker_widget.h"
#include "model/prototypes/prototype.h"
#include "model/makers/prototype_maker.h"
#include "model/prototypes/design_element.h"
#include "model/motifs/inferred_motif.h"
#include "model/motifs/irregular_tools.h"
#include "model/motifs/motif.h"
#include "model/motifs/radial_motif.h"
#include "model/settings/configuration.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "gui/viewers/debug_view.h"
#include "gui/top/view_controller.h"

using std::make_shared;

///////////////////////////////////////////////////////////////////////////
///
///     This view paints all selected DELs in a selected Prototyp
///
///////////////////////////////////////////////////////////////////////////

MotifView::MotifView() : LayerController("Motif View",false)
{
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
    DesignElementPtr del = Sys::prototypeMaker->getSelectedDEL();
    if (config->motifEnlarge && del)
    {
        baseT = DesignElementButton::resetViewport(-2,del,Sys::view->rect()) * getModelTransform();
    }
    else
    {
        baseT  = getLayerTransform();
    }

    auto proto  = Sys::prototypeMaker->getSelectedPrototype();
    if (!proto) return;

    auto tiling = proto->getTiling();
    for (const auto & del : Sys::prototypeMaker->getSelectedDELs(MVD_DELEM))
    {
        auto motif  = del->getMotif();     // for now just a single motif
        auto tile   = del->getTile();
        //qDebug() << "MotifView  this=" << this << "del:" << _dep.get() << "fig:" << _fig.get();

        QTransform placement;
        if (!config->motifEnlarge)
        {
            placement = tiling->getFirstPlacement(tile);
        }

        QTransform T1 = placement * baseT;
        QTransform T2;
        if (config->motifMultiView)
        {
            // Motif View deels with unit scaled moitf
            // But in multi-view has to also deal with DEL transform
            //  T2 = motif->getDELTransform() * T1;
                T2 = tile->getTransform() * T1;     // bugfix 25AUG24
        }
        else
        {
            T2 = T1;
        }
        qDebug().noquote() << "MotifView::transform" << Transform::info(T2);

        if (motif->getDebugMap())
        {
            //FIXME - hack alert!  debug view needs this info
            Sys::debugView->setTransform(T2);
        }

        // paint boundaries
        if (config->showTileBoundary)
        {
            painter->setPen(QPen(Qt::magenta,lineWidth));
            paintTileBoundary(painter,tile,T2);
        }

        if (config->showMotifBoundary)
        {
            painter->setPen(QPen(Qt::red,lineWidth));
            paintMotifBoundary(painter,motif,T2);
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
            pt = T2.map(pt);
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
                pt = T2.map(pt);
                drawCenterSymbol(painter,pt,QColor(Qt::green),QColor(Qt::red));
            }
        }

        // paint motif
        if (config->showMotif)
        {
            painter->setPen(QPen(Qt::blue,lineWidth));
            if (motif->isRadial())
            {
                RadialPtr rp = std::dynamic_pointer_cast<RadialMotif>(motif);
                paintRadialMotifMap(painter,rp,T2);
                if (Sys::highlightUnit)
                {
                    MapPtr map = rp->getUnitMap();
                    painter->save();
                    painter->setPen(QPen(Qt::red, config->motifViewWidth +1.0));
                    paintMap(painter,map,T2);
                    painter->restore();
                }
            }
            else
            {
                paintExplicitMotifMap(painter, motif,T2);
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
                    //painter->drawLine(baseT.map(c->other), baseT.map(c->position));
                    painter->drawLine(T2.map(c->other), T2.map(c->position));
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

void MotifView::paintRadialMotifMap(QPainter *painter, RadialPtr rp, QTransform & tr)
{
    // Optimize for the case of a RadialFigure.
    qDebug() << "paintRadialMotifMap" << rp->getMotifDesc();
    MapPtr map = rp->getMotifMap();
    if (map)
    {
        paintMap(painter,map,tr);
    }
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
    for (ExtenderPtr ep : motif->getExtenders())
    {
        const ExtendedBoundary & eb = ep->getExtendedBoundary();

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
    if (Sys::view->isActiveLayer(VIEW_MOTIF_MAKER))
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

    auto proto  = Sys::prototypeMaker->getSelectedPrototype();
    if (!proto) return;

    auto tiling = proto->getTiling();
    for (const auto & del : Sys::prototypeMaker->getSelectedDELs(MVD_DELEM))
    {
        auto tile   = del->getTile();
        QPolygonF poly = tile->getPolygon();

        QTransform placement = tiling->getFirstPlacement(tile);
        poly = placement.map(poly);

        if (poly.containsPoint(mpt,Qt::OddEvenFill))
        {
            auto widget   = Sys::prototypeMaker->getWidget();
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

