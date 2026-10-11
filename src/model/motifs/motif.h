#pragma once

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

#include "model/motifs/extender.h"
//#include "sys/geometry/debug_map.h"
//#include "sys/geometry/map.h"

typedef std::shared_ptr<class Motif>     MotifPtr;
typedef std::shared_ptr<class Map>       MapPtr;

extern const QString sMotifType[];

class Motif
{
   friend class ExplicitMapMotif;

public:
    // these are the types of figure which can be made
    enum class Type
    {
        Undefined,
        Radial,
        Rosette,
        Rosette2,

        Star,
        Star2,

        MapMotif,
        Inferred,
        AdaptiveRosette,
        Hourglass,
        Intersect,
        Girih,
        AdaptiveStar,
        ExplicitTile,
        NoMapMotif,

        MaxMotifType = NoMapMotif
    };

    Motif();
    Motif(const Motif & other);
    virtual ~Motif();

    virtual void    build()                 = 0;

    virtual QTransform getMotifTransform()  = 0;

    virtual QString getMotifDesc()          = 0;

    virtual void    resetMotifMap();
    virtual MapPtr  getMotifMap();
    virtual MapPtr  getExistingMotifMap();
    virtual MapPtr  getValidMotifMap();
    virtual bool    hasValidMotifMap();

    virtual void    setN(int n)                     { _n = n; }
    int             getN() const                    { return _n; }

    inline void     setTile(TilePtr tile)           { _tile = tile; }
    inline TilePtr  getTile()                       { return _tile; }

    void            setMotifType(Type type)         { motifType = type; }
    Type            getMotifType()  const           { return motifType; }
    QString         getMotifTypeString()            { return sMotifType[(int)getMotifType()]; }
    static QString  getTypeString(Type type)        { return sMotifType[(int)type]; }

    virtual void    setMotifScale(qreal scale)      { motifScale = scale; }
    virtual qreal   getMotifScale()                 { return motifScale; }
    void            setMotifRotate(qreal rot)       { motifRotate = rot; }
    qreal           getMotifRotate()                { return motifRotate; }

    QPolygonF &     getMotifBoundary()              { return _motifBoundary; }
    void            setMotifBoundary(QPolygonF p)   { _motifBoundary = p; }

    virtual ExtenderPtr createExtender()  = 0;
    void            addExtender(ExtenderPtr ep);
    void            deleteExtender(ExtenderPtr extender);
    ExtenderPtr     getExtender(uint i);
    QVector<ExtenderPtr> & getExtenders()           { return _extenders; }
    QVector<ExtenderPtr> get_extenders()  const     { return _extenders; }
    int             numExtenders()                  { return _extenders.count(); }

    bool            isIrregular() const;
    bool            isRadial() const;

    virtual int     maxVersions() = 0;
    void            setVersion(int ver)             { _motifVersion = ver; }
    int             getVersion()                    { return _motifVersion; }

    void            setCleanse(uint val)            { cleanseVal = val; }
    void            setCleanseSensitivity(qreal val){ sensitivity = val; }
    uint            getCleanse()                    { return cleanseVal; }
    qreal           getCleanseSensitivity()         { return sensitivity; }

    virtual bool    equals(const  MotifPtr other);
    static int      modulo(int i, int sz);

    virtual void    dump() = 0;
    uint            getMotifDebug()                 { return _motifDebug; }
    void            setMotifDebug(uint val)         { _motifDebug = val;  }

    static int      refs;

protected:
    void            cleanseMotifMap();

    // data
    qreal            motifRotate;       // degrees
    qreal            motifScale;

    // specific data
    Type            motifType;

    // generated
    MapPtr           motifMap;          // generated - the final scaled and rotated map
    qreal            sensitivity;       // cleanse sensitivity
    uint             cleanseVal;

    QVector<ExtenderPtr> _extenders;

private:
    int                  _n;            // number of sides = number of points
    TilePtr              _tile;
    QPolygonF            _motifBoundary;
    uint                 _motifDebug;   // set as needed
    int                  _motifVersion;
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
