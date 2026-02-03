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
#include "model/styles/colorset.h"
#include "model/settings/tristate.h"
#include "model/tilings/tiling_header.h"
#include "model/tilings/tiling_unit.h"

#define MAX_UNIQUE_TILE_INDEX 7

class GeoGraphics;

typedef std::shared_ptr<class Tile>             TilePtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class PlacedTile>       PlacedTilePtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class BackgroundImage>  BkgdImagePtr;

class SaveStatus
{
public:
    SaveStatus(Tiling * parent);

    void    init();             // initialises from parent
    bool    needsSaving();      // compares with parents current state

    TilingHeader    header;
    TilingUnit      unit;

private:
    Tiling *        parent;
};


// Translations to tile the plane. Two needed for two-dimensional plane.
// (Of course, more complex tiling exists with rotations and mirrors.)
// (They are not supported.)

class Tiling : public LayerController
{
    Q_OBJECT

public:
    Tiling();
    Tiling(Tiling * other);
    ~Tiling();

    void                copy(TilingPtr other);  // makes a unique duplicate

    bool                isEmpty();
    bool                hasIntrinsicOverlaps();
    bool                hasTiledOverlaps();

    void                paint(QPainter *painter) override;
    void                draw(GeoGraphics * gg);

    VersionedName       getVName()        const             { return name; }
    void                setVName(VersionedName & name)      { this->name = name; }
    QString             getDescription() const              { return desc; }
    void                setDescription(QString descrip )    { desc = descrip; }
    QString             getAuthor()      const              { return author; }
    void                setAuthor(QString auth)             { author = auth; }
    int                 getVersion();
    void                setVersion(int ver);

    // Data
    inline TilingHeader & hdr()   { return _header; }
    inline TilingUnit   & unit()  { return _tilingUnit;}

    void                createViewablePlacedTiles();
    int                 numViewable()                       { return _viewable.count(); }
    PlacedTiles &       getViewablePlacements()             { return _viewable; }
    QTransform          getFirstPlacement(TilePtr tile)     { return _tilingUnit.getFirstPlacement(tile); }

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

    // for tiling maker view and tiling view
    void                setViewable(bool enb)   { _view = enb; }
    bool                isViewable()            { return _view; }

    void                setTilingViewChanged()  { _tilingViewChange = true;  }
    bool                isTilingViewChanged()   { return _tilingViewChange;  }
    void                resetTilingViewChanged(){ _tilingViewChange = false; }

    SaveStatus *        getSaveStatus()          { return  _saveStatus; }
    bool                requiresSaving();

    ColorGroup &        legacyTileColors()      { return _legacyTileColors; }

    void                iamaLayerController() override {}

    QString             info();

    void                setLegacyModelConverted(bool set) { _legacyCenterConverted = set; }
    bool                legacyModelConverted()            { return _legacyCenterConverted; }
    void                correctBackgroundAlignment(BkgdImagePtr bip);

    static int          refs;

public slots:
     void slot_mousePressed(QPointF spt, Qt::MouseButton btn) override;
     void slot_mouseDragged(QPointF spt)       override;
     void slot_mouseMoved(QPointF spt)         override;
     void slot_mouseReleased(QPointF spt)      override;
     void slot_mouseDoublePressed(QPointF spt) override;

     void slot_mouseTranslate(uint sigid, QPointF spt)     override;
     void slot_wheel_scale(uint sigid, qreal delta)  override;
     void slot_wheel_rotate(uint sigid, qreal delta) override;

     void slot_scale(uint sigid, int amount)  override;
     void slot_rotate(uint sigid, int amount) override;
     void slot_moveX(uint sigid, qreal amount)  override;
     void slot_moveY(uint sigid, qreal amount)  override;

protected:
     void drawPlacedTile(GeoGraphics * g2d, PlacedTilePtr ptp);

private:
    int                 version;
    VersionedName       name;
    QString             desc;
    QString             author;

    TilingHeader        _header;
    TilingUnit          _tilingUnit;
    ColorGroup          _legacyTileColors;   // tile colors now stored in style  - TileColors

    PlacedTiles         _viewable;
    bool                _tilingViewChange;
    bool                _view;          // view in TilingMakerView

    SaveStatus *        _saveStatus;

    BkgdImagePtr        bip;
    Tristate            intrinsicOverlaps;
    Tristate            tiledOverlaps;

    bool                _legacyCenterConverted;
};

#endif

