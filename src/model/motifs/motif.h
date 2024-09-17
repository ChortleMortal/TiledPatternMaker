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
#include "sys/enums/emotiftype.h"
#include "model/motifs/extender.h"
#include "model/motifs/motif_connector.h"
#include "sys/geometry/debug_map.h"

typedef std::shared_ptr<class Motif>     MotifPtr;
typedef std::shared_ptr<class Map>       MapPtr;

class Motif : public std::enable_shared_from_this<Motif>
{
   friend class ExplicitMapMotif;

public:
    Motif();
    Motif(const Motif & other);
    Motif(MotifPtr other);
    virtual ~Motif();

    virtual void    resetMotifMap()         = 0;
    virtual void    buildMotifMap()         = 0;

    virtual QTransform getMotifTransform()  = 0;
    virtual QString getMotifDesc()          = 0;
    virtual MapPtr  getMotifMap()           = 0;
    virtual MapPtr  getExistingMotifMap()           { return motifMap; }

    QTransform      getDELTransform();
    void            scaleAndRotate(MapPtr map);

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

    ExtenderPtr     addExtender();
    void            deleteExtender(ExtenderPtr extender);
    ExtenderPtr     getExtender(uint i);
    const QVector<ExtenderPtr> & getExtenders()     { return _extenders; }
    void            cleanExtenders();

    ConnectPtr      addConnector();
    ConnectPtr      addRadialConnector()            { connector = std::make_shared<MotifConnector>();  return connector; }
    ConnectPtr      getRadialConnector() const      { return connector; }
    void            deleteRadialConnector()         { connector.reset(); }

    bool            isIrregular() const;
    bool            isRadial() const;

    void            setVersion(int ver)             { version = ver; }
    int             getVersion()                    { return version; }

    void            setCleanse(uint val)            { cleanseVal = val; }
    void            setCleanseSensitivity(qreal val){ sensitivity = val; }
    uint            getCleanse()                    { return cleanseVal; }
    qreal           getCleanseSensitivity()         { return sensitivity; }

    virtual bool    equals(const  MotifPtr other);

    virtual void    dump() = 0;

    static int      modulo(int i, int sz);

    DebugMap *      getDebugMap()                   { return debugMap; }
    uint            getDbgVal()                     { return dbgVal; }

    static int      refs;

protected:
    void            annotateEdges();
    void            drawAnnotation(QPainter *painter, QTransform T);

    // data
    qreal            motifRotate;       // degrees
    qreal            motifScale;

    // specific data
    eMotifType       motifType;

    // connector
    ConnectPtr       connector;

    // generated
    MapPtr           motifMap;          // generated - the final scaled and rotated map
    qreal            sensitivity;       // cleanse sensitivity
    uint             cleanseVal;

    DebugMap *       debugMap;          // generated
    uint             dbgVal;            // set as needed

private:
    void             setMotifBoundary(QPolygonF p)   { _motifBoundary = p; }

    int                  _n;            // number of sides = number of points
    TilePtr              _tile;
    QPolygonF            _motifBoundary;
    QVector<ExtenderPtr> _extenders;
    int                  version;
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
