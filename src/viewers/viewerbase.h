#ifndef VIEWERBASE_H
#define VIEWERBASE_H

#include <QBrush>

class GeoGraphics;

typedef std::shared_ptr<class Feature>          FeaturePtr;
typedef std::shared_ptr<class Figure>           FigurePtr;

class ViewerBase
{
public:
    static void  drawFeature(GeoGraphics * gg, FeaturePtr feature, QBrush brush, QPen pen);
    static void  drawFigure (GeoGraphics * gg, FigurePtr figure,   QPen pen);
};

#endif // VIEWERBASE_H
