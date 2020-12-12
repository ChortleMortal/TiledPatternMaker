#include "viewers/faceset_view.h"
#include "base/configuration.h"

FaceSetView::FaceSetView(WeakFacesPtr faces) : Layer("FaceSetView",LTYPE_VIEW)
{
    wfaces = faces;
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

    drawCenter(painter);
}

void  FaceSetView::draw(GeoGraphics * gg )
{
    FacesPtr faces = wfaces.lock();
    if (!faces)
    {
        return;
    }

    const FaceSet & fset = faces->getAllFaces();
    for (auto face : fset)
    {
        gg->drawPolygon( face->getPolygon(),Qt::green,3);
    }

    Configuration * config     = Configuration::getInstance();
    WeakFacePtr selectedFace   = config->selectedFace;
    FacePtr face               = selectedFace.lock();
    if (face)
    {
        gg->drawPolygon(face->getPolygon(),Qt::red,3);
    }
}
