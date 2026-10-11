#include "gui/panels/page_motif_maker.h"
#include "gui/top/controlpanel.h"
#include "gui/top/view_controller.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/motif_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/motifs/adaptive_girih.h"
#include "model/motifs/adaptive_hourglass.h"
#include "model/motifs/adaptive_inference.h"
#include "model/motifs/adaptive_intersect.h"
#include "model/motifs/adaptive_rosette.h"
#include "model/motifs/adaptive_star.h"
#include "model/motifs/adaptive_tile.h"
#include "model/motifs/explicit_map_motif.h"
#include "model/motifs/rosette.h"
#include "model/motifs/rosette2.h"
#include "model/motifs/star.h"
#include "model/motifs/star2.h"
#include "model/prototypes/prototype.h"
#include "model/prototypes/tile_motif.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/sys.h"

using std::make_shared;
using std::dynamic_pointer_cast;

#define E2STR(x) #x

static QString mm_states[]
{
    E2STR(Empty),
    E2STR(Single),
    E2STR(Multi)
 };

////////////////////////////////////////////////////////////////////////////
//
// Build a Prototype containing all the information about the
// design in its current state.  This will then be used to construct
// the final map.  It's also used to infer maps for tiles.
//
// The prototype clones the tile motifs stored in the editor,
// so multiple prototypes can be fired off for different purposes and
// those prototypes can be consumed if needed.  This follows nicely
// in my philosophy of "fire and forget" UIs -- the different
// phases of a process should be independent.
//
// Casper - the different phases of the process are parts of on unified system
// so 'never forget'
//
// Casper - prototypes are owned by mosaics, storage here is only weak pointers,
// so prototypes must be propagated to the mosaic maker
//

MotifMaker::MotifMaker()
{}

void MotifMaker::init()
{
    mosaicMaker = Sys::mosaicMaker;

    setPropagate(true);
    setForceWidgetRefresh(false);

    connect (this, &MotifMaker::sig_updateView, Sys::systemViewController, &ViewController::slot_updateView);
}

void MotifMaker::unload()
{
    protoData.erase();
    motifData.erase();
}

ProtoPtr MotifMaker::createPrototypeFromTililing(const TilingPtr &tiling)
{
    qDebug().noquote() << "MotifMaker::createPrototypeFromTililing" << tiling->getDescription();
    ProtoPtr prototype = make_shared<Prototype>(tiling);

    QVector<TilePtr> uniqueTiles = tiling->unit().getUniqueTiles();
    qDebug() << "Create new tile motifs";
    int count = 0;
    for (auto & tile : std::as_const(uniqueTiles))
    {
        // NOTE this takes order from order of unique tiles
        TileMotifPtr timo = make_shared<TileMotif>(tiling,tile);
        prototype->addTileMotif(timo);
        if (++count > MAX_UNIQUE_TILE_INDEX) qWarning() << "Large number of unique tiles in tiling count:" << count;
    }
    return prototype;
}

void MotifMaker::sm_takeDown(QVector<ProtoPtr> & prototypes)
{
    qDebug() << "MotifMaker::sm_takeDown" << "Num DELs:" << prototypes.first()->numTileMotifs();

    erasePrototypes();

    Q_ASSERT(prototypes.size());
    motifData.usePrototype(prototypes.first());
    protoData.setPrototypes(prototypes);

    QVector<TilingPtr> tilings;
    for (const auto & proto : std::as_const(prototypes))
    {
        auto tiling = proto->getTiling();
        if (tiling)
        {
            tilings.push_back(tiling);
        }
    }

    if (tilings.count())
    {
        Sys::tilingMaker->sm_takeDown(tilings, TilingMaker::Event::LoadFromMosaic);
    }
    else
        qInfo() << "Mosaic has no tilings";
}

/* There are 4 possible actions
   1. erase all proto, create new proto, and add
   2. create new proto using tiling and add
   3. replace tiling in current proto
   4. reset prototype maps
*/

void MotifMaker::sm_eraseAllAdd(const ProtoPtr & prototype)
{
    erasePrototypes();

    protoData.setPrototype(prototype);
    TileMotifSet tms = prototype->getTileMotifs();

    motifData.setTileMotifs(tms);

    protoData.setPrototype(prototype);
    motifData.usePrototype(prototype);
}

