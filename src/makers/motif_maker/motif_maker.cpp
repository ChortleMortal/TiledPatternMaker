#include <QMessageBox>

#include "makers/motif_maker/motif_maker.h"
#include "makers/motif_maker/motif_button.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "motifs/explicit_motif.h"
#include "motifs/inference_engine.h"
#include "motifs/extended_rosette.h"
#include "motifs/extended_star.h"
#include "motifs/rosette_connect.h"
#include "motifs/star.h"
#include "motifs/star_connect.h"
#include "mosaic/design_element.h"
#include "mosaic/prototype.h"
#include "panels/page_motif_maker.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "tile/tile.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"
#include "tiledpatternmaker.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"

using std::make_shared;

typedef std::shared_ptr<class ExplicitMotif>   ExplicitPtr;
typedef std::shared_ptr<InferenceEngine>                  InferPtr;

MotifMaker * MotifMaker::mpThis = nullptr;

#define E2STR(x) #x

static QString mm_states[]
{
    E2STR(MM_EMPTY),
    E2STR(MM_SINGLE),
    E2STR(MM_MULTI)
 };

MotifMaker * MotifMaker::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new MotifMaker;
    }
    return mpThis;
}

MotifMaker::MotifMaker()
{
}

void MotifMaker::init()
{
    config          = Configuration::getInstance();
    vcontrol        = ViewControl::getInstance();
    tilingMaker     = TilingMaker::getSharedInstance();
    mosaicMaker     = MosaicMaker::getInstance();
    propagate       = true;
}

void MotifMaker::unload()
{
    _activeTile.reset();
    _selectedPrototype.reset();
    _selectedDesignElement.reset();
    _prototypes.clear();
    _selectedDesignElements.clear();
}

PrototypePtr MotifMaker::createPrototype(const TilingPtr &tiling)
{
    PrototypePtr prototype = make_shared<Prototype>(tiling);

    QVector<TilePtr> uniqueTiles = tiling->getUniqueTiles();
    qDebug() << "Create new design elements";
    int count = 0;
    for (auto & tile : qAsConst(uniqueTiles))
    {
        // NOTE this takes order from order of unique tiles
        DesignElementPtr dep = make_shared<DesignElement>(tile);
        prototype->addElement(dep);
        if (++count > MAX_UNIQUE_TILE_INDEX)
            qWarning() << "Large number of unique tiles in tiling count:" << count;
    }
    return prototype;
}

