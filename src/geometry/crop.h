#pragma once
#ifndef CROP_H
#define CROP_H

#include <QRectF>
#include "enums/eborder.h"
#include "geometry/circle.h"
#include "geometry/edgepoly.h"

typedef std::shared_ptr<class Crop>     CropPtr;

//
//  The Crop class defines the parameters of the crop and whether it is to be applied or embeeded or both
//  The Prototype class uses the crop to modify its map using fucntions in the map class
//
//  Borders also use the crop class, but borders form a layer superimposed on top of prototype maps
//
//  Standalone crops are stored in Model Units
//  Border Crops are stored in Screen Units because they are the frames of the canvas
//  TODO - review whether crops should use screen units for borders

class Crop
{
public:
    Crop();
    Crop(CropPtr other);

    void         draw(QPainter * painter, QTransform t, bool active);

    QPointF      getCenter();

    void         setRect(QRectF & rect);
    QRectF &     getRect() { return _rect; }

    void         setCircle(Circle & c);
    Circle &     getCircle() { return _circle; }

    void         setPolygon(int sides, qreal scale = 1.0, qreal rotDegrees = 0.0);
    void         setPolygon(QPolygonF & p);
    QPolygonF &  getPolygon() { return _poly; }

    void         setType(eCropType type) { _cropType = type; }
    eCropType    getCropType()           { return _cropType; }
    QString      getCropString();

    void         setAspect(eAspectRatio ar) { _aspect = ar; adjust(); }
    eAspectRatio getAspect() { return _aspect; }

    void         setAspectVertical(bool set) { _vAspect = set; adjust(); }
    bool         getAspectVertical() { return _vAspect; }

    void        transform(QTransform t);

    void        setEmbed(bool embed) { _embed = embed; }
    void        setApply(bool apply) { _apply = apply; }
    bool        getEmbed()           { return _embed; }
    bool        getApply()           { return _apply; }

    void        dbgInfo();

protected:
    void        adjust();

    eCropType    _cropType;

private:
    eAspectRatio _aspect;
    bool         _vAspect;

    QPolygonF    _poly;          // model units as crops, screen units for borders
    Circle       _circle;        // model units as crops, screen units for borders
    QRectF       _rect;          // model units as crops, screen units for borders

    bool         _embed;
    bool         _apply;
};

#endif // CROP_H
