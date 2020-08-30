#ifndef FACESETVIEW_H
#define FACESETVIEW_H

#include "geometry/faces.h"
#include "viewers/geo_graphics.h"

class FaceSetView : public Layer
{
public:
    FaceSetView(WeakFacesPtr faces);

    virtual void   paint(QPainter *painter) override;
    virtual void   draw( GeoGraphics * gg );

private:
    WeakFacesPtr wfaces;
};

#endif // FACESETVIEW_H
