#pragma once
#ifndef MOTIF_H
#define MOTIF_H

////////////////////////////////////////////////////////////////////////////
//
// Figure.java
//
// Making the user interface operate directly on maps would be
// a hassle.  Maps are a very low level geometry and topological
// structure.  Not so good for interacting with users.  So I
// define a motif class, which is a higher level structure --
// an object that knows how to build maps.  Subclasses of MOtaif
// understand different ways of bulding maps, but have the advantage
// of being parameterizable at a high level.

#include <QPolygonF>
#include <QGraphicsItem>
#include <QPainter>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <QDebug>
#endif
#include "enums/emotiftype.h"
#include "motifs/extended_boundary.h"
#include "geometry/debug_map.h"

typedef std::shared_ptr<class Motif>     MotifPtr;
typedef std::shared_ptr<class Map>       MapPtr;
typedef std::shared_ptr<class DebugMap>  DebugMapPtr;

class Motif
{
   friend class ExplicitMapMotif;

public:
    Motif();
    Motif(const Motif & other);
    Motif(MotifPtr other);
    virtual ~Motif();

    virtual void    resetMotifMaps()        = 0;
    virtual void    buildMotifMaps()        = 0;

    virtual QTransform getMotifTransform()  = 0;
    virtual MapPtr  getMotifMap()           = 0;
    virtual QString getMotifDesc()          = 0;

    QTransform      getDELTransform();
    void            scaleAndRotate();

    virtual void    setN(int n)                     { _n = n; }
    int             getN()                          { return _n; }

    inline void     setTile(TilePtr tile)           { _tile = tile; }
    inline TilePtr  getTile()                       { return _tile; }

    void            setMotifType(eMotifType ft)     { motifType = ft; }
    eMotifType      getMotifType()  const           { return motifType; }
    QString         getMotifTypeString()            { return sMotifType[getMotifType()]; }
    static QString  getTypeString(eMotifType type)  { return sMotifType[type]; }

    virtual void    setMotifScale(qreal scale)      { motifScale = scale; }
    virtual qreal   getMotifScale()                 { return motifScale; }
    void            setMotifRotate(qreal rot)       { motifRotate = rot; }
    qreal           getMotifRotate()                { return motifRotate; }

    void            buildMotifBoundary();
    QPolygonF &     getMotifBoundary()              { return _motifBoundary; }

    void            buildExtendedBoundary();
    const ExtendedBoundary& getExtendedBoundary()   { return _extendedBoundary; }
    ExtendedBoundary &      getRWExtendedBoundary() { return _extendedBoundary; }

    bool            isIrregular() const;
    bool            isRadial() const;

    void            setVersion(int ver)             { version = ver; }
    int             getVersion()                    { return version; }

    virtual bool    equals(const  MotifPtr other);

    virtual void    dump() = 0;

    static int      modulo(int i, int sz);

    DebugMapPtr     getDebugMap()                   { return debugMap; }

    static int      refs;

protected:
    void            annotateEdges();
    void            drawAnnotation(QPainter *painter, QTransform T);

    // data
    qreal            motifRotate;       // degrees
    qreal            motifScale;

    // specific data
    eMotifType       motifType;

    // generated
    MapPtr           motifMap;          // generated - the final scaled and rotated map
    DebugMapPtr      debugMap;          // generated

private:
    void            setMotifBoundary(QPolygonF p)   { _motifBoundary = p; }

    int              _n;                // number of sides = number of points
    TilePtr          _tile;
    QPolygonF        _motifBoundary;
    ExtendedBoundary _extendedBoundary;
    int              version;
};

class Points : public QVector<QPointF>
{
public:
    Points() {}
    Points(const QPolygonF & other);

    QPointF get(int index)  { return at(mod(index)); }
    QPointF next(int index) { return at(mod(index + 1)); }
    QPointF prev(int index) { return at(mod(index -1 )); }

    void    translate(QPointF p);
    void    transform(QTransform  t);

private:
    inline int mod(int i)
    {
        int sz = size();
        if (i >= 0)
            return i % sz;
        else
            return (sz >= 0 ? sz : -sz) - 1 + (i + 1) % sz;
    }
};


#endif
