#include "viewers/prototype_view.h"
#include "viewers/viewcontrol.h"
#include "viewers/viewerbase.h"
#include "motifs/motif.h"
#include "geometry/map.h"
#include "geometry/fill_region.h"
#include "makers/motif_maker/motif_maker.h"
#include "misc/geo_graphics.h"
#include "mosaic/design_element.h"
#include "mosaic/prototype.h"
#include "settings/configuration.h"
#include "settings/configuration.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"

using std::make_shared;

PrototypeViewPtr PrototypeView::spThis;

PrototypeViewPtr PrototypeView::getSharedInstance()
{
    if (!spThis)
    {
        spThis = make_shared<PrototypeView>();
    }
    return spThis;
}

PrototypeView::PrototypeView() : LayerController("PrototypeView")
{
    colors.setColors(config->protoViewColors);
}

void PrototypeView::paint(QPainter *painter)
{
    lineWidth = config->protoviewWidth;

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);

    drawCenter(painter);
}

void PrototypeView::draw( GeoGraphics * gg )
{
    TilingPtr tiling = proto->getTiling();
    if (!tiling)
        return;

    qDebug() << "PrototypeView  proto="  << proto.get();

    QVector<PlacedDesignElement> pdels;
    for (auto & placedTile : qAsConst(tiling->getData().getPlacedTiles()))
    {
        TilePtr tile   = placedTile->getTile();
        QTransform T   = placedTile->getTransform();
        MotifPtr motif = proto->getMotif(tile);

        PlacedDesignElement pdel(tile,motif,T);
        pdels.push_back(pdel);
    }

    int mode = Layer::config->protoViewMode;
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

    if (mode & (PROTO_DRAW_TILES | PROTO_DRAW_MOTIFS))
    {
        FillRegion flood(proto->getTiling(),ViewControl::getInstance()->getFillData());
        QVector<QTransform> transforms = flood.getTransforms();
        for (auto T1 : qAsConst(transforms))
        {
            for (auto & placedDesignElement : qAsConst(pdels))
            {
                QTransform T0 = placedDesignElement.getTransform();
                QTransform T2 = T0 * T1;

                gg->pushAndCompose(T2);

                if (mode & PROTO_DRAW_TILES)
                {
                    ViewerBase::drawTile(gg,placedDesignElement.getTile(),QBrush(),QPen(colors.tileColor,lineWidth));
                }

                if (mode & PROTO_DRAW_MOTIFS)
                {
                    auto motif = placedDesignElement.getMotif();
                    if (motif->getDisplay())
                    {
                        ViewerBase::drawMotif(gg,motif,QPen(colors.motifColor,lineWidth));
                    }
                }

                gg->pop();
            }
        }
    }

    if (mode & PROTO_DRAW_DESIGN_ELEMENT)
    {
        // do two passes so selected writes over
        for (auto & placedDesignElement : qAsConst(pdels))
        {
            TilePtr  tile = placedDesignElement.getTile();
            if (tile != selectedDEL.wtilep.lock())
            {
                drawPlacedDesignElement(gg, placedDesignElement, QPen(colors.delMotifColor,lineWidth), QBrush(colors.tileBrushColor), QPen(colors.delTileColor,lineWidth),false);
            }
        }
    }

    if (mode & (PROTO_DEL_TILES | PROTO_DEL_MOTIFS))
    {
        for (auto & placedDesignElement : qAsConst(pdels))
        {
            QTransform T0 = placedDesignElement.getTransform();
            gg->pushAndCompose(T0);

            if (mode & PROTO_DEL_TILES)
            {
                QPen pen(colors.delTileColor,lineWidth);
                ViewerBase::drawTile(gg,placedDesignElement.getTile(),QBrush(),pen);
            }
            if  (mode & PROTO_DEL_MOTIFS)
            {

                auto figure = placedDesignElement.getMotif();
                if (figure->getDisplay())
                {
                    QPen pen(colors.delMotifColor,lineWidth);
                    ViewerBase::drawMotif(gg,placedDesignElement.getMotif(),pen);
                }
            }
            gg->pop();
        }
    }

    // always do this
    for (auto & placed : qAsConst(pdels))
    {
        TilePtr  tile = placed.getTile();
        if (tile == selectedDEL.wtilep.lock())
        {
            drawPlacedDesignElement(gg, placed, QPen(colors.delMotifColor,lineWidth), QBrush(), QPen(colors.delTileColor,lineWidth),true);
        }
    }
}

void PrototypeView::drawPlacedDesignElement(GeoGraphics * gg, const PlacedDesignElement &  pde, QPen motifPen, QBrush tileBrush, QPen tilePen, bool selected)
{
    QTransform T = pde.getTransform();
    gg->pushAndCompose(T);

    TilePtr fp = pde.getTile();
    QPen pen;
    if (selected)
        pen = QPen(Qt::red,lineWidth);
    else
        pen = tilePen;
    ViewerBase::drawTile(gg,fp,tileBrush,pen);

    // Draw the figure
    MotifPtr fig = pde.getMotif();
    if (fig->getDisplay())
    {
        ViewerBase::drawMotif(gg,fig,motifPen);
    }
    gg->pop();
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
    mapColor            = QColor(colors.at(index++));
    tileColor        = QColor(colors.at(index++));
    motifColor         = QColor(colors.at(index++));
    delTileColor     = QColor(colors.at(index++));
    delMotifColor      = QColor(colors.at(index++));
    tileBrushColor   = QColor(colors.at(index++));
}
