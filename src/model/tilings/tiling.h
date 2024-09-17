#pragma once
#ifndef TILING
#define TILING

////////////////////////////////////////////////////////////////////////////
//
// Tiling.java
//
// The representation of a tiling, which will serve as the skeleton for
// Islamic designs.  A Tiling has two translation vectors and a set of
// placedTiles that make up a translational unit.  The idea is that
// the whole tiling can be replicated across the plane by placing
// a copy of the translational unit at every integer linear combination
// of the translation vectors.  In practice, we only draw at those
// linear combinations within some viewport.

#include <QStack>
#include <QObject>
#include "gui/viewers/layer_controller.h"
#include "model/settings/tristate.h"
#include "model/tilings/tiling_data.h"

#define MAX_UNIQUE_TILE_INDEX 7

class GeoGraphics;

typedef std::shared_ptr<class Tile>          TilePtr;
typedef std::shared_ptr<class PlacedTile>    PlacedTilePtr;
typedef std::shared_ptr<class Map>           MapPtr;
typedef std::shared_ptr<class BackgroundImage>  BkgdImagePtr;

// Translations to tile the plane. Two needed for two-dimensional plane.
// (Of course, more complex tiling exists with rotations and mirrors.)
// (They are not supported.)

class Tiling : public LayerController, public std::enable_shared_from_this<Tiling>
{
    Q_OBJECT

public:
    Tiling();
    ~Tiling();

    void                copy(TilingPtr other);  // makes a unique duplicate
    bool                isEmpty();
    bool                hasIntrinsicOverlaps();
    bool                hasTiledOverlaps();

    void                paint(QPainter *painter) override;
    void                draw(GeoGraphics * gg);

    VersionedName       getName()        const              { return name; }
    void                setName(VersionedName & name)       { this->name = name; }
    QString             getDescription() const              { return desc; }
    void                setDescription(QString descrip )    { desc = descrip; }
    QString             getAuthor()      const              { return author; }
    void                setAuthor(QString auth)             { author = auth; }
    int                 getVersion();
    void                setVersion(int ver);

    void                setTranslationVectors(QPointF t1, QPointF t2, QPointF origin) { db.setTranslationVectors(t1,t2, origin); }
    QPointF             getTranslateOrigin()                                          { return db.getTranslateOrigin(); }
    void                setCanvasSettings(const CanvasSettings & settings)            { db.setCanvasSettings(settings); }

    void                addPlacedTile(const PlacedTilePtr pfm);
    void                removePlacedTile(PlacedTilePtr pf);
    void                removeExcludeds();
    void                replaceTilingUnit(TilingUnit & tilingUnit);
    void                clearPlacedTiles();

    void                creaateViewablePlacedTiles();
    TilingPlacements  & getViewablePlacements() { return _viewable; }
    TilingPlacements    getTilingUnitPlacements(bool all = false) const;
    TilingPlacements    getExcluded() const  { return db.getExcluded(); };

    int                 numUnique()   { return getUniqueTiles().count(); }
    int                 numExcluded() { return db.getTilingUnit2().numExcluded(); }
    int                 numIncluded() { return db.getTilingUnit2().numIncluded(); }
    int                 numAll()      { return db.getTilingUnit2().numAll(); }
    int                 numViewable() { return _viewable.count(); }

    int                 numPlacements(TilePtr tile);
    Placements          getPlacements(TilePtr tile);
    QTransform          getFirstPlacement(TilePtr tile);
    QVector<TilePtr>    getUniqueTiles();

    const Xform &       getModelXform() override;
    void                setModelXform(const Xform & xf, bool update) override;

    // Data
    const TilingData &  getData()  const                    { return db; }
    const TilingData    getDataCopy()                       { return db.copy(); }
    void                setData(TilingData td)              { db = td; }

    TilingUnit &        getTilingUnit()                     { return db.getTilingUnit2(); }
    const CanvasSettings& getCanvasSettings() const         { return db.getCanvasSettings(); }

    BkgdImagePtr        getBkgdImage()                      { return bip; }
    void                setBkgdImage(BkgdImagePtr bp)       { bip = bp; }
    void                removeBkgdImage()                   { bip.reset(); }

    void                resetOverlaps()                     { intrinsicOverlaps.reset(); tiledOverlaps.reset(); }

    // map operations
    MapPtr              createMapSingle();
    MapPtr              createMapFullSimple();
    MapPtr              createMapFull();
    MapPtr              debug_createFilledMap();
    MapPtr              debug_createProtoMap();

    eViewType           iamaLayer() override { return VIEW_TILING; }
    void                iamaLayerController() override {}

    QString             info() const;

    static int          refs;

public slots:
     void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
     void slot_mouseDragged(QPointF spt)       override;
     void slot_mouseTranslate(QPointF spt)     override;
     void slot_mouseMoved(QPointF spt)         override;
     void slot_mouseReleased(QPointF spt)      override;
     void slot_mouseDoublePressed(QPointF spt) override;

     void slot_wheel_scale(qreal delta)  override;
     void slot_wheel_rotate(qreal delta) override;

     void slot_scale(int amount)  override;
     void slot_rotate(int amount) override;
     void slot_moveX(qreal amount)  override;
     void slot_moveY(qreal amount)  override;

protected:
    void    drawPlacedTile(GeoGraphics * g2d, PlacedTilePtr pf);

private:
    int                 version;
    VersionedName       name;
    QString             desc;
    QString             author;

    TilingData          db;

    TilingPlacements    _viewable;

    BkgdImagePtr        bip;

    Tristate            intrinsicOverlaps;
    Tristate            tiledOverlaps;
};

#endif

