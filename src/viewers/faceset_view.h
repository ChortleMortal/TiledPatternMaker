#ifndef FACESETVIEW_H
#define FACESETVIEW_H

#include "geometry/faces.h"
#include "viewers/geo_graphics.h"

class FaceSetView : public Layer
{
public:
    FaceSetView(WeakColorMakerPtr cm);

    virtual void   paint(QPainter *painter) override;
    virtual void   draw( GeoGraphics * gg );

private:
    WeakColorMakerPtr wcm;
};

#endif // FACESETVIEW_H
