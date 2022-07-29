////////////////////////////////////////////////////////////////////////////
//
// Tiling.java
//
// The representation of a tiling, which will serve as the skeleton for
// Islamic designs.  A Tiling has two translation vectors and a set of
// PlacedFeatures that make up a translational unit.  The idea is that
// the whole tiling can be replicated across the plane by placing
// a copy of the translational unit at every integer linear combination
// of the translation vectors.  In practice, we only draw at those
// linear combinations within some viewport.

#ifndef TILING
#define TILING

#include "geometry/xform.h"
#include "settings/tristate.h"
#include "settings/model_settings.h"

#define MAX_UNIQUE_FEATURE_INDEX 7

typedef std::shared_ptr<class Feature>          FeaturePtr;
typedef std::shared_ptr<class PlacedFeature>    PlacedFeaturePtr;
typedef std::shared_ptr<class Map>              MapPtr;

class FeatureGroup : public  QVector<QPair<FeaturePtr,QVector<PlacedFeaturePtr>>>
{
public:
    bool containsFeature(FeaturePtr fp);
    QVector<PlacedFeaturePtr> & getPlacements(FeaturePtr fp);
};

// Translations to tile the plane. Two needed for two-dimensional plane.
// (Of course, more complex tiling exists with rotations and mirrors.)
// (They are not supported.)

class Tiling : public std::enable_shared_from_this<Tiling>
{
public:

    enum eTilingState
    {
        EMPTY,
        LOADED,
        MODIFED
    };

    Tiling();
    Tiling(QString name, QPointF t1, QPointF t2);
    Tiling(Tiling * other);
    ~Tiling();

    bool        isEmpty();

    bool        hasIntrinsicOverlaps();
    bool        hasTiledOverlaps();

    QString     getName()        const { return name; }
    QString     getDescription() const { return desc; }
    QString     getAuthor()      const { return author; }

    void        setName(QString n)               { name = n; }
    void        setDescription(QString descrip ) { desc = descrip; }
    void        setAuthor(QString auth)          { author = auth; }

    // Feature management.
    // Added features are embedded into a PlacedFeature.
    void        add(const PlacedFeaturePtr pf );
    void        add(FeaturePtr f, QTransform T );
    void        remove(PlacedFeaturePtr pf);

    void                      setPlacedFeatures(QVector<PlacedFeaturePtr> & features);
    const QVector<PlacedFeaturePtr> & getPlacedFeatures() { return placed_features; }
    int                       countPlacedFeatures() const { return placed_features.size(); }
    int                       numPlacements(FeaturePtr fp);
    QVector<QTransform>       getPlacements(FeaturePtr fp);
    QTransform                getPlacement(FeaturePtr fp, int index);

    QVector<FeaturePtr>       getUniqueFeatures();

    void        setTrans1(QPointF pt) { t1=pt; }
    void        setTrans2(QPointF pt) { t2=pt; }
    QPointF     getTrans1() const { return t1; }
    QPointF     getTrans2() const { return t2; }

    FeatureGroup regroupFeatures();      // the map was deadly, it reordered

    QString     dump() const;

    void        setCanvasXform(const Xform & xf) { canvasXform = xf; }
    Xform &     getCanvasXform()                 { return canvasXform; }

    int         getVersion();
    void        setVersion(int ver);

    eTilingState getState();
    void         setState(eTilingState state);

    // settings
    ModelSettings & getSettings() { return settings; }

    // map operations
    MapPtr  createMapSingle();
    MapPtr  createMapFullSimple();
    MapPtr  createMapFull();

    static const QString defaultName;
    static int  refs;

protected:
    QVector<QTransform> getFillTranslations();

private:
    int             version;
    QString         name;
    QString         desc;
    QString         author;
    ModelSettings   settings;
    QPointF         t1;
    QPointF         t2;
    QVector<PlacedFeaturePtr> placed_features;
    Xform           canvasXform;

    eTilingState    state;
    Tristate        intrinsicOverlaps;
    Tristate        tiledOveraps;
};

#endif

