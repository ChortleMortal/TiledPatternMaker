#pragma once
#ifndef EXTENDEDBOUNDARY_H
#define EXTENDEDBOUNDARY_H

#include <QPolygonF>
#include <memory>
#include <QDebug>

typedef std::shared_ptr<class Tile>  TilePtr;

class ExtendedBoundary
{
public:
    ExtendedBoundary();
    ExtendedBoundary(const ExtendedBoundary & other);

    ExtendedBoundary& operator=(const ExtendedBoundary& rhs) {
        sides    = rhs.sides;
        scale    = rhs.scale;
        boundary = rhs.boundary;
        return *this;}

    const QPolygonF getPoly() const;

    void buildRadial();
    void buildExplicit(TilePtr tile);

    void setRadial(bool radial)      { this->radial   = radial; }
    void setSides(int sides)         { this->sides    = sides; }
    void setScale(qreal scale)       { this->scale    = scale; }

    int   getSides()    const { return sides; }
    qreal getScale()    const { return scale; }

    bool equals(const ExtendedBoundary & other);
    bool isCircle() const { return sides <= 2; }

protected:
    void set(const QPolygonF & p);

private:
    int   sides;
    qreal scale;
    bool  radial;

    QPolygonF boundary;
};

#endif // EXTENDEDBOUNDARY_H
