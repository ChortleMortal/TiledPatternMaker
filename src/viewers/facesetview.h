#ifndef FACESETVIEW_H
#define FACESETVIEW_H

#include "geometry/Faces.h"
#include "viewers/GeoGraphics.h"

class FaceSetView : public Layer
{
public:
    FaceSetView(FaceSet * set);

    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;
    virtual void   paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    virtual void   draw( GeoGraphics * gg );

private:
    FaceSet * fset;
};

#endif // FACESETVIEW_H
