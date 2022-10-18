#include <QDebug>
#include "motifs/explicit_motif.h"
#include "geometry/map.h"
#include "geometry/loose.h"
#include "geometry/transform.h"

typedef std::shared_ptr<class ExplicitMotif>   ExplicitPtr;

ExplicitMotif::ExplicitMotif(MapPtr map, eMotifType motifType, int sides)
    : InferenceEngine()
{
    explicitMap = map;
    setMotifType(motifType);
    init(sides);
}

ExplicitMotif::ExplicitMotif(const Motif &motif, MapPtr map, eMotifType motifType, int sides)
    : InferenceEngine(motif)
{
    explicitMap = map;
    setMotifType(motifType);
    init(sides);
}

ExplicitMotif::ExplicitMotif(const Motif & motif, eMotifType motifType, int sides)
    : InferenceEngine(motif)
{
    setMotifType(motifType);
    init(sides);
}

ExplicitMotif::ExplicitMotif(const ExplicitMotif & other) : InferenceEngine(other)
{
    skip        = other.skip;
    d           = other.d;
    s           = other.s;
    q           = other.q;
    r_flexPt    = other.r_flexPt;
    progressive = other.progressive;
}

ExplicitMotif::~ExplicitMotif()
{}

void ExplicitMotif::init(int sides)
{
    this->n = sides;  // was sides = 10; // girih + intersect + rosette + star
    skip  = 3.0;         // girih
    d     = 2.0;         // hourglass + intersect + star
    s     = 1;           // hourglass + intersect + star
    q     = 0.0;         // rosette
    r_flexPt    = 0.5;   // rosette
    progressive = false; // intersect    
}


bool ExplicitMotif::equals(const MotifPtr other)
{
    ExplicitPtr otherp = std::dynamic_pointer_cast<ExplicitMotif>(other);
    if (!otherp)
        return  false;

    if (getMotifType() != other->getMotifType())
        return false;

    if (n != otherp->n)
        return false;

    if (d != otherp->d)
        return  false;

    if (s != otherp->s)
        return false;

    if (q != otherp->q)
        return  false;

    if (r_flexPt != otherp->r_flexPt)
        return  false;

    if (progressive != otherp->progressive)
        return  false;

    if (!Motif::equals(other))
        return false;

     return true;
}

void ExplicitMotif::setExplicitMap(MapPtr map)
{
    explicitMap = map;
    motifMap.reset();
}


MapPtr ExplicitMotif::getMap()
{
    if (!motifMap)
    {
        buildMaps();
    }
    return motifMap;
}

void ExplicitMotif::buildMaps()
{
    if (explicitMap)
    {
        if (!Loose::equals(motifScale,1.0) || !Loose::zero(motifRotate))
        {
            QTransform t = QTransform::fromScale(motifScale,motifScale) * Transform::rotate(motifRotate);
            motifMap = explicitMap->copy();
            explicitMap->transformMap(t);
        }
        else
        {
            motifMap = explicitMap;
        }
    }
    else
    {
        qWarning("No Explicit Map");
    }
}

void ExplicitMotif::resetMaps()
{
    motifMap.reset();
}

void ExplicitMotif::dump()
{
    qDebug() << "Infer skip:" << skip << "d:" << d << "s:" << s
             << "q:" << q  << "r_flexPt" << r_flexPt << ((progressive) ? "progressive" : "");
}

