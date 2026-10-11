#include "model/makers/motif_view_data.h"
#include "model/motifs/motif.h"
#include "model/prototypes/tile_motif.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/tilings/tiling.h"
#include "sys/sys.h"

/*
 *  MotifViewData
 *
 *  Prototype data is associated with the selected prototype in the PrototypeMaker.
 *  Whereas the protomaker supports more than one prototype, the motif maker supports
 *  only one TileMotifSet.
 *
 *  This  is operated on by controls associated with both the ProtypeView and the MotifView
 *  which both show  TileMotifs in its view and largely operate on a specific selected TileMotif.
 *
 *  Only one TileMotif is selected, several can be shown (not hidden)
 */

MotifViewData::MotifViewData()
{}

void MotifViewData::erase()
{
    delegatedTileMotif.reset();
    tileMotifInfo.clear();
}

void MotifViewData::usePrototype(ProtoPtr prototype)
{
    activeProto = prototype;

    TileMotifSet timos = prototype->getTileMotifs();
    setTileMotifs(timos);

    auto delegated = getDelegatedTileMotif();
    if ((!delegated || !timos.contains(delegated)) && !timos.isEmpty())
    {
        delegatedTileMotif = timos.first();
        show(timos.first());
    }
}

void MotifViewData::setTileMotifs(TileMotifSet & timos)
{
    tileMotifInfo.clear();
    for (const auto & tm : std::as_const(timos))
    {
        TileMotifInfo info(tm,true);
        tileMotifInfo.push_back(info);
    }

#if 0
    if (timos.size())
        selectedTileMotif = timos.first();
    else
        selectedTileMotif.reset();
#endif
}

void MotifViewData::show(TileMotifPtr tileMotif)
{
#if 0
    selectedTileMotif = tileMotif;
#endif
    for (auto & info : tileMotifInfo)
    {
        auto tm = info.weakTileMotif.lock();
        if (tm == tileMotif)
        {
            info.visible = true;
        }
        else if (Sys::config->motifMakerViewType == MotifMakerView::ViewType::Single)
        {
            // in single mode, only one is visible
            info.visible = false;
        }
    }
}

void MotifViewData::hide(TileMotifPtr tileMotif)
{
#if 0
    selectedTileMotif.reset();
#endif
    for (auto & info : tileMotifInfo)
    {
        auto tm = info.weakTileMotif.lock();
        if (tm == tileMotif)
        {
            info.visible = false;
            break;
        }
    }
}

TileMotifSet MotifViewData::getTileMotifs()
{
    TileMotifSet designUnit;
    for (auto & info : tileMotifInfo)
    {
        auto tm =  info.weakTileMotif.lock();
        if (tm)
        {
            designUnit.push_back(tm);
        }
    }
    return designUnit;
}

TileMotifSet MotifViewData::getVisibleTileMotifs()
{
    TileMotifSet timos;
    for (auto & info : tileMotifInfo)
    {
        if (info.visible)
        {
            auto tm =  info.weakTileMotif.lock();
            if (tm)
            {
                timos.push_back(tm);
            }
        }
    }
    return timos;
}

void MotifViewData::rebuildCurrentMotif()
{
    // this is triggered when toggling debug of motif or unusually when regularity is changed
    auto tm = delegatedTileMotif.lock();
    if (tm)
    {
        auto motif = tm->getMotif();
        motif->resetMotifMap();
        motif->build();
        show(tm);
    }
}

TileMotifPtr MotifViewData::getTileMotif(TilePtr tile)
{
    for (const auto & info : std::as_const(tileMotifInfo))
    {
        auto tm  = info.weakTileMotif.lock();
        if (tm)
        {
            if (tm->getTile() == tile)
            {
                return tm;
            }
        }
    }
    return TileMotifPtr();
}

bool MotifViewData::isHidden(MotifPtr motif)
{
    for (auto & info : tileMotifInfo)
    {
        auto tm  = info.weakTileMotif.lock();
        if (tm && tm->getMotif() == motif)
        {
            return !info.visible;
        }
    }
    return false;
}

bool MotifViewData::isHidden(TileMotifPtr dep) const
{
    for (auto & info : tileMotifInfo)
    {
        auto tm  = info.weakTileMotif.lock();
        if (tm && tm == dep)
        {
            return !info.visible;
        }
    }
    return false;
}

bool MotifViewData::isVisible(TileMotifPtr dep) const
{
    for (auto & info : tileMotifInfo)
    {
        auto tm  = info.weakTileMotif.lock();
        if (tm && tm == dep)
        {
            return info.visible;
        }
    }
    return false;
}
