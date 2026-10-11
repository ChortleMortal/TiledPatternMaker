#include "gui/model_editors/motif_edit/motif_maker_widget.h"
#include "gui/top/system_view.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/motif_maker_view.h"
#include "model/makers/motif_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/adaptive_inference.h"
#include "model/motifs/adaptive_tools.h"
#include "model/motifs/motif.h"
#include "model/motifs/radial_motif.h"
#include "model/prototypes/prototype.h"
#include "model/prototypes/tile_motif.h"
#include "model/settings/configuration.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/map.h"
#include "sys/geometry/transform.h"
#include "sys/sys/debugflags.h"

using std::make_shared;

///////////////////////////////////////////////////////////////////////////
///
///     This view paints all selected DELs in a selected Prototyp
///
///////////////////////////////////////////////////////////////////////////

MotifMakerView::MotifMakerView() : LayerController(Type::MotifMaker,Model::Derived,ZLevel::Standard_Z0,"Motif Maker")
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

    auto proto  = Sys::motifMaker->motifData.getActivePrototype();
    if (!proto) return;

    auto tiling = proto->getTiling();

    baseT = getLayerTransform();

    // create design unit
    TileMotifSet  tileMotifSet;
    if (Sys::config->motifMakerViewType == ViewType::All)
    {
        tileMotifSet = Sys::motifMaker->motifData.getTileMotifs();
    }
    else
    {
        tileMotifSet = Sys::motifMaker->motifData.getVisibleTileMotifs();
    }

    for (TileMotifPtr & timo : tileMotifSet)
    {
        auto motif  = timo->getMotif();
        auto tile   = timo->getTile();

        Placements placements;

        switch (Sys::config->motifMakerViewType)
        {
        case ViewType::Single:
            placements.push_back(tiling->unit().getFirstPlacement(tile,proto->getCanvas()->getDistorionTransform()));
            break;

        case ViewType::Multiple:
            placements.push_back(tiling->unit().getFirstPlacement(tile,proto->getCanvas()->getDistorionTransform()));
            break;

        case ViewType::All:
            placements = tiling->unit().getPlacements(tile,proto->getCanvas()->getDistorionTransform());
            break;

        case ViewType::Undefined:
            break;
        }

        for (QTransform & placement : placements)
        {
            usedTransform = placement * baseT;   // order is critical

            QPointF center      = tile->getCenter();
            QPointF tile_center = usedTransform.map(center);

            // draw boundaries
            if (Sys::config->showTileBoundary)
            {
                painter->setPen(QPen(Qt::magenta,lineWidth));
                paintTileBoundary(painter,tile,usedTransform);
            }

            if (Sys::config->showMotifBoundary)
            {
                painter->setPen(QPen(Qt::red,lineWidth));
                paintMotifBoundary(painter,motif,usedTransform);
            }

            if (Sys::config->showExtendedBoundary)
            {
                painter->setPen(QPen(Qt::yellow,lineWidth));
                paintExtendedBoundary(painter,motif,usedTransform);
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
                    pt = usedTransform.map(pt);
                    drawCenterSymbol(painter,pt,QColor(Qt::green),QColor(Qt::red));
                }
            }

            // draw motif
            if (Sys::config->showMotif)
            {
                painter->setPen(QPen(Qt::blue,lineWidth));

                MapPtr map = motif->getValidMotifMap();
                if (map)
                {
                    paintMap(painter,map,usedTransform);

                    if (Sys::flags->flagged(MOTIF_FLAG_2))
                    {
                        int idx = 0;
                        for (auto & edge : map->getEdges())
                        {
                            Sys::debugMapPaint->insertDebugMark(baseT.map(edge->getMidPoint()), QString::number(idx++), Qt::darkGreen,true);
                        }
                    }
                }

                if (motif->isRadial() && Sys::highlightUnit)
                {
                    RadialPtr rp = std::dynamic_pointer_cast<RadialMotif>(motif);
                    map = make_shared<Map>("Unit map");
                    auto & rays = rp->getUnitRays();
                    for (auto & ray : rays.get())
                    {
                        map->mergeMap(ray.getMap());
                    }

                    qreal scale = rp->getMotifFitScale();
                    if (!Loose::equals(scale,1.0))
                    {
                        QTransform t = QTransform::fromScale(scale,scale);
                        map->transform(t);
                    }

                    painter->save();
                    painter->setPen(QPen(Qt::red, Sys::config->motifViewWidth +1.0));
                    paintMap(painter,map,usedTransform);
                    painter->restore();
                }
            }

            if (motif->getMotifType() == Motif::Type::Inferred)
            {
                auto exp = std::dynamic_pointer_cast<AdaptiveInference>(motif);
                if (exp->hasDebugContacts())
                {
                    painter->save();
                    painter->setPen(QPen(Qt::yellow,lineWidth));
                    for (auto & c : exp->getDebugContacts())
                    {
                        //painter->drawLine(baseT.map(c->other), baseT.map(c->position));
                        painter->drawLine(usedTransform.map(c->other), usedTransform.map(c->position));
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
    for (QPointF & pt : p)
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
            qreal radius = eb.getRadius() * Transform::scalex(tr);
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

    if (!viewControl()->isEnabled(Layer::Type::MotifMaker)) return;

    // in multiView delegates the clicked motif
    if (Sys::config->motifMakerViewType == ViewType::Single)
        return;

    auto proto  = Sys::motifMaker->motifData.getActivePrototype();
    if (!proto) return;

    auto tiling = proto->getTiling();

    TileMotifSet    timos;
    Placements      fillPlacements;

    if (Sys::config->motifMakerViewType == ViewType::All)
    {
        timos = Sys::motifMaker->motifData.getTileMotifs();
        FillData fillData = tiling->hdr().getTilingCanvas()->getFillData();
        FillRegion flood(tiling.get(),fillData);
        fillPlacements = flood.getPlacements(Sys::config->repeatMode);
    }
    else
    {
        timos = Sys::motifMaker->motifData.getVisibleTileMotifs();
        fillPlacements.push_back(QTransform());
    }

    // this is what signals and slots are for - generating an emit
    QPointF mpt = screenToModel(spt);
    for (auto & timo : timos)
    {
        auto tile             = timo->getTile();
        QPolygonF poly        = tile->getPolygon();
        Placements placements = tiling->unit().getPlacements(tile);

        for (QTransform & t0 : placements)
        {
            for (QTransform & fplace : fillPlacements)
            {
                QTransform t = fplace * t0;
                auto poly2   = t.map(poly);
                if (poly2.containsPoint(mpt,Qt::OddEvenFill))
                {
                    emit sig_delegateTileMotif(timo);
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

void MotifMakerView::setDontAutoFit(bool checked)
{
    if (checked == dontAutoFit)
        return;

    if (checked && viewMode != ViewType::Single)
        return;     // can only enter from Single mode

    dontAutoFit = checked;
    updateSessionState();
}

void MotifMakerView::setViewMode(ViewType newMode)
{
    if (viewMode == newMode)
        return;

    viewMode = newMode;
    updateSessionState();
}

void MotifMakerView::setLayerEnabled(bool enb)
{
    if (enb == layerEnabled)
        return;

    layerEnabled = enb;
    updateSessionState();
}

// single choke point: prepare fires once on entering the session, restore fires once on leaving it,
// no matter which of viewMode/dontAutoFit/layerEnabled triggered the transition
void MotifMakerView::updateSessionState()
{
    bool wanted = layerEnabled && (viewMode == ViewType::Single || dontAutoFit);
    if (wanted == sessionActive)
        return;

    if (wanted)
    {
        //qDebug() << "save for restoration";
        emit Sys::systemViewController->getView()->sig_prepare();
        sessionActive = true;

        if (!dontAutoFit)
        {
            // fit fresh to whatever's currently delegated - a plain prepare leaves everyone
            // wherever they happened to be, which is stale the moment the session (re)starts
            TileMotifPtr timo = Sys::motifMaker->motifData.getDelegatedTileMotif();
            if (timo)
                enlarge(timo);
        }
        else
        {
            // resume the frozen fit rather than leaving everyone at the just-restored baseline
            emit Sys::systemViewController->getView()->sig_enlarge(Sys::nextSigid(),lastFit);
        }
    }
    else
    {
        //qDebug() << "restore";
        emit Sys::systemViewController->getView()->sig_restore(Sys::nextSigid());
        sessionActive = false;
    }
}

void MotifMakerView::enlarge(TileMotifPtr timo)
{
    Xform xf;
    if (getFitXform(xf,timo))
    {
        //qDebug() << "ENLARGE";
        lastFit = xf;
        emit Sys::systemViewController->getView()->sig_enlarge(Sys::nextSigid(),xf);
    }
}

bool MotifMakerView::getFitXform(Xform & xf, TileMotifPtr timo, qreal margin)
{
    if (!sessionActive || dontAutoFit)
        return false;

    // Computes the model Xform (scale + translate, no rotation) that fits modelBBox into the
    // current view, with some breathing room. Caller applies it via setModelXform().

    QRectF boundingRect = timo->boundingRect();
    if (boundingRect.width() <= 0 || boundingRect.height() <= 0)
        return false;

    auto proto  = Sys::motifMaker->motifData.getActivePrototype();
    auto tiling = proto->getTiling();
    QRectF baseRect = tiling->unit().getFirstPlacement(timo->getTile()).mapRect(boundingRect);
    baseRect = getCanvasTransform().mapRect(baseRect);

    QSizeF d = baseRect.size();
    d *= margin;
    if (d.width() <= 0 || d.height() <= 0)
        return false;

    QRect viewRect = viewControl()->getViewRect();

    qreal xscale = viewRect.width()  / d.width();
    qreal yscale = viewRect.height() / d.height();
    qreal scale  = qMin(xscale,yscale);

    QPointF targetTranslate = QPointF(viewRect.center()) - baseRect.center() * scale;

    // getModelTransform() scales/rotates about (canvasCenter - xf.translate), which makes the
    // effective screen-space contribution of xf.translate equal to xf.translate * xf.scale -
    // see how slot_moveX/moveY (layer_controller.cpp) divide by scale before adding, for the
    // same reason. Back that out here so the requested translate lands exactly.
    QPointF canvasCenter = viewControl()->getCanvas()->getCenter();
    QPointF translate    = (targetTranslate - canvasCenter * (1.0 - scale)) / scale;

    xf.setScale(scale);
    xf.setTranslate(translate);

    return true;
}