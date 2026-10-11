#include "gui/top/view_controller.h"
#include "gui/viewers/geo_graphics.h"
#include "gui/viewers/prototype_view.h"
#include "gui/viewers/viewer_services.h"
#include "model/makers/motif_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/motif.h"
#include "model/prototypes/tile_motif.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/settings/configuration.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/fill_region.h"
#include "sys/geometry/map.h"
#include "sys/geometry/neighbour_map.h"
#include "sys/sys.h"

using std::make_shared;

PrototypeView::PrototypeView() : LayerController(Type::Prototype,Model::Derived,ZLevel::Standard_Z0,"Prototype")
{
    colors.setColors(Sys::config->protoViewColors);
}

PrototypeView::~PrototypeView()
{}

void PrototypeView::paint(QPainter *painter)
{
    lineWidth = Sys::config->protoviewWidth;

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QTransform tr = getLayerTransform();

    GeoGraphics geoGraphics(painter,tr);

    gg = & geoGraphics;

    draw();
    
    drawLayerModelCenter(painter);
}

void PrototypeView::draw()
{
    //qDebug() << "PrototypeView::draw  mode=" << config->protoViewMode;
    //Sys::prototypeMaker->dumpData(MVD_PROTO);

    const int mode = Sys::config->protoViewMode;

    for (const auto & proto : Sys::motifMaker->protoData.getPrototypes())
    {
        if (proto->hasContent() && Sys::motifMaker->protoData.isVisible(proto))
        {
            drawProto(mode, proto);
        }
        for (const TileMotifPtr & timo : proto->getTileMotifs())
        {
            if (Sys::motifMaker->protoData.isVisible(timo))
            {
                drawTileMotif(mode,proto,timo);
            }
        }
    }
}

