#include "viewers/prototype_view.h"
#include "tile/tile.h"
#include "viewers/viewcontrol.h"
#include "viewers/viewerbase.h"
#include "motifs/motif.h"
#include "geometry/map.h"
#include "geometry/fill_region.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "misc/geo_graphics.h"
#include "mosaic/design_element.h"
#include "makers/prototype_maker/prototype.h"
#include "settings/configuration.h"
#include "settings/configuration.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"

using std::make_shared;

PrototypeView * PrototypeView::mpThis = nullptr;

PrototypeView * PrototypeView::getInstance()
{
    if (!mpThis)
    {
        mpThis =  new PrototypeView();
    }
    return mpThis;
}

void PrototypeView::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

PrototypeView::PrototypeView() : LayerController("PrototypeView")
{
    protoMaker = PrototypeMaker::getInstance();

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

    drawCenter(painter);
}

void PrototypeView::draw()
{
    qDebug() << "PrototypeView::draw";

    auto data = protoMaker->getProtoMakerData();

    for (auto & proto : data->getPrototypes())
    {
        if (!data->isHidden(MVD_PROTO,proto))
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

        for(auto & edge : qAsConst(map->getEdges()))
        {
            edges.push_back(edge);
        }

        QPen pen(colors.mapColor,lineWidth);
        edges.draw(gg, pen);
        edges.drawPts(gg, pen);
    }

    FillRegion flood(proto->getTiling(),ViewControl::getInstance()->getFillData());
    Placements fillPlacements = flood.getPlacements(config->repeatMode);


    if (mode & (PROTO_ALL_TILES | PROTO_ALL_MOTIFS))
    {
        for (auto & T1 : fillPlacements)
        {
            for (auto & del : proto->getDesignElements())
            {
                auto motif = del->getMotif();
                auto tile  = del->getTile();

                auto tilePlacements = tiling->getPlacements(tile);
                for (auto & T0 : tilePlacements)
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
        QPen motifPen(colors.delMotifColor,lineWidth);

        // paint background of each DEL, then its tile edges and motifs
        for (auto & del : proto->getDesignElements())
        {
            auto motif  = del->getMotif();
            auto tile   = del->getTile();
            EdgePoly ep = tile->getEdgePoly();

            auto tilePlacements = tiling->getPlacements(tile);
            for (auto & T0 : tilePlacements)
            {
                gg->pushAndCompose(T0);

                gg->fillEdgePoly(ep, colors.tileBrushColor);
                gg->drawEdgePoly(ep, colors.tileBrushColor, lineWidth);

                if (mode & PROTO_DEL_TILES)
                {
                    gg->drawEdgePoly(ep,colors.delTileColor, lineWidth);
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
        for (auto & del : proto->getDesignElements())
        {
            if (!data->isHidden(MVD_PROTO,del))
            {
                auto motif = del->getMotif();
                auto tile  = del->getTile();
                auto tilePlacements = tiling->getPlacements(tile);
                for (auto & T0 : tilePlacements)
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
        for (auto & del : proto->getDesignElements())
        {
            if (!data->isHidden(MVD_PROTO,del))
            {
                auto motif = del->getMotif();
                auto tile  = del->getTile();
                auto T0 = tiling->getPlacements(tile).first();
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
