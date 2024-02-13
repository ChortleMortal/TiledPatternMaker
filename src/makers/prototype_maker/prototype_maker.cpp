#include <QMessageBox>

#include "makers/prototype_maker/prototype_maker.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/motif_maker/design_element_button.h"
#include "makers/prototype_maker/prototype.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "mosaic/design_element.h"
#include "motifs/extended_rosette.h"
#include "motifs/extended_star.h"
#include "motifs/girih_motif.h"
#include "motifs/hourglass_motif.h"
#include "motifs/inferred_motif.h"
#include "motifs/intersect_motif.h"
#include "motifs/irregular_motif.h"
#include "motifs/irregular_rosette.h"
#include "motifs/irregular_star.h"
#include "motifs/rosette.h"
#include "motifs/rosette2.h"
#include "motifs/rosette_connect.h"
#include "motifs/star.h"
#include "motifs/star2.h"
#include "motifs/star_connect.h"
#include "motifs/tile_motif.h"
#include "misc/sys.h"
#include "panels/page_motif_maker.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "tile/placed_tile.h"
#include "tile/tile.h"
#include "tile/tiling.h"
#include "tiledpatternmaker.h"
#include "viewers/view_controller.h"

using std::make_shared;
using std::dynamic_pointer_cast;

PrototypeMaker * PrototypeMaker::mpThis = nullptr;

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

PrototypeMaker * PrototypeMaker::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new PrototypeMaker;
    }
    return mpThis;
}

void PrototypeMaker::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

PrototypeMaker::PrototypeMaker()
{
}

void PrototypeMaker::init()
{
    config          = Configuration::getInstance();
    vcontrol        = Sys::viewController;
    tilingMaker     = TilingMaker::getInstance();
    mosaicMaker     = MosaicMaker::getInstance();
    propagate       = true;
}

void PrototypeMaker::unload()
{
    protoData.erase();
}

void PrototypeMaker::setPropagate(bool val)
{
    propagate = val;
    if (val)
    {
        sm_resetMaps();
        sm_buildMaps();
        mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_MOTIF_CHANGED);
        if (vcontrol->isEnabled(VIEW_PROTOTYPE) || vcontrol->isEnabled(VIEW_MAP_EDITOR))
        {
            //vcontrol->slot_reconstructView();
            Sys::view->update();
        }
    }
}

ProtoPtr PrototypeMaker::createPrototype(const TilingPtr &tiling)
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
        prototype->addElement(dep);
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
            prototype->addElement(dep);
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
        prototype->removeElement(del);
    }

    prototype->wipeoutProtoMap();
}

void PrototypeMaker::sm_takeDown(QVector<ProtoPtr> & prototypes)
{
    qDebug() << "PrototypeMaker::takeDown() Num DELs:" << prototypes.first()->numDesignElements();

    erasePrototypes();

    protoData.setPrototypes(prototypes);

    QVector<TilingPtr> tilings;
    for (const auto & p : std::as_const(prototypes))
    {
        tilings.push_back(p->getTiling());
    }

    tilingMaker->sm_takeDown(tilings, TILM_LOAD_FROM_MOSAIC);

    protoData.select(MVD_DELEM,prototypes.first(),false);
    protoData.select(MVD_PROTO,prototypes.first(),false);
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

    protoData.add(prototype);

    protoData.select(MVD_DELEM, prototype,false);
    protoData.select(MVD_PROTO, prototype,false);
}

void PrototypeMaker::sm_eraseCurrentAdd(const ProtoPtr & prototype)
{
    auto proto = protoData. getSelectedPrototype();
    protoData.remove(proto);

    protoData.add(prototype);

    protoData.select(MVD_DELEM, prototype,false);
    protoData.select(MVD_PROTO, prototype,false);
}

void PrototypeMaker::sm_Add(const ProtoPtr & prototype)
{
    protoData.add(prototype);

    protoData.select(MVD_DELEM, prototype,false);
    protoData.select(MVD_PROTO, prototype,false);
}

void PrototypeMaker::sm_replaceTiling(const ProtoPtr &prototype, const TilingPtr &tiling)
{
    prototype->replaceTiling(tiling);

    protoData.select(MVD_DELEM, prototype,false);
    protoData.select(MVD_PROTO, prototype,false);
}

void PrototypeMaker::sm_resetMaps()
{
    for (const auto & prototype : protoData.getPrototypes())
    {
        prototype->wipeoutProtoMap();
    }
}

void PrototypeMaker::sm_buildMaps()
{
    for (const auto & prototype : protoData.getPrototypes())
    {
        prototype->getProtoMap();
    }
}

