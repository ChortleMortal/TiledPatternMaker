#pragma once
#ifndef CROP_H
#define CROP_H

#include <QRectF>
#include "enums/eborder.h"
#include "geometry/circle.h"
#include "geometry/edgepoly.h"

typedef std::shared_ptr<class Crop>     CropPtr;
typedef std::shared_ptr<class Tile>  TilePtr;

class Crop
{
public:
    Crop();
    Crop(CropPtr other);

    virtual void  draw(QPainter * painter, QTransform t, bool active);

    QPointF      getCenter();

    void         setRect(QRectF & rect);
    QRectF       getRect();

    void         setCircle(CirclePtr c);
    CirclePtr    getCircle() { return circle; }

    void         setPolygon(int sides, qreal scale = 1.0, qreal rotDegrees = 0.0);
    void         setPolygon(QPolygonF & p);
    QPolygonF    getPolygon();
    TilePtr      getTile() { return poly; }

    void         setType(eCropType type) { _cropType = type; }
    eCropType    getCropType() { return _cropType; }
    QString      getCropString();

    void         setAspect(eAspectRatio ar) { _aspect = ar; adjust(); }
    eAspectRatio getAspect() { return _aspect; }

    void         setAspectVertical(bool set) { _vAspect = set; adjust(); }
    bool         getAspectVertical() { return _vAspect; }

    void        transform(QTransform t);

    void        embed() { _embed = true; }
    void        apply() { _apply = true; }
    void        use()   { _embed = true;  _apply = true; }
    void        unuse() { _embed = false; _apply = false;}
    bool        isEmbedded() { return _embed; }
    bool        isApplied()  { return _apply; }
    bool        isUsed()     { if (_embed && _apply) return true; else return false; }

protected:
    void        adjust();

    bool         _embed;
    bool         _apply;
    eCropType    _cropType;
    eAspectRatio _aspect;
    bool         _vAspect;

    TilePtr    poly;          // model units
    CirclePtr     circle;        // model units
    QRectF       _rect;          // model units

private:
};

#endif // CROP_H
