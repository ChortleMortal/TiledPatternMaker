#include <QDebug>

#include "geometry/map.h"
#include "misc/utilities.h"
#include "mosaic/design_element.h"
#include "motifs/irregular_motif.h"
#include "motifs/irregular_rosette.h"
#include "motifs/irregular_star.h"
#include "motifs/tile_motif.h"
#include "motifs/motif.h"
#include "motifs/rosette.h"
#include "motifs/star.h"
#include "tile/tile.h"

using std::make_shared;
using std::shared_ptr;

typedef shared_ptr<IrregularMotif> ExplicitPtr;

int DesignElement::refs = 0;

////////////////////////////////////////////////////////////////////////////
//
// DesignElement.java
//
// A DesignElement is the core of the process of building a finished design.
// It's a Tile together with a Motif.  The Tile comes from the
// tile library and will be used to determine where to place copies of the
// Motif, which is designed by the user.

DesignElement::DesignElement(const TilePtr & tile, const MotifPtr & motif)
{
    this->tile   = tile;
    this->motif  = motif;
    refs++;
}

DesignElement::DesignElement(const TilePtr & tile)
{
    this->tile = tile;
    createMotifFromTile();
    refs++;
}

DesignElement::DesignElement(const DesignElementPtr & dep)
{
    tile   = dep->tile;
    motif  = dep->motif;
    refs++;
}

DesignElement::DesignElement(const DesignElement & other)
{
    tile   = other.tile;
    motif  = other.motif;
    refs++;
}


DesignElement::DesignElement()
{
    refs++;
}

DesignElement::~DesignElement()
{
    refs--;
}

TilePtr DesignElement::getTile() const
{
    return tile;
}

MotifPtr DesignElement::getMotif() const
{
    return motif;
}

void DesignElement::createMotifFromTile()
{
    if (tile->isRegular())
    {
        motif = make_shared<Rosette>(tile->numPoints(), 0.0, 3, 0);
    }
    else
    {
        EdgePoly & ep = tile->getEdgePoly();
        MapPtr     fm = make_shared<Map>("tile map",ep);
        auto tmotif   = make_shared<TileMotif>();
        tmotif->setN(tile->numSides());
        tmotif->setup(tile);
        tmotif->buildMotifMaps();
        motif = tmotif;
    }

    motif->setMotifScale(tile->getScale());
    motif->setMotifRotate(tile->getRotation());

    ExtendedBoundary & eb = motif->getRWExtendedBoundary();
    eb.sides = tile->numPoints();
    eb.scale = tile->getScale();
    QPolygonF poly = tile->getPolygon();
    eb.set(poly);
}

void DesignElement::recreateMotifFromChangedTile()
{
    Q_ASSERT(tile);
    Q_ASSERT(motif);

    motif->resetMotifMaps();

    Q_ASSERT(validMotifRegularity());

    int newsides = tile->numSides();
    if (motif->getN() != newsides)
    {
        motif->setN(newsides);  // this should do the trick

        ExtendedBoundary & extended = motif->getRWExtendedBoundary();
        extended.sides = newsides;
        if (motif->isRadial())
        {
            extended.buildRadial();
        }
        else
        {
            extended.buildExplicit(tile);
        }
    }
}