void PrototypeMaker::sm_takeUp(const TilingPtr & tiling, ePROM_Event event, const TilePtr tile)
{
    eMMState state = sm_getState();
    qDebug().noquote() << "PrototypeMaker::sm_takeUp() state:" << mm_states[state] << "event:" << sPROM_Events[event];

    switch (event)
    {
    case PROM_LOAD_EMPTY:
    {
        auto proto = createPrototype(tiling);
        sm_eraseAllAdd(proto);
        mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_LOAD_EMPTY);
    }   break;

    case PROM_LOAD_SINGLE:
        if (state == MM_EMPTY)
        {
            auto proto = createPrototype(tiling);
            sm_eraseAllAdd(proto);
            mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_LOAD_SINGLE);
        }
        else if (state == MM_SINGLE)
        {
            auto proto = createPrototype(tiling);
            sm_eraseAllAdd(proto);
            mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_LOAD_SINGLE);
        }
        else if (state == MM_MULTI)
        {
            bool createNew = askNewProto();
            if (createNew)
            {
                auto proto = createPrototype(tiling);
                sm_eraseCurrentAdd(proto);
                mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_LOAD_MULTI);
            }
            else
            {
                auto proto = protoData.getSelectedPrototype();
                sm_replaceTiling(proto, tiling);
                mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_RELOAD_MULTI);
            }
        }
        break;

    case PROM_RELOAD_SINGLE:
        if (state == MM_SINGLE)
        {
             auto proto = protoData.getSelectedPrototype();
             sm_replaceTiling(proto,tiling);
             mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_RELOAD_SINGLE);
        }
        else if (state == MM_MULTI)
        {
             auto proto = protoData.getSelectedPrototype();
             sm_replaceTiling(proto,tiling);
             mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_RELOAD_SINGLE);
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
            auto proto = createPrototype(tiling);
            sm_Add(proto);
            mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_LOAD_MULTI);
        }
        else if (state == MM_MULTI)
        {
            auto proto = createPrototype(tiling);
            sm_Add(proto);
            mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_LOAD_MULTI);

        }
        break;

    case PROM_RELOAD_MULTI:
        if (state == MM_EMPTY)
        {
            qWarning("PrototypeMaker : invalid state (MM_EMPTY)");
        }
        else if (state == MM_SINGLE)
        {
            auto proto = protoData.getSelectedPrototype();
            sm_replaceTiling(proto,tiling);
            mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_RELOAD_MULTI);
        }
        else if (state == MM_MULTI)
        {
            auto proto = protoData.getSelectedPrototype();
            sm_replaceTiling(proto,tiling);
            mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_RELOAD_MULTI);
        }
        break;

    case PROM_MOTIF_CHANGED:
        if (propagate)
        {
            sm_resetMaps();
            sm_buildMaps();
            mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_MOTIF_CHANGED);
        }
        break;

    case PROM_TILE_ROTATION_CHANGED:
    case PROM_TILE_SCALE_CHANGED:
        if (tile)
        {
            auto del = protoData.getDesignElement(tile);
            if (del)
            {
                auto motif = del->getMotif();
                motif->resetMotifMaps();
                if (propagate)
                {
                    sm_resetMaps();
                    sm_buildMaps();
                    mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_TILE_CHANGED);
                }
                protoData.select(MVD_DELEM,del,config->motifMultiView);
            }
        }
        break;

    case PROM_TILE_REGULARITY_CHANGED:
        // This means changes to: sides,scale,rot,regularity
        // which in turn impacts the motif
        if (tile)
        {
            auto proto = protoData.getSelectedPrototype();
            Q_ASSERT(tiling == proto->getTiling());
            auto del = protoData.getDesignElement(tile);
            if (del)
            {
                del->recreateMotifWhenRgularityChanged();

                if (propagate)
                {
                    mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_TILE_CHANGED);
                }

                protoData.select(MVD_DELEM,del,config->motifMultiView);
            }
        }
        break;

    case PROM_TILE_NUM_SIDES_CHANGED:
    case PROM_TILE_EDGES_CHANGED:
        if (tile)
        {
            auto proto = protoData.getSelectedPrototype();
            Q_ASSERT(tiling == proto->getTiling());
            auto del = protoData.getDesignElement(tile);
            if (del)
            {
                del->recreateMotifFromChangedTile();

                if (propagate)
                {
                    mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_TILE_CHANGED);
                }

                protoData.select(MVD_DELEM,del,config->motifMultiView);
            }
        }
        break;

    case PROM_TILING_ADDED:
    case PROM_TILING_DELETED:
        // This means additsions/deletions of tiles
        // This does not change motifs - but MotifMaker should be refreshed
        if (state == MM_EMPTY)
        {
            auto proto = createPrototype(tiling);
            sm_eraseAllAdd(proto);
            mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_TILING_CHANGED);
        }
        else
        {
            auto protos = protoData.getPrototypes();
            auto proto  = protoData.getSelectedPrototype();
            proto->wipeoutProtoMap();
            recreatePrototypeFromTiling(tiling,proto);
            protoData.setPrototypes(protos);          // rebuilds data
            protoData.select(MVD_DELEM,proto,false);  // this updates the menu buttons,etc.
            protoData.select(MVD_PROTO,proto,false);  // this updates the menu buttons,etc.
            if (propagate)
            {
                mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_TILING_CHANGED);
            }
        }
        break;

    case PROM_TILING_CHANGED:
    {
        // This means changes to: placements,transform,fill-vectors,repeats
        // This does not change motifs - but MotifMaker should be refreshed
        Q_ASSERT(!tile);
        auto proto = protoData.getSelectedPrototype();
        Q_ASSERT(proto);
        proto->wipeoutProtoMap();
        protoData.select(MVD_DELEM,proto,false);  // this updates the menu buttons,etc.
        protoData.select(MVD_PROTO,proto,false);  // this updates the menu buttons,etc.
        if (propagate)
        {
            mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_TILING_CHANGED);
        }
    }   break;

    case PROM_RENDER:
        // ignores propagate flag
        sm_resetMaps();
        sm_buildMaps();
        mosaicMaker->sm_takeUp(protoData.getPrototypes(),MOSM_RENDER);
        break;
    }

    if (vcontrol->isEnabled(VIEW_MOTIF_MAKER))
    {
        Sys::view->update();
    }
}

