#include <QDebug>

#include "model/motifs/radial_extender.h"
#include "model/motifs/radial_motif.h"
#include "model/tilings/tile.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/intersect.h"

///////////////////////////////////////////////////////////////////////////////
///
///  RadialExtender
///
///////////////////////////////////////////////////////////////////////////////

RadialExtender::RadialExtender(Motif * motif) : Extender(motif)
{
    setFitScale(1.0);
}

RadialExtender::RadialExtender(const Extender & other) : Extender(other)
{
    setFitScale(1.0);
}

void RadialExtender::extend()
{
    Q_ASSERT(motif);
    Q_ASSERT(motif->getTile());

    RadialMotif * radial = static_cast<RadialMotif*>(motif);
    extend(radial->getFullRays());
}

void RadialExtender::extend(RaySet & rays)
{
    Q_ASSERT(motif);
    Q_ASSERT(motif->getTile());

    buildExtendedBoundary();

    if (!rays.valid())
    {
        qWarning() << "Invalid ray";
        rays.debug(1);
    }

    if (getExtendRays())
    {
        extendRaysToBoundary(rays);
    }

    if (getExtendTipsToBound() || static_cast<RadialMotif*>(motif)->hasTipsToMove())
    {
        extendRaysToBoundaryPerp(rays);
    }

    if (getExtendBoundaryToTile())
    {
        extendRaysToTilePerp(rays,motif->getTile());
    }

    // after other extenders
    uint method = getConnectRays();
    if (method > 0)  // connect to each other
    {
        extendRays(method,rays);
    }

    if (getStellation() > 0)
    {
        stellate(static_cast<RadialMotif*>(motif),getStellation(),getFitToTile());
    }
}

void RadialExtender::extendRaysToBoundary(RaySet & rays)
{
    if (!rays.valid()) return;

    const ExtendedBoundary & eb = getExtendedBoundary();
    const QPolygonF & eboundary = eb.getPoly();
    qreal radius                = eb.getRadius();
    QGraphicsEllipseItem circle(-radius,-radius,radius * 2.0, radius * 2.0);

    int idx = 0;
    QVector<RadialRay> & rayvec = rays.get();
    for (int i = 0; i < rayvec.size(); i++)
    {
        if (tipMoves(rays,i))
            continue;   // motif tip type overrides

        RadialRay & ray = rayvec[i];
        QLineF l1 = ray.getRayTip();
        l1  = Geo::extendLine(l1,10.0);      // extends
        if (!eb.isCircle())
        {
            l1  = Geo::clipLine(l1,eboundary);   // clips
            QPointF pt = l1.p2();
            ray.addTip(pt);
        }
        else
        {
            QPointF a;
            QPointF b;
            int points = Geo::circleLineIntersectionPoints(circle,radius,l1,a,b);
            if (points == 2)
            {
                qreal dista = Geo::dist2(a,ray.getTip());
                qreal distb = Geo::dist2(b,ray.getTip());
                if (dista < distb)
                    ray.addTip(a);
                else
                    ray.addTip(b);
            }
            else if (points == 1)
            {
                if (a.isNull())
                    ray.addTip(b);
                else
                    ray.addTip(a);
            }
        }
        if (motif->getMotifDebug() & 0x100) ray.debug(idx++);
    }
}

void RadialExtender::extendRaysToBoundaryPerp(RaySet & rays)
{
    if (!rays.valid()) return;

    const ExtendedBoundary & eb = getExtendedBoundary();
    if (eb.isCircle())
    {
        qWarning() << "Extender does not support extension to circle";
        return;
    }

    const QPolygonF & eboundary = eb.getPoly();
    EdgePoly ep(eboundary);

    bool addTips = getExtendTipsToBound();

    QVector<RadialRay> & rayvec = rays.get();
    for (int i = 0; i < rayvec.size(); i++)
    {
        RadialRay & ray = rayvec[i];
        bool moveTip    = tipMoves(rays,i);
        if (!moveTip && !addTips)
            continue;

        QPointF tip = ray.getTip();

        // draw line normal to the boundary from this point
        // until it intersects the tile

        QPointF apt;
        qreal dist = 1000;
        for (QLineF & line : ep.getLines())
        {
            QLineF pline = perpLine(line,tip);
            QPointF isect;
            if (Intersect::getIntersection(line,pline,isect))
            {
                qreal len = QLineF(tip,isect).length();
                if (len < dist)
                {
                    dist = len;
                    apt  = isect;
                }
            }
        }

        if (moveTip)
            ray.setPt(0,apt);
        else
            ray.addTip(apt);
    }
}

