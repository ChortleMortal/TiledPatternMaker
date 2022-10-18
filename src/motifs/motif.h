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

#ifndef MOTIF_H
#define MOTIF_H

#include <QPolygonF>
#include <QGraphicsItem>
#include <QPainter>
#include "enums/emotiftype.h"

typedef std::shared_ptr<class Motif>     MotifPtr;
typedef std::shared_ptr<class Tile>      TilePtr;
typedef std::shared_ptr<class Map>       MapPtr;
typedef std::shared_ptr<class DebugMap>  DebugMapPtr;

class ExtendedBoundary
{
public:
    ExtendedBoundary();
    ExtendedBoundary(const ExtendedBoundary & other);

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

class Motif
{
public:
    Motif();
    Motif(const Motif & other);
    virtual ~Motif();

    virtual MapPtr  getMap()        = 0;
    virtual void    resetMaps()     = 0;
    virtual void    buildMaps()     = 0;
    virtual QString getMotifDesc()  = 0;

    virtual DebugMapPtr  getDebugMap()              { return debugMap; }

    virtual void    setN(int n)                     { this->n = n; }
    int             getN();

    QPolygonF       getPoints()                     { return points; }
    int             size()                          { return points.size(); }

    void            setMotifType( eMotifType ft)      { motifType = ft; }
    eMotifType        getMotifType()                  { return motifType; }
    QString         getMotifTypeString()            { return sTileType[getMotifType()]; }
    static QString  getTypeString(eMotifType type)    { return sTileType[type]; }

    virtual void    setMotifScale(qreal scale)      { motifScale = scale; }
    virtual qreal   getMotifScale()                 { return motifScale; }
    void            setMotifRotate(qreal rot)       { motifRotate = rot; }
    qreal           getMotifRotate()                { return motifRotate; }

    void            buildExtendedRadialBoundary()   { extendedBoundary.buildRadial(); }
    void            buildExtendedExplicitBoundary();

    void            setMotifBoundary(QPolygonF p)   { //Q_ASSERT(p.isClosed());
                                                      //qDebug() << "set fig b" << p;
                                                      motifBoundary = p; }
    QPolygonF &     getMotifBoundary()              { //Q_ASSERT(figureBoundary.isClosed());
                                                      //qDebug() << "get fig b" << figureBoundary4;
                                                      return motifBoundary; }

    const ExtendedBoundary &  getExtendedBoundary()   { return extendedBoundary; }
    ExtendedBoundary &        getRWExtendedBoundary() { return extendedBoundary; }

    bool            isExplicit();
    bool            isRadial();

    bool            getDisplay()                    { return display; }
    void            setDisplay(bool val)            { display = val; }

    virtual bool    equals(const  MotifPtr other);

    static int      refs;

protected:

    void            annotateEdges();
    void            drawAnnotation(QPainter *painter, QTransform T);

    MapPtr          motifMap;
    DebugMapPtr     debugMap;
    QPolygonF       points;

    int             n;                      // number of sides = number of points

    qreal           motifRotate;
    qreal           motifScale;

private:
    QPolygonF        motifBoundary;      // currently only set for radial motifs
    ExtendedBoundary extendedBoundary;

    eMotifType         motifType;

    bool             display;                // used by protoview
};

#endif