void MotifMaker::sm_Add(const ProtoPtr & prototype)
{
    protoData.add(prototype);
    motifData.usePrototype(prototype);
}

void MotifMaker::sm_replaceTiling(const ProtoPtr &prototype, const TilingPtr &tiling)
{
    prototype->replaceTiling(tiling);

    protoData.setPrototype(prototype);
    motifData.usePrototype(prototype);

    setForceWidgetRefresh(true);
}

ProtoPtr MotifMaker::sm_mergeProtos(const TilingPtr &tiling)
{
    // the trouble is the other prototypes may be gone as they were only weak pointers

    QVector<ProtoPtr> protos = protoData.getPrototypes();
    Q_ASSERT(protos.size() > 1);
    auto proto = protos.first();
    proto->exactReplaceTiling(tiling);
    for (int i=1; i < protos.size(); i++)
    {
        auto otherProto = protos[i];
        otherProto->exactReplaceTiling(tiling);
        auto timos = otherProto->getTileMotifs();
        for (auto & timo : timos)
        {
            proto->addTileMotif(timo);
        }
        protoData.remove(otherProto);     // there must be at least two
    }

    protoData.setPrototype(proto);
    motifData.usePrototype(proto);

    return proto;
}

void MotifMaker::sm_resetMotifMaps()
{
    for (const auto & prototype : protoData.getPrototypes())
    {
        auto timos = prototype->getTileMotifs();
        for (auto & timo : timos)
        {
            auto motif = timo->getMotif();
            motif->resetMotifMap();
        }
    }
}

void MotifMaker::sm_resetProtoMaps()
{
    for (const auto & prototype : protoData.getPrototypes())
    {
        prototype->wipeoutProtoMap();
    }
}

void MotifMaker::sm_buildMaps()
{
    for (const auto & prototype : protoData.getPrototypes())
    {
        prototype->getProtoMap();       // always called in a take up situation, not from loader/take down
    }
}

