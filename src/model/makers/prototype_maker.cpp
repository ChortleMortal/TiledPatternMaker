#include <QMessageBox>

#include "gui/model_editors/motif_edit/design_element_button.h"
#include "gui/model_editors/motif_edit/motif_maker_widget.h"
#include "gui/panels/page_motif_maker.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/prototype_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/motifs/explicit_map_motif.h"
#include "model/motifs/girih_motif.h"
#include "model/motifs/hourglass_motif.h"
#include "model/motifs/inferred_motif.h"
#include "model/motifs/intersect_motif.h"
#include "model/motifs/irregular_motif.h"
#include "model/motifs/irregular_rosette.h"
#include "model/motifs/irregular_star.h"
#include "model/motifs/rosette.h"
#include "model/motifs/rosette2.h"
#include "model/motifs/star.h"
#include "model/motifs/star2.h"
#include "model/motifs/tile_motif.h"
#include "model/prototypes/design_element.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/sys.h"

using std::make_shared;
using std::dynamic_pointer_cast;

#define E2STR(x) #x

static QString mm_states[]
{
    E2STR(MM_EMPTY),
    E2STR(MM_SINGLE),
    E2STR(MM_MULTI)
 };

////////////////////////////////////////////////////////////////////////////
//
// Build a Prototype containing all the information about the
// design in its current state.  This will then be used to construct
// the final map.  It's also used to infer maps for tiles.
//
// The prototype clones the design elements stored in the editor,
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

PrototypeMaker::PrototypeMaker()
{
}

void PrototypeMaker::init()
{
    mosaicMaker      = Sys::mosaicMaker;

    motifMakerWidget = nullptr;

    setPropagate(true);
    setForceWidgetRefresh(false);

    initData(this);

    connect (this, &PrototypeMaker::sig_updateView, Sys::viewController, &SystemViewController::slot_updateView);
}

void PrototypeMaker::unload()
{
    erase();
}

ProtoPtr PrototypeMaker::createPrototypeFromTililing(const TilingPtr &tiling)
{
    qDebug().noquote() << "PrototypeMaker::createPrototype" << tiling->getDescription();
    ProtoPtr prototype = make_shared<Prototype>(tiling);

    QVector<TilePtr> uniqueTiles = tiling->unit().getUniqueTiles();
    qDebug() << "Create new design elements";
    int count = 0;
    for (auto & tile : std::as_const(uniqueTiles))
    {
        // NOTE this takes order from order of unique tiles
        DELPtr dep = make_shared<DesignElement>(tiling,tile);
        prototype->addDesignElement(dep);
        if (++count > MAX_UNIQUE_TILE_INDEX) qWarning() << "Large number of unique tiles in tiling count:" << count;
    }
    return prototype;
}

void PrototypeMaker::recreatePrototypeFromTiling(const TilingPtr &tiling, ProtoPtr prototype)
{
    // this handles additions and deletions

    Q_ASSERT(prototype->getTiling() == tiling);

    int count = 0;
    QVector<TilePtr> uniqueTiles = tiling->unit().getUniqueTiles();
    for (const auto & tile : std::as_const(uniqueTiles))
    {
        if (++count > MAX_UNIQUE_TILE_INDEX)
            qWarning() << "Large number of unique tiles in tiling count:" << count;

        bool found = false;
        for (const auto & del : std::as_const(prototype->getDesignElements()))
        {
            if (del->getTile() == tile)
            {
                found = true;
                if (!del->validMotifRegularity())
                {
                    del->recreateMotifFromChangedTile();
                }
                break;
            }
        }
        if (!found)
        {
            // creates a default TileMotif
            DELPtr dep = make_shared<DesignElement>(tiling,tile);
            prototype->addDesignElement(dep);
        }
    }

    QVector<DELPtr> forDeletion;
    for (const auto & del : std::as_const(prototype->getDesignElements()))
    {
        TilePtr tile = del->getTile();
        if (!uniqueTiles.contains(tile))
        {
            forDeletion.push_back(del);
        }
    }

    for (const auto & del : forDeletion)
    {
        prototype->removeDesignElement(del);
    }

    prototype->wipeoutProtoMap();
}

