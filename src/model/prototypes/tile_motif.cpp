#include <QDebug>

#include "model/motifs/adaptive_motif.h"
#include "model/motifs/adaptive_rosette.h"
#include "model/motifs/adaptive_star.h"
#include "model/motifs/adaptive_tile.h"
#include "model/motifs/motif.h"
#include "model/motifs/rosette.h"
#include "model/motifs/rosette2.h"
#include "model/motifs/star.h"
#include "model/motifs/star2.h"
#include "model/prototypes/tile_motif.h"
#include "model/tilings/tile.h"
#include "sys/geometry/map.h"       // required
#include "sys/qt/utilities.h"

using std::make_shared;
using std::shared_ptr;

typedef shared_ptr<AdaptiveMotif> AdaptiveMotifPtr;

int TileMotif::refs = 0;

////////////////////////////////////////////////////////////////////////////
//
// TileMotif.java
//
// A TileMotif is the core of the process of building a finished design.
// It's a Tile together with a Motif.  The Tile comes from the
// tile library and will be used to determine where to place copies of the
// Motif, which is designed by the user.

TileMotif::TileMotif(const TilingPtr & tiling,  const TilePtr & tile, const MotifPtr & motif)
{
    this->tiling = tiling;
    this->tile   = tile;
    this->motif  = motif;
    motif->setTile(tile);
    refs++;
}

TileMotif::TileMotif(const TilingPtr & tiling, const TilePtr & tile)
{
    this->tiling = tiling;
    this->tile   = tile;
    createMotifFromTile();
    refs++;
}

TileMotif::TileMotif(const TilingPtr & tiling, const TileMotifPtr & dep)
{
    this->tiling = tiling;
    tile         = dep->tile;
    motif        = dep->motif;
    refs++;
}

TileMotif::TileMotif(const TilingPtr & tiling, const TileMotif & other)
{
    this->tiling = tiling;
    tile         = other.tile;
    motif        = other.motif;
    refs++;
}

TileMotif::TileMotif(const TilingPtr & tiling)
{
    this->tiling = tiling;
    refs++;
}

TileMotif::~TileMotif()
{
    refs--;
}

void TileMotif::createMotifFromTile()
{
    if (tile->isRegular())
    {
        // make a Rosette motif
        motif = make_shared<Rosette>(tile->numPoints(), 0.0, 3);
        motif->setTile(tile);
    }
    else
    {
        // make a AdaptiveTile
        auto tmotif   = make_shared<AdaptiveTile>();
        tmotif->setN(tile->numEdges());
        tmotif->setTile(tile);
        motif = tmotif;
    }

    motif->build();
}

void TileMotif::recreateMotifFromChangedTile()
{
    Q_ASSERT(tile);
    Q_ASSERT(motif);

    int newsides = tile->numEdges();
    if (motif->getN() != newsides)
    {
        motif->setN(newsides);  // this should do the trick
    }

    motif->setTile(tile);
    motif->resetMotifMap();
    motif->build();
}

void TileMotif::recreateMotifWhenRgularityChanged()
{
    // the tile has possibly had its regularity flipped
    // the tile is now correct, but the motoif may need adjusting
    if (tile->isRegular())
    {
        AdaptiveMotifPtr oldMotif = std::dynamic_pointer_cast<AdaptiveMotif>(motif);
        if (!oldMotif)
        {
            Q_ASSERT(std::dynamic_pointer_cast<RadialMotif>(motif));
            return;     // the motif is already radial
        }

        // Making a radial motif from an explicit
        switch (motif->getMotifType())
        {
        case Motif::Type::AdaptiveRosette:
        {
            auto rose = make_shared<Rosette>(*oldMotif.get());
            rose->setQ(oldMotif->q);
            rose->setS(oldMotif->s);
            motif = rose;
        }   break;

        case Motif::Type::AdaptiveStar:
        {
            auto star = make_shared<Star>(*oldMotif.get());
            star->setD(oldMotif->d);
            star->setS(oldMotif->s);
            motif = star;
        }   break;

        case Motif::Type::Inferred:
        case Motif::Type::Hourglass:
        case Motif::Type::Intersect:
        case Motif::Type::Girih:
        {
            auto rose = make_shared<Rosette>(*oldMotif.get());
            rose->setQ(oldMotif->q);
            rose->setS(oldMotif->s);
            motif = rose;
        }   break;
            
        case Motif::Type::MapMotif:
            motif = make_shared<Rosette>(oldMotif->getN(), 0.0, 3);
            break;
            
        case Motif::Type::NoMapMotif:
            motif = make_shared<Rosette>(tile->numPoints(), 0.0, 3);
            break;

        default:
            qWarning().noquote() << "Unexpected motif type" << sMotifType[(int)motif->getMotifType()];
            break;
        }
    }
    else
    {
        // making an explicit motif from a radial
        switch (motif->getMotifType())
        {
        case Motif::Type::Rosette:
        {
            auto oldMotif = std::dynamic_pointer_cast<Rosette>(motif);
            Q_ASSERT(oldMotif);
            auto irose = make_shared<AdaptiveRosette>(*oldMotif.get());
            Q_ASSERT(irose);
            irose->setTile(tile);
            irose->init(oldMotif->getQ(), 0.5, oldMotif->getS(), 0.0);
            motif = irose;
        }   break;

        case Motif::Type::Rosette2:
        {
            auto oldMotif = std::dynamic_pointer_cast<Rosette2>(motif);
            Q_ASSERT(oldMotif);
            auto irose = make_shared<AdaptiveRosette>(*oldMotif.get());
            Q_ASSERT(irose);
            irose->setTile(tile);
            // V2 knee: q = height, r = offset along edge, as fractions of edge length; same k
            qreal edgeLen = Tile(oldMotif->getN()).edgeLen();
            irose->init(oldMotif->getKneeX() / edgeLen, oldMotif->getKneeY() / 2.0, oldMotif->getS(), oldMotif->getK());
            irose->setVersion(2);
            motif = irose;
        }   break;

        case Motif::Type::Star:
        {
            auto oldMotif = std::dynamic_pointer_cast<Star>(motif);
            Q_ASSERT(oldMotif);
            auto istar = make_shared<AdaptiveStar>(*oldMotif.get());
            Q_ASSERT(istar);
            istar->setTile(tile);
            istar->init(oldMotif->getD(), oldMotif->getS());
            motif = istar;
        }   break;

        case Motif::Type::Star2:
        {
            auto oldMotif = std::dynamic_pointer_cast<Star2>(motif);
            Q_ASSERT(oldMotif);
            auto istar = make_shared<AdaptiveStar>(*oldMotif.get());
            Q_ASSERT(istar);
            istar->setTile(tile);
            istar->init(0, oldMotif->getS());
            motif = istar;
        }   break;

        default:
            qWarning().noquote() << "Unexpected motif type" << sMotifType[(int)motif->getMotifType()];
            break;
        }
    }

    for (auto & extender : motif->getExtenders())
    {
        auto & boundary = extender->getRWExtendedBoundary();
        boundary.setRadial(tile->isRegular());
    }

    motif->setTile(tile);
    motif->build();
#if 0
    for (ExtenderPtr extender : motif->getExtenders())
    {
        extender->buildExtendedBoundary();
        extender->extend();
    }
#endif
}

