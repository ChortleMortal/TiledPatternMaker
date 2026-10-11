#pragma once

#include <QObject>
#include <QFrame>
#include "model/makers/motif_view_data.h"
#include "model/makers/proto_view_data.h"
#include "model/motifs/motif.h"

typedef std::shared_ptr<class PlacedTile>    PlacedTilePtr;

class MotifEvent;

extern const QString sMOTM_Events[];

class MotifMaker : public QObject
{
    Q_OBJECT

    enum class State
    {
        Empty = 0,
        Single,
        Multi
    };

public:

    enum class eMotifMakerEvent
    {
        LoadEmpty = 0,
        LoadSingle,
        LoadMulti,
        ReloadSingle,
        ReloadMulti,
        LoadReplace,
        ReplaceAll,
        TilingDeleted,
        TilingChanged,                  // singleton, repeats, vectors
        TilingModified,                 // all placements
        UniqueTileTransformChanged,     // scale,rot, mirror
        UniqueTileNatureChanged,        // regularity, sides,
        UniqueTileRegularityChanged,    // flip
        PlaceTileTransformChanged,      // pos,scale,rot
        TilingUnitChanged,
        MotifChanged                    // from motif maker
    };

    using Event = eMotifMakerEvent;

    MotifMaker();

    void        init();
    void        unload();

    // state machine
    void        sm_takeDown(QVector<ProtoPtr> &prototypes);
    void        sm_takeUp(MotifEvent & motifEvent);
    void        sm_resetProtoMaps();
    void        sm_resetMotifMaps();

    // operations
    void        erasePrototypes();

    inline bool forceWidgetRefresh()            { return _forceWidgetRefresh; }
    void        setForceWidgetRefresh(bool set) { _forceWidgetRefresh = set; }

    MotifPtr    create(Motif::Type type, TileMotifPtr tileMotif);
    MotifPtr    duplicateMotif(MotifPtr motif);
    bool        duplicateTileMotif();

public:
    MotifViewData motifData;
    ProtoViewData protoData;

signals:
    void        sig_updateView();

public slots:
    void        slot_propagateChanged(bool val);

protected:
    State       sm_getState();
    void        sm_eraseAllAdd(const ProtoPtr & prototype);
    void        sm_Add(const ProtoPtr & prototype);
    void        sm_replaceTiling(const ProtoPtr & prototype,const TilingPtr & tiling);
    ProtoPtr    sm_mergeProtos(const TilingPtr &tiling);
    void        sm_buildMaps();

    ProtoPtr    createPrototypeFromTililing(const TilingPtr & tiling);

private:
    void        setPropagate(bool enb)    { _pm_propagate = enb; }
    bool        getPropagate()            { return _pm_propagate; }

    class MosaicMaker   * mosaicMaker;
    bool                 _pm_propagate;
    bool                 _forceWidgetRefresh;
};

class MotifEvent
{
public:
    using Event = MotifMaker::eMotifMakerEvent;

    Event               event;
    TilingPtr           tiling;
    PlacedTilePtr       ptile;      // selected Tile
    TilePtr             tile;
    QVector<TilingPtr>  oldTilings;
};