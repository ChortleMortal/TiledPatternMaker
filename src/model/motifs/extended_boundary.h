#pragma once

#include <QPolygonF>
#include <QDebug>
#include <QTransform>

class Motif;

typedef std::shared_ptr<class Tile>  TilePtr;
typedef std::shared_ptr<class Motif> MotifPtr;

class ExtendedBoundary
{
public:
    ExtendedBoundary(uint nsides);
    ExtendedBoundary(const ExtendedBoundary & other);

    ExtendedBoundary& operator=(const ExtendedBoundary& rhs) {
        sides    = rhs.sides;
        scale    = rhs.scale;
        rotate   = rhs.rotate;
        tileScale= rhs.tileScale;
        boundary = rhs.boundary;
        return *this;}

    const QPolygonF getPoly() const;

    void  buildRadial(TilePtr tile);
    void  buildExplicit(TilePtr tile);

    void  setRadial(bool radial)    { this->radial = radial; }

    void  setSides(int sides)       { this->sides = sides; }
    int   getSides()    const       { return sides; }

    void  setScale(qreal scale)     { this->scale = scale; }
    qreal getScale()    const       { return scale; }
    qreal getRadius()   const       { return scale * tileScale; }

    void  setRotate(qreal rotate)   { this->rotate = rotate; }
    qreal getRotate()    const      { return rotate; }

    QTransform getTransform() const { return QTransform::fromScale(scale,scale).rotate(rotate); };

    bool equals(const ExtendedBoundary & other);
    bool isCircle() const { return sides <= 2; }
    bool isUnity(Motif *motif);

protected:
    void set(const QPolygonF & p);

private:
    int   sides;
    qreal scale;
    qreal rotate;
    qreal tileScale;
    bool  radial;

    QPolygonF boundary;
};
