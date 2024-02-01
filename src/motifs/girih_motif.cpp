#include <QtMath>
#include "girih_motif.h"
#include "tile/tile.h"
#include "geometry/map.h"

GirihMotif::GirihMotif() : IrregularGirihBranches()
{
    setMotifType(MOTIF_TYPE_GIRIH);
}

void GirihMotif::init(qreal starSkip)
{
    this->skip = starSkip;
}

GirihMotif::GirihMotif(const Motif & motif) : IrregularGirihBranches(motif)
{
    setMotifType(MOTIF_TYPE_GIRIH);
}

void GirihMotif::buildMotifMaps()
{
    Q_ASSERT(getTile());
    motifMap = std::make_shared<Map>("Girih Map");
    inferGirih();
    scaleAndRotate();
    completeMap();
    buildMotifBoundary();
    buildExtendedBoundary();
}

//void GirihMotif::inferGirih(TilePtr tile, int starSides, qreal skip)
void GirihMotif::inferGirih()
{
    qDebug() << "Infer::inferGirih";

    // We use the number of side of the star and how many side it
    // hops over from branch to branch (where 1 would mean drawing
    // a polygon) and deduce the inner angle of the star branches.
    // We support fractional side skipping.
    qreal qstarSides           = static_cast<qreal>(getN());
    qreal polygonInnerAngle    = M_PI * (qstarSides-2.0) / qstarSides;
    qreal starBranchInnerAngle = (skip * polygonInnerAngle) - ((skip - 1) * M_PI);
    qreal requiredRotation     = (M_PI - starBranchInnerAngle) / 2;

#if 0
    // Get the index of a good transform for this tile.
    int cur       = findPrimaryTile(tile);
    MidsPtr pmain = allMotifMids[cur];
    mids          = pmain->getTileMidPoints();
    corners       = pmain->getTransformedPoints();
#else
    corners = getTile()->getPoints();
    mids    = getTile()->getEdgePoly().getMids();
#endif

    int side_count = mids.size();
    for (int side = 0; side < side_count; ++side)
    {
        MapPtr branchMap = buildGirihBranch(side, requiredRotation);
        motifMap->mergeMap(branchMap);     // merge even if empty (null) map
    }

#if 0
    motifMap->transformMap(pmain->getTransform().inverted() );
    qDebug().noquote() << motifMap->namedSummary();
#endif
}