void MotifMaker::sm_takeUp(MotifEvent &motifEvent)
{
    Event event    = motifEvent.event;
    auto tiling    = motifEvent.tiling;
    State state    = sm_getState();

    qDebug().noquote() << "MotifMaker::sm_takeUp" << "state:" << mm_states[(int)state] << "event:" << sMOTM_Events[(int)event];

    switch (event)
    {
    case Event::LoadEmpty:
    {
        auto proto = createPrototypeFromTililing(tiling);
        sm_eraseAllAdd(proto);

        MosaicEvent mosaicEvent;
        mosaicEvent.event     = MosaicMaker::Event::LoadEmpty;
        mosaicEvent.prototype = proto;
        mosaicMaker->sm_takeUp(mosaicEvent);
    }   break;

    case Event::LoadSingle:
    {
        // starts fresh with this tiling, whatever the state
        auto proto = createPrototypeFromTililing(tiling);
        sm_eraseAllAdd(proto);

        MosaicEvent mosaicEvent;
        mosaicEvent.event     = MosaicMaker::Event::LoadProtoSingle;
        mosaicEvent.prototype = proto;
        mosaicMaker->sm_takeUp(mosaicEvent);
    }   break;

    case Event::ReloadSingle:
        switch (state)
        {
        case State::Empty:
            qWarning("PrototypeMaker : invalid state (MM_EMPTY)");
            break;

        case State::Single:
        case State::Multi:
        {
            auto proto = motifData.getActivePrototype();
            sm_replaceTiling(proto,tiling);

            MosaicEvent mosaicEvent;
            mosaicEvent.event     = MosaicMaker::Event::ReloadProtoSingle;
            mosaicEvent.prototype = proto;
            mosaicMaker->sm_takeUp(mosaicEvent);
        }   break;
        }
        break;

    case Event::LoadMulti:
        switch (state)
        {
        case State::Empty:
            qWarning("PrototypeMaker : invalid state (MM_EMPTY)");
            break;

        case State::Single:
        case State::Multi:
        {
            auto proto = createPrototypeFromTililing(tiling);
            sm_Add(proto);

            MosaicEvent mosaicEvent;
            mosaicEvent.event     = MosaicMaker::Event::LoadProtoMulti;
            mosaicEvent.prototype = proto;
            mosaicMaker->sm_takeUp(mosaicEvent);
        }   break;
        }
        break;

    case Event::ReloadMulti:
        switch (state)
        {
        case State::Empty:
            qWarning("PrototypeMaker : invalid state (MM_EMPTY)");
            break;

        case State::Single:
        case State::Multi:
        {
            auto proto = motifData.getActivePrototype();
            sm_replaceTiling(proto,tiling);

            MosaicEvent mosaicEvent;
            mosaicEvent.event         = MosaicMaker::Event::ReloadProtoMulti;
            mosaicEvent.prototype     = proto;
            mosaicEvent.oldPrototypes = { proto };
            mosaicMaker->sm_takeUp(mosaicEvent);
        }   break;
        }
        break;

    case Event::LoadReplace:
    {
        auto proto = protoData.getPrototype(motifEvent.oldTilings.first());
        sm_replaceTiling(proto,tiling);

        MosaicEvent mosaicEvent;
        mosaicEvent.event         = MosaicMaker::Event::ReloadProtoMulti;
        mosaicEvent.prototype     = proto;
        mosaicEvent.oldPrototypes = { proto };
        mosaicMaker->sm_takeUp(mosaicEvent);
    }   break;

    case Event::ReplaceAll:
    {
        QVector<ProtoPtr> oldProtos = protoData.getPrototypes();   // captured before sm_mergeProtos consolidates them
        auto proto = sm_mergeProtos(tiling);

        MosaicEvent mosaicEvent;
        mosaicEvent.event         = MosaicMaker::Event::ReloadProtoMulti;
        mosaicEvent.prototype     = proto;
        mosaicEvent.oldPrototypes = oldProtos;
        mosaicMaker->sm_takeUp(mosaicEvent);
    }   break;

    case Event::TilingDeleted:
    {
        auto removed = protoData.remove(tiling);

        MosaicEvent mosaicEvent;
        mosaicEvent.event     = MosaicMaker::Event::ProtoDeleted;
        mosaicEvent.prototype = removed;
        mosaicMaker->sm_takeUp(mosaicEvent);
    }   break;

    case Event::TilingChanged:
    case Event::TilingModified:
    {
        // This means changes to: placements,transform,fill-vectors,repeats
        // This does not change motifs - but MotifMaker should be refreshed
        Q_ASSERT(!motifEvent.tile);
        Q_ASSERT(!motifEvent.ptile);
        ProtoPtr proto = protoData.getPrototype(tiling);
        if (proto)
        {
            protoData.remove(proto);
            protoData.add(proto);           // this updates the menu buttons,etc.
            motifData.usePrototype(proto);  // this updates the menu buttons,etc.
            if (getPropagate())
            {
                proto->wipeoutProtoMap();

                MosaicEvent mosaicEvent;
                mosaicEvent.event     = MosaicMaker::Event::ProtoChanged;
                mosaicEvent.prototype = proto;
                mosaicMaker->sm_takeUp(mosaicEvent);
            }
        }
    }   break;

    case Event::UniqueTileTransformChanged:
    case Event::UniqueTileNatureChanged:
    {
        auto tile = motifEvent.tile;
        Q_ASSERT(tile);
        auto timo = motifData.getTileMotif(tile);
        if (timo)
        {
            auto motif = timo->getMotif();
            if (motif)
            {
                motif->resetMotifMap();
                if (getPropagate())
                {
                    sm_resetProtoMaps();
                    MosaicEvent mosaicEvent;
                    mosaicEvent.event     = MosaicMaker::Event::ProtoChanged;
                    mosaicEvent.prototype = protoData.getPrototype(tiling);
                    mosaicMaker->sm_takeUp(mosaicEvent);
                }
                motifData.show(timo);
            }
        }
    }   break;

    case Event::UniqueTileRegularityChanged:
    {
        // This means changes to: sides,scale,rot,regularity
        // which in turn impacts the motif
        auto tile = motifEvent.tile;
        Q_ASSERT(tile);
        auto proto = protoData.getPrototype(tiling);
        if (proto)
        {
            //Q_ASSERT(tiling == proto->getTiling());
            auto timo = motifData.getTileMotif(tile);
            if (timo)
            {
                timo->recreateMotifWhenRgularityChanged();
                if (getPropagate())
                {
                    sm_resetProtoMaps();
                    MosaicEvent mosaicEvent;
                    mosaicEvent.event     = MosaicMaker::Event::ProtoChanged;
                    mosaicEvent.prototype = protoData.getPrototype(tiling);
                    mosaicMaker->sm_takeUp(mosaicEvent);
                }
                motifData.show(timo);
            }
        }
    }   break;

    case Event::PlaceTileTransformChanged:
    {
        PlacedTilePtr ptile = motifEvent.ptile;
        Q_ASSERT(ptile);
        auto proto = protoData.getPrototype(tiling);
        if (proto)
        {
            //Q_ASSERT(tiling == proto->getTiling());
            Q_ASSERT(proto);
            auto timo = motifData.getTileMotif(ptile->getTile());  // turns out it doesn't matter if unique or not
            if (timo)
            {
                timo->recreateMotifFromChangedTile();
                if (getPropagate())
                {
                    sm_resetProtoMaps();
                    MosaicEvent mosaicEvent;
                    mosaicEvent.event     = MosaicMaker::Event::ProtoChanged;
                    mosaicEvent.prototype = protoData.getPrototype(tiling);
                    mosaicMaker->sm_takeUp(mosaicEvent);
                }
                motifData.show(timo);
            }
        }
    }   break;

    case Event::TilingUnitChanged:
        // This means additions/deletions of tiles
        // This does not change motifs - but MotifMakerWidget should be refreshed
        Q_ASSERT(!motifEvent.ptile);
        Q_ASSERT(!motifEvent.tile);
        switch (state)
        {
        case State::Empty:
        {
            auto proto = createPrototypeFromTililing(tiling);
            proto->setMosaic(mosaicMaker->getMosaic());
            sm_eraseAllAdd(proto);
            MosaicEvent mosaicEvent;
            mosaicEvent.event     = MosaicMaker::Event::ProtoChanged;
            mosaicEvent.prototype = proto;
            mosaicMaker->sm_takeUp(mosaicEvent);
        }   break;

        case State::Single:
        case State::Multi:
        {
            auto proto = protoData.getPrototype(tiling);
            if (proto)
            {
                proto->recreateFromTiling();

                protoData.remove(proto);
                protoData.add(proto);           // this updates the menu buttons,etc.
                motifData.usePrototype(proto);  // this updates the menu buttons,etc.
                if (getPropagate())
                {
                    MosaicEvent mosaicEvent;
                    mosaicEvent.event     = MosaicMaker::Event::ProtoChanged;
                    mosaicEvent.prototype = proto;
                    mosaicMaker->sm_takeUp(mosaicEvent);
                }
            }
        }   break;
        }
        break;

    case Event::MotifChanged:
        // note: MotifMakerWidget detects an in-place motif swap itself (see
        // _contentHasChanged()), so this does not need to force a widget refresh -
        // doing so on every scale/rotate/sides tweak flashed the lower-third editor

        if (getPropagate())
        {
            sm_resetProtoMaps();
            MosaicEvent mosaicEvent;
            mosaicEvent.event     = MosaicMaker::Event::ProtoChanged;
            mosaicEvent.prototype = protoData.getPrototype(tiling);
            mosaicMaker->sm_takeUp(mosaicEvent);
        }
        break;
    }

    if (Sys::systemViewController->isEnabled(Layer::Type::MotifMaker))
    {
        emit sig_updateView();
    }
}

