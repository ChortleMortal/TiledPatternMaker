////////////////////////////////////////////////////////////////////////////
//
// Rosette2
//
// Rosette2 is an alternative to Kaplan's Rosette, which relaxes
// Kaplans constraint on Knee position which is only a partial truth
// Also adds more vraiations

#include "model/motifs/rosette2.h"
#include "model/motifs/rosette.h"
#include "model/tilings/tile.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "sys/sys/debugflags.h"
#include <QDebug>

const QString sTipType[] =
{
    "Repeat",
    "Outward",
    "Inward",
    "Extended Inward",
    "Outward Point",
    "Outward to Boundary",
    "Outward Point to Boundary"
};

typedef std::shared_ptr<Rosette2> Rosette2Ptr;

using std::make_shared;

Rosette2::Rosette2(int nsides, qreal kneeX, qreal kneeY, int ss, qreal kk, bool c) : RadialMotif(nsides)
{
    this->kneeX = kneeX;
    this->kneeY = kneeY;
    s           = ss;
    k           = kk;
    constrain   = c;
    initTipTypes();
    setMotifType(Type::Rosette2);
}

Rosette2::Rosette2(const Rosette2 & other) : RadialMotif(other)
{
    kneeX       = other.kneeX;
    kneeY       = other.kneeY;
    s           = other.s;
    k           = other.k;
    constrain   = other.constrain;
    initTipTypes();
    setMotifType(Type::Rosette2);
}

Rosette2::Rosette2(const Motif & motif) : RadialMotif(motif)
{
    kneeX       = 0.35;
    kneeY       = 0.25;
    s           = 2;
    k           = 0.0;
    constrain   = false;
    initTipTypes();
    setMotifType(Type::Rosette2);
}

// same knee point and knee ray as the Rosette (s, inscribe, onPoint copied by RadialMotif)
Rosette2::Rosette2(const Rosette & rosette) : RadialMotif(rosette)
{
    QLineF  keyLine = rosette.getKeyLine();
    QPointF knee    = keyLine.p1();

    Tile atile(getN());
    xRange = 1.0;
    yRange = atile.edgeLen() / 2.0;

    kneeX       = (1.0 - knee.x()) / xRange;
    kneeY       = knee.y() / yRange;

    QPointF edgePt = Geo::perpPt(atile.getEdge(getN()-1), knee);
    QLineF  kneeRay(edgePt, knee);
    k           = kneeRay.angleTo(keyLine);
    if (k > 180.0)
        k -= 360.0;

    constrain   = false;
    initTipTypes();
    setMotifType(Type::Rosette2);
}

TipType Rosette2::getTipType(int tip)
{
    return tipTypes[tip % tipTypes.size()];
}

bool Rosette2::hasTipsToMove()
{
    for (TipType tt : std::as_const(tipTypes))
    {
        if (isBoundaryTip(tt))
            return true;
    }
    return false;
}

// fullRays are replicated from unitRays in LR pairs, so the pair index maps back to the unit tip type
bool Rosette2::moveTipToBoundary(int rayIndex)
{
    if (unitTipTypes.isEmpty())
        return false;

    return isBoundaryTip(unitTipTypes[(rayIndex / 2) % unitTipTypes.size()]);
}

bool Rosette2::equals(const MotifPtr other)
{
    Rosette2Ptr otherp = std::dynamic_pointer_cast<Rosette2>(other);
    if (!otherp)
        return  false;

    if (kneeX != otherp->kneeX)
        return  false;

    if (kneeY != otherp->kneeY)
        return  false;

    if (k != otherp->k)
        return false;

    if (s != otherp->s)
        return false;

    if (tipTypes.size() != otherp->tipTypes.size())
        return false;

    for (int i = 0; i < tipTypes.size(); i++)
    {
        if (tipTypes[i] != otherp->tipTypes[i])
            return false;
    }

    if (!Motif::equals(other))
        return false;

     return true;
}