// the motif's tip type moves this ray's tip to the boundary, overriding the other tip extensions
bool RadialExtender::tipMoves(RaySet & rays, int i)
{
    return (i < rays.unextendedSize) && static_cast<RadialMotif*>(motif)->moveTipToBoundary(i);
}

void RadialExtender::extendRaysToTilePerp(RaySet & rays, TilePtr tile)
{
    if (!rays.valid()) return;

    QVector<RadialRay> & rayvec = rays.get();
    for (int i = 0; i < rayvec.size(); i++)
    {
        if (tipMoves(rays,i))
            continue;   // motif tip type overrides

        RadialRay & ray = rayvec[i];
        QPointF tip = ray.getTip();

        // draw line normal to the tile boundary from this point
        // until it intersects the tile

        QPointF apt;
        qreal dist = 1000;
        for (const EdgePtr & edge : tile->getEdgePoly().get())
        {
            QLineF pline = perpLine(edge->getLine(),tip);
            QPointF isect;
            if (Intersect::getIntersection(edge->getLine(),pline,isect))
            {
                qreal len = QLineF(tip,isect).length();
                if (len < dist)
                {
                    dist = len;
                    apt  = isect;
                }
            }
        }
        ray.addTip(apt);
    }
}

void RadialExtender::addExtenderRaysToBoundary(RaySet & rays)
{
    const ExtendedBoundary & eb = getExtendedBoundary();
    const QPolygonF & eboundary = eb.getPoly();
    EdgePoly ep(eboundary);

    int sz = rays.size();   // prior to additions
    for (int i=0; i < sz; i+=(rays.cadencePairs *2))
    {
        RadialRay & ray  = rays[(i+3) % sz];
        RadialRay newRay = addExtenderRayToBoundary(ray,ep);
        rays.add(newRay);

        RadialRay & ray2  = rays[(i+4) % sz];
        RadialRay newRay2 = addExtenderRayToBoundary(ray2,ep);
        rays.add(newRay2);
    }
}

RadialRay RadialExtender::addExtenderRayToBoundary(RadialRay & ray, EdgePoly & boundary)
{
    QLineF rayLine(ray.pt(2), ray.pt(1));

    QPointF apt;
    qreal dist = 1000;
    for (QLineF & line : boundary.getLines())
    {
        QPointF isect;
        rayLine.intersects(line,&isect);
        qreal len = QLineF(ray.pt(0),isect).length();
        if (!Loose::zero(len) && (len < dist))
        {
            dist = len;
            apt  = isect;
        }
    }

    RadialRay newRay;
    newRay.addTip(apt);
    newRay.addTail(ray.pt(1));
    return  newRay;
}

void RadialExtender::extendRays(uint method, RaySet & rays)
{
    qDebug() << "RadialExtender::extendRays method = " << method << "set size =" << rays.size();

    if (motif->getMotifDebug() & 0x100)
    {
        int index = 0;
        for (RadialRay & ray : rays.get())
        {
            Sys::debugMapMotif->insertDebugMark(ray.getTip(),QString::number(index),Qt::red);
            Sys::debugMapMotif->insertDebugLine(ray.getRayTip(),Qt::blue);
            index++;
        }
    }

    int sz = rays.size();
    for (int i=0; i < sz; i += 4)
    {
        switch (method)
        {
        case 1:
        {
            QPointF tipA = rays[i+3].getTip();
            QPointF tipB = rays[i+5].getTip();
            rays[i].addTip(tipA);
            rays[i+2].addTip(tipB);
        }   break;

        case 2:
        {
            QPointF tipA = rays[i].getTip();
            QPointF tipB = rays[i+3].getTip();
            rays[i+2].addTip(tipA);       // do first
            rays[i+1].addTip(tipB);
        }   break;

        case 3:
        {
            QPointF tipA = rays[i].getTip();
            QPointF tipB = rays[i+2].getTip();
            rays[i+3].addTip(tipA);
            rays[i].addTip(tipB);
        }   break;

        case 4:
        {
            QPointF tipA = rays[i+3].getTip();
            QPointF tipB = rays[i+2].getTip();
            rays[i].addTip(tipA);
            rays[i+1].addTip(tipB);
        }   break;
        }
    }

    switch(method)
    {
    case 5:
        addExtenderRaysToBoundary(rays);
        joinRays(rays,rays.cadencePairs,0);
        break;
    case 6:
        addExtenderRaysToBoundary(rays);
        intersectRays(rays,rays.cadencePairs,0);
        break;
    case 7:
        extendRaysToBoundary(rays);
        break;

    case 8:
        //extendRaysToBoundary(rays);
        joinRays(rays,rays.cadencePairs,0);
        break;

    case 9:
        //extendRaysToBoundary(rays);
        intersectRays(rays,rays.cadencePairs,1);
        break;
    }
    //qDebug() << "RadialExtender::extendRays END " << "set size =" << set.size();
}