void PrototypeMaker::erasePrototypes()
{
    protoData.erase();
}

PrototypeMaker::eMMState PrototypeMaker::sm_getState()
{
    int count = protoData.getPrototypes().count();
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

// duplication is used to superimpose a second figure in the same tile
// but also a duplicate figure is needed in the tiling to make the system happy
bool PrototypeMaker::duplicateDesignElement()
{
    auto del  = protoData.getSelectedDEL();
    if (!del) return false;

    auto tile  = del->getTile();
    auto motif = del->getMotif();
    qDebug() << "Copying" << sMotifType[motif->getMotifType()];

    auto newTile = make_shared<Tile>(tile);

    // find placed tiles in the tiling and duplicate it
    TilingPtr tiling = protoData.getSelectedPrototype()->getTiling();
    const PlacedTiles & placedTiles = tiling->getInTiling();
    PlacedTiles newPlacedTiles;
    for (const auto & placedTile : std::as_const(placedTiles))
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
        tiling->add(placedTile);
    }

    // rebuild prototype
    auto protos = protoData.getPrototypes();
    auto proto = protoData.getSelectedPrototype();
    recreatePrototypeFromTiling(tiling,proto);
    protoData.setPrototypes(protos);    //  rebuilds data

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

    case MOTIF_TYPE_CONNECT_STAR:
    {
        auto connect = dynamic_pointer_cast<StarConnect>(motif);
        Q_ASSERT(connect);
        newMotif = make_shared<StarConnect>(*connect.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_CONNECT_ROSETTE:
    {
        auto connect = dynamic_pointer_cast<RosetteConnect>(motif);
        Q_ASSERT(connect);
        newMotif = make_shared<RosetteConnect>(*connect.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_EXTENDED_ROSETTE:
    {
        auto extend = dynamic_pointer_cast<ExtendedRosette>(motif);
        Q_ASSERT(extend);
        newMotif = make_shared<ExtendedRosette>(*extend.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_EXTENDED_ROSETTE2:
    {
        auto extend = dynamic_pointer_cast<ExtendedRosette2>(motif);
        Q_ASSERT(extend);
        newMotif = make_shared<ExtendedRosette2>(*extend.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_EXTENDED_STAR:
    {
        auto extend = dynamic_pointer_cast<ExtendedStar>(motif);
        Q_ASSERT(extend);
        newMotif = make_shared<ExtendedStar>(*extend.get());
        Q_ASSERT(newMotif);
    } break;
        
    case MOTIF_TYPE_EXTENDED_STAR2:
    {
        auto extend = dynamic_pointer_cast<ExtendedStar2>(motif);
        Q_ASSERT(extend);
        newMotif = make_shared<ExtendedStar2>(*extend.get());
        Q_ASSERT(newMotif);
    } break;

    case MOTIF_TYPE_EXPLICIT_MAP:
    {
        auto explicitMotif = dynamic_pointer_cast<IrregularMotif>(motif);
        Q_ASSERT(explicitMotif);
        newMotif = make_shared<IrregularMotif>(*explicitMotif.get());
        Q_ASSERT(newMotif);
    } break;
        
    case MOTIF_TYPE_IRREGULAR_NO_MAP:
    {
        auto explicitMotif = dynamic_pointer_cast<IrregularMotif>(motif);
        Q_ASSERT(explicitMotif);
        newMotif = make_shared<IrregularMotif>(*explicitMotif.get());
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
    QMessageBox box(ControlPanel::getInstance());
    box.setIcon(QMessageBox::Question);
    QPushButton *createNewButton  = box.addButton("Create new Prototype", QMessageBox::ActionRole);
    QPushButton *replaceButton    = box.addButton("Replace tiling in existing Prototype", QMessageBox::ActionRole);
    Q_UNUSED(replaceButton);
    box.exec();

    bool createNew = (box.clickedButton() == createNewButton);
    return createNew;
}

