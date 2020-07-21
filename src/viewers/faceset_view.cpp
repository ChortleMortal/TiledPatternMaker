#include "faceset_view.h"
#include "base/configuration.h"
#include "viewers/workspace_viewer.h"

FaceSetView::FaceSetView(FaceSet * set) : Layer("FaceSetView")
{
    fset = set;
}

void FaceSetView::paint(QPainter *painter)
{
    qDebug() << "FaceSetView::paint";

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);
    layerPen = QPen(Qt::black,3);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);

    if (config->showCenter)
    {
        QPointF pt = getCenter();
        qDebug() << "style layer center=" << pt;
        painter->setPen(QPen(Qt::green,3));
        painter->setBrush(QBrush(Qt::green));
        painter->drawEllipse(pt,13,13);
    }
}

void  FaceSetView::draw(GeoGraphics * gg )
{
    for (auto face : *fset)
    {
        gg->drawPolygon( face->getPolygon(),Qt::green,3);
    }

    Configuration * config = Configuration::getInstance();
    FacePtr selectedFace   = config->selectedFace;
    if (selectedFace)
    {
        gg->drawPolygon(selectedFace->getPolygon(),Qt::red,3);
    }
}
