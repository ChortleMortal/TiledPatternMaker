#include "gui/viewers/motif_maker_view.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/map.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/transform.h"
#include "gui/model_editors/motif_edit/design_element_button.h"
#include "gui/model_editors/motif_edit/motif_maker_widget.h"
#include "model/prototypes/prototype.h"
#include "model/makers/prototype_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/prototypes/design_element.h"
#include "model/motifs/inferred_motif.h"
#include "model/motifs/irregular_tools.h"
#include "model/motifs/motif.h"
#include "model/motifs/radial_motif.h"
#include "model/settings/configuration.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "gui/top/system_view_controller.h"

using std::make_shared;

///////////////////////////////////////////////////////////////////////////
///
///     This view paints all selected DELs in a selected Prototyp
///
///////////////////////////////////////////////////////////////////////////

MotifMakerView::MotifMakerView() : LayerController(VIEW_MOTIF_MAKER,DERIVED,"Motif Maker")
{
}

MotifMakerView::~MotifMakerView()
{
    //qDebug() << "MotifView destructor";
}

void MotifMakerView::paint(QPainter *painter)
{
    //qDebug() << "MotifMakerView::paint";

#ifdef INVERT_MAP
    // invert
    painter->translate(0,view->height());
    painter->scale(1.0, -1.0);
#endif
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    qreal lineWidth = Sys::config->motifViewWidth;

    auto proto  = Sys::prototypeMaker->getSelectedPrototype();
    if (!proto) return;
    auto tiling = proto->getTiling();

    DesignElementPtr del = Sys::prototypeMaker->getSelectedDEL();

    QTransform baseT;

    if (Sys::config->motifMakerView == MOTIF_VIEW_SOLO && del)
    {
        baseT = DesignElementButton::resetViewport(-2,del,Sys::viewController->viewRect());
    }
    else
    {
        baseT  = getLayerTransform();
    }


    // create design unit
    DesignUnit  designUnit;

    if (Sys::config->motifMakerView == MOTIF_VIEW_DESIGN_UNIT)
    {
        designUnit = Sys::prototypeMaker->getAllDELs();
    }
    else
    {
        designUnit = Sys::prototypeMaker->getSelectedDELs(MVD_DELEM);
    }

    for (auto & del : designUnit)
    {
        auto motif  = del->getMotif();
        auto tile   = del->getTile();

        QVector<QTransform> del_placements;

        switch (Sys::config->motifMakerView)
        {
        case MOTIF_VIEW_SOLO:
            del_placements.push_back(QTransform());
            break;

        case MOTIF_VIEW_SELECTED:
            del_placements.push_back(tiling->unit().getFirstPlacement(tile));
            break;

        case MOTIF_VIEW_DESIGN_UNIT:
            del_placements = tiling->unit().getPlacements(tile);
            break;
        }

        for (auto & del_placement : del_placements)
        {
            QTransform t = del_placement * baseT;   // order is critical

            QPointF center      = tile->getCenter();
            QPointF tile_center = t.map(center);

            if (Sys::config->motifMakerView == MOTIF_VIEW_SOLO)
            {
                QPointF diff_center = Sys::viewController->viewRect().center() - tile_center;
                t = t * QTransform::fromTranslate(diff_center.x(),diff_center.y());
            }

            tile_center = t.map(center);

            // draw boundaries
            if (Sys::config->showTileBoundary)
            {
                painter->setPen(QPen(Qt::magenta,lineWidth));
                paintTileBoundary(painter,tile,t);
            }

            if (Sys::config->showMotifBoundary)
            {
                painter->setPen(QPen(Qt::red,lineWidth));
                paintMotifBoundary(painter,motif,t);
            }

            if (Sys::config->showExtendedBoundary)
            {
                painter->setPen(QPen(Qt::yellow,lineWidth));
                paintExtendedBoundary(painter,motif,t);
            }

            // draw centres
            drawLayerModelCenter(painter);

            if (Sys::config->showTileCenter)
            {
                drawCenterSymbol(painter,tile_center,QColor(Qt::red),QColor(Qt::green));
            }

            if (Sys::config->showMotifCenter)
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
                    pt = t.map(pt);
                    drawCenterSymbol(painter,pt,QColor(Qt::green),QColor(Qt::red));
                }
            }

            // draw motif
            if (Sys::config->showMotif)
            {
                painter->setPen(QPen(Qt::blue,lineWidth));
                if (motif->isRadial())
                {
                    RadialPtr rp = std::dynamic_pointer_cast<RadialMotif>(motif);
                    paintRadialMotifMap(painter,rp,t);

                    if (Sys::highlightUnit)
                    {
                        MapPtr map = rp->getUnitMap();
                        painter->save();
                        painter->setPen(QPen(Qt::red, Sys::config->motifViewWidth +1.0));
                        paintMap(painter,map,t);
                        painter->restore();
                    }
                }
                else
                {
                    paintExplicitMotifMap(painter, motif,t);
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
                        painter->drawLine(t.map(c->other), t.map(c->position));
                    }
                    painter->restore();
                }
            }
        }
    }
}