void DesignElement::recreateMotifUsingTile()
{
    // the tile has possibly had its regularity flipped
    // the tile is now correct, but the motoif may need adjusting
    if (tile->isRegular())
    {
        ExplicitPtr oldMotif = std::dynamic_pointer_cast<IrregularMotif>(motif);
        if (!oldMotif)
        {
            Q_ASSERT(std::dynamic_pointer_cast<RadialMotif>(motif));
            return;     // the motif is already radial
        }

        // Making a radial motif from an explicit
        switch (motif->getMotifType())
        {
        case MOTIF_TYPE_IRREGULAR_ROSETTE:
            motif = make_shared<Rosette>(oldMotif->getN(), oldMotif->q, oldMotif->s, 0);
            break;

        case MOTIF_TYPE_IRREGULAR_STAR:
            motif = make_shared<Star>(oldMotif->getN(), oldMotif->d, oldMotif->s);
            break;

        case MOTIF_TYPE_INFERRED:
        case MOTIF_TYPE_HOURGLASS:
        case MOTIF_TYPE_INTERSECT:
        case MOTIF_TYPE_GIRIH:
            motif = make_shared<Rosette>(oldMotif->getN(), oldMotif->q, oldMotif->s, 0);
            break;
            
        case MOTIF_TYPE_EXPLICIT_MAP:
            motif = make_shared<Rosette>(oldMotif->getN(), 0.0, 3, 0);
            break;
            
        case MOTIF_TYPE_IRREGULAR_NO_MAP:
            motif = make_shared<Rosette>(tile->numPoints(), 0.0, 3, 0);
            break;

        default:
            qWarning().noquote() << "Unexpected motif type" << sMotifType[motif->getMotifType()];
            break;
        }
    }
    else
    {
        // making an explicit motif from a rdial
        switch (motif->getMotifType())
        {
        case MOTIF_TYPE_ROSETTE:
        case MOTIF_TYPE_EXTENDED_ROSETTE:
        case MOTIF_TYPE_CONNECT_ROSETTE:
        {
            auto oldMotif = std::dynamic_pointer_cast<Rosette>(motif);
            Q_ASSERT(oldMotif);
            auto irose = make_shared<IrregularRosette>(*motif.get());
            irose->init(oldMotif->getQ(), oldMotif->getK(), oldMotif->getS());
            irose->setup(tile);
            irose->buildMotifMaps();
            //rose->setMotifBoundary(tile->getPolygon());
            //ExtendedBoundary & eb = rose->getRWExtendedBoundary();
            //eb.set(tile->getPolygon());
            motif = irose;
        }
            break;

        case MOTIF_TYPE_STAR:
        case MOTIF_TYPE_CONNECT_STAR:
        case MOTIF_TYPE_EXTENDED_STAR:
        {
            auto oldMotif = std::dynamic_pointer_cast<Star>(motif);
            Q_ASSERT(oldMotif);
            auto istar = make_shared<IrregularStar>(*motif.get());
            istar->init(oldMotif->getD(), oldMotif->getS());
            istar->setup(tile);
            istar->buildMotifMaps();
            //star->setMotifBoundary(tile->getPolygon());
            //ExtendedBoundary & eb = star->getRWExtendedBoundary();
            //eb.set(tile->getPolygon());
            motif = istar;
        }
            break;

        default:
            qWarning().noquote() << "Unexpected motif type" << sMotifType[motif->getMotifType()];
            break;
        }
    }

#if 0 // Methinks not needed any more becaude of export data
    motif->setMotifScale(tile->getScale());
    motif->setMotifRotate(tile->getRotation());

    ExtendedBoundary & eb = motif->getRWExtendedBoundary();
    eb.sides = tile->numPoints();
    eb.scale = tile->getScale();
    QPolygonF poly = tile->getPolygon();
    eb.set(poly);
#endif
}

void DesignElement::setMotif(const MotifPtr & motif)
{
    //qDebug() << "oldfig =" << figure.get() << "newfig =" << fig.get();
    this->motif = motif;
}

void DesignElement::replaceTile(const TilePtr & tile)
{
    if (tile->isSimilar(tile))
    {
        this->tile = tile;
    }
    else
    {
        this->tile = tile;
        recreateMotifFromChangedTile();
    }
}

bool DesignElement::validMotifRegularity()
{
   if (motif->getMotifType() == MOTIF_TYPE_EXPLCIT_TILE)
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
        return motif->isExplicit() ? true : false;
   }
}

QString DesignElement::toString()
{
    return QString("this=%1 tile=%2 motif=%3").arg(Utils::addr(this)).arg(Utils::addr(tile.get())).arg(Utils::addr(motif.get()));
}

void DesignElement::describe()
{

    if (motif)
    {
        auto map = motif->getMotifMap();
        QString mapstring = (map) ? map->namedSummary() : "No map";
        qDebug().noquote()  << "Motif:" << motif.get()  << motif->getMotifDesc() << mapstring;
    }
    else
        qDebug().noquote()  << "Motif: 0";
    if (tile)
        qDebug().noquote()  << "Tile:" << tile.get()  << "sides:" << tile->numSides() << ((tile->isRegular()) ? "regular" : "irregular");
    else
        qDebug().noquote()  << "Tile: 0";
}
