#include "facesetview.h"
#include "base/configuration.h"
#include "viewers/workspaceviewer.h"

FaceSetView::FaceSetView(FaceSet * set) : Layer("FaceSetView")
{
    fset = set;
}

QRectF FaceSetView::boundingRect() const
{
    QRectF rect = wsViewer->GetCanvasSettings().getCanvasRect();
    return rect;
}

void FaceSetView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    qDebug() << "FaceSetView::paint";
    Q_UNUSED(option)
    Q_UNUSED(widget)

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
