////////////////////////////////////////////////////////////////////////////
//
// Rosette2
//
// Rosette2 is an alternative to Kaplan's Rosette, which relaxes
// Kaplans constraint on Knee position which is only a partial truth
// Also adds more vraiations

#include <QDebug>
#include "model/motifs/rosette2.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/intersect.h"
#include "sys/geometry/transform.h"
#include "model/tilings/tile.h"

typedef std::shared_ptr<Rosette2> Rosette2Ptr;

using std::make_shared;

Rosette2::Rosette2(const Motif & fig,  int nsides, qreal kneeX, qreal kneeY, int ss, qreal kk, bool c) : RadialMotif(fig, nsides)
{
    this->kneeX = kneeX;
    this->kneeY = kneeY;
    s           = ss;
    k           = kk;
    constrain   = c;
    tipTypes    = TIP_TYPE2_OUTER;
    setTipMode(TIP_MODE_REGULAR);
    setMotifType(MOTIF_TYPE_ROSETTE2);
}

Rosette2::Rosette2(int nsides, qreal kneeX, qreal kneeY, int ss, qreal kk, bool c) : RadialMotif(nsides)
{
    this->kneeX = kneeX;
    this->kneeY = kneeY;
    s           = ss;
    k           = kk;
    constrain   = c;
    tipTypes    = TIP_TYPE2_OUTER;
    setTipMode(TIP_MODE_REGULAR);
    setMotifType(MOTIF_TYPE_ROSETTE2);
}

Rosette2::Rosette2(const Rosette2 & other) : RadialMotif(other)
{
    kneeX       = other.kneeX;
    kneeY       = other.kneeY;
    s           = other.s;
    k           = other.k;
    constrain   = other.constrain;
    tipTypes    = other.tipTypes;
    setTipMode(other.getTipMode());
    setMotifType(MOTIF_TYPE_ROSETTE2);
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

    if (tipTypes != otherp->tipTypes)
        return false;

    if (!Motif::equals(other))
        return false;

     return true;
}

bool Rosette2::pointOnLineLessThan(QPointF p1, QPointF p2)
{
    return  Geo::dist2(kneePt,p1) < Geo::dist2(kneePt,p2);
}