void PrototypeMaker::sm_takeDown(QVector<ProtoPtr> & prototypes)
{
    qDebug() << "PrototypeMaker::sm_takeDown" << "Num DELs:" << prototypes.first()->numDesignElements();

    erasePrototypes();

    setPrototypes(prototypes);

    QVector<TilingPtr> tilings;
    for (const auto & proto : std::as_const(prototypes))
    {
        auto tiling = proto->getTiling();
        Q_ASSERT(tiling);
        tilings.push_back(tiling);
    }

    Sys::tilingMaker->sm_takeDown(tilings, TILM_LOAD_FROM_MOSAIC);

    select(MVD_DELEM,prototypes.first(),false);
    select(MVD_PROTO,prototypes.first(),false);
}

/* There are 5 possible actions
   1. erase all proto, create new proto, and add
   2. erase current proto, create new proto, and add
   3. create new proto using tiling and add
   4. replace tiling in current proto
   5. reset prototype maps
*/

void PrototypeMaker::sm_eraseAllAdd(const ProtoPtr & prototype)
{
    erasePrototypes();

    add(prototype);

    select(MVD_DELEM, prototype,false);
    select(MVD_PROTO, prototype,false);
}

void PrototypeMaker::sm_eraseCurrentAdd(const ProtoPtr & prototype)
{
    auto proto =  getSelectedPrototype();
    remove(proto);

    add(prototype);

    select(MVD_DELEM, prototype,false);
    select(MVD_PROTO, prototype,false);
}

void PrototypeMaker::sm_Add(const ProtoPtr & prototype)
{
    add(prototype);

    select(MVD_DELEM, prototype,false);
    select(MVD_PROTO, prototype,false);
}

void PrototypeMaker::sm_replaceTiling(const ProtoPtr &prototype, const TilingPtr &tiling)
{
    prototype->replaceTiling(tiling);

    select(MVD_DELEM, prototype,false);
    select(MVD_PROTO, prototype,false);
    setForceWidgetRefresh(true);
}

ProtoPtr PrototypeMaker::sm_mergeProtos(const TilingPtr &tiling)
{
    // the trouble is the other prototypes may be gone as they were only weak pointers

    QVector<ProtoPtr> protos = getPrototypes();
    Q_ASSERT(protos.size() > 1);
    auto proto = protos.first();
    proto->exactReplaceTiling(tiling);
    for (int i=1; i < protos.size(); i++)
    {
        auto otherProto = protos[i];
        otherProto->exactReplaceTiling(tiling);
        auto dels = otherProto->getDesignElements();
        for (auto & del : dels)
        {
            proto->addDesignElement(del);
        }
        remove(otherProto);
    }

    select(MVD_DELEM, proto,false);
    select(MVD_PROTO, proto,false);

    return proto;
}

void PrototypeMaker::sm_resetMotifMaps()  // FIXME granularity
{
    for (const auto & prototype : getPrototypes())
    {
        auto dels = prototype->getDesignElements();
        for (auto & del : dels)
        {
            auto motif = del->getMotif();
            motif->resetMotifMap();
        }
    }
}

void PrototypeMaker::sm_resetProtoMaps()  // FIXME granularity
{
    for (const auto & prototype : getPrototypes())
    {
        prototype->wipeoutProtoMap();
    }
}


void PrototypeMaker::sm_buildMaps()
{
    for (const auto & prototype : getPrototypes())
    {
        prototype->getProtoMap(true);       // always called in a take up situation, not from loader/take down
    }
}