void PrototypeView::drawProto(const int mode, ProtoPtr proto)
{
    Q_ASSERT(proto);

    if (mode & SHOW_MAP)
    {
        MapPtr map = proto->getProtoMap();  // builds on demand

        //qDebug() << "PrototypeView  proto="  << proto.get() << "protoMap" << map.get();

        QPen pen(colors.mapColor,lineWidth);

        NeighbourMap nmap(map);

        for (const auto & edge : std::as_const(map->getEdges()))
        {
            gg->drawEdge(edge,pen);
#if 0
            QColor c1,c2;

            int num1 = nmap.getNeighbours(edge->v1)->numNeighbours();
            switch (num1)
            {
            case 2:
                c1 = Qt::red;
                break;
            case 3:
                c1 = Qt::green;
                break;
            case 4:
                c1 = Qt::cyan;
                break;
            default:
                c1 =  Qt::yellow;
                break;
            }

            int num2 = nmap.getNeighbours(edge->v2)->numNeighbours();
            switch (num2)
            {
            case 2:
                c2 = Qt::red;
                break;
            case 3:
                c2 = Qt::green;
                break;
            case 4:
                c2 = Qt::cyan;
                break;
            default:
                c2 =  Qt::yellow;
                break;
            }

            gg->drawCircle(edge->v1->pt,6,QPen(Qt::red),QBrush(c1));
            gg->drawCircle(edge->v2->pt,6,QPen(Qt::red),QBrush(c2));
            gg->drawCircle(edge->v1->pt,3,pen, QBrush(pen.color()));
            gg->drawCircle(edge->v2->pt,3,pen, QBrush(pen.color()));
#else
            gg->drawCircle(edge->v1->pt,2,QPen(Qt::red),QBrush(Qt::red));
            gg->drawCircle(edge->v2->pt,2,QPen(Qt::red),QBrush(Qt::red));
#endif
        }
    }

    TilingPtr tiling = proto->getTiling();
    if (!tiling)
    {
        return;     // tiling-less (explicit map) prototype - nothing further to draw
    }

    FillRegion flood(tiling.get(),proto->getCanvas()->getFillData());
    Placements fillPlacements = flood.getPlacements(Sys::config->repeatMode);

    if (mode & (SHOW_TILES | SHOW_MOTIFS | SHOW_EXT_BOUNDARY))
    {
        for (const auto & T1 : std::as_const(fillPlacements))
        {
            for (const auto & del : std::as_const(proto->getTileMotifs()))
            {
                auto motif = del->getMotif();
                auto tile  = del->getTile();

                auto tilePlacements = tiling->unit().getPlacements(tile,proto->getCanvas()->getDistorionTransform());
                for (const auto & T0 : std::as_const(tilePlacements))
                {
                    QTransform T2 = T0 * T1;
                    gg->pushAndCompose(T2);

                    if (mode & SHOW_TILES)
                    {
                        ViewerServices::drawTile(gg,tile,QBrush(),QPen(colors.tileColor,lineWidth));
                    }

                    if (mode & SHOW_MOTIFS)
                    {
                        ViewerServices::drawMotif(gg,motif,QPen(colors.motifColor,lineWidth));
                    }

                    if (mode & SHOW_EXT_BOUNDARY)
                    {
                        ViewerServices::drawExtendedBoundary(gg,motif,QPen(colors.extBoundaryColor,lineWidth));
                    }

                    gg->pop();
                }
            }
        }
    }

    if (mode & SHOW_TILING_UNIT)
    {
        QPen motifTile(colors.delTileColor,1,         Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QPen motifPen(colors.delMotifColor,lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QPen pen(colors.tileBrushColor,    1,         Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QPen pen2(colors.tileBrushColor,   lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        // paint background of each DEL, then its tile edges and motifs
        for (const auto & del : std::as_const(proto->getTileMotifs()))
        {
            auto motif  = del->getMotif();
            auto tile   = del->getTile();
            const EdgePoly & ep = tile->getEdgePoly();

            auto tilePlacements = tiling->unit().getPlacements(tile,proto->getCanvas()->getDistorionTransform());
            for (const auto & T0 : std::as_const(tilePlacements))
            {
                gg->pushAndCompose(T0);

                gg->fillEdgePoly(ep,pen);
                gg->drawEdgePoly(ep,pen2);

                if (mode & SHOW_TU_TILES)
                {
                    gg->drawEdgePoly(ep,motifTile);
                }

                if (mode & SHOW_TU_MOTIFS)
                {
                    ViewerServices::drawMotif(gg,motif,motifPen);
                }

                gg->pop();
            }
        }
    }
}

void PrototypeView::drawTileMotif(const int mode, ProtoPtr proto, TileMotifPtr del)
{
    TilingPtr tiling = proto->getTiling();
    if (!tiling)
    {
        qWarning() << "Tiling not found in Prototype";
        return;
    }

    if (mode & SHOW_ALL_TU_TILES)
    {
        auto motif = del->getMotif();
        auto tile  = del->getTile();
        auto tilePlacements = tiling->unit().getPlacements(tile,proto->getCanvas()->getDistorionTransform());
        for (const auto & T0 : std::as_const(tilePlacements))
        {
            gg->pushAndCompose(T0);

            if (mode & SHOW_SELECTED_TU_TILES)
            {
                QPen pen(colors.visibleTileColor,lineWidth);
                ViewerServices::drawTile(gg,tile,QBrush(),pen);
            }
            if  (mode & SHOW_SELECTED_TU_MOTIFS)
            {
                QPen pen(colors.visibleMotifColor,lineWidth);
                ViewerServices::drawMotif(gg,motif,pen);
            }
            gg->pop();
        }
    }
    else
    {
        auto motif = del->getMotif();
        auto tile  = del->getTile();
        auto placements = tiling->unit().getPlacements(tile,proto->getCanvas()->getDistorionTransform());
        QTransform T0;
        if (placements.size())
        {
            T0 = placements.first();
            if (!T0.isIdentity())
            {
                gg->pushAndCompose(T0);
            }
        }
        if (mode & SHOW_SELECTED_TU_TILES)
        {
            QPen pen(colors.visibleTileColor,lineWidth);
            ViewerServices::drawTile(gg,tile,QBrush(),pen);
        }
        if  (mode & SHOW_SELECTED_TU_MOTIFS)
        {
            QPen pen(colors.visibleMotifColor,lineWidth);
            ViewerServices::drawMotif(gg,motif,pen);
        }
        if (!T0.isIdentity())
        {
            gg->pop();
        }
    }
}

void PrototypeView::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
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
    visibleTileColor = QColor(Qt::black);
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
    qsl << extBoundaryColor.name(QColor::HexArgb);
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
    extBoundaryColor = (index < colors.size()) ? QColor(colors.at(index++)) : QColor(Qt::yellow);
}
