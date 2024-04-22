#include "viewers/prototype_view.h"
#include "tile/tile.h"
#include "viewers/view_controller.h"
#include "viewers/viewerbase.h"
#include "motifs/motif.h"
#include "geometry/map.h"
#include "geometry/fill_region.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "misc/geo_graphics.h"
#include "misc/sys.h"
#include "mosaic/design_element.h"
#include "makers/prototype_maker/prototype.h"
#include "settings/configuration.h"
#include "settings/configuration.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"

using std::make_shared;

PrototypeView::PrototypeView() : LayerController("PrototypeView",false)
{
    protoMaker = Sys::prototypeMaker;

    colors.setColors(config->protoViewColors);
}

PrototypeView::~PrototypeView()
{}

void PrototypeView::paint(QPainter *painter)
{
    lineWidth = config->protoviewWidth;

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QTransform tr = getLayerTransform();

    GeoGraphics geoGraphics(painter,tr);

    gg = & geoGraphics;

    draw();
    
    drawLayerModelCenter(painter);
}

void PrototypeView::draw()
{
    qDebug() << "PrototypeView::draw  mode=" << config->protoViewMode;

    const auto data = protoMaker->getProtoMakerData();

    data->dumpData(MVD_PROTO);

    for (const auto & proto : data->getPrototypes())
    {
        if (proto->hasContent() && !data->isHidden(MVD_PROTO,proto))
        {
            drawProto(proto);
        }
    }
}

