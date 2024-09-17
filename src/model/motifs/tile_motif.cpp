#include <QDebug>
#include "model/motifs/tile_motif.h"
#include "model/tilings/tile.h"
#include "sys/geometry/edgepoly.h"
#include "sys/geometry/vertex.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/map.h"

TileMotif::TileMotif() : IrregularMotif()
{
    setMotifType(MOTIF_TYPE_EXPLCIT_TILE);
}

TileMotif::TileMotif(const Motif &other) : IrregularMotif(other)
{
    setMotifType(MOTIF_TYPE_EXPLCIT_TILE);
    if (!other.isIrregular())
    {
        // this is used as a conversion from radial to irregular
        motifScale  = 1.0;
        motifRotate = 0.0;
    }
}

void TileMotif::infer()
{
    qDebug() << "TileMotif::inferMotif";

    motifMap = std::make_shared<Map>("Tile motif map");

    auto epoly = getTile()->getEdgePoly();

    for (auto edge : std::as_const(epoly))
    {
        // this makes new eges and vertices since they can get altered in the map
        VertexPtr v1 = motifMap->insertVertex(edge->v1->pt);
        VertexPtr v2 = motifMap->insertVertex(edge->v2->pt);
        EdgePtr newEdge = motifMap->insertEdge(v1,v2);
        if (edge->isCurve())
        {
            bool convex = edge->isConvex();
            QPointF ac  = edge->getArcCenter();
            newEdge->setCurvedEdge(ac,convex,(edge->getType() == EDGETYPE_CHORD));
        }
    }

    qDebug().noquote() << motifMap->summary();
}