void RadialExtender::joinRays(RaySet &rays, int rayIndex, int ptIndex)
{
    // creates a new ray instead of extending one ray to other
    // keeps rays the same
    int sz = rays.unextendedSize;;
    for (int i=0; i < sz; i += (rays.cadencePairs*2))
    {
        QPointF p1 = rays[(i+rayIndex) %sz].pt(ptIndex);
        QPointF p2 = rays[(i+rayIndex +1) %sz].pt(ptIndex);
        RadialRay r;
        r.addTail(p1);
        r.addTip(p2);
        rays.add(r);
    }
}

void RadialExtender::intersectRays(RaySet &rays, int rayIndex, int ptIndex)
{
    int sz = rays.unextendedSize;
    for (int i=0; i < sz; i+= (rays.cadencePairs*2))
    {
        auto ray1 = rays[(i+rayIndex) %sz];
        auto ray2 = rays[(i+rayIndex+1) %sz];
        QLineF l1(ray1.pt(ptIndex+1),ray1.pt(ptIndex));
        QLineF l2(ray2.pt(ptIndex+1),ray2.pt(ptIndex));
        QPointF isect;
        l1.intersects(l2,&isect);

        RadialRay r1;
        r1.addTail(l1.p2());
        r1.addTip(isect);
        rays.add(r1);

        RadialRay  r2;
        r2.addTail(l2.p2());
        r2.addTip(isect);
        rays.add(r2);
    }
}

// This was from Kaplan's connector code - but is completely re-written
// Stellation was also only applied to regular radial rosettes. For this resizing
// to fit within the tile was straightforward. Now, stellation can be tried for
// any tiling, but can fail, and secondly resizing to fit is optional
void RadialExtender::stellate(RadialMotif * motif, uint stellation, bool fitTile)
{
    if (stellation == 0)
        return;

    struct sTip
    {
        int rayIndex;
        QPointF pt;
    };

    constexpr uint secondTips[5] = {3,5,7,9,11};

    QList<sTip> additions;

    int secondtip = secondTips[(stellation-1) % 5];

    RaySet & fullset     = motif->getFullRays();
    const RaySet copyset = motif->getFullRays();

    Q_ASSERT(fullset.equals(copyset));

    if (fullset.size() < 4)
    {
        qInfo() << "cannot connect: full ray size =" << fullset.size();
        return;
    }

    // create list of additions
    // cannot apply directly because need to determines isects for
    // unmodified tips
    qreal calcScale = 1.0;
    for (int i=0; i < fullset.size(); i+=2)
    {
        QLineF line1 = fullset[i].getRayTip();
        QLineF line2 = fullset[i+secondtip].getRayTip();

        // Find intersection and add this to the tips
        QPointF isect;
        if (line1.intersects(line2, &isect) !=  QLineF::NoIntersection)
        {
                sTip st1(i,isect);
                additions.push_back(st1);

                sTip st2(i+secondtip,isect);
                additions.push_back(st2);
        }
    }
    // add the new tip points (if any)
    for (auto & st : additions)
    {
        Sys::debugMapMotif->insertDebugMark(st.pt,"TIP",Qt::red);
        fullset[st.rayIndex].addTip(st.pt);
    }

    if (fullset.equals(copyset))
    {
        // weeds out cases where stellation does not work
        qInfo() << "Stellation has failed";
        return;
    }

    if (fitTile)
    {
        QPointF farPt = fullset.furthestPointFromOrigin();
        qreal len = QLineF(QPointF(0,0),farPt).length();
        calcScale = motif->getTile()->getScale()/len;

        // only scale down where stellation goes outside of tile
        //qInfo() << "Using scale" << calcScale;
        setFitScale(calcScale);
        fullset.transform(QTransform().scale(calcScale,calcScale));
    }
}
