#include <QMessageBox>

#include "model/makers/prototype_maker.h"
#include "model/makers/mosaic_maker.h"
#include "gui/model_editors/motif_edit/design_element_button.h"
#include "gui/model_editors/motif_edit/motif_maker_widget.h"
#include "model/prototypes/prototype.h"
#include "model/makers/tiling_maker.h"
#include "model/prototypes/design_element.h"
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
#include "sys/sys.h"
#include "gui/panels/page_motif_maker.h"
#include "gui/top/controlpanel.h"
#include "model/settings/configuration.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/tiledpatternmaker.h"
#include "gui/top/view_controller.h"

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
    config          = Sys::config;
    vcontrol        = Sys::viewController;
    tilingMaker     = Sys::tilingMaker;
    mosaicMaker     = Sys::mosaicMaker;
    propagate       = true;

    connect (this, &PrototypeMaker::sig_updateView, Sys::view, &View::slot_update);
}

void PrototypeMaker::unload()
{
    erase();
}

void PrototypeMaker::setPropagate(bool val)
{
    propagate = val;
    if (val)
    {
        sm_resetMaps();
        sm_buildMaps();
        mosaicMaker->sm_takeUp(getPrototypes(),MOSM_MOTIF_CHANGED);

        if (vcontrol->isEnabled(VIEW_PROTOTYPE) || vcontrol->isEnabled(VIEW_MAP_EDITOR))
        {
            //vcontrol->slot_reconstructView();
            emit sig_updateView();
        }
    }
}

ProtoPtr PrototypeMaker::createPrototypeFromTililing(const TilingPtr &tiling)
{
    qDebug().noquote() << "PrototypeMaker::createPrototype" << tiling->getDescription();
    ProtoPtr prototype = make_shared<Prototype>(tiling);

    QVector<TilePtr> uniqueTiles = tiling->getUniqueTiles();
    qDebug() << "Create new design elements";
    int count = 0;
    for (auto & tile : std::as_const(uniqueTiles))
    {
        // NOTE this takes order from order of unique tiles
        DesignElementPtr dep = make_shared<DesignElement>(tile);
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
    QVector<TilePtr> uniqueTiles = tiling->getUniqueTiles();
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
            DesignElementPtr dep = make_shared<DesignElement>(tile);
            prototype->addDesignElement(dep);
        }
    }

    QVector<DesignElementPtr> forDeletion;
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
    qDebug() << "PrototypeMaker::takeDown() Num DELs:" << prototypes.first()->numDesignElements();

    erasePrototypes();

    setPrototypes(prototypes);

    QVector<TilingPtr> tilings;
    for (const auto & proto : std::as_const(prototypes))
    {
        auto tiling = proto->getTiling();
        Q_ASSERT(tiling);
        tilings.push_back(tiling);
    }

    tilingMaker->sm_takeDown(tilings, TILM_LOAD_FROM_MOSAIC);

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
}

void PrototypeMaker::sm_resetMaps()
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
        prototype->getProtoMap(true);       // always called in a take up situayion, not from loader/take down
    }
}