void TileMotif::setMotif(const MotifPtr & motif)
{
    //qDebug() << "oldfig =" << figure.get() << "newfig =" << fig.get();
    this->motif = motif;
}

void TileMotif::exactReplaceTile(const TilePtr & tile)
{
    motif->setTile(tile);
    motif->resetMotifMap();
    this->tile = tile;
}

void TileMotif::replaceModifiedTile(const TilePtr & tile)
{
    // always rebuild - never assume the old motif is still valid as-is
    exactReplaceTile(tile);

    if (!validMotifRegularity())
    {
        // the tile's regular/irregular family no longer matches the motif's
        // (radial vs adaptive) - convert to the right family (Rosette/Rosette2/
        // Star/Star2 <-> AdaptiveRosette/AdaptiveStar), preserving Q/S where
        // the conversion supports it
        recreateMotifWhenRgularityChanged();
    }
    else
    {
        // same family still fits - keep the same motif type (e.g. Rosette2
        // stays Rosette2) and just adapt to the new side count/shape
        recreateMotifFromChangedTile();
    }
}

bool TileMotif::validMotifRegularity()
{
   if (motif->getMotifType() == Motif::Type::ExplicitTile)
   {
       return true;     // always valid
   }
   if (tile->isRegular())
   {
       return motif->isRadial() ? true : false;
   }
   else
   {
       // irregular
       return motif->isIrregular() ? true : false;
   }
}

QRectF TileMotif::boundingRect() const
{
    // Get the bounding box of all the tile's vertices and all the
    // motif's map vertices. In other words, the region needed to show the TileMotif.

    if (!motif)
        return QRectF();

    double xmin = 999.0;
    double xmax = -999.0;
    double ymin = 999.0;
    double ymax = -999.0;

    QPolygonF pts = tile->getPoints();
    for (QPointF p : std::as_const(pts))
    {
        qreal x = p.x();
        qreal y = p.y();

        xmin = qMin( xmin, x );
        xmax = qMax( xmax, x );
        ymin = qMin( ymin, y );
        ymax = qMax( ymax, y );
    }
    auto map = motif->getValidMotifMap();
    if (map)
    {
        auto & vertices = map->getVertices();
        for (auto & v : vertices)
        {
            auto p = v->pt;
            qreal x = p.x();
            qreal y = p.y();

            xmin = qMin( xmin, x );
            xmax = qMax( xmax, x );
            ymin = qMin( ymin, y );
            ymax = qMax( ymax, y );
        }
    }

    return QRectF(xmin, ymin, xmax-xmin, ymax-ymin);
}

QString TileMotif::toString()
{
    return QString("this=%1 tile=%2 motif=%3").arg(Utils::addr(this)).arg(Utils::addr(tile.get())).arg(Utils::addr(motif.get()));
}

void TileMotif::describe()
{
    if (motif)
    {
        auto map = motif->getMotifMap();
        QString mapstring = (map) ? map->summary() : "No map";
        qDebug().noquote()  << "Motif:" << motif.get()  << motif->getMotifDesc() << mapstring;
    }
    else
        qDebug().noquote()  << "Motif: 0";
    if (tile)
        qDebug().noquote()  << "Tile:" << tile.get()  << "sides:" << tile->numEdges() << ((tile->isRegular()) ? "regular" : "irregular");
    else
        qDebug().noquote()  << "Tile: 0";
}
