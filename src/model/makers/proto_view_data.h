#pragma once

#include <QVector>
#include <QDebug>

#include "model/makers/motif_view_data.h"

typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class TileMotif>        TileMotifPtr;

typedef std::weak_ptr<class Prototype>          WeakProtoPtr;
typedef std::weak_ptr<class TileMotif>          WeakTileMotifPtr;
typedef QVector<TileMotifPtr>                   TileMotifSet;

/*
 * The Prototype Maker puts information into the ProtoTypeData which is
 * read (shared) directly by both the PrototypeMakerView and the Motif Maker View.
 * The controlling menus for these makers use specific indices into the data
 * MVD_TILE_MOTIF the TileMotif view and MVP_PROT for the Prottype view)
 *
 */

struct ProtoInfo
{
    WeakProtoPtr wproto;
    bool         visibile;
};

class ProtoViewData
{
    friend class MotifMaker;

public:
    ProtoViewData() {}

    void                    setPrototype(ProtoPtr proto);
    void                    setPrototypes(QVector<ProtoPtr> & protos);

    void                    remove(ProtoPtr proto);
    void                    add(ProtoPtr proto);

    ProtoPtr                getPrototype(TilingPtr tiling);
    const QVector<ProtoPtr> getPrototypes();
    const QVector<ProtoPtr> getVisiblePrototypes();

    int                     numTileMotifs()         { return tileMotifInfo.size(); }

    void                    show(ProtoPtr proto, bool show);
    bool                    isVisible(ProtoPtr proto);

    void                    show(TileMotifPtr time, bool show);
    bool                    isVisible(TileMotifPtr timo) const;

protected:
    void                    erase();

    ProtoPtr                remove(TilingPtr tiling);
    void                    remove(TileMotifPtr atimo);

    void                    setTileMotifs(TileMotifSet & timos);
    void                    addTileMotifs(TileMotifSet & timos);

private:
    QVector<ProtoInfo>      protoInfo;
    QVector<TileMotifInfo>  tileMotifInfo;
};