void MotifMaker::erasePrototypes()
{
    protoData.erase();
}

MotifMaker::State MotifMaker::sm_getState()
{
    int count = protoData.getPrototypes().count();
    if (count > 1)
    {
        return  State::Multi;
    }
    else if (count == 0)
    {
        return State::Empty;
    }
    else
    {
        Q_ASSERT(count == 1);
        return State::Single;
    }
}

void MotifMaker::slot_propagateChanged(bool val)
{
    setPropagate(val);
    if (val)
    {
        sm_resetProtoMaps();
        sm_buildMaps();

        for (auto & proto : protoData.getPrototypes())
        {
            MosaicEvent mosaicEvent;
            mosaicEvent.event     = MosaicMaker::Event::ProtoChanged;
            mosaicEvent.prototype = proto;
            mosaicMaker->sm_takeUp(mosaicEvent);
        }

        if (Sys::systemViewController->isEnabled(Layer::Type::Prototype) || Sys::systemViewController->isEnabled(Layer::Type::MapEditor))
        {
            //vcontrol->slot_reconstructView();
            emit sig_updateView();
        }
    }
}

MotifPtr MotifMaker::create(Motif::Type type, TileMotifPtr tileMotif)
{
    MotifPtr motif = tileMotif->getMotif();

    MotifPtr m;
    switch (type)
    {
    case Motif::Type::Undefined:
    case Motif::Type::Radial:
        qCritical("unexpected motif type");
        break;
    case Motif::Type::Star:
        m =   make_shared<Star>(*motif);
        break;
    case Motif::Type::Star2:
        m = make_shared<Star2>(*motif);
        break;
    case Motif::Type::Rosette:
        m = make_shared<Rosette>(*motif);
        break;
    case Motif::Type::Rosette2:
        if (auto rosette = std::dynamic_pointer_cast<Rosette>(motif))
            m = make_shared<Rosette2>(*rosette);
        else
            m = make_shared<Rosette2>(*motif);
        break;
    case Motif::Type::MapMotif:
        m =  make_shared<ExplicitMapMotif>(*motif);
        break;
    case Motif::Type::NoMapMotif:
        m = make_shared<IrregularNoMap>(*motif);
        break;
    case Motif::Type::Inferred:
        m = make_shared<AdaptiveInference>(*motif);
        break;
    case Motif::Type::AdaptiveRosette:
        m = make_shared<AdaptiveRosette>(*motif);
        break;
    case Motif::Type::Hourglass:
        m = make_shared<AdaptiveHourglass>(*motif);
        break;
    case Motif::Type::Intersect:
        m = make_shared<AdaptiveIntersect>(*motif);
        break;
    case Motif::Type::Girih:
        m = make_shared<AdaptiveGirih>(*motif);
        break;
    case Motif::Type::AdaptiveStar:
        m = make_shared<AdaptiveStar>(*motif);
        break;
    case Motif::Type::ExplicitTile:
        m = make_shared<AdaptiveTile>(*motif);
        break;
    }

    m->setTile(tileMotif->getTile());

    tileMotif->setMotif(m);

    return m;
}

