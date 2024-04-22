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
#include "misc/layer_controller.h"
#include "settings/tristate.h"
#include "tile/tiling_data.h"

#define MAX_UNIQUE_TILE_INDEX 7

typedef std::shared_ptr<class Tile>          TilePtr;
typedef std::shared_ptr<class PlacedTile>    PlacedTilePtr;
typedef std::shared_ptr<class Map>           MapPtr;
typedef std::shared_ptr<class BackgroundImage>  BkgdImagePtr;

class GeoGraphics;

class TileGroup : public  QVector<QPair<TilePtr,PlacedTiles>>
{
public:
    bool containsTile(TilePtr fp);
    PlacedTiles & getTilePlacements(TilePtr fp);
};

// Translations to tile the plane. Two needed for two-dimensional plane.
// (Of course, more complex tiling exists with rotations and mirrors.)
// (They are not supported.)

class Tiling : public LayerController, public std::enable_shared_from_this<Tiling>
{
    Q_OBJECT

public:
    Tiling();
    ~Tiling();

    void                paint(QPainter *painter) override;
    void                draw(GeoGraphics * gg);

    void                setTitle(QString n)              { title = n; }
    void                setDescription(QString descrip ) { desc = descrip; }
    void                setAuthor(QString auth)          { author = auth; }
    void                setVersion(int ver);

    void                setTranslationVectors(QPointF t1, QPointF t2, QPointF origin) { db.setTranslationVectors(t1,t2, origin); }
    QPointF             getTranslateOrigin() { return db.getTranslateOrigin(); }
    void                setCanvasSettings(const CanvasSettings & settings) { db.setCanvasSettings(settings); }

    bool                isEmpty();

    bool                hasIntrinsicOverlaps();
    bool                hasTiledOverlaps();

    QString             getTitle()       const { return title; }
    QString             getDescription() const { return desc; }
    QString             getAuthor()      const { return author; }

    void                add(const PlacedTilePtr pfm);   // Added tiles are put into a placedTile.
    void                replace(PlacedTiles & tiles);       // Added tiles are put into a placedTile.
    void                remove(PlacedTilePtr pf);
    void                clear();

    int                 numPlacements(TilePtr tile);
    Placements          getPlacements(TilePtr tile);
    QTransform          getFirstPlacement(TilePtr tile);
    QVector<TilePtr>    getUniqueTiles();
    TileGroup           regroupTiles();      // the map was deadly, it reordered

    int                 getVersion();

    const Xform &       getModelXform() override;
    void                setModelXform(const Xform & xf, bool update) override;

    // Data
    const TilingData &  getData()  const              { return db; }
    void                setData(TilingData & td)      { db = td; }
    const PlacedTiles & getInTiling() const           { return db.getInTiling(); }
    const CanvasSettings& getCanvasSettings() const     { return db.getSettings(); }

    BkgdImagePtr        getBkgdImage()                  { return bip; }
    void                setBkgdImage(BkgdImagePtr bp)   { bip = bp; }
    void                removeBkgdImage()               { bip.reset(); }

    void                resetOverlaps() { intrinsicOverlaps.reset(); tiledOverlaps.reset(); }

    // map operations
    MapPtr              createMapSingle();
    MapPtr              createMapFullSimple();
    MapPtr              createMapFull();
    MapPtr              debug_createFilledMap();
    MapPtr              debug_createProtoMap();

    eViewType           iamaLayer() override { return VIEW_TILING; }
    void                iamaLayerController() override {}

    QString             info() const;

    static const QString defaultName;
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
    QString             title;
    QString             desc;
    QString             author;

    TilingData          db;

    BkgdImagePtr        bip;

    Tristate            intrinsicOverlaps;
    Tristate            tiledOverlaps;
};

#endif