void PrototypeMaker::sm_takeUp(const TilingPtr & tiling, ePROM_Event event, const TilePtr tile)
{
    qDebug().noquote() << __FUNCTION__ << "- start state:" << mm_states[sm_getState()] << "event:" << sPROM_Events[event];

    eMMState state = sm_getState();
    switch (event)
    {
    case PROM_LOAD_EMPTY:
    {
        auto proto = createPrototypeFromTililing(tiling);
        sm_eraseAllAdd(proto);
        mosaicMaker->sm_takeUp(getPrototypes(),MOSM_LOAD_EMPTY);
    }   break;

    case PROM_LOAD_SINGLE:
        if (state == MM_EMPTY)
        {
            auto proto = createPrototypeFromTililing(tiling);
            sm_eraseAllAdd(proto);
            mosaicMaker->sm_takeUp(getPrototypes(),MOSM_LOAD_SINGLE);
        }
        else if (state == MM_SINGLE)
        {
            auto proto = createPrototypeFromTililing(tiling);
            sm_eraseAllAdd(proto);
            mosaicMaker->sm_takeUp(getPrototypes(),MOSM_LOAD_SINGLE);
        }
        else if (state == MM_MULTI)
        {
            bool createNew = askNewProto();
            if (createNew)
            {
                auto proto = createPrototypeFromTililing(tiling);
                sm_eraseCurrentAdd(proto);
                mosaicMaker->sm_takeUp(getPrototypes(),MOSM_LOAD_MULTI);
            }
            else
            {
                auto proto = getSelectedPrototype();
                sm_replaceTiling(proto, tiling);
                mosaicMaker->sm_takeUp(getPrototypes(),MOSM_RELOAD_MULTI);
            }
        }
        break;

    case PROM_RELOAD_SINGLE:
        if (state == MM_SINGLE)
        {
             auto proto = getSelectedPrototype();
             sm_replaceTiling(proto,tiling);
             mosaicMaker->sm_takeUp(getPrototypes(),MOSM_RELOAD_SINGLE);
        }
        else if (state == MM_MULTI)
        {
             auto proto = getSelectedPrototype();
             sm_replaceTiling(proto,tiling);
             mosaicMaker->sm_takeUp(getPrototypes(),MOSM_RELOAD_SINGLE);
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
            mosaicMaker->sm_takeUp(getPrototypes(),MOSM_LOAD_MULTI);
        }
        else if (state == MM_MULTI)
        {
            auto proto = createPrototypeFromTililing(tiling);
            sm_Add(proto);
            mosaicMaker->sm_takeUp(getPrototypes(),MOSM_LOAD_MULTI);

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
            mosaicMaker->sm_takeUp(getPrototypes(),MOSM_RELOAD_MULTI);
        }
        else if (state == MM_MULTI)
        {
            auto proto = getSelectedPrototype();
            sm_replaceTiling(proto,tiling);
            mosaicMaker->sm_takeUp(getPrototypes(),MOSM_RELOAD_MULTI);
        }
        break;

    case PROM_MOTIF_CHANGED:
        if (propagate)
        {
            sm_resetMaps();
            mosaicMaker->sm_takeUp(getPrototypes(),MOSM_MOTIF_CHANGED);
        }
        break;

    case PROM_TILE_ROTATION_CHANGED:
    case PROM_TILE_SCALE_CHANGED:
        if (tile)
        {
            auto del = getDesignElement(tile);
            if (del)
            {
                auto motif = del->getMotif();
                motif->resetMotifMap();
                if (propagate)
                {
                    sm_resetMaps();
                    mosaicMaker->sm_takeUp(getPrototypes(),MOSM_TILE_CHANGED);
                }
                select(MVD_DELEM,del,config->motifMultiView);
            }
        }
        break;

    case PROM_TILE_REGULARITY_CHANGED:
        // This means changes to: sides,scale,rot,regularity
        // which in turn impacts the motif
        if (tile)
        {
            auto proto = getSelectedPrototype();
            Q_ASSERT(tiling == proto->getTiling());
            auto del = getDesignElement(tile);
            if (del)
            {
                del->recreateMotifWhenRgularityChanged();

                if (propagate)
                {
                    mosaicMaker->sm_takeUp(getPrototypes(),MOSM_TILE_CHANGED);
                }

                select(MVD_DELEM,del,config->motifMultiView);
            }
        }
        break;

    case PROM_TILE_NUM_SIDES_CHANGED:
    case PROM_TILE_EDGES_CHANGED:
        if (tile)
        {
            auto proto = getSelectedPrototype();
            Q_ASSERT(tiling == proto->getTiling());
            auto del = getDesignElement(tile);
            if (del)
            {
                del->recreateMotifFromChangedTile();

                if (propagate)
                {
                    mosaicMaker->sm_takeUp(getPrototypes(),MOSM_TILE_CHANGED);
                }

                select(MVD_DELEM,del,config->motifMultiView);
            }
        }
        break;

    case PROM_TILES_ADDED:
    case PROM_TILES_DELETED:
        // This means additsions/deletions of tiles
        // This does not change motifs - but MotifMaker should be refreshed
        if (state == MM_EMPTY || !getSelectedPrototype()->hasContent() || getSelectedPrototype()->getTiling() != tiling)
        {
            qDebug().noquote() << __FUNCTION__ << "- added1 state:" << mm_states[sm_getState()];
            auto proto = createPrototypeFromTililing(tiling);
            sm_eraseAllAdd(proto);
            qDebug().noquote() << __FUNCTION__ << "- added2 state:" << mm_states[sm_getState()];
            mosaicMaker->sm_takeUp(getPrototypes(),MOSM_TILING_CHANGED);
            qDebug().noquote() << __FUNCTION__ << "- added3 state:" << mm_states[sm_getState()];
        }
        else
        {
            auto protos = getPrototypes();
            auto proto  = getSelectedPrototype();
            proto->wipeoutProtoMap();
            recreatePrototypeFromTiling(tiling,proto);
            setPrototypes(protos);          // rebuilds data
            select(MVD_DELEM,proto,false);  // this updates the menu buttons,etc.
            select(MVD_PROTO,proto,false);  // this updates the menu buttons,etc.
            if (propagate)
            {
                mosaicMaker->sm_takeUp(getPrototypes(),MOSM_TILING_CHANGED);
            }
        }
        break;

    case PROM_TILING_DELETED:
    {
        remove(tiling);
        mosaicMaker->sm_takeUp(getPrototypes(),MOSM_TILING_DELETED);
    }   break;

    case PROM_TILING_CHANGED:
    {
        // This means changes to: placements,transform,fill-vectors,repeats
        // This does not change motifs - but MotifMaker should be refreshed
        Q_ASSERT(!tile);
        auto proto = getSelectedPrototype();
        Q_ASSERT(proto);
        proto->wipeoutProtoMap();
        select(MVD_DELEM,proto,false);  // this updates the menu buttons,etc.
        select(MVD_PROTO,proto,false);  // this updates the menu buttons,etc.
        if (propagate)
        {
            mosaicMaker->sm_takeUp(getPrototypes(),MOSM_TILING_CHANGED);
        }
    }   break;

    case PROM_RENDER:
        // ignores propagate flag
        sm_resetMaps();
        sm_buildMaps();
        mosaicMaker->sm_takeUp(getPrototypes(),MOSM_RENDER);
        break;

    default:
        qCritical().noquote() << __FUNCTION__ << "unsuported event" << sPROM_Events[event];
        break;
    }

    qDebug().noquote() << __FUNCTION__ << "- end0 state:" << mm_states[sm_getState()];
    if (vcontrol->isEnabled(VIEW_MOTIF_MAKER))
    {
//        emit sig_updateView();
    }

    qDebug().noquote() << __FUNCTION__ << "- end1 state:" << mm_states[sm_getState()];
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

void PrototypeMaker::selectDesignElement(DesignElementPtr delp)
{
    auto motifmaker = getWidget();
    auto btn        = motifmaker->getButton(delp);
    motifmaker->delegate(btn,config->motifMultiView,true);
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
    const TilingPlacements tilingUnit = tiling->getTilingUnitPlacements();
    TilingPlacements newPlacedTiles;
    for (const auto & placedTile : std::as_const(tilingUnit))
    {
        if (placedTile->getTile() == tile)
        {
            PlacedTilePtr newPlacedTile = make_shared<PlacedTile>(newTile,placedTile->getTransform());
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
        tiling->addPlacedTile(placedTile);
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

