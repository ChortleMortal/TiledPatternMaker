#include <QDebug>
#include "motifs/tile_motif.h"
#include "tile/tile.h"
#include "geometry/edgepoly.h"
#include "geometry/vertex.h"
#include "geometry/edge.h"
#include "geometry/map.h"

TileMotif::TileMotif() : IrregularMotif()
{
    setMotifType(MOTIF_TYPE_EXPLCIT_TILE);
}

TileMotif::TileMotif(const Motif &other) : IrregularMotif(other)
{
    setMotifType(MOTIF_TYPE_EXPLCIT_TILE);
}

void TileMotif::buildMotifMaps()
{
    Q_ASSERT(tile);
    motifMap = std::make_shared<Map>("Tile motif map");
    inferTile(tile);
    completeMap(motifMap);
}

void TileMotif::inferTile(TilePtr tile)
{
    qDebug() << "TileMotif::inferMotif";

    EdgePoly epoly = tile->getEdgePoly();

    for (auto edge : epoly)
    {
        // this makes new eges and vertices since they can get altered in the map
        VertexPtr v1 = motifMap->insertVertex(edge->v1->pt);
        VertexPtr v2 = motifMap->insertVertex(edge->v2->pt);
        EdgePtr newEdge = motifMap->insertEdge(v1,v2);
        if (edge->isCurve())
        {
            bool convex = edge->isConvex();
            QPointF ac  = edge->getArcCenter();
            newEdge->setArcCenter(ac,convex,(edge->getType() == EDGETYPE_CHORD));
        }
    }

    qDebug().noquote() << motifMap->namedSummary();
}
