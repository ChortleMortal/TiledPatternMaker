#pragma once
#ifndef EXTENDER_H
#define EXTENDER_H

#include <QtCore>
#include "sys/qt/unique_qvector.h"
#include "model/motifs/extended_boundary.h"
#include "model/motifs/radial_ray.h"

typedef std::shared_ptr<class Vertex>     VertexPtr;
typedef std::shared_ptr<class Map>        MapPtr;
typedef std::shared_ptr<class Tile>       TilePtr;
typedef std::shared_ptr<class Extender>   ExtenderPtr;
typedef std::shared_ptr<class Motif>      MotifPtr;
typedef std::weak_ptr<class Motif>        WeakMotifPtr;

class Motif;
class Star2;

class Extender
{
    friend class MotifView;
    friend class RadialMotif;

public:
    Extender(MotifPtr motif);
    Extender(const Extender & other);

    Extender& operator=(const Extender& rhs)  {
        _extendedBoundary   = rhs._extendedBoundary;
        _extendTipsToBound  = rhs._extendTipsToBound;
        _extendRays         = rhs._extendRays;
        _extendTipsToTile   = rhs._extendTipsToTile;
        _connectRays        = rhs._connectRays;
        _motif              = rhs._motif;
        return *this; }

    bool    equals(ExtenderPtr other);
    bool    isUnity();

    void    extendMotifMap(QTransform Tr = QTransform());
    void    buildExtendedBoundary();

    inline const ExtendedBoundary& getExtendedBoundary()   { return _extendedBoundary; }
    inline ExtendedBoundary &      getRWExtendedBoundary() { return _extendedBoundary; }

    void    setExtendRays(bool extend)          { _extendRays           = extend; }
    void    setExtendTipsToBound(bool extend)   { _extendTipsToBound    = extend; }
    void    setExtendTipsToTile(bool extend)    { _extendTipsToTile     = extend; }
    void    setConnectRays(uint connect)        { _connectRays          = connect; }
    void    setEmbedBoundary(bool set)          { _embedBoundary        = set; }
    void    setEmbedTile(bool set)              { _embedTile            = set; }

    bool    getExtendRays()              { return _extendRays; }
    bool    getExtendTipsToBound()       { return _extendTipsToBound; }
    bool    getExtendBoundaryToTile()    { return _extendTipsToTile; }
    uint    getConnectRays()             { return _connectRays; }
    bool    getEmbedBoundary()           { return _embedBoundary; }
    bool    getEmbedTile()               { return _embedTile; }

    void    dump();

protected:
#if 1
    void    extendMotifRays(MapPtr motifMap);
    void    extendBoundaryMap(MapPtr motifMap, TilePtr tile);
    void    extendTipsToBoundary(MapPtr motifMap, QTransform unitRotationTr);
    void    connectOuterVertices(MapPtr motifMap);
#endif
    void    embedBoundary(MapPtr motifMap);
    void    embedTile(MapPtr motifMap, TilePtr tile);

    void    extendRayToBoundary(RadialRay & ray);
    void    extendRayToTilePerp(RadialRay & ray, TilePtr tile);
    void    extendRayToBoundaryPerp(RadialRay & ray);
    void    connectRays(uint method, RaySet & set1, RaySet & set2);

    qreal   len(VertexPtr v1, VertexPtr v2);
    QLineF  perpLine(QLineF line, QPointF pt);

private:
    bool    _extendRays;
    bool    _extendTipsToBound;
    bool    _extendTipsToTile;
    uint    _connectRays;
    bool    _embedBoundary;
    bool    _embedTile;

    ExtendedBoundary _extendedBoundary;
    WeakMotifPtr     _motif;

    UniqueQVector<VertexPtr> newVertices;
};

#endif

