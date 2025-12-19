#ifdef __linux__
#include <cfloat>
#endif
#include <QColor>
#include <QPolygonF>

#include "model/styles/outline_casing.h"
#include "model/styles/casing_side.h"
#include "sys/geometry/arcdata.h"
#include "sys/geometry/neighbour_map.h"

////////////////////////////////////////////////////////////////////////////
///
/// OutlineCasing
///
////////////////////////////////////////////////////////////////////////////

OutlineCasing::OutlineCasing(CasingSet *owner, const EdgePtr &edge, qreal width)
{
    this->owner = owner;
    s1 = nullptr;
    s2 = nullptr;

    wedge       = edge;
    this->width = width;
}

void OutlineCasing::init()
{
    auto edge = wedge.lock();

    auto n1 = owner->getNeighbouringCasings(edge->v1);
    auto n2 = owner->getNeighbouringCasings(edge->v2);

    s1 = new CasingSide(this,SIDE_1,n1,edge->v1);
    s2 = new CasingSide(this,SIDE_2,n2,edge->v2);

    if (edge->isLine())
    {
        s1->createSide1(edge, width);
        s2->createSide2(edge, width);
    }
    else
    {
        createCurved();
    }
}

OutlineCasing::~OutlineCasing()
{
    if (s1)
    {
        delete s1;
        s1 = nullptr;
    }
    if (s2)
    {
        delete s2;
        s2 = nullptr;
    }
}

void OutlineCasing::setPainterPath()
{
    path.clear();

    EdgePtr edge = wedge.lock();
    if (!edge) return;

    if (edge->getType() == EDGETYPE_LINE)
    {
        path.moveTo(s2->outer);
        path.lineTo(s2->mid);
        path.lineTo(s2->inner);
        path.lineTo(s1->inner);
        path.lineTo(s1->mid);
        path.lineTo(s1->outer);
        path.lineTo(s2->outer);
    }
    else if (edge->getType() == EDGETYPE_CURVE)
    {
        path.moveTo(s2->outer);
        path.lineTo(s2->mid);
        path.lineTo(s2->inner);

        ArcData ad1(QLineF(s2->inner,s1->inner),edge->getArcCenter(),edge->getCurveType());
        path.arcTo(ad1.rect(),ad1.start(),-ad1.span());

        path.lineTo(s1->mid);
        path.lineTo(s1->outer);

        ArcData ad2(QLineF(s1->outer,s2->outer),edge->getArcCenter(),edge->getCurveType());
        path.arcTo(ad2.rect(),ad2.start(),ad2.span());
    }
}

bool OutlineCasing::validate()
{
    if (getEdge()->isCurve())
    {
        if (!validateCurves())
            return  false;
    }

    return true;
}