void PrototypeView::drawProto(ProtoPtr proto)
{
    Q_ASSERT(proto);

    TilingPtr tiling = proto->getTiling();
    if (!tiling)
    {
        qWarning() << "Tiling not found in Prototype";
        return;
    }

    qDebug() << "PrototypeView  proto="  << proto.get();

    mode = config->protoViewMode;

    if (mode & PROTO_DRAW_MAP)
    {
        MapPtr map = proto->getProtoMap();
        qDebug() << "PrototypeView  proto="  << proto.get() << "protoMap" << map.get();

        QPen pen(colors.mapColor,lineWidth);

        EdgePoly edges(map->getEdges());    // this is not really an EdgePoly it is a vector of Edges
        edges.draw(gg, pen);
        edges.drawPts(gg, pen);
    }
    
    FillRegion flood(proto->getTiling().get(),Sys::viewController->getCanvas().getFillData());
    Placements fillPlacements = flood.getPlacements(config->repeatMode);

    if (mode & (PROTO_ALL_TILES | PROTO_ALL_MOTIFS))
    {
        for (const auto & T1 : std::as_const(fillPlacements))
        {
            for (const auto & del : std::as_const(proto->getDesignElements()))
            {
                auto motif = del->getMotif();
                auto tile  = del->getTile();

                auto tilePlacements = tiling->getPlacements(tile);
                for (const auto & T0 : std::as_const(tilePlacements))
                {
                    QTransform T2 = T0 * T1;
                    gg->pushAndCompose(T2);

                    if (mode & PROTO_ALL_TILES)
                    {
                        ViewerBase::drawTile(gg,tile,QBrush(),QPen(colors.tileColor,lineWidth));
                    }

                    if (mode & PROTO_ALL_MOTIFS)
                    {
                        ViewerBase::drawMotif(gg,motif,QPen(colors.motifColor,lineWidth));
                    }

                    gg->pop();
                }
            }
        }
    }

    if (mode & PROTO_DRAW_PROTO)
    {
        QPen motifTile(colors.delTileColor,1,         Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QPen motifPen(colors.delMotifColor,lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QPen pen(colors.tileBrushColor,    1,         Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QPen pen2(colors.tileBrushColor,   lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        // paint background of each DEL, then its tile edges and motifs
        for (const auto & del : std::as_const(proto->getDesignElements()))
        {
            auto motif  = del->getMotif();
            auto tile   = del->getTile();
            const EdgePoly & ep = tile->getEdgePoly();

            auto tilePlacements = tiling->getPlacements(tile);
            for (const auto & T0 : std::as_const(tilePlacements))
            {
                gg->pushAndCompose(T0);

                gg->fillEdgePoly(ep,pen);
                gg->drawEdgePoly(ep,pen2);

                if (mode & PROTO_DEL_TILES)
                {
                    gg->drawEdgePoly(ep,motifTile);
                }

                if (mode & PROTO_DEL_MOTIFS)
                {
                    ViewerBase::drawMotif(gg,motif,motifPen);
                }

                gg->pop();
            }
        }
    }

    auto data = protoMaker->getProtoMakerData();

    if (mode & PROTO_ALL_VISIBLE)
    {
        for (const auto & del : std::as_const(proto->getDesignElements()))
        {
            if (!data->isHidden(MVD_PROTO,del))
            {
                auto motif = del->getMotif();
                auto tile  = del->getTile();
                auto tilePlacements = tiling->getPlacements(tile);
                for (const auto & T0 : std::as_const(tilePlacements))
                {
                    gg->pushAndCompose(T0);

                    if (mode & PROTO_VISIBLE_TILE)
                    {
                        QPen pen(colors.visibleTileColor,lineWidth);
                        ViewerBase::drawTile(gg,tile,QBrush(),pen);
                    }
                    if  (mode & PROTO_VISIBLE_MOTIF)
                    {
                        QPen pen(colors.visibleMotifColor,lineWidth);
                        ViewerBase::drawMotif(gg,motif,pen);
                    }
                    gg->pop();
                }
            }
        }
    }
    else
    {
        for (const auto & del : std::as_const(proto->getDesignElements()))
        {
            if (!data->isHidden(MVD_PROTO,del))
            {
                auto motif = del->getMotif();
                auto tile  = del->getTile();
                auto placements = tiling->getPlacements(tile);
                QTransform T0;
                if (placements.size())
                {
                    T0 = placements.first();
                    if (!T0.isIdentity())
                    {
                        gg->pushAndCompose(T0);
                    }
                }
                if (mode & PROTO_VISIBLE_TILE)
                {
                    QPen pen(colors.visibleTileColor,lineWidth);
                    ViewerBase::drawTile(gg,tile,QBrush(),pen);
                }
                if  (mode & PROTO_VISIBLE_MOTIF)
                {
                    QPen pen(colors.visibleMotifColor,lineWidth);
                    ViewerBase::drawMotif(gg,motif,pen);
                }
                if (!T0.isIdentity())
                {
                    gg->pop();
                }
            }
        }
    }
}

void PrototypeView::setModelXform(const Xform & xf, bool update)
{
    Q_ASSERT(!_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << xf.info() << (isUnique() ? "unique" : "common");
    viewControl->setCurrentModelXform(xf,update);
}

const Xform & PrototypeView::getModelXform()
{
    Q_ASSERT(!_unique);
    if (debug & DEBUG_XFORM) qInfo().noquote() << "SET" << getLayerName() << viewControl->getCurrentModelXform().info() << (isUnique() ? "unique" : "common");
    return viewControl->getCurrentModelXform();
}

void PrototypeView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{ Q_UNUSED(spt); Q_UNUSED(btn); }
void PrototypeView::slot_mouseDragged(QPointF spt)
{ Q_UNUSED(spt); }
void PrototypeView::slot_mouseMoved(QPointF spt)
{ Q_UNUSED(spt); }
void PrototypeView::slot_mouseReleased(QPointF spt)
{ Q_UNUSED(spt); }
void PrototypeView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }

ProtoViewColors::ProtoViewColors()
{
    visibleTileColor = QColor(Qt::darkCyan);
    visibleMotifColor = QColor(Qt::darkBlue);
}

QStringList ProtoViewColors::getColors()
{
    QStringList qsl;
    qsl << mapColor.name(QColor::HexArgb);
    qsl << tileColor.name(QColor::HexArgb);
    qsl << motifColor.name(QColor::HexArgb);
    qsl << delTileColor.name(QColor::HexArgb);
    qsl << delMotifColor.name(QColor::HexArgb);
    qsl << tileBrushColor.name(QColor::HexArgb);
    return qsl;
}

void ProtoViewColors::setColors(QStringList & colors)
{
    int index = 0;
    mapColor         = QColor(colors.at(index++));
    tileColor        = QColor(colors.at(index++));
    motifColor       = QColor(colors.at(index++));
    delTileColor     = QColor(colors.at(index++));
    delMotifColor    = QColor(colors.at(index++));
    tileBrushColor   = QColor(colors.at(index++));
}
