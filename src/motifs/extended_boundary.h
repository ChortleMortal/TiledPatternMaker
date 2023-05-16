#pragma once
#ifndef EXTENDEDBOUNDARY_H
#define EXTENDEDBOUNDARY_H

#include <QPolygonF>
#include <memory>

typedef std::shared_ptr<class Tile>  TilePtr;

class ExtendedBoundary
{
public:
    ExtendedBoundary();
    ExtendedBoundary(const ExtendedBoundary & other);

    ExtendedBoundary& operator=(const ExtendedBoundary& rhs) {
        sides   = rhs.sides;
        scale   = rhs.scale;
        rotate  = rhs.rotate;
        boundary2 = rhs.boundary2;
        return *this;}

    void set(const QPolygonF & p);
    const QPolygonF & get() const;

    void buildRadial();
    void buildExplicit(TilePtr tile);

    bool equals(const ExtendedBoundary & other);
    bool isCircle() const { return sides <= 2; }

    int   sides;
    qreal scale;
    qreal rotate;

private:
    QPolygonF boundary2;
};

#endif // EXTENDEDBOUNDARY_H
