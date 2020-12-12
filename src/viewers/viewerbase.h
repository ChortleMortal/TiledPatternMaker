#ifndef VIEWERBASE_H
#define VIEWERBASE_H

#include <QtCore>
#include "base/shared.h"

class GeoGraphics;

class ViewerBase
{
public:
    static void  drawFeature(GeoGraphics * gg, FeaturePtr feature, QBrush brush, QPen pen);
    static void  drawFigure (GeoGraphics * gg, FigurePtr figure,   QPen pen);
};

#endif // VIEWERBASE_H