void PrototypeMaker::sm_takeUp(ProtoEvent &protoEvent)
{
    auto event     = protoEvent.event;
    auto tiling    = protoEvent.tiling;
    eMMState state = sm_getState();

    qDebug().noquote() << "PrototypeMaker::sm_takeUp" << "state:" << mm_states[state] << "event:" << sPROM_Events[event];

    switch (event)
    {
    case PROM_LOAD_EMPTY:
    {
        auto proto = createPrototypeFromTililing(tiling);
        sm_eraseAllAdd(proto);

        MosaicEvent mosaicEvent;
        mosaicEvent.event     = MOSM_LOAD_PROTO_EMPTY;
        mosaicEvent.prototype = proto;
        mosaicMaker->sm_takeUp(mosaicEvent);
    }   break;

    case PROM_LOAD_SINGLE:
        if (state == MM_EMPTY)
        {
            auto proto = createPrototypeFromTililing(tiling);
            sm_eraseAllAdd(proto);

            MosaicEvent mosaicEvent;
            mosaicEvent.event     = MOSM_LOAD_PROTO_SINGLE;
            mosaicEvent.prototype = proto;
            mosaicMaker->sm_takeUp(mosaicEvent);
        }
        else if (state == MM_SINGLE)
        {
            auto proto = createPrototypeFromTililing(tiling);
            sm_eraseAllAdd(proto);

            MosaicEvent mosaicEvent;
            mosaicEvent.event     = MOSM_LOAD_PROTO_SINGLE;
            mosaicEvent.prototype = proto;
            mosaicMaker->sm_takeUp(mosaicEvent);
        }
        else if (state == MM_MULTI)
        {
            bool createNew = askNewProto();
            if (createNew)
            {
                auto proto = createPrototypeFromTililing(tiling);
                sm_eraseCurrentAdd(proto);

                MosaicEvent mosaicEvent;
                mosaicEvent.event     = MOSM_LOAD_PROTO_MULTI;
                mosaicEvent.prototype = proto;
                mosaicMaker->sm_takeUp(mosaicEvent);
            }
            else
            {
                auto proto = getSelectedPrototype();
                sm_replaceTiling(proto, tiling);

                MosaicEvent mosaicEvent;
                mosaicEvent.event     = MOSM_RELOAD_PROTO_MULTI;
                mosaicEvent.prototype = proto;
                mosaicMaker->sm_takeUp(mosaicEvent);
            }
        }
        break;

    case PROM_RELOAD_SINGLE:
        if (state == MM_SINGLE)
        {
             auto proto = getSelectedPrototype();
             sm_replaceTiling(proto,tiling);

             MosaicEvent mosaicEvent;
             mosaicEvent.event     = MOSM_RELOAD_PROTO_SINGLE;
             mosaicEvent.prototype = proto;
             mosaicMaker->sm_takeUp(mosaicEvent);
        }
        else if (state == MM_MULTI)
        {
             auto proto = getSelectedPrototype();
             sm_replaceTiling(proto,tiling);

             MosaicEvent mosaicEvent;
             mosaicEvent.event     = MOSM_RELOAD_PROTO_SINGLE;
             mosaicEvent.prototype = proto;
             mosaicMaker->sm_takeUp(mosaicEvent);
        }
        else
        {
            Q_ASSERT(state == MM_EMPTY);
            qWarning("PrototypeMaker : invalid state (MM_EMPTY)");
        }
        break;

    case PROM_LOAD_MULTI:
        if (state == MM_EMPTY)
        {
            qWarning("PrototypeMaker : invalid state (MM_EMPTY)");
        }
        else if (state == MM_SINGLE)
        {
            auto proto = createPrototypeFromTililing(tiling);
            sm_Add(proto);

            MosaicEvent mosaicEvent;
            mosaicEvent.event     = MOSM_LOAD_PROTO_MULTI;
            mosaicEvent.prototype = proto;
            mosaicMaker->sm_takeUp(mosaicEvent);
        }
        else if (state == MM_MULTI)
        {
            auto proto = createPrototypeFromTililing(tiling);
            sm_Add(proto);

            MosaicEvent mosaicEvent;
            mosaicEvent.event     = MOSM_LOAD_PROTO_MULTI;
            mosaicEvent.prototype = proto;
            mosaicMaker->sm_takeUp(mosaicEvent);
        }
        break;

    case PROM_RELOAD_MULTI:
        if (state == MM_EMPTY)
        {
            qWarning("PrototypeMaker : invalid state (MM_EMPTY)");
        }
        else if (state == MM_SINGLE)
        {
            auto proto = getSelectedPrototype();
            sm_replaceTiling(proto,tiling);

            MosaicEvent mosaicEvent;
            mosaicEvent.event     = MOSM_RELOAD_PROTO_MULTI;
            mosaicEvent.prototype = proto;
            mosaicMaker->sm_takeUp(mosaicEvent);
        }
        else if (state == MM_MULTI)
        {
            auto proto = getSelectedPrototype();
            sm_replaceTiling(proto,tiling);

            MosaicEvent mosaicEvent;
            mosaicEvent.event     = MOSM_RELOAD_PROTO_MULTI;
            mosaicEvent.prototype = proto;
            mosaicMaker->sm_takeUp(mosaicEvent);
        }
        break;

    case PROM_REPLACE:
    {
        auto proto = getPrototype(protoEvent.oldTilings.first());
        sm_replaceTiling(proto,tiling);

        MosaicEvent mosaicEvent;
        mosaicEvent.event     = MOSM_RELOAD_PROTO_MULTI;
        mosaicEvent.prototype = proto;
        mosaicMaker->sm_takeUp(mosaicEvent);
    }   break;

    case PROM_REPLACE_ALL:
    {
        auto proto = sm_mergeProtos(tiling);

        MosaicEvent mosaicEvent;
        mosaicEvent.event     = MOSM_RELOAD_PROTO_MULTI;  // FIXME is this right
        mosaicEvent.prototype = proto;
        mosaicMaker->sm_takeUp(mosaicEvent);
    }   break;


    case PROM_TILING_DELETED:
    {
        auto proto = remove(tiling);

        MosaicEvent mosaicEvent;
        mosaicEvent.event     = MOSM_PROTO_DELETED;
        mosaicEvent.prototype = proto;
        mosaicMaker->sm_takeUp(mosaicEvent);
    }   break;

    case PROM_TILING_CHANGED:
    case PROM_TILING_MODIFED:
    {
        // This means changes to: placements,transform,fill-vectors,repeats
        // This does not change motifs - but MotifMaker should be refreshed
        Q_ASSERT(!protoEvent.tile);
        Q_ASSERT(!protoEvent.ptile);
        auto proto = getPrototype(tiling);
        if (proto)
        {
            select(MVD_DELEM,proto,false);  // this updates the menu buttons,etc.
            select(MVD_PROTO,proto,false);  // this updates the menu buttons,etc.
            if (getPropagate())
            {
                proto->wipeoutProtoMap();

                MosaicEvent mosaicEvent;
                mosaicEvent.event     = MOSM_PROTO_CHANGED;
                mosaicEvent.prototype = proto;
                mosaicMaker->sm_takeUp(mosaicEvent);
            }
        }
    }   break;

    case PROM_TILE_UNIQUE_TRANS_CHANGED:
    case PROM_TILE_UNIQUE_NATURE_CHANGED:
    {
        auto tile = protoEvent.tile;
        Q_ASSERT(tile);
        auto del = getDesignElement(tile);
        if (del)
        {
            auto motif = del->getMotif();
            if (motif)
            {
                motif->resetMotifMap();
                if (getPropagate())
                {
                    sm_resetProtoMaps();
                    MosaicEvent mosaicEvent;
                    mosaicEvent.event     = MOSM_PROTO_CHANGED;
                    mosaicEvent.prototype = getPrototype(tiling);
                    mosaicMaker->sm_takeUp(mosaicEvent);
                }
                select(MVD_DELEM,del,(Sys::config->motifMakerView == MOTIF_VIEW_SELECTED));
            }
        }
    }   break;

    case PROM_TILE_UNIQUE_REGULARITY_CHANGED:
    {
        // This means changes to: sides,scale,rot,regularity
        // which in turn impacts the motif
        auto tile = protoEvent.tile;
        Q_ASSERT(tile);
        auto proto = getPrototype(tiling);
        if (proto)
        {
            //Q_ASSERT(tiling == proto->getTiling());
            auto del = getDesignElement(tile);
            if (del)
            {
                del->recreateMotifWhenRgularityChanged();
                if (getPropagate())
                {
                    sm_resetProtoMaps();
                    MosaicEvent mosaicEvent;
                    mosaicEvent.event     = MOSM_PROTO_CHANGED;
                    mosaicEvent.prototype = getPrototype(tiling);
                    mosaicMaker->sm_takeUp(mosaicEvent);
                }
                select(MVD_DELEM,del,(Sys::config->motifMakerView == MOTIF_VIEW_SELECTED));
            }
        }
    }   break;

    case PROM_TILE_PLACED_TRANS_CHANGED:
    {
        auto ptile = protoEvent.ptile;
        Q_ASSERT(ptile);
        auto proto = getPrototype(tiling);
        if (proto)
        {
            //Q_ASSERT(tiling == proto->getTiling());
            Q_ASSERT(proto);
            auto del = getDesignElement(ptile->getTile());  // turns out it doesn't matter if unique or not
            if (del)
            {
                del->recreateMotifFromChangedTile();
                if (getPropagate())
                {
                    sm_resetProtoMaps();
                    MosaicEvent mosaicEvent;
                    mosaicEvent.event     = MOSM_PROTO_CHANGED;
                    mosaicEvent.prototype = getPrototype(tiling);
                    mosaicMaker->sm_takeUp(mosaicEvent);
                }
                select(MVD_DELEM,del,(Sys::config->motifMakerView == MOTIF_VIEW_SELECTED));
            }
        }
    }   break;

    case PROM_TILING_UNIT_CHANGED:
        // This means additions/deletions of tiles
        // This does not change motifs - but MotifMakerWidget should be refreshed
        Q_ASSERT(!protoEvent.ptile);
        Q_ASSERT(!protoEvent.tile);
        if (state == MM_EMPTY)
        {
            auto proto = createPrototypeFromTililing(tiling);
            sm_eraseAllAdd(proto);
            MosaicEvent mosaicEvent;
            mosaicEvent.event     = MOSM_PROTO_CHANGED;
            mosaicEvent.prototype = proto;
            mosaicMaker->sm_takeUp(mosaicEvent);
        }
        else
        {
            auto proto = getPrototype(tiling);
            if (proto)
            {
                proto->wipeoutProtoMap();
                recreatePrototypeFromTiling(tiling,proto);

                select(MVD_DELEM,proto,false);  // this updates the menu buttons,etc.
                select(MVD_PROTO,proto,false);  // this updates the menu buttons,etc.
                if (getPropagate())
                {
                    MosaicEvent mosaicEvent;
                    mosaicEvent.event     = MOSM_PROTO_CHANGED;
                    mosaicEvent.prototype = proto;
                    mosaicMaker->sm_takeUp(mosaicEvent);
                }
            }
        }
        break;

    case PROM_MOTIF_CHANGED:
        if (getPropagate())
        {
            sm_resetProtoMaps();
            MosaicEvent mosaicEvent;
            mosaicEvent.event     = MOSM_PROTO_CHANGED;
            mosaicEvent.prototype = getPrototype(tiling);
            mosaicMaker->sm_takeUp(mosaicEvent);
        }
        break;

    default:
        qCritical().noquote() << "PrototypeMaker::sm_takeUp" << "unsuported event" << sPROM_Events[event];
        break;
    }

    if (Sys::viewController->isEnabled(VIEW_MOTIF_MAKER))
    {
        emit sig_updateView();
    }
}

