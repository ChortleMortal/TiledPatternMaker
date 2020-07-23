#ifndef VIEWERBASE_H
#define VIEWERBASE_H

#include <QtCore>
#include "base/shared.h"

class GeoGraphics;

class ViewerBase
{
public:
    static void  drawFeature(GeoGraphics * gg, FeaturePtr fp, QBrush brush, QPen pen);
    static void  drawFigure(GeoGraphics * gg, FigurePtr fig, QPen pen);
};

#endif // VIEWERBASE_H
