#include <QDebug>

#include "mosaic/design_element.h"
#include "motifs/explicit_motif.h"
#include "motifs/motif.h"
#include "motifs/rosette.h"
#include "geometry/map.h"
#include "misc/utilities.h"
#include "tile/tile.h"

using std::make_shared;

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
    createMotif();
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

void DesignElement::createMotif()
{
    if (tile->isRegular())
    {
        motif = make_shared<Rosette>(tile->numPoints(), 0.0, 3, 0);
    }
    else
    {
        EdgePoly & ep = tile->getEdgePoly();
        MapPtr     fm = make_shared<Map>("tile map",ep);
        motif         = make_shared<ExplicitMotif>(fm,MOTIF_TYPE_EXPLICIT_TILE,tile->numSides());
    }

    motif->setMotifScale(tile->getScale());
    motif->setMotifRotate(tile->getRotation());

    ExtendedBoundary & eb = motif->getRWExtendedBoundary();
    eb.sides = tile->numPoints();
    eb.scale = tile->getScale();
    QPolygonF poly = tile->getPolygon();
    eb.set(poly);
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
        createMotif();
    }
}

bool DesignElement::validMotif()
{
   if (motif->getMotifType() == MOTIF_TYPE_EXPLICIT_TILE)
   {
       return true;     // always valid
   }
   if (tile->isRegular())
   {
       if (motif->isRadial())
            return true;
        else
           return false;
   }
   else
   {
        if (motif->isExplicit())
        {
            return true;
        }
        else
        {
            return false;
        }
   }
}

QString DesignElement::toString()
{
    return QString("this=%1 tile=%2 motif=%3").arg(Utils::addr(this)).arg(Utils::addr(tile.get())).arg(Utils::addr(motif.get()));
}

void DesignElement::describe()
{
    if (motif)
        qDebug().noquote()  << "Motif:" << motif.get()  << motif->getMotifDesc() << motif->getMap()->namedSummary();
    else
        qDebug().noquote()  << "Motif: 0";
    if (tile)
        qDebug().noquote()  << "Tile:" << tile.get()  << "sides:" << tile->numSides() << ((tile->isRegular()) ? "regular" : "irregular");
    else
        qDebug().noquote()  << "Tile: 0";
}