void PrototypeMaker::erasePrototypes()
{
    erase();
}

PrototypeMaker::eMMState PrototypeMaker::sm_getState()
{
    int count = getPrototypes().count();
    if (count > 1)
    {
        return  MM_MULTI;
    }
    else if (count == 0)
    {
        return MM_EMPTY;
    }
    else
    {
        Q_ASSERT(count == 1);
        return MM_SINGLE;
    }
}

void PrototypeMaker::selectDesignElement(DELPtr delp)
{
    auto motifmaker = getWidget();
    auto btn        = motifmaker->getButton(delp);
    motifmaker->delegate(btn,(Sys::config->motifMakerView == MOTIF_VIEW_SELECTED),true);
}

// duplication is used to superimpose a second figure in the same tile
// but also a duplicate figure is needed in the tiling to make the system happy
bool PrototypeMaker::duplicateDesignElement()
{
    auto del  = getSelectedDEL();
    if (!del) return false;

    auto tile  = del->getTile();
    auto motif = del->getMotif();
    qDebug() << "Copying" << sMotifType[motif->getMotifType()];

    auto newTile = make_shared<Tile>(tile);

    // find placed tiles in the tiling and duplicate it
    TilingPtr tiling = getSelectedPrototype()->getTiling();
    const PlacedTiles tilingUnit = tiling->unit().getIncluded();
    PlacedTiles newPlacedTiles;
    for (const auto & placedTile : std::as_const(tilingUnit))
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
    auto protos = getPrototypes();
    auto proto = getSelectedPrototype();
    recreatePrototypeFromTiling(tiling,proto);
    setPrototypes(protos);    //  rebuilds data

    return true;
}

