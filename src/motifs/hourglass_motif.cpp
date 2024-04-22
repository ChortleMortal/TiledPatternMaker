#include "motifs/hourglass_motif.h"
#include "geometry/map.h"
#include "tile/tile.h"

HourglassMotif::HourglassMotif()  : IrregularStarBranches()
{
    setMotifType(MOTIF_TYPE_HOURGLASS);
}

void HourglassMotif::init(qreal d, int s)
{
    this->d = d;
    this->s = s;
}

HourglassMotif::HourglassMotif(const Motif & other) : IrregularStarBranches(other)
{
    setMotifType(MOTIF_TYPE_HOURGLASS);
}

void HourglassMotif::buildMotifMaps()
{
    Q_ASSERT(getTile());
    motifMap = std::make_shared<Map>("HourglassMotif map");
    inferHourglass();
    scaleAndRotate();
    completeMap();
    buildMotifBoundary();
    buildExtendedBoundary();
}

// Hourglass inferring.
void HourglassMotif::inferHourglass()
{
    qDebug() << "Infer::inferHourglass";

    corners = getTile()->getPoints();
    mids    = getTile()->getMids();

    // Fix the s value to be between [0, side_count / 2 - 1]
    // instead of [1, side_count / 2].
    int side_modulo;
    if ( (getN() & 1) != 0 )
    {
        side_modulo = getN();
    }
    else
    {
        side_modulo = getN() / 2;
    }
    int clamp_s = s % side_modulo;

    for ( int side = 0; side < getN(); ++side )
    {
        qreal side_frac = static_cast<qreal>(side) / static_cast<qreal>(getN());
        qreal hour_d_pos = (side             ) % side_modulo != clamp_s ? d : 1.0;
        qreal hour_d_neg = (side + getN() - 1) % side_modulo != clamp_s ? d : 1.0;

        motifMap->mergeMap(buildStarHalfBranchV1( hour_d_pos, 1, side_frac,  1));
        motifMap->mergeMap(buildStarHalfBranchV1( hour_d_neg, 1, side_frac, -1));
    }

    //motifMap->transformMap(pmain->getTransform().inverted());
    qDebug().noquote() << motifMap->summary();
}