// duplication is used to superimpose a second figure in the same tile
// but also a duplicate figure is needed in the tiling to make the system happy
bool MotifMaker::duplicateTileMotif()
{
    auto timo  = motifData.getDelegatedTileMotif();
    if (!timo) return false;

    auto tile  = timo->getTile();
    auto motif = timo->getMotif();
    qDebug() << "Copying" << sMotifType[(int)motif->getMotifType()];

    auto newTile = make_shared<Tile>(tile);

    // find placed tiles in the tiling and duplicate it
    TilingPtr tiling = motifData.getActivePrototype()->getTiling();
    const PlacedTileSet tileSet = tiling->unit().getIncluded();
    PlacedTileSet newPlacedTiles;
    for (const auto & placedTile : std::as_const(tileSet))
    {
        if (placedTile->getTile() == tile)
        {
            PlacedTilePtr newPlacedTile = make_shared<PlacedTile>(newTile,placedTile->getPlacement());
            newPlacedTiles.push_back(newPlacedTile);
        }
    }

    if (newPlacedTiles.isEmpty())
    {
        return false;
    }

    // put new placed tiles intp the tiling
    for (const auto & placedTile : newPlacedTiles)
    {
        tiling->unit().addPlacedTile(placedTile);
    }

    // rebuild prototype
    auto proto  = motifData.getActivePrototype();
    proto->recreateFromTiling();

    // setup the view data again
    motifData.usePrototype(proto);  // set it up again

    auto protos = protoData.getPrototypes();
    protoData.setPrototypes(protos);    //  rebuilds data

    return true;
}

