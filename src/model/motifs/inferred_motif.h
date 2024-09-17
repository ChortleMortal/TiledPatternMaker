#pragma once
#ifndef INFERRED_MOTIF_H
#define INFERRED_MOTIF_H

#include <QMap>
#include <QTransform>
#include <QDebug>
#include "model/motifs/irregular_motif.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"

typedef std::shared_ptr<class AdjacenctTile>    AdjacentTilePtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class TileMidPoints>    MidsPtr;
typedef std::shared_ptr<class Contact>          ContactPtr;

typedef std::weak_ptr<Prototype>                WeakProtoPtr;

class InferredMotif : public IrregularMotif
{
public:
    enum eKind
    {
        // The different kinds of connections that can be made between contacts, in increasing order of badness.
        // This is used to compare two possible connections.
        INSIDE_EVEN 	= 0,
        INSIDE_COLINEAR = 1,
        INSIDE_UNEVEN 	= 2,
        OUTSIDE_EVEN 	= 3,
        OUTSIDE_UNEVEN 	= 4,
        INFER_NONE 		= 5
    };

    InferredMotif();
    InferredMotif(const InferredMotif & other);
    InferredMotif(const Motif & other);
    InferredMotif(MotifPtr other);

    void                setupInfer(ProtoPtr proto);
    virtual QString     getMotifDesc()    override { return "InferredMotif"; }
    virtual void        dump()          override { qDebug().noquote() << getMotifDesc(); }

    bool                hasDebugContacts() { return debugContacts; }
    const QVector<ContactPtr> & getDebugContacts() { return debugContactPts; }

protected:
    void                infer() override;    // "Normal" magic inferring

    QVector<ContactPtr> buildContacts(MidsPtr pp, const QVector<AdjacentTilePtr> &adjs);
    bool                isColinear( QPointF p, QPointF q, QPointF a );
    int                 lexCompareDistances(eKind kind1, qreal dist1, eKind kind2, qreal dist2 );

    int                      findPrimaryTile(TilePtr tile);
    AdjacentTilePtr          getAdjacency(QPointF main_point, int main_idx );
    QVector<AdjacentTilePtr> getAdjacenctTiles(MidsPtr pp, int main_idx );


private:
    WeakProtoPtr            wProto;

    QMap<TilePtr,MapPtr>    adjacentTileMaps;
    TilingPtr               tiling;
    QVector<MidsPtr>        allMotifMids;

    bool                    debugContacts;
    QVector<ContactPtr>     debugContactPts;

};

#endif // INFERRED_MOTIF_H
