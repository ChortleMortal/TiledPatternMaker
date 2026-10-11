#include "model/makers/proto_view_data.h"
#include "model/prototypes/tile_motif.h"
#include "model/prototypes/prototype.h"
#include "model/tilings/tiling.h"

/*
 *  ProtoViewData
 *
 *  Prototype view data is associates with the Prototype View.
 *
 *  The PrototypeView shows the whole selected prototype, and various parts of which can
 *  be shown/hidden.
 *
 */

void ProtoViewData::setPrototype(ProtoPtr proto)
{
    erase();

    ProtoInfo info(proto,true);
    protoInfo.push_back(info);

    auto timos = proto->getTileMotifs();
    setTileMotifs(timos);
}

void ProtoViewData::setPrototypes(QVector<ProtoPtr> & protos)
{
    erase();

    for (const auto & proto : std::as_const(protos))
    {
        ProtoInfo info(proto,true);
        protoInfo.push_back(info);

        auto timos = proto->getTileMotifs();
        addTileMotifs(timos);
    }
}

ProtoPtr ProtoViewData::getPrototype(TilingPtr tiling)
{
    const auto & protos = getPrototypes();
    for (auto & proto : std::as_const(protos))
    {
        if (proto->getTiling() == tiling)
        {
            return proto;
        }
    }
    ProtoPtr pp;
    return pp;
}

const QVector<ProtoPtr> ProtoViewData::getPrototypes()
{
    QVector<ProtoPtr> vec;
    for (const auto & info : std::as_const(protoInfo))
    {
        auto proto = info.wproto.lock();
        if (proto)
        {
            vec.push_back(proto);
        }
    }
    return vec;
}

const QVector<ProtoPtr> ProtoViewData::getVisiblePrototypes()
{
    QVector<ProtoPtr> vec;
    for (ProtoInfo &  info : protoInfo)
    {
        auto proto = info.wproto.lock();
        if (proto && info.visibile)
        {
            vec.push_back(proto);
        }
    }
    return vec;
}

void ProtoViewData::show(ProtoPtr proto, bool show)
{
    for (auto & info : protoInfo)
    {
        auto aproto = info.wproto.lock();
        if (aproto && aproto == proto)
        {
            info.visibile = show;
            return;
        }
    }
}

bool ProtoViewData::isVisible(ProtoPtr proto)
{
    for (auto & info : protoInfo)
    {
        auto aproto = info.wproto.lock();
        if (aproto && aproto == proto)
        {
            return info.visibile;
        }
    }
    return false;
}

void ProtoViewData::show(TileMotifPtr timo, bool show)
{
    for (auto & info : tileMotifInfo)
    {
        auto tm  = info.weakTileMotif.lock();
        if (tm && tm == timo)
        {
            info.visible = show;
            return;
        }
    }
}

bool ProtoViewData::isVisible(TileMotifPtr timo) const
{
    for (auto & info : tileMotifInfo)
    {
        auto tm  = info.weakTileMotif.lock();
        if (tm && tm == timo)
        {
            return info.visible;
        }
    }
    return false;
}

void ProtoViewData::erase()
{
    protoInfo.clear();
    tileMotifInfo.clear();
}

void ProtoViewData::add(ProtoPtr proto)
{
    ProtoInfo info(proto,true);
    protoInfo.push_back(info);

    addTileMotifs(proto->getTileMotifs());
}

void ProtoViewData::remove(ProtoPtr proto)
{
    auto timos = proto->getTileMotifs();
    for (auto & timo : timos)
    {
        remove(timo);
    }

    QVector<ProtoInfo> cleaned;
    for (const auto & info : std::as_const(protoInfo))
    {
        auto aproto = info.wproto.lock();
        if (aproto != proto)
        {
            cleaned.push_back(info);
        }
    }
    protoInfo = cleaned;
}

ProtoPtr ProtoViewData::remove(TilingPtr tiling)
{
    ProtoPtr forRemoval;

    for (const auto & prototype : getPrototypes())
    {
        if (prototype->getTiling() == tiling)
        {
            forRemoval = prototype;
            break;
        }
    }

    if (forRemoval)
    {
        remove(forRemoval);
    }

    return forRemoval;
}

void ProtoViewData::remove(TileMotifPtr atimo)
{
    TileMotifInfo * forRemoval = nullptr;

    for (auto & info : tileMotifInfo)
    {
        if (info.weakTileMotif.lock() == atimo)
        {
            forRemoval = &info;
            break;
        }
    }

    if (forRemoval)
    {
        QVector<TileMotifInfo> cleaned;

        for (auto & info : tileMotifInfo)
        {
            if (&info != forRemoval)
            {
                cleaned.push_back(info);
            }
        }
        tileMotifInfo = cleaned;
    }
}

void ProtoViewData::setTileMotifs(TileMotifSet & timos)
{
    tileMotifInfo.clear();

    addTileMotifs(timos);
}

void ProtoViewData::addTileMotifs(TileMotifSet & timos)
{
    for (const auto & tm : std::as_const(timos))
    {
        TileMotifInfo info(tm,false);
        tileMotifInfo.push_back(info);
    }
}