void MotifMaker::recreatePrototype(const TilingPtr &tiling)
{
    PrototypePtr prototype = getSelectedPrototype();
    Q_ASSERT(prototype->getTiling() == tiling);

    QVector<TilePtr> uniqueTiles = tiling->getUniqueTiles();
    int count = 0;
    for (auto tile : qAsConst(uniqueTiles))
    {
        if (++count > MAX_UNIQUE_TILE_INDEX)
            qWarning() << "Large number of unique tiles in tiling count:" << count;

        bool found = false;
        for (auto del : qAsConst(prototype->getDesignElements()))
        {
            if (del->getTile() == tile)
            {
                found = true;
                if (!del->validMotif())
                {
                    del->createMotif();
                }
                break;
            }
        }
        if (!found)
        {
            DesignElementPtr dep = make_shared<DesignElement>(tile);
            prototype->addElement(dep);
        }
    }

    QVector<DesignElementPtr> forDeletion;
    for (auto del : qAsConst(prototype->getDesignElements()))
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

void MotifMaker::recreateMotifs(const TilingPtr &tiling)
{
    PrototypePtr prototype = getSelectedPrototype();
    Q_ASSERT(prototype->getTiling() == tiling);

    for (auto del : qAsConst(prototype->getDesignElements()))
    {
        TilePtr tile = del->getTile();

        MotifPtr motif  = del->getMotif();
        if (!del->validMotif())
        {
            del->createMotif();
            motif  = del->getMotif();
        }

        int sides = tile->numSides();
        if (tile->isRegular())
        {
            auto radial = std::dynamic_pointer_cast<RadialMotif>(motif);
            Q_ASSERT(radial);
            if (sides != motif->getN())
            {
                motif->setN(sides);
                ExtendedBoundary & extended = motif->getRWExtendedBoundary();
                extended.sides = sides;
                extended.buildRadial();
            }
            radial->resetMaps();
        }
        else
        {
            auto exp = std::dynamic_pointer_cast<ExplicitMotif>(motif);
            Q_ASSERT(exp);
            if (sides != exp->getN())
            {
                exp->setN(tile->numSides());
                ExtendedBoundary & extended = motif->getRWExtendedBoundary();
                extended.sides = sides;
                extended.buildExplicit(tile);
            }
            exp->resetMaps();
        }
    }
}

void MotifMaker::sm_takeDown(const PrototypePtr & prototype)
{
    qDebug() << "MotifMaker::takeDown()";

    _prototypes.push_front(prototype);

    setSelectedPrototype(prototype);

    tilingMaker->sm_take(prototype->getTiling(), SM_LOAD_FROM_MOSAIC);
}

/* There are 5 possible actions
   1. erase all proto, create new proto, and add
   2. erase current proto, create new proto, and add
   3. create new proto using tiling and add
   4. replace tiling in current proto
   5. reset prototype maps
*/

void MotifMaker::sm_eraseAllCreateAdd(const TilingPtr &tiling)
{
    erasePrototypes();
    PrototypePtr prototype = createPrototype(tiling);
    _prototypes.push_front(prototype);
    setSelectedPrototype(prototype);
}

void MotifMaker::sm_eraseCurrentCreateAdd(const TilingPtr &tiling)
{
    _prototypes.removeOne(getSelectedPrototype());
    PrototypePtr prototype = createPrototype(tiling);
    _prototypes.push_front(prototype);
    setSelectedPrototype(prototype);
}

void MotifMaker::sm_createAdd(const TilingPtr &tiling)
{
    PrototypePtr prototype = createPrototype(tiling);
    _prototypes.push_front(prototype);
    setSelectedPrototype(prototype);
}

void MotifMaker::sm_replaceTiling(const PrototypePtr &prototype, const TilingPtr &tiling)
{
    prototype->setTiling(tiling);
    setSelectedPrototype(prototype);
}

void MotifMaker::sm_resetMaps()
{
    for (auto prototype : qAsConst(_prototypes))
    {
        prototype->wipeoutProtoMap();
    }
}

void  MotifMaker::sm_takeUp(const TilingPtr & tiling, eSM_Event event)
{
    eMMState state = sm_getState();
    qDebug().noquote() << "MotifMaker::sm_take() state:" << mm_states[state] << "event:" << sSM_Events[event];

    QVector<PrototypePtr> protos;
    switch (event)
    {
    case SM_LOAD_EMPTY:
        sm_eraseAllCreateAdd(tiling);
        protos.push_back(getSelectedPrototype());
        mosaicMaker->sm_takeUp(protos,SM_LOAD_EMPTY);
        emit sig_tilingChoicesChanged();
        break;

    case SM_LOAD_SINGLE:
        if (state == MM_EMPTY)
        {
            sm_eraseAllCreateAdd(tiling);
            //mosaicMaker->sm_takeUp(selectedPrototype,event);
        }
        else
        {
             if (state == MM_SINGLE)
             {
#if 0
                 bool createNew = askNewProto();
                 if (createNew)
                 {
                     sm_eraseAllCreateAdd(tiling);
                     mosaicMaker->sm_takeUp(selectedPrototype,SM_LOAD_SINGLE);
                 }
                 else
                 {
                     sm_replaceTiling(selectedPrototype, tiling);
                     mosaicMaker->sm_takeUp(selectedPrototype,SM_RELOAD_SINGLE);
                 }
#else
                 sm_eraseAllCreateAdd(tiling);
                 protos.push_back(getSelectedPrototype());
                 mosaicMaker->sm_takeUp(protos,SM_LOAD_SINGLE);
#endif
             }
             else if (state == MM_MULTI)
             {
                 bool createNew = askNewProto();
                 if (createNew)
                 {
                     sm_eraseCurrentCreateAdd(tiling);
                     protos.push_back(getSelectedPrototype());
                     mosaicMaker->sm_takeUp(protos,SM_LOAD_MULTI);
                 }
                 else
                 {
                     auto proto = getSelectedPrototype();
                     sm_replaceTiling(proto, tiling);
                     protos.push_back(proto);
                     mosaicMaker->sm_takeUp(protos,SM_RELOAD_MULTI);
                 }
             }
        }
        emit sig_tilingChoicesChanged();
        break;

    case SM_RELOAD_SINGLE:
        if (state == MM_SINGLE)
        {
            auto proto = getSelectedPrototype();
            sm_replaceTiling(proto,tiling);
            protos.push_back(proto);
            mosaicMaker->sm_takeUp(protos,SM_RELOAD_SINGLE);
        }
        else if (state == MM_MULTI)
        {
            auto proto = getSelectedPrototype();
            sm_replaceTiling(proto,tiling);
            protos.push_back(proto);
            mosaicMaker->sm_takeUp(protos,SM_RELOAD_SINGLE);
        }
        else
        {
            Q_ASSERT(state == MM_EMPTY);
            qWarning("MotifMaker : invalid state");
        }
        emit sig_tilingChoicesChanged();
        break;

    case SM_LOAD_MULTI:
        if (state == MM_EMPTY)
        {
            qWarning("Invalid state");
        }
        else if (state == MM_SINGLE)
        {
            sm_createAdd(tiling);
            protos.push_back(getSelectedPrototype());
            mosaicMaker->sm_takeUp(protos,SM_LOAD_MULTI);
        }
        else if (state == MM_MULTI)
        {
            sm_createAdd(tiling);
            protos.push_back(getSelectedPrototype());
            mosaicMaker->sm_takeUp(protos,SM_LOAD_MULTI);

        }
        emit sig_tilingChoicesChanged();
        break;

    case SM_RELOAD_MULTI:
        if (state == MM_EMPTY)
        {
            qWarning("Invalid state");
        }
        else if (state == MM_SINGLE)
        {
            auto proto = getSelectedPrototype();
            sm_replaceTiling(proto,tiling);
            protos.push_back(proto);
            mosaicMaker->sm_takeUp(protos,SM_RELOAD_MULTI);
        }
        else if (state == MM_MULTI)
        {
            auto proto = getSelectedPrototype();
            sm_replaceTiling(proto,tiling);
            protos.push_back(proto);
            mosaicMaker->sm_takeUp(protos,SM_RELOAD_MULTI);
        }
        emit sig_tilingChoicesChanged();
        break;

    case SM_MOTIF_CHANGED:
        sm_resetMaps();
        if (propagate)
        {
            mosaicMaker->sm_takeUp(protos,SM_MOTIF_CHANGED);
        }
        break;

    case SM_TILE_CHANGED:
        Q_ASSERT(tiling == getSelectedPrototype()->getTiling());
        recreateMotifs(tiling);
        if (propagate)
        {
            mosaicMaker->sm_takeUp(protos,SM_TILE_CHANGED);
        }
        emit sig_tileChanged();
        break;

    case SM_TILING_CHANGED:
        if (state == MM_EMPTY)
        {
            sm_eraseAllCreateAdd(tiling);
        }
        else
        {
            recreatePrototype(tiling);
            emit sig_tilingChanged();
            if (propagate)
            {
                mosaicMaker->sm_takeUp(protos,SM_TILING_CHANGED);
            }
        }
        break;

    case SM_RENDER:
        sm_resetMaps();
        for (auto& prototype : _prototypes )
        {
            protos.push_back(prototype);
        }
        if (propagate)
        {
            mosaicMaker->sm_takeUp(protos,SM_RENDER);
        }
        break;

    case SM_LOAD_FROM_MOSAIC:
        qWarning("Invalid mode");
        break;
    }

    if (vcontrol->isEnabled(VIEW_MOTIF_MAKER))
        vcontrol->update();
}

const QVector<PrototypePtr> & MotifMaker::getPrototypes()
{
    return _prototypes;
}

void MotifMaker::removePrototype(TilingPtr tiling)
{
    QVector<PrototypePtr> forRemoval;

    for (auto prototype : qAsConst(_prototypes))
    {
        if (prototype->getTiling() == tiling)
        {
            forRemoval.push_back(prototype);
        }
    }

    for (auto& prototype : forRemoval)
    {
        _prototypes.removeAll(prototype);
        if (getSelectedPrototype() == prototype)
        {
            resetSelectedPrototype();
        }
    }
}

void MotifMaker::erasePrototypes()
{
    _prototypes.clear();
    resetSelectedPrototype();
}

eMMState MotifMaker::sm_getState()
{
    if (_prototypes.size() > 1)
    {
        return  MM_MULTI;
    }
    else if (_prototypes.size() == 0)
    {
        return MM_EMPTY;
    }
    else
    {
        Q_ASSERT(getSelectedPrototype());
        Q_ASSERT(_prototypes.size() == 1);
        if (getSelectedPrototype()->hasContent())
        {
            return MM_SINGLE;
        }
        else
        {
            return MM_EMPTY;
        }
    }
}

// duplication is used to superimpose a second figure in the same tile
// but also a duplicate figure is needed in the tiling to make the system happy
bool MotifMaker::dupolicateMotif()
{
    auto del  = getSelectedDesignElement();
    if (!del) return false;
    auto tile  = del->getTile();
    auto motif = del->getMotif();
    qDebug() << "Copying" << sTileType[motif->getMotifType()];

    // find placed tile in the tiling and duplicate it
    bool found = false;
    TilePtr newTile;
    TilingPtr tiling = getSelectedPrototype()->getTiling();
    const QVector<PlacedTilePtr> & pfps = tiling->getData().getPlacedTiles();
    for (auto pfp : pfps)
    {
        if (pfp->getTile() == tile)
        {
            PlacedTilePtr newplacedTile = make_shared<PlacedTile>(pfp);
            // put placed tile in the tiling
            tiling->add(newplacedTile);
            newTile = newplacedTile->getTile();
            found = true;
            break;
        }
    }
    if (!found)
        return false;

    // make a copy of the figure
    MotifPtr newMotif;
    if (motif->isRadial())
    {
        switch (motif->getMotifType())
        {
        case MOTIF_TYPE_ROSETTE:
        {
            auto rosette = std::dynamic_pointer_cast<Rosette>(motif);
            Q_ASSERT(rosette);
            newMotif = make_shared<Rosette>(*rosette.get());
            Q_ASSERT(newMotif);
        }
            break;
        case MOTIF_TYPE_STAR:
        {
            auto star = std::dynamic_pointer_cast<Star>(motif);
            Q_ASSERT(star);
            newMotif = make_shared<Star>(*star.get());
            Q_ASSERT(newMotif);
        }
            break;
        case MOTIF_TYPE_CONNECT_STAR:
        {
            auto connect = std::dynamic_pointer_cast<StarConnect>(motif);
            Q_ASSERT(connect);
            newMotif = make_shared<StarConnect>(*connect.get());
            Q_ASSERT(newMotif);
        }
            break;
        case MOTIF_TYPE_CONNECT_ROSETTE:
        {
            auto connect = std::dynamic_pointer_cast<RosetteConnect>(motif);
            Q_ASSERT(connect);
            newMotif = make_shared<RosetteConnect>(*connect.get());
            Q_ASSERT(newMotif);
        }
            break;
        case MOTIF_TYPE_EXTENDED_ROSETTE:
       {
            auto extend = std::dynamic_pointer_cast<ExtendedRosette>(motif);
            Q_ASSERT(extend);
            newMotif = make_shared<ExtendedRosette>(*extend.get());
            Q_ASSERT(newMotif);
        }
            break;
        case MOTIF_TYPE_EXTENDED_STAR:
        {
            auto extend = std::dynamic_pointer_cast<ExtendedStar>(motif);
            Q_ASSERT(extend);
            newMotif = make_shared<ExtendedStar>(*extend.get());
            Q_ASSERT(newMotif);
        }
            break;
        default:
        case MOTIF_TYPE_RADIAL:
            return false;
        }
    }
    else
    {
        auto explicitFig = std::dynamic_pointer_cast<ExplicitMotif>(motif);
        Q_ASSERT(explicitFig);
        newMotif = make_shared<ExplicitMotif>(*explicitFig.get());
        Q_ASSERT(newMotif);
    }

    // make a new design element and put it in the prototype
    qDebug() << newTile->summary() << sTileType[newMotif->getMotifType()];
    DesignElementPtr newDesignElement = make_shared<DesignElement>(newTile,newMotif);
    getSelectedPrototype()->addElement(newDesignElement);
    getSelectedPrototype()->wipeoutProtoMap();
    return true;
}

void MotifMaker::deleteActiveTile()
{
    TilePtr fp = getActiveTile();
    if (!fp)
        return;

    auto pp = getSelectedPrototype();
    if (!pp)
        return;

    QVector<PlacedTilePtr> deletions;
    TilingPtr tiling = pp->getTiling();
    const QVector<PlacedTilePtr> & pfps = tiling->getData().getPlacedTiles();
    for (auto pfp : pfps)
    {
        if (pfp->getTile() == fp)
        {
            deletions.push_back(pfp);
        }
    }
    for (auto pfp : deletions)
    {
    	tiling->remove(pfp);
    }
    pp->removeElement(getSelectedDesignElement());
    pp->wipeoutProtoMap();
}

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

void MotifMaker::setSelectedPrototype(const PrototypePtr &pp)
{
    if (_selectedPrototype == pp)
    {
        return;
    }

    _selectedPrototype = pp;
    if (pp && pp->hasContent())
    {
        _selectedDesignElement = pp->getDesignElement(0);  // first
        _selectedDesignElements.push_back(_selectedDesignElement);
        _activeTile         = _selectedDesignElement->getTile();
    }
    else
        _selectedDesignElement.reset();
}

PrototypePtr MotifMaker::getSelectedPrototype()
{
    return _selectedPrototype;
}

void MotifMaker::setSelectedDesignElement(const DesignElementPtr &del)
{
    _selectedDesignElement = del;
    if (!config->motifMultiView)
    {
        _selectedDesignElements.clear();
    }
    _selectedDesignElements.push_back(del);
}

PrototypePtr MotifMaker::findPrototypeByName(const TilingPtr &tiling)
{
    PrototypePtr pp;
    for (auto prototype : qAsConst(_prototypes))
    {
        if (prototype->getTiling()->getName() == tiling->getName())
        {
            return prototype;
        }
    }
    return pp;
}

bool MotifMaker::askNewProto()
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
