#pragma once

#include <QVector>
#include <QDebug>

typedef std::shared_ptr<class Motif>            MotifPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class TileMotif>        TileMotifPtr;
typedef std::shared_ptr<class Tile>             TilePtr;

typedef std::weak_ptr<class TileMotif>          WeakTileMotifPtr;
typedef std::weak_ptr<class Prototype>          WeakProtoPtr;
typedef QVector<TileMotifPtr>                   TileMotifSet;

/*
 * The Prototype Maker puts information into the ProtoTypeData which is
 * read (shared) directly by both the PrototypeMakerView and the Motif Maker View.
 * The controlling menus for these makers use specific indices into the data
 * MVD_TILE_MOTIF the TileMotif view and MVP_PROT for the Prottype view)
 *
 */

struct TileMotifInfo
{
    WeakTileMotifPtr weakTileMotif;
    bool             visible;
};

class MotifViewData
{
    friend class MotifMaker;

public:
    MotifViewData();
    
    void                    usePrototype(ProtoPtr prototype);
    ProtoPtr                getActivePrototype()    { return activeProto.lock(); }

    int                     numTileMotifs()         { return tileMotifInfo.size(); }
    int                     numVisibleTileMotifs()  { return getVisibleTileMotifs().count(); }
    TileMotifSet            getTileMotifs();
    TileMotifSet            getVisibleTileMotifs();

    TileMotifPtr            getTileMotif(TilePtr tile);

    void                    setDelegatedTileMotif(TileMotifPtr timo) { delegatedTileMotif = timo; }
    TileMotifPtr            getDelegatedTileMotif()                  { return delegatedTileMotif.lock(); }
    bool                    isDelegated(TileMotifPtr tileMotif)      { return (tileMotif == delegatedTileMotif.lock()); }

    void                    show (TileMotifPtr tileMotif);
    void                    hide(TileMotifPtr tileMotif);

    bool                    isVisible(TileMotifPtr del) const;
    bool                    isHidden(TileMotifPtr del) const;
    bool                    isHidden(MotifPtr motif);

    void                    rebuildCurrentMotif();                 // used only for debug

protected:
    void                    setTileMotifs(TileMotifSet & timos);
    void                    erase();

private:
    WeakProtoPtr            activeProto;
    WeakTileMotifPtr        delegatedTileMotif;
    QVector<TileMotifInfo>  tileMotifInfo;
};