void MotifMakerView::paintExplicitMotifMap(QPainter *painter, MotifPtr motif, QTransform & tr)
{
    //qDebug() << "paintExplicitMotifMap" << motif->getMotifDesc();
    MapPtr map = motif->getMotifMap();
    if (map)
    {
        //map->verifyMap( "paintExplicitFigure");
        paintMap(painter,map,tr);
    }
}

void MotifMakerView::paintRadialMotifMap(QPainter *painter, RadialPtr rp, QTransform & tr)
{
    // Optimize for the case of a RadialFigure.
    //qDebug() << "paintRadialMotifMap" << rp->getMotifDesc();
    MapPtr map = rp->getMotifMap();
    if (map)
    {
        paintMap(painter,map,tr);
    }
}

void MotifMakerView::paintTileBoundary(QPainter *painter,TilePtr tile, QTransform & tr)
{
    // draw tile
    // qDebug() << "scale" << feat->getScale();
    //qDebug().noquote() << feat->toBaseString();
    //qDebug().noquote() << feat->toString();
    const EdgePoly & ep = tile->getEdgePoly();
    ep.paint(painter,tr,true);
}

void MotifMakerView::paintMotifBoundary(QPainter *painter, MotifPtr motif, QTransform &tr)
{
    // show boundaries
    QPolygonF p = motif->getMotifBoundary();
    p = tr.map(p);
    painter->drawPolygon(p);
    painter->setPen(QPen(Qt::blue,1));
    painter->setBrush(Qt::blue);
    for (QPointF pt : p)
    {
        painter->drawEllipse(pt,4.0,4.0);
    }
    painter->setBrush(Qt::NoBrush);
}

void MotifMakerView::paintExtendedBoundary(QPainter *painter, MotifPtr motif, QTransform & tr)
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

void MotifMakerView::paintMap(QPainter * painter, MapPtr map, QTransform &tr)
{
    //map->verify("figure", true, true, true);
    //qDebug() << "MotifView::paintMap" <<  map->namedSummary();
    map->paint(painter,tr,false,false,false,true);
}

void MotifMakerView::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    Q_UNUSED(spt);
    Q_UNUSED(btn);

    // in multiView delegates the clicked motif

    if (Sys::config->motifMakerView == MOTIF_VIEW_SOLO)
        return;

    QPointF mpt = screenToModel(spt);

    auto proto  = Sys::prototypeMaker->getSelectedPrototype();
    if (!proto) return;

    auto tiling = proto->getTiling();

    QVector<DesignElementPtr> dels;
    Placements                fillPlacements;

    if (Sys::config->motifMakerView == MOTIF_VIEW_DESIGN_UNIT)
    {
        dels = Sys::prototypeMaker->getAllDELs();
        FillData fillData = tiling->hdr().getCanvasSettings().getFillData();
        FillRegion flood(tiling.get(),fillData);
        fillPlacements = flood.getPlacements(Sys::config->repeatMode);
    }
    else
    {
        dels = Sys::prototypeMaker->getSelectedDELs(MVD_DELEM);
        fillPlacements.push_back(QTransform());
    }

    for (auto & del : dels)
    {
        auto tile   = del->getTile();
        QPolygonF poly = tile->getPolygon();

        auto placements = tiling->unit().getPlacements(tile);

        for (QTransform & placement : placements)
        {
            for (QTransform & fplace : fillPlacements)
            {
                QTransform t = fplace * placement;
                auto poly2   = t.map(poly);
                if (poly2.containsPoint(mpt,Qt::OddEvenFill))
                {
                    auto widget   = Sys::prototypeMaker->getWidget();
                    auto btn      = widget->getButton(del);
                    if (btn)
                    {
                        widget->delegate(btn,(Sys::config->motifMakerView == MOTIF_VIEW_SELECTED),true);
                    }
                    return;
                }
            }
        }
    }
}

void MotifMakerView::slot_mouseDragged(QPointF spt)
{ Q_UNUSED(spt); }
void MotifMakerView::slot_mouseMoved(QPointF spt)
{ Q_UNUSED(spt); }
void MotifMakerView::slot_mouseReleased(QPointF spt)
{ Q_UNUSED(spt); }
void MotifMakerView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

