#include <QPainter>
#include <QDebug>
#include "style/filled.h"
#include "geometry/dcel.h"
#include "geometry/map.h"
#include "misc/geo_graphics.h"
#include "makers/prototype_maker/prototype.h"

using std::shared_ptr;

////////////////////////////////////////////////////////////////////////////
//
// Filled.java
//
// A rendering style that converts the map to a collection of
// polygonal faces.  The faces are divided into two groups according to
// a two-colouring of the map (which is always possible for the
// kinds of Islamic designs we're building).
//
// The code to build the faces from the map is contained in
// geometry.Faces.

Filled::Filled(const ProtoPtr & proto, int algorithm ) : Style(proto)
{
    draw_inside_blacks    = true;
    draw_outside_whites   = true;
    this->algorithm       = algorithm;
}

Filled::Filled(const StylePtr & other) : Style(other)
{
    shared_ptr<Filled> filled = std::dynamic_pointer_cast<Filled>(other);
    if (filled)
    {
        draw_inside_blacks    = filled->draw_inside_blacks;
        draw_outside_whites   = filled->draw_outside_whites;
        algorithm             = filled->algorithm;
    }
    else
    {
        draw_inside_blacks    = true;
        draw_outside_whites   = true;
        algorithm             = 0;
    }
}

Filled::~Filled()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting Filled";
    whiteColorSet.clear();
    blackColorSet.clear();
    colorGroup.clear();
#endif
}


void  Filled::setAlgorithm(int val)
{
    algorithm = val;
    resetStyleRepresentation();
}

// Style overrides.

void Filled::resetStyleRepresentation()
{
    //dcel.reset();
    colorMaker.clear();

}

void Filled::createStyleRepresentation()
{
    // Filled does not use a style map, just a DCEL
    auto dcel = getPrototype()->getDCEL();
    Q_ASSERT(dcel);

    colorMaker.buildFaceSets(algorithm,dcel.get());
}

void Filled::updateStyleRepresentation()
{
    auto dcel = getPrototype()->getDCEL();
    Q_ASSERT(dcel);
    colorMaker.assignColors(algorithm,dcel.get());
}

void Filled::draw(GeoGraphics * gg)
{
    if (!isVisible() || !getPrototype()->getDCEL())
    {
        return;
    }

    switch (algorithm)
    {
    case 0:
        //qDebug() << "Filled::draw() algorithm=" << algorithm;
        drawDCEL(gg);
        break;

    case 1:
        //qDebug() << "Filled::draw() algorithm=" << algorithm;
        drawDCEL(gg);
        break;

    case 2:
        //qDebug() << "Filled::draw() algorithm 2 :" << faceGroups.size() << faceGroups.totalSize();
        drawDCELNew2(gg);
        break;

    case 3:
        //qDebug() << "Filled::draw() algorithm 3 :" << faceGroups.size() << faceGroups.totalSize();
        drawDCELNew3(gg);
        break;
    }
}

void Filled::drawDCEL(GeoGraphics * gg)
{
    if (draw_outside_whites)
    {
        QColor color = colorMaker.whiteColorSet.getFirstColor().color;
        QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        for (auto & face : std::as_const(colorMaker.whiteFaces))
        {
            const EdgePoly & ep = *face.get();
            gg->fillEdgePoly(ep, pen);
            color = colorMaker.whiteColorSet.getNextColor().color;
            pen.setColor(color);
        }
    }

    if (draw_inside_blacks)
    {
        QColor color = colorMaker.blackColorSet.getFirstColor().color;
        QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        for (auto & face : std::as_const(colorMaker.blackFaces))
        {
            const EdgePoly & ep = *face.get();
            gg->fillEdgePoly(ep, pen);
            color = colorMaker.blackColorSet.getNextColor().color;
            pen.setColor(color);
        }
    }
}

void Filled::drawDCELNew2(GeoGraphics *gg)
{
    // each face set has a single color in whiteColorSet
    int row = 0;
    for (FaceSetPtr & fset : colorMaker.faceGroups)
    {
        if (fset->selected)
        {
            QPen pen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            for (const FacePtr & face : std::as_const(*fset))
            {
                EdgePoly & ep = *face.get();
                gg->fillEdgePoly(ep,pen);
            }
        }
        else
        {
            TPColor tpcolor =  colorMaker.whiteColorSet.getColor(row);
            if (!tpcolor.hidden)
            {
                QPen pen(tpcolor.color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                for (const FacePtr & face : std::as_const(*fset))
                {
                    EdgePoly & ep = *face.get();
                    gg->fillEdgePoly(ep,pen);
                }
            }
        }
        row++;
    }
}

void Filled::drawDCELNew3(GeoGraphics *gg)
{
    // each face set has a set of colors
    for (const FaceSetPtr & fset : std::as_const(colorMaker.faceGroups))
    {
        if (fset->selected)
        {
            QPen pen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            for (FacePtr & face : *fset)
            {
                EdgePoly & ep = *face.get();
                gg->fillEdgePoly(ep,pen);
            }
            continue;
        }

        ColorSet *  cset = fset->pColorSet;

        if (cset->isHidden())
        {
            continue;
        }

        cset->resetIndex();

        for (const FacePtr & face : std::as_const(*fset))
        {
            Q_ASSERT(face->isClockwise());
            TPColor tpc = cset->getNextColor();
            if (tpc.hidden)
            {

                continue;
            }
            QPen pen(tpc.color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            EdgePoly & ep = *face.get();
            gg->fillEdgePoly(ep,pen);
        }
    }
}
