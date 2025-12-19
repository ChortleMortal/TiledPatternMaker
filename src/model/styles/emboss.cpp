#include "model/styles/emboss.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "gui/viewers/geo_graphics.h"

using std::make_shared;

////////////////////////////////////////////////////////////////////////////
//
// Emboss.java
//
// A rendering style for maps that pretends that the map is carves out
// of wood with a diamond cross section and shines a directional light
// on the wood from some angle.  The map is drawn as a collection of
// trapezoids, and the darkness of each trapezoid is controlled by the
// angle the trapezoid makes with the light source.  The result is a
// simple but highly effective 3D effect, similar to flat-shaded 3D
// rendering.
//
// In practice, we can make this a subclass of RenderOutline -- it uses the
// same pre-computed point array, and just add one parameter and overloads
// the draw function.


// Creation.

Emboss::Emboss(ProtoPtr proto) : Outline(proto)
{
    setAngle(M_PI * 0.25 );
}

Emboss::Emboss(StylePtr other) : Outline(other)
{
    std::shared_ptr<Emboss> emb = std::dynamic_pointer_cast<Emboss>(other);
    if (emb)
    {
        angle   = emb->angle;
        light_x = emb->light_x;
        light_y = emb->light_y;
        greys   = emb->greys;
    }
    else
    {
        setAngle(M_PI * 0.25 );
        setGreys();
    }
}

Emboss::~Emboss()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting emboss";
#endif
}

// Style overrides.

void Emboss::draw(GeoGraphics * gg)
{
    if (!isVisible())
    {
        return;
    }

    auto map = getProtoMap();    // from prototype
    for (auto & casing : std::as_const(casings))
    {
        drawTrap(gg, casing->side(SIDE_2)->mid, casing->side(SIDE_2)->inner, casing->side(SIDE_1)->inner, casing->side(SIDE_1)->mid);
        drawTrap(gg, casing->side(SIDE_1)->mid, casing->side(SIDE_1)->outer, casing->side(SIDE_2)->outer, casing->side(SIDE_2)->mid);
        
        if (drawOutline != OUTLINE_NONE)
        {
            QPen pen;
            if (drawOutline == OUTLINE_SET)
            {
                pen = QPen(outline_color,Transform::scalex(gg->getTransform() * outline_width * 0.5));
            }
            else
            {
                Q_ASSERT(drawOutline == OUTLINE_DEFAULT);
                pen = QPen(Qt::black,1);
            }
            pen.setJoinStyle(join_style);
            pen.setCapStyle(cap_style);

            OutlineCasingPtr ocp = std::static_pointer_cast<OutlineCasing>(casing);
            QPolygonF poly = ocp->getPoly();
            gg->drawPolygon(poly,pen);
            gg->drawLine(casing->side(SIDE_2)->mid, casing->side(SIDE_1)->mid,pen);
        }
    }
}

void Emboss::drawTrap(GeoGraphics * gg, QPointF a, QPointF b, QPointF c, QPointF d )
{
    QPointF N = a - d;
    Geo::perpD(N);
    Geo::normalizeD(N);

    // dd is a normalized floating point value corresponding to
    // the brightness to use.
    qreal dd = 0.5 * ( N.x() * light_x + N.y() * light_y + 1.0 );

    // Quantize to sixteen grey values.
    int bb = static_cast<int>(16.0 * dd);
    QColor color = greys[bb];

    QPolygonF trap_pts;
    trap_pts << a << b << c << d;
    gg->fillPolygon(trap_pts,color);
}

// Data.

qreal Emboss::getAngle()
{
    return angle;
}

void Emboss::setAngle(qreal angle )
{
    this->angle = angle;
    light_x = qCos( angle );
    light_y = qSin( angle );
}

void Emboss::setColorSet(ColorSet & cset)
{
    Outline::setColorSet(cset);
    setGreys();
}

void Emboss::setGreys()
{
    float h;
    float s;
    float b;

    getColorSet()->getFirstTPColor().color.getHsvF(&h,&s,&b);

    greys.clear();

    for( int idx = 0; idx < 17; ++idx )
    {
        float t  = (float)idx / 16.0f;
        float s1 = (1.0f-t)*0.7f + t*0.99f;
        float b1 = (1.0f-t)*0.4f + t*0.99f;
        float ss = s * s1;
        float bb = b * b1;

        QColor c;
        c.setHsvF(h, ss, bb);
        greys << c;
    }
}