// kneeX is the %age of the distance to the center, so is always scaled by 1.0
// kneey is the %age of half the edge width, so scaaling is depends on n (sides)
void Rosette2::buildUnitRays()
{
    setMotifDebug(0x00);

    qDebug().noquote() << "Rosette2::buildUnit n:" << getN() << "x:" << kneeX << "y:" << kneeY << "s:" << s << "k" << k
                       << "Tr:" << Transform::info(getRadialTransform(1)) << "rot" << getMotifRotate() << "scale" << getMotifScale();

    Tile atile(getN());     // not scaled or rotated
    xRange = 1.0;
    yRange = atile.edgeLen() /2.0;

    qreal x = xRange * kneeX;
    qreal y = yRange * kneeY;

    kneePt = QPointF(1.0 - x, y);

    if (constrain)
    {
        calcConstraintLine();
        kneePt = Geo::getClosestPoint(constraint,kneePt);
    }

    QPointF kneePtMirror(kneePt.x(),-kneePt.y());

    QPointF edgePt = Geo::perpPt(atile.getEdge(getN()-1),kneePt);

    QLineF  ray(edgePt,kneePt);         // ray from tile to knee point
    ray.setLength(2.0);                 // extend ray
    ray.setP1(kneePt);                  // the real ray
    ray.setAngle(ray.angle() +k );      // factors in knee angle (k)

    QLineF rayMirror(kneePtMirror, QPointF(ray.p2().x(), -ray.p2().y()));

    QPointF outerTip(1.0, 0.0);         // standard tip

    if (getMotifDebug() & 0x01)
    {
        Sys::debugMapCreate->insertDebugMark(kneePt,"kneePt",Qt::red);
        Sys::debugMapCreate->insertDebugMark(kneePtMirror,"kneePtMirror",Qt::red);
        Sys::debugMapCreate->insertDebugLine(ray,Qt::red);
        Sys::debugMapCreate->insertDebugLine(rayMirror,Qt::green);
    }

    RadialRay epoints;
    epoints.addTip(kneePt);

    for (int idx = 0; idx <= getN(); idx++)
    {
        rayMirror = getRadialTransform(1).map(rayMirror);

        QPointF isect;
        if (ray.intersects(rayMirror,&isect) == QLineF::BoundedIntersection)
        {
            epoints.addTail(isect);
        }
    }

    epoints.sort(kneePt);

    if (epoints.size() > (s+1))
    {
        epoints.resize(s+1);
    }

    if (getMotifDebug() & 0x02)
    {
        epoints.debug(1);
    }

    // in the clockwise direction epoints are the right hand - and the mirror is the left hand
    QLineF  kneeLine(kneePt,kneePtMirror);
    QPointF innerTip = Geo::reflectPoint(outerTip,kneeLine);    // outer tip reflected over kneeLine

    // build the real unit rays
    unitTipTypes.clear();
    for (int i=0; i < tipTypes.size(); i++)
    {
        TipType tt = getTipType(i);
        if (tt != TipType::REPEAT)
        {
            unitTipTypes.push_back(tt);     // each case below adds one LR pair
        }

        switch (tt)
        {
        case TipType::OUTER:
        case TipType::OUTER_TO_BOUNDARY:
            buildUnitRays(outerTip,epoints,i);
            break;

        case TipType::INNER:
            buildUnitRays(innerTip,epoints,i);
            break;

        case TipType::OUTER_POINT:
        case TipType::OUTER_POINT_TO_BOUNDARY:
        {
            // point one-quarter along the knee line from the knee
            RadialRay pointed;
            pointed.set(epoints);
            pointed.addTip(kneePt + 0.25 * (kneePtMirror - kneePt));
            buildUnitRays(outerTip,pointed,i);
        }   break;

        case TipType::FLIPPED:
        {
            int index = buildUnitRays(innerTip,epoints,i);

            // find the two isects
            // Note motif may have different number of sides than containing tile
            QPolygonF  poly = getTile()->getBasePoints();   // not scaled and rotated

            RadialRay & rayL = unitRays[index];
            QLineF lineL(rayL.pt(2),rayL.pt(1));
            QPointF isectL;
            Geo::intersectPoly(lineL,poly,isectL);

            if (Sys::flags->flagged(SHOW_UNIT_RAYS))
            {
                Sys::debugMapMotif->insertDebugMark(rayL.pt(2),"L2");
                Sys::debugMapMotif->insertDebugMark(rayL.pt(1),"L1");
                Sys::debugMapMotif->insertDebugMark(isectL,"ISECT-L");
            }

            RadialRay & rayR = unitRays[index+1];
            QLineF lineR(rayR.pt(2),rayR.pt(1));
            QPointF isectR;
            Geo::intersectPoly(lineR,poly,isectR);

            if (Sys::flags->flagged(SHOW_UNIT_RAYS))
            {
                Sys::debugMapMotif->insertDebugMark(rayR.pt(2),"R2");
                Sys::debugMapMotif->insertDebugMark(rayR.pt(1),"R1");
                Sys::debugMapMotif->insertDebugMark(isectR,"ISECT-R");
            }

            // use the two shortest
            QLineF lineL1(rayL.pt(1),isectL);
            qreal lenL = lineL1.length();

            QLineF lineR1(rayR.pt(1),isectR);
            qreal lenR = lineR1.length();

            if (Loose::equals(lenR,lenL))
            {
                rayL.setPt(1,isectL);
                rayR.setPt(1,isectR);
            }
            else if (lenL < lenR)
            {
                rayL.setPt(1,isectL);
                lineR1.setLength(lenL);
                rayR.setPt(1,lineR1.p2());
            }
            else
            {
                rayR.setPt(1,isectR);
                lineL1.setLength(lenR);
                rayL.setPt(1,lineL1.p2());
            }
        }   break;

        case TipType::REPEAT:
            break;
    }
    }
    if (getMotifDebug() & 0x08) unitRays.debug(0);

    QTransform t1 = getMotifTransform();
    unitRays.transform(t1);

    if (getMotifDebug() & 0x10) unitRays.debug(0);
    if (getMotifDebug() & 0x20) unitRays.debug(1);

    if (inscribe)
    {
        qreal circumradius = 1.0 / (cos(M_PI / (qreal)getN()));
        QTransform t = QTransform::fromScale(circumradius,circumradius);
        unitRays.transform(t);
    }

    if (onPoint)
    {
        qreal angle = 180.0/qreal(getN());
        QTransform t;
        t.rotate(angle);
        unitRays.transform(t);
    }

    if (getMotifDebug() & 0x40) unitRays.debug(0);
}

