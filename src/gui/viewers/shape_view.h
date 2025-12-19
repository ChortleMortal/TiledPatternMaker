#pragma once
#ifndef SHAPEVIEWER_H
#define SHAPEVIEWER_H

#include <QPainterPath>
#include "gui/viewers/layer.h"


class Polyform;

class LegacyShapeViewer : public Layer
{
public:
    LegacyShapeViewer();

    void addPolyform(Polyform * p) { polyforms.push_back(p); }

    QVector<Polyform*> & getPolyforms() { return polyforms;}

    void enableAntialiasing(bool enable) { _antiAliasPolys = enable; }

protected:
    void    paint(QPainter *painter) override;

    QVector<Polyform *>  polyforms;

    bool            _antiAliasPolys;
    QPainterPath    ppath;
    QPen            ppPen;
    QBrush          ppBrush;
};

#endif // SHAPEVIEWER_H
