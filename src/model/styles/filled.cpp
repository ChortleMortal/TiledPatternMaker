#include <QPainter>
#include <QDebug>
#include "model/styles/filled.h"
#include "sys/geometry/dcel.h"
#include "gui/viewers/geo_graphics.h"
#include "model/prototypes/prototype.h"

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

Filled::Filled(const ProtoPtr & proto, eFillType algorithm ) : Style(proto)
{
    draw_inside_blacks    = true;
    draw_outside_whites   = true;
    this->algorithm       = algorithm;
    colorMaker            = new ColorMaker(this);
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
        algorithm             = FILL_ORIGINAL;
    }
    colorMaker                = new ColorMaker(this);
}

Filled::~Filled()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting Filled";
    whiteColorSet.clear();
    blackColorSet.clear();
    colorGroup.clear();
#endif
    delete colorMaker;
}


void  Filled::setAlgorithm(eFillType val)
{
    algorithm = val;
    resetStyleRepresentation();
}

// Style overrides.

void Filled::resetStyleRepresentation()
{
    colorMaker->clear();
    created = false;
}

void Filled::createStyleRepresentation()
{
    if (!created)
    {
        // Filled does not use a style map, just a DCEL
        FilledDCELPtr dcel = prototype->getFilledDCEL();
        if (dcel)
        {
            colorMaker->buildFaceSets(algorithm, dcel);
        }
        created = true;
    }
}

void Filled::draw(GeoGraphics * gg)
{
    if (!isVisible() || !getPrototype()->getFilledDCEL())
    {
        return;
    }

    switch (algorithm)
    {
    case FILL_ORIGINAL:
        //qDebug() << "Filled::draw() algorithm=" << algorithm;
        drawDCEL(gg);
        break;

    case FILL_TWO_FACE:
        //qDebug() << "Filled::draw() algorithm=" << algorithm;
        drawDCEL(gg);
        break;

    case FILL_MULTI_FACE:
        //qDebug() << "Filled::draw() algorithm 2 :" << faceGroups.size() << faceGroups.totalSize();
        drawDCELNew2(gg);
        break;

    case FILL_MULTI_FACE_MULTI_COLORS:
        //qDebug() << "Filled::draw() algorithm 3 :" << faceGroups.size() << faceGroups.totalSize();
        drawDCELNew3(gg);
        break;

    case FILL_DIRECT_FACE:
        drawDCELDirectColor(gg);
        break;
    }
}

void Filled::drawDCEL(GeoGraphics * gg)
{
    if (draw_outside_whites)
    {
        QColor color =  whiteColorSet.getFirstTPColor().color;
        QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        for (auto & face : std::as_const(colorMaker->whiteFaces))
        {
            const EdgePoly & ep = *face.get();
            gg->fillEdgePoly(ep, pen);
            color = whiteColorSet.getNextTPColor().color;
            pen.setColor(color);
        }
    }

    if (draw_inside_blacks)
    {
        QColor color = blackColorSet.getFirstTPColor().color;
        QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        for (auto & face : std::as_const(colorMaker->blackFaces))
        {
            const EdgePoly & ep = *face.get();
            gg->fillEdgePoly(ep, pen);
            color = blackColorSet.getNextTPColor().color;
            pen.setColor(color);
        }
    }
}

void Filled::drawDCELNew2(GeoGraphics *gg)
{
    // each face set has a single color in whiteColorSet
    int row = 0;
    for (FaceSetPtr & fset : colorMaker->faceGroups)
    {
        if (fset->selected)
        {
            QPen pen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            for (const FacePtr & face : std::as_const(*fset))
            {
                EdgeSet & ep = *face.get();
                gg->fillEdgePoly(ep,pen);
            }
        }
        else
        {
            TPColor tpcolor =  whiteColorSet.getTPColor(row);
            if (!tpcolor.hidden)
            {
                QPen pen(tpcolor.color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                for (const FacePtr & face : std::as_const(*fset))
                {
                    EdgeSet & ep = *face.get();
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
    for (const FaceSetPtr & fset : std::as_const(colorMaker->faceGroups))
    {
        if (fset->selected)
        {
            QPen pen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            for (FacePtr & face : *fset)
            {
                EdgeSet & ep = *face.get();
                gg->fillEdgePoly(ep,pen);
            }
            continue;
        }

        ColorSet * cset = fset->colorSet;
        if (!cset)
        {
            continue;
        }

        if (cset->isHidden())
        {
            continue;
        }

        cset->resetIndex();

        for (const FacePtr & face : std::as_const(*fset))
        {
            TPColor tpc = cset->getNextTPColor();
            if (tpc.hidden)
            {

                continue;
            }
            QPen pen(tpc.color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            EdgeSet & ep = *face.get();
            gg->fillEdgePoly(ep,pen);
        }
    }
}

void Filled::drawDCELDirectColor(GeoGraphics *gg)
{
    FilledDCELPtr dcel = getPrototype()->getFilledDCEL();
    if (!dcel) return;

    const FaceSet & faces = dcel->getFaceSet();
    if (faces.size() != numColorIndices())
    {
        qWarning() << "Filled faces=" << faces.size() << "indices" << numColorIndices();
    }

    for (const FacePtr & face : faces)
    {
        int index = face->iPalette;
        if (index >=0)
        {
            QColor color = getColorFromPalette(index);
            QPen pen(color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            const EdgePoly & ep = *face.get();
            gg->fillEdgePoly(ep, pen);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
// Palette colors
//
//////////////////////////////////////////////////////////////////////////////////

int  Filled::getColorIndex(int faceIndex)
{
    if (faceIndex < faceColorIndices.size())
    {
        return faceColorIndices[faceIndex];
    }
    else
    {
        return -1;
    }
}

void Filled::setColorIndex(int faceIndex,int colorIndex)
{
    if (faceIndex < faceColorIndices.size())
    {
        faceColorIndices[faceIndex] = colorIndex;
    }
}

void Filled::removeColorIndex(int index)
{
    FilledDCELPtr dcel = getPrototype()->getFilledDCEL();
    if (!dcel) return;

    colorMaker->removePaletteIndex(dcel,index);
}