// kneeX is the %age of the distance to the center, so is always scaled by 1.0
// kneey is the %age of half the edge width, so scaaling is depends on n (sides)
void Rosette2::buildUnitMap()
{
  //motifDebug = 0x15
  //motifDebug = 0x04
  //motifDebug = 0x05;
    motifDebug = 0;

    if (motifDebug > 0)
    {
        Sys::debugMapCreate->wipeout();
    }

    qDebug().noquote() << "Rosette2::buildUnit n:" << getN() << "x:" << kneeX << "y:" << kneeY << "s:" << s << "k" << k
                       << "Tr:" << Transform::info(radialRotationTr) << "rot" << getMotifRotate() << "scale" << getMotifScale();

    Tile atile(getN());
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

    if (motifDebug & 0x01)
    {
        Sys::debugMapCreate->insertDebugMark(kneePt,"kneePt",Qt::red);
        Sys::debugMapCreate->insertDebugLine(ray,Qt::red);
        Sys::debugMapCreate->insertDebugLine(rayMirror,Qt::blue);
    }

    UniqueQVector<QPointF> epoints;

    epoints.push_back(kneePt);

    for (int idx = 0; idx <= getN(); idx++)
    {
        rayMirror = radialRotationTr.map(rayMirror);

        QPointF isect;
        if (ray.intersects(rayMirror,&isect) == QLineF::BoundedIntersection)
        {
            epoints.push_back(isect);
        }
    }

    std::sort(epoints.begin(), epoints.end(), [this] (QPointF a, QPointF b) { return pointOnLineLessThan(a, b); });

    if (epoints.size() > (s+1))
    {
        epoints.resize(s+1);
    }

    if (motifDebug & 0x04)
    {
        qDebug() << "epoints" << epoints;
        int idx = 0;
        for (auto & ept :  std::as_const(epoints))
        {
            Sys::debugMapCreate->insertDebugMark(ept,QString("pt%1").arg(idx++),Qt::red);
        }
    }

    // build the rays
    raySet1.clear();
    raySet2.clear();

    QLineF  kneeLine(kneePt,kneePtMirror);
    QPointF innerTip = Geo::reflectPoint(outerTip,kneeLine);    // outer tip reflected over kneeLine
    QTransform rotTr = getUnitRotationTransform();

    if (getTipMode() == TIP_MODE_REGULAR)
    {
        // really only one should be selected
        if (getTipTypes() & TIP_TYPE2_OUTER)
        {
            buildRay(raySet1,outerTip,epoints);
            raySet2 = raySet1;
        }
        else if (getTipTypes() & TIP_TYPE2_INNER)
        {
            buildRay(raySet1,innerTip,epoints);
            raySet2 = raySet1;
        }
        else if (getTipTypes() & TIP_TYPE2_FLIPPED)
        {
            buildRay(raySet1,innerTip,epoints);

            QTransform td = getDELTransform();
            QLineF edge = atile.getEdge(getN()-1);
            edge = td.map(edge);

            QPointF ep2 = Geo::perpPt(edge,raySet1.ray1.pt(1));
            raySet1.ray1.setPt(1,ep2);
            QPointF ep3 = Geo::perpPt(edge,raySet1.ray2.pt(1));
            raySet1.ray2.setPt(1,ep3);

            raySet2 = raySet1;
        }
        raySet2.transform(rotTr);
    }
    else
    {
        Q_ASSERT(getTipMode() == TIP_MODE_ALTERNATE);
        // really only two should be selected
        int raysdone = 0;

        if (getTipTypes() & TIP_TYPE2_OUTER)
        {
            buildRay(raySet1,outerTip,epoints);
            raysdone++;
        }

        if (getTipTypes() & TIP_TYPE2_INNER)
        {
            if (raysdone)
            {
                buildRay(raySet2,innerTip,epoints);
                raysdone++;
            }
            else
            {
                buildRay(raySet1,innerTip,epoints);
                raysdone++;
            }
        }

        if (getTipTypes() & TIP_TYPE2_FLIPPED)
        {
            if (raysdone == 1)
            {
                buildRay(raySet2,innerTip,epoints);

                QTransform td = getDELTransform();
                QLineF edge = atile.getEdge(getN()-1);
                edge = td.map(edge);

                QPointF ep2 = Geo::perpPt(edge,raySet2.ray1.pt(1));
                raySet2.ray1.setPt(1,ep2);
                QPointF ep3 = Geo::perpPt(edge,raySet2.ray2.pt(1));
                raySet2.ray2.setPt(1,ep3);
            }
        }
        raySet2.transform(rotTr);
    }

    if (inscribe)
    {
        qreal circumradius = 1.0 / (cos(M_PI / (qreal)getN()));
        QTransform t = QTransform::fromScale(circumradius,circumradius);
        raySet1.transform(t);
        raySet2.transform(t);
    }

    if (onPoint)
    {
        qreal angle = 180.0/qreal(getN());
        QTransform t;
        t.rotate(angle);
        raySet1.transform(t);
        raySet2.transform(t);
    }

    raySet1.debug();
}

void  Rosette2::buildRay(RaySet &  set, QPointF tip, const QVector<QPointF> & epoints)
{
    set.ray1.set(epoints);
    set.ray1.addTip(tip);

    set.ray2.addTip(tip);
    for (auto pt : epoints)
    {
        set.ray2.addTail(QPointF(pt.x(), -pt.y()));
    }

    if (motifDebug & 0x01) set.debug();
    QTransform t = getDELTransform();
    set.transform(t);
    if (motifDebug & 0x01) set.debug();
}

void Rosette2::calcConstraintLine()
{
    qreal   don         = get_don();
    qreal   qr_outer    = 1.0 / qCos( M_PI * don );
    QPointF r_outer     = QPointF(0.0, qr_outer);
    QPointF up_outer    = getArc( 0.5 * don ) * qr_outer;
    QPointF down_outer  = up_outer - r_outer;
    QPointF bisector    = down_outer * 0.5;

    constraint  = QLineF(bisector, up_outer);

    if (motifDebug > 0x10)
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
