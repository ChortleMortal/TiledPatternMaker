#include "viewers/prototype_view.h"
#include "viewers/viewcontrol.h"
#include "viewers/viewerbase.h"
#include "geometry/map.h"
#include "geometry/fill_region.h"
#include "makers/motif_maker/motif_maker.h"
#include "misc/geo_graphics.h"
#include "mosaic/design_element.h"
#include "mosaic/prototype.h"
#include "settings/configuration.h"
#include "settings/configuration.h"
#include "tile/placed_feature.h"
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

PrototypeView::PrototypeView() : LayerController("ProtoFeatureView")
{
    colors.setColors(config->protoViewColors);
}

void PrototypeView::paint(QPainter *painter)
{
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

    qDebug() << "ProtoFeatureView  proto="  << proto.get();

    QVector<PlacedDesignElement> pdels;
    for (auto & placedFeature : qAsConst(tiling->getPlacedFeatures()))
    {
        FeaturePtr feature  = placedFeature->getFeature();
        QTransform T        = placedFeature->getTransform();
        FigurePtr fig       = proto->getFigure(feature );

        PlacedDesignElement pdel(feature,fig,T);
        pdels.push_back(pdel);
    }

    int mode = Layer::config->protoViewMode;
    if (mode & PROTO_DRAW_MAP)
    {
        MapPtr map = proto->getProtoMap();
        qDebug() << "ProtoFeatureView  proto="  << proto.get() << "protoMap" << map.get();

        for(auto & edge : qAsConst(map->getEdges()))
        {
            edges.push_back(edge);
        }

        QPen pen(colors.mapColor,3);
        edges.draw(gg, pen);
        edges.drawPts(gg, pen);
    }

    if (mode & (PROTO_DRAW_FEATURES | PROTO_DRAW_FIGURES))
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

                if (mode & PROTO_DRAW_FEATURES)
                {
                    ViewerBase::drawFeature(gg,placedDesignElement.getFeature(),QBrush(),QPen(colors.featureColor,3));
                }

                if (mode & PROTO_DRAW_FIGURES)
                {
                    ViewerBase::drawFigure(gg,placedDesignElement.getFigure(),QPen(colors.figureColor,3));
                }

                gg->pop();
            }
        }
    }

    if (mode & PROTO_DRAW_DESIGN_ELEMENT)
    {
        // do two passes so selected writees over
        for (auto & placedDesignElement : qAsConst(pdels))
        {
            FeaturePtr  feature = placedDesignElement.getFeature();
            bool selected = (feature == ViewControl::getInstance()->getSelectedFeature());
            if (!selected)
            {
                drawPlacedDesignElement(gg, placedDesignElement, QPen(colors.delFigureColor,3), QBrush(colors.featureBrushColor), QPen(colors.delFeatureColor,3),selected);
            }
        }
    }

    if (mode & (PROTO_DEL_FEATURES | PROTO_DEL_FIGURES))
    {
        for (auto placedDesignElement : pdels)
        {
            QTransform T0 = placedDesignElement.getTransform();
            gg->pushAndCompose(T0);

            if (mode & PROTO_DEL_FEATURES)
            {
                QPen pen(colors.delFeatureColor,3);
                ViewerBase::drawFeature(gg,placedDesignElement.getFeature(),QBrush(),pen);
            }
            if  (mode & PROTO_DEL_FIGURES)
            {
                QPen pen(colors.delFigureColor,3);
                ViewerBase::drawFigure(gg,placedDesignElement.getFigure(),pen);
            }
            gg->pop();
        }
    }

    // always do this
    for (auto placed : qAsConst(pdels))
    {
        FeaturePtr  feature = placed.getFeature();
        bool selected = (feature == ViewControl::getInstance()->getSelectedFeature());
        if (selected)
        {
            drawPlacedDesignElement(gg, placed, QPen(colors.delFigureColor,3), QBrush(), QPen(colors.delFeatureColor,3),selected);
        }
    }
}

void PrototypeView::drawPlacedDesignElement(GeoGraphics * gg, const PlacedDesignElement &  pde, QPen figurePen, QBrush featureBrush, QPen featurePen, bool selected)
{
    QTransform T = pde.getTransform();
    gg->pushAndCompose(T);

    FeaturePtr fp = pde.getFeature();
    QPen pen;
    if (selected)
        pen = QPen(Qt::red,3);
    else
        pen = featurePen;
    ViewerBase::drawFeature(gg,fp,featureBrush,pen);

    // Draw the figure
    FigurePtr fig = pde.getFigure();
    ViewerBase::drawFigure(gg,fig,figurePen);

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
    qsl << featureColor.name(QColor::HexArgb);
    qsl << figureColor.name(QColor::HexArgb);
    qsl << delFeatureColor.name(QColor::HexArgb);
    qsl << delFigureColor.name(QColor::HexArgb);
    qsl << featureBrushColor.name(QColor::HexArgb);
    return qsl;
}

void ProtoViewColors::setColors(QStringList & colors)
{
    int index = 0;
    mapColor            = QColor(colors.at(index++));
    featureColor        = QColor(colors.at(index++));
    figureColor         = QColor(colors.at(index++));
    delFeatureColor     = QColor(colors.at(index++));
    delFigureColor      = QColor(colors.at(index++));
    featureBrushColor   = QColor(colors.at(index++));
}
