#pragma once

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
#include <QLineF>
#include "gui/viewers/layer_controller.h"
#include "model/tilings/backgroundimage.h"
#include "model/styles/colorset.h"
#include "model/settings/tristate.h"
#include "model/tilings/tiling_header.h"
#include "model/tilings/tiling_unit.h"
#include "sys/sys/versioning.h"

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
    TilingUnitController      unit;
    QVector<QLineF> constructionLines;

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
    QTransform          getDistortion();        // from the tiling's own canvas
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
    inline TilingUnitController   & unit()  { return _tilingUnit;}

    QTransform          getFirstPlacement(TilePtr tile)     { return _tilingUnit.getFirstPlacement(tile); }

    const BkgdRecord &  getBkgdRecord()                     { return bkgdRecord; }
    void                setBkgdRecord(const BkgdRecord & r) { bkgdRecord = r; }

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

    void                setTilingViewChanged()  { _tilingViewChange = true; /*qDebug() << "tiling view change";*/ }
    bool                isTilingViewChanged()   { return _tilingViewChange;  }
    void                resetTilingViewChanged(){ _tilingViewChange = false; }

    void                setDrawSet(const PlacedTileSet & set) { _drawSet = set;}
    const PlacedTileSet & getDrawSet()                        { return _drawSet; }

    const QVector<QLineF> & getConstructionLines() const                        { return _constructionLines; }
    void                    setConstructionLines(const QVector<QLineF> & lines) { _constructionLines = lines; }

    SaveStatus *        getSaveStatus()         { return  _saveStatus; }
    bool                requiresSaving();

    ColorGroup &        legacyTileColors()      { return _legacyTileColors; }

    void                iamaLayerController() override {}

    QString             info();

    void                setLegacyModelConverted(bool set) { _legacyCenterConverted = set; }
    bool                legacyModelConverted()            { return _legacyCenterConverted; }
    void                correctBackgroundAlignment(BkgdRecord & rec);

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

    TilingHeader         _header;
    TilingUnitController _tilingUnit;
    ColorGroup           _legacyTileColors;   // tile colors now stored in style  - TileColors

    SaveStatus *        _saveStatus;

    BkgdRecord          bkgdRecord;
    Tristate            intrinsicOverlaps;
    Tristate            tiledOverlaps;

    bool                _legacyCenterConverted;

    // for use by tling maker view
    bool                _tilingViewChange;
    bool                _view;          // view in TilingMakerView
    PlacedTileSet       _drawSet;
    QVector<QLineF>     _constructionLines;     // Tiling Maker's construction lines, persisted with the tiling
};

