#pragma once
#ifndef SHAPEVIEWER_H
#define SHAPEVIEWER_H

#include <QPainterPath>
#include "misc/layer.h"


class Polyform;
class Configuration;

class ShapeViewer : public Layer
{
public:
    ShapeViewer();

    void addPolyform(Polyform * p) { polyforms.push_back(p); }

    QVector<Polyform*> & getPolyforms() { return polyforms;}

    void enableAntialiasing(bool enable) { _antiAliasPolys = enable; }

    void iamaLayer() override {}

protected:
    void    paint(QPainter *painter) override;

    QVector<Polyform *>  polyforms;

    bool            _antiAliasPolys;
    QPainterPath    ppath;
    QPen            ppPen;
    QBrush          ppBrush;

private:
    Configuration * config;
};

#endif // SHAPEVIEWER_H