MotifPtr PrototypeMaker::duplicateMotif(MotifPtr motif)
{
    // make a copy of the motif
    MotifPtr newMotif;
    switch (motif->getMotifType())
    {
    case MOTIF_TYPE_ROSETTE:
    {
        auto rosette = dynamic_pointer_cast<Rosette>(motif);
        Q_ASSERT(rosette);
        newMotif = make_shared<Rosette>(*rosette.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_ROSETTE2:
    {
        auto rosette = dynamic_pointer_cast<Rosette2>(motif);
        Q_ASSERT(rosette);
        newMotif = make_shared<Rosette2>(*rosette.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_STAR:
    {
        auto star = dynamic_pointer_cast<Star>(motif);
        Q_ASSERT(star);
        newMotif = make_shared<Star>(*star.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_STAR2:
    {
        auto star = dynamic_pointer_cast<Star2>(motif);
        Q_ASSERT(star);
        newMotif = make_shared<Star2>(*star.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_EXPLICIT_MAP:
    {
        auto explicitMotif = dynamic_pointer_cast<ExplicitMapMotif>(motif);
        Q_ASSERT(explicitMotif);
        newMotif = make_shared<ExplicitMapMotif>(*explicitMotif.get());
        Q_ASSERT(newMotif);
    } break;
        
    case MOTIF_TYPE_IRREGULAR_NO_MAP:
    {
        auto explicitMotif = dynamic_pointer_cast<IrregularNoMap>(motif);
        Q_ASSERT(explicitMotif);
        newMotif = make_shared<IrregularNoMap>(*explicitMotif.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_INFERRED:
    {
        auto infer = dynamic_pointer_cast<InferredMotif>(motif);
        Q_ASSERT(infer);
        newMotif = make_shared<InferredMotif>(*infer.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_IRREGULAR_ROSETTE:
    {
        auto rose = dynamic_pointer_cast<IrregularRosette>(motif);
        Q_ASSERT(rose);
        newMotif = make_shared<IrregularRosette>(*rose.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_HOURGLASS:
    {
        auto hour = dynamic_pointer_cast<HourglassMotif>(motif);
        Q_ASSERT(hour);
        newMotif = make_shared<HourglassMotif>(*hour.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_INTERSECT:
    {
        auto isect = dynamic_pointer_cast<IntersectMotif>(motif);
        Q_ASSERT(isect);
        newMotif = make_shared<IntersectMotif>(*isect.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_GIRIH:
    {
        auto girih = dynamic_pointer_cast<GirihMotif>(motif);
        Q_ASSERT(girih);
        newMotif = make_shared<GirihMotif>(*girih.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_IRREGULAR_STAR:
    {
        auto star = dynamic_pointer_cast<IrregularStar>(motif);
        Q_ASSERT(star);
        newMotif = make_shared<IrregularStar>(*star.get());
        Q_ASSERT(newMotif);
    }
    break;

    case MOTIF_TYPE_EXPLCIT_TILE:
    {
        auto tile = std::dynamic_pointer_cast<TileMotif>(motif);
        Q_ASSERT(tile);
        newMotif = make_shared<TileMotif>(*tile.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_UNDEFINED:
    case MOTIF_TYPE_RADIAL:
        qCritical("duplicateMotif : Unexpected Motif type");
        break;
    }

    return newMotif;
}

void PrototypeMaker::slot_propagateChanged(bool val)
{
    setPropagate(val);
    if (val)
    {
        sm_resetProtoMaps();
        sm_buildMaps();

        for (auto & proto : getPrototypes())
        {
            MosaicEvent mosaicEvent;
            mosaicEvent.event     = MOSM_PROTO_CHANGED;
            mosaicEvent.prototype = proto;
            mosaicMaker->sm_takeUp(mosaicEvent);
        }

        if (Sys::viewController->isEnabled(VIEW_PROTOTYPE) || Sys::viewController->isEnabled(VIEW_MAP_EDITOR))
        {
            //vcontrol->slot_reconstructView();
            emit sig_updateView();
        }
    }
}

bool PrototypeMaker::askNewProto()
{
    QMessageBox box(Sys::controlPanel);
    box.setIcon(QMessageBox::Question);
    QPushButton *createNewButton  = box.addButton("Create new Prototype", QMessageBox::ActionRole);
    QPushButton *replaceButton    = box.addButton("Replace tiling in existing Prototype", QMessageBox::ActionRole);
    Q_UNUSED(replaceButton);
    box.exec();

    bool createNew = (box.clickedButton() == createNewButton);
    return createNew;
}