int Rosette2::buildUnitRays(QPointF tip, const RadialRay & epoints, int side)
{
    RadialRay rayR;
    rayR.set(epoints);
    rayR.addTip(tip);

    RadialRay rayL;
    for (const QPointF & pt : epoints.get())
    {
        rayL.addTail(QPointF(pt.x(), -pt.y()));
    }
    rayL.addTip(tip);

    QTransform t = getRadialTransform(side);
    rayL.transform(t);
    rayR.transform(t);

    // clockwise
    unitRays.add(rayL);
    unitRays.add(rayR);

    return unitRays.size() - 2;     // index of first ray added here
}

void Rosette2::calcConstraintLine()
{
    qreal   qr_outer    = 1.0 / qCos( M_PI * don() );
    QPointF r_outer     = QPointF(0.0, qr_outer);
    QPointF up_outer    = getArc( 0.5 * don() ) * qr_outer;
    QPointF down_outer  = up_outer - r_outer;
    QPointF bisector    = down_outer * 0.5;

    constraint  = QLineF(bisector, up_outer);

    if (getMotifDebug() > 0x80)
    {
        Sys::debugMapCreate->insertDebugLine(constraint,Qt::blue);
        Sys::debugMapCreate->insertDebugMark(up_outer,"up_outer",Qt::red);
        Sys::debugMapCreate->insertDebugMark(bisector,"bisector",Qt::red);
    }
}

bool Rosette2::convertConstrained()
{
    if (!constrain) return false;

    qreal  x    = kneePt.x() / xRange;
    qreal  y    = kneePt.y() / yRange;

    kneeX       = 1.0 - x;
    kneeY       = y;
    constrain   = false;

    return true;
}
