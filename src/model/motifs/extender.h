#pragma once

#include <QLineF>

#include "model/motifs/extended_boundary.h"
#include "model/motifs/radial_ray.h"
#include "sys/geometry/edge_poly.h"

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
    Extender(Motif * motif);
    Extender(const Extender & other);

    Extender& operator=(const Extender& rhs);

    void    reparent(Motif * motif)  { this->motif = motif; }
    Motif * getParent()              { return motif; }

    bool    equals(ExtenderPtr other);
    bool    isUnity();

    virtual void    extend() = 0;
    void    buildExtendedBoundary();

    inline const ExtendedBoundary& getExtendedBoundary()   { return _extendedBoundary; }
    inline ExtendedBoundary &      getRWExtendedBoundary() { return _extendedBoundary; }

    void    setExtendRays(bool extend)          { _extendRays           = extend; }
    void    setExtendTipsToBound(bool extend)   { _extendTipsToBound    = extend; }
    void    setExtendTipsToTile(bool extend)    { _extendTipsToTile     = extend; }
    void    setConnectRays(uint connect)        { _connectRays          = connect; }
    void    setEmbedBoundary(bool set)          { _embedBoundary        = set; }
    void    setEmbedTile(bool set)              { _embedTile            = set; }
    void    setStellate(uint stellate)          { _stellation           = stellate; }
    void    setFitToTile(bool set)              { _fitToTile            = set; }
    void    setFitScale(qreal scale)            { _fitScale             = scale; }

    bool    getExtendRays()              { return _extendRays; }
    bool    getExtendTipsToBound()       { return _extendTipsToBound; }
    bool    getExtendBoundaryToTile()    { return _extendTipsToTile; }
    uint    getConnectRays()             { return _connectRays; }
    bool    getEmbedBoundary()           { return _embedBoundary; }
    bool    getEmbedTile()               { return _embedTile; }
    uint    getStellation()              { return _stellation; }
    bool    getFitToTile()               { return _fitToTile; }
    qreal   getFitScale()                { return _fitScale; }

    void    dump();

protected:
    void    embedBoundaryInMap(MapPtr motifMap);
    void    embedTileInMap(MapPtr motifMap, TilePtr tile);

    qreal   len(VertexPtr v1, VertexPtr v2);
    QLineF  perpLine(QLineF line, QPointF pt);

    Motif * motif;

private:
    bool    _extendRays;
    bool    _extendTipsToBound;
    bool    _extendTipsToTile;
    uint    _connectRays;
    uint    _stellation;
    bool    _embedBoundary;
    bool    _embedTile;
    bool    _fitToTile;
    qreal   _fitScale;  // calculated

    ExtendedBoundary _extendedBoundary;
};

