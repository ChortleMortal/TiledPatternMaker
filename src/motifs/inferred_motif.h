#pragma once
#ifndef INFERRED_MOTIF_H
#define INFERRED_MOTIF_H

#include <QMap>
#include <QTransform>
#include <QDebug>
#include "motifs/irregular_motif.h"
#include "tile/placed_tile.h"
#include "tile/tile.h"

typedef std::shared_ptr<class AdjacenctTile>    AdjacentTilePtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class TileMidPoints>    MidsPtr;
typedef std::shared_ptr<class Contact>          ContactPtr;

typedef std::weak_ptr<Prototype>                WeakProtoPtr;

class InferredMotif : public IrregularMotif
{
public:
    InferredMotif();
    InferredMotif(const InferredMotif & other);
    InferredMotif(const Motif & other);
    InferredMotif(MotifPtr other);

    void                setupInfer(ProtoPtr proto);
    virtual void        buildMotifMaps()  override;
    virtual QString     getMotifDesc()    override { return "InferredMotif"; }
    virtual void        report()          override { qDebug().noquote() << getMotifDesc(); }

    bool                hasDebugContacts() { return debugContacts; }
    const QVector<ContactPtr> & getDebugContacts() { return debugContactPts; }

protected:
    void                infer(ProtoPtr proto);    // "Normal" magic inferring

    QVector<ContactPtr> buildContacts(MidsPtr pp, const QVector<AdjacentTilePtr> &adjs);
    bool                isColinear( QPointF p, QPointF q, QPointF a );
    int                 lexCompareDistances(int kind1, qreal dist1, int kind2, qreal dist2 );

    int                      findPrimaryTile(TilePtr tile);
    AdjacentTilePtr          getAdjacency(QPointF main_point, int main_idx );
    QVector<AdjacentTilePtr> getAdjacenctTiles(MidsPtr pp, int main_idx );


private:
    WeakProtoPtr            proto;

    QMap<TilePtr,MapPtr>    adjacentTileMaps;
    TilingPtr               tiling;
    QVector<MidsPtr>        allMotifMids;

    bool                    debugContacts;
    QVector<ContactPtr>     debugContactPts;

};

#endif // INFERRED_MOTIF_H
