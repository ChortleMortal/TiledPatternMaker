#include <QDebug>
#include <QtMath>

#include "model/motifs/motif_connector.h"
#include "sys/geometry/debug_map.h"
#include "model/motifs/radial_motif.h"
#include "sys/geometry/intersect.h"
#include "sys/geometry/geo.h"

MotifConnector::MotifConnector()
{
    cscale    = 1.0;
}

qreal MotifConnector::build(RadialMotif * motif)
{
    RaySet & set1 = motif->getRaySet1();
    RaySet & set2 = motif->getRaySet2();

    // Extend Rays
    QLineF line1 = set1.ray2.getRay();
    QLineF line1a = Geo::extendLine(line1,10.0);

    QLineF line2 = set2.ray1.getRay();
    QLineF line2a = Geo::extendLine(line2,10.0);

    if (motif->getMotifDebug() & 0x08)
    {
        Sys::debugMapCreate->insertDebugLine(line1a,Qt::blue);
        Sys::debugMapCreate->insertDebugLine(line2a,Qt::blue);
    }

    // Find intersection and add this to the tips
    QPointF isect;
    if (Intersect::getIntersection(line1a, line2a, isect))
    {
        set1.ray2.addTip(isect);
        set2.ray1.addTip(isect);
        if (motif->getMotifDebug() & 0x02)
                                                                                                                                                                                                                                                                                                                       {
            Sys::debugMapCreate->insertDebugMark(isect,"isect",Qt::red);
            set1.debug();
            set2.debug();
        }
    }
    else
    {
        qWarning() << "MotifConnector::build - rays do not intersect, cannot connect";
        return motif->getMotifScale();
    }

    // calculate the scale factor to plase intersection on the motif boundary polygon
    // assumes size 1.0 for the motif
    qreal len = QLineF(QPointF(0,0),isect).length();
    cscale = 1.0/len;

    // put intersection on the next ray
    QTransform t0   = QTransform().rotateRadians(2.0 * M_PI * motif->get_don());
    QPointF isect2 = t0.map(isect);
    set2.ray2.addTip(isect2);

    // put intersection on the previous rat
    QTransform t1 = QTransform().rotateRadians(-2.0 * M_PI * motif->get_don());
    QPointF isect3 = t1.map(isect);
    set1.ray1.addTip(isect3);

    // use scale factor and rotate the intersects to be on the mid-points of polygon edge
    set1.transform(QTransform().rotateRadians(M_PI * motif->get_don()));
    set1.transform(QTransform().scale(cscale,cscale));
    set2.transform(QTransform().rotateRadians(M_PI * motif->get_don()));
    set2.transform(QTransform().scale(cscale,cscale));

    motif->setMotifScale(1.0);
    qDebug() << "Connector scale =" << cscale;

    emit sig_scaleChanged();

    return cscale;
}



