#include "facesetview.h"
#include "base/canvas.h"
#include "base/configuration.h"

FaceSetView::FaceSetView(FaceSet * set) : Layer("FaceSetView")
{
    fset = set;
}

QRectF FaceSetView::boundingRect() const
{
    Canvas * canvas  = Canvas::getInstance();
    return canvas->getCanvasSettings().getRectF();
}


void FaceSetView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    qDebug() << "FaceSetView::paint";
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);
    layerPen = QPen(Qt::black,3);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);
    draw(&gg);
}


void  FaceSetView::draw(GeoGraphics * gg )
{
    layerPen.setColor(Qt::green);
    for (auto it = fset->begin(); it != fset->end(); it++)
    {
        FacePtr fp = *it;
        PolyPtr pp = fp->getPolygon();
        gg->drawPolygon(*pp,layerPen,QBrush());
    }

    Configuration * config = Configuration::getInstance();
    FacePtr selectedFace   = config->selectedFace;
    if (selectedFace)
    {
        PolyPtr pp = selectedFace->getPolygon();
        layerPen.setColor(Qt::red);
        gg->drawPolygon(*pp,layerPen,QBrush());
    }
}
