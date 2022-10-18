#ifndef INFERENCE_ENGINE_H
#define INFERENCE_ENGINE_H

#include <QMap>
#include <QTransform>
#include <QDebug>

typedef std::shared_ptr<class Prototype>        PrototypePtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Tile>             TilePtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class MidPoints>        MidsPtr;
typedef std::shared_ptr<class AdjacenycInfo>    AdjacencyPtr;
typedef std::shared_ptr<class IntersectionInfo> IntersectionPtr;
typedef std::shared_ptr<class EdgesLengthInfo>  EdgesLenPtr;
typedef std::shared_ptr<class Contact>          ContactPtr;

#include "motif.h"

struct Branch
{
    QPointF tipPoint;
    QPointF qePoint;
    QPointF fPoint;
};

class InferenceEngine : public Motif
{
public:
    InferenceEngine();
    InferenceEngine(const Motif & fig);

    void setupInfer(PrototypePtr proto);
    MapPtr newExplicitMap();

    void infer(                     TilePtr tile);                                       // "Normal" magic inferring
    void inferMotif(                TilePtr tile);                                       // make a motif from the tiling itself
    void inferIntersectProgressive( TilePtr tile, int starSides, qreal starSkip, int s); // Progressive intersect inferring
    void inferIntersect(            TilePtr tile, int starSides, qreal starSkip, int s); // Intersect inferring
    void inferGirih(                TilePtr tile, int starSides, qreal starSkip);        // Girih inferring
    void inferRosette(              TilePtr tile, qreal q, int s, qreal r);              // Rosette inferring
    void inferStar(                 TilePtr tile, qreal d, int s);                       // Star inferring
    void inferHourglass(            TilePtr tile, qreal d, int s);                       // Hourglass inferring

private:
    int                     findPrimaryTile(TilePtr tile);
    AdjacencyPtr            getAdjacency(QPointF main_point, int main_idx );
    QVector<AdjacencyPtr>   getAdjacencies(MidsPtr pp, int main_idx );

    QVector<ContactPtr>     buildContacts(MidsPtr pp, const QVector<AdjacencyPtr> &adjs);

    static QPointF          getArc(qreal frac, const QPolygonF & pts);

    static QPolygonF        buildStarBranchPoints(qreal d, int s, qreal side_frac, qreal sign, QPolygonF mid_points);
    static MapPtr           buildStarHalfBranch ( qreal d, int s, qreal side_frac, qreal sign, QPolygonF mid_points );

    static QPointF          buildGirihHalfBranch(int side, bool leftBranch, qreal requiredRotation, QPolygonF points, QPolygonF midPoints );
    static IntersectionInfo findClosestIntersection(int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation, QPolygonF points, QPolygonF midPoints );
    static MapPtr           buildGirihBranch(int side, qreal requiredRotation, QPolygonF points, QPolygonF midPoints );

    static int              getIntersectionRank(int side, bool isLeft, QList<IntersectionPtr> infos);
    static QList<EdgesLenPtr> buildIntersectEdgesLengthInfos(int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation, QPolygonF points, QPolygonF midPoints);
    static QList<IntersectionPtr> buildIntersectionInfos(int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation, QPolygonF points, QPolygonF midPoints);

    Branch           buildRosetteBranchPoints( int side, qreal q, int s, qreal r, qreal side_frac, qreal sign, QPolygonF mid_points, QPolygonF corner_points);
    QPolygonF        buildRosetteIntersections(int side, qreal q, int s, qreal r, qreal side_frac, qreal sign, QPolygonF mid_points, QPolygonF corner_points, Branch &branch);
    MapPtr           buildRosetteHalfBranch(   int side, qreal q, int s, qreal r, qreal side_frac, qreal sign, QPolygonF mid_points, QPolygonF corner_points);

    static bool      isColinear( QPointF p, QPointF q, QPointF a );
    static int       lexCompareDistances(int kind1, qreal dist1, int kind2, qreal dist2 );

protected:
    MapPtr                  explicitMap;

private:
    TilingPtr               tiling;
    QMap<TilePtr,MapPtr>    tileMaps;
    QVector<MidsPtr>        allMotifMids;
    bool                    debug;
    int                     debugSide;
};


// Transformed mid-points of a tile.
class MidPoints
{
public:
    MidPoints(TilePtr tile, QTransform T, QVector<QPointF> &mids );

    QPolygonF   getMidPoints() { return midPoints; }

    TilePtr          tile;
    QTransform       T;

    // The transformed midpoints of the tile's edges.
    // Since the tilings are edge to edge, a tile edge can
    // be uniquely identified by its midpoint.  So we can
    // compare two edges by just comparing their midpoints
    // instead of comparing the endpoints pairwise.
private:
    QPolygonF midPoints;        // these are placed
};

// Information about what tile and edge on that tile is adjacent
// to a given edge on a given tile.
class AdjacenycInfo
{
public:
    AdjacenycInfo(TilePtr tile, QTransform T, qreal tolerance );

    TilePtr     tile;
    QTransform  T;
    qreal       tolerance;

};

// Information about intersection of a side left or right in-going edge
// with a given side right or left edge.
class IntersectionInfo
{
public:
    IntersectionInfo();
    IntersectionInfo(int side, int otherSide, bool otherIsLeft, qreal dist2, QPointF i );

    bool equals(    IntersectionInfo & other );
    int  compareTo( IntersectionInfo & other );

    int     side;           // Which side of the tile this describes.
    int     otherSide;      // Which other side it meets.
    bool    otherIsLeft;    // True if the other side is the left edge.
    qreal   dist2;          // The square of the distance (square to avoid a square root op.)
    QPointF intersection;   // The intersection point.
};

class EdgesLengthInfo
{
public:
    EdgesLengthInfo(int side1, bool isLeft1, int side2, bool isLeft2, int ic, qreal dist2, QPointF i);

    bool equals  (  EdgesLengthInfo & other );
    int  compareTo( EdgesLengthInfo & other );

    static bool lessThan(const EdgesLengthInfo *&e1, const EdgesLengthInfo *&e2);

    int     side1;          // Which side of the tile this describes.
    bool    isLeft1;        // True if first side is left edge.
    int     side2;          // Which side of the tile this describes.
    bool    isLeft2;        // True if second side is left edge.
    int     intersection_count;
    qreal   dist2;          // The square of the distance (square to avoid a square root op.)
    QPointF intersection;   // The intersection point.
};

// The information about one point of contact on the boundary of the region being inferred.
class Contact
{
public:

    enum eColinear
    {
        COLINEAR_NONE   = 0,
        COLINEAR_MASTER = 1,
        COLINEAR_SLAVE  = 2
    };

    Contact(QPointF position, QPointF other);

    void dump() { qDebug() << "position:" << position << "other:" << other << "end" << end << "isect" << isect << "colinear"  << colinear << "taken" << taken; }

    QPointF     position;
    QPointF		other;

    QPointF 	end;
    QPointF 	isect;
    ContactPtr  isect_contact;
    eColinear   colinear;
    bool        taken;
};

#endif // INFERENCE_ENGINE_H