MotifPtr MotifMaker::duplicateMotif(MotifPtr motif)
{
    // make a copy of the motif
    MotifPtr newMotif;
    switch (motif->getMotifType())
    {
    case Motif::Type::Rosette:
    {
        auto rosette = dynamic_pointer_cast<Rosette>(motif);
        Q_ASSERT(rosette);
        newMotif = make_shared<Rosette>(*rosette.get());
        Q_ASSERT(newMotif);
    } break;

    case Motif::Type::Rosette2:
    {
        auto rosette = dynamic_pointer_cast<Rosette2>(motif);
        Q_ASSERT(rosette);
        newMotif = make_shared<Rosette2>(*rosette.get());
        Q_ASSERT(newMotif);
    } break;

    case Motif::Type::Star:
    {
        auto star = dynamic_pointer_cast<Star>(motif);
        Q_ASSERT(star);
        newMotif = make_shared<Star>(*star.get());
        Q_ASSERT(newMotif);
    } break;

    case Motif::Type::Star2:
    {
        auto star = dynamic_pointer_cast<Star2>(motif);
        Q_ASSERT(star);
        newMotif = make_shared<Star2>(*star.get());
        Q_ASSERT(newMotif);
    } break;

    case Motif::Type::MapMotif:
    {
        auto explicitMotif = dynamic_pointer_cast<ExplicitMapMotif>(motif);
        Q_ASSERT(explicitMotif);
        newMotif = make_shared<ExplicitMapMotif>(*explicitMotif.get());
        Q_ASSERT(newMotif);
    } break;

    case Motif::Type::NoMapMotif:
    {
        auto explicitMotif = dynamic_pointer_cast<IrregularNoMap>(motif);
        Q_ASSERT(explicitMotif);
        newMotif = make_shared<IrregularNoMap>(*explicitMotif.get());
        Q_ASSERT(newMotif);
    } break;

    case Motif::Type::Inferred:
    {
        auto infer = dynamic_pointer_cast<AdaptiveInference>(motif);
        Q_ASSERT(infer);
        newMotif = make_shared<AdaptiveInference>(*infer.get());
        Q_ASSERT(newMotif);
    } break;

    case Motif::Type::AdaptiveRosette:
    {
        auto rose = dynamic_pointer_cast<AdaptiveRosette>(motif);
        Q_ASSERT(rose);
        newMotif = make_shared<AdaptiveRosette>(*rose.get());
        Q_ASSERT(newMotif);
    } break;

    case Motif::Type::Hourglass:
    {
        auto hour = dynamic_pointer_cast<AdaptiveHourglass>(motif);
        Q_ASSERT(hour);
        newMotif = make_shared<AdaptiveHourglass>(*hour.get());
        Q_ASSERT(newMotif);
    } break;

    case Motif::Type::Intersect:
    {
        auto isect = dynamic_pointer_cast<AdaptiveIntersect>(motif);
        Q_ASSERT(isect);
        newMotif = make_shared<AdaptiveIntersect>(*isect.get());
        Q_ASSERT(newMotif);
    } break;

    case Motif::Type::Girih:
    {
        auto girih = dynamic_pointer_cast<AdaptiveGirih>(motif);
        Q_ASSERT(girih);
        newMotif = make_shared<AdaptiveGirih>(*girih.get());
        Q_ASSERT(newMotif);
    } break;

    case Motif::Type::AdaptiveStar:
    {
        auto star = dynamic_pointer_cast<AdaptiveStar>(motif);
        Q_ASSERT(star);
        newMotif = make_shared<AdaptiveStar>(*star.get());
        Q_ASSERT(newMotif);
    }
    break;

    case Motif::Type::ExplicitTile:
    {
        auto tile = std::dynamic_pointer_cast<AdaptiveTile>(motif);
        Q_ASSERT(tile);
        newMotif = make_shared<AdaptiveTile>(*tile.get());
        Q_ASSERT(newMotif);
    } break;

    case Motif::Type::Undefined:
    case Motif::Type::Radial:
        qCritical("duplicateMotif : Unexpected Motif type");
        break;
    }

    return newMotif;
}

const QString sMOTM_Events[] =
    {
        E2STR(LoadEmpty),
        E2STR(LoadSingle),
        E2STR(LoadMulti),
        E2STR(ReloadSingle),
        E2STR(ReloadMulti),
        E2STR(LoadReplace),
        E2STR(ReplaceAll),
        E2STR(TilingDeleted),
        E2STR(TilingChanged),
        E2STR(TilingModified),
        E2STR(UniqueTileTransformChanged),
        E2STR(UniqueTileNatureChanged),
        E2STR(UniqueTileNatureChanged),
        E2STR(PlaceTileTransformChanged),
        E2STR(TilingUnitChanged),
        E2STR(MotifChanged),
};
