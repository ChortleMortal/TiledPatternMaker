/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "style/filled.h"
#include <QPainter>

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



// Creation.

Filled::Filled(PrototypePtr proto, PolyPtr bounds, int algorithm ) : Style(proto,bounds)
{
    draw_inside_blacks    = true;
    draw_outside_whites   = true;
    this->algorithm       = algorithm;
}

Filled::Filled(const Style  &other) : Style(other)
{
    try
    {
        const Filled & filled = dynamic_cast<const Filled&>(other);
        draw_inside_blacks    = filled.draw_inside_blacks;
        draw_outside_whites   = filled.draw_outside_whites;
        whiteColorSet         = filled.whiteColorSet;
        blackColorSet         = filled.blackColorSet;
        algorithm             = filled.algorithm;
    }
    catch(std::bad_cast exp)
    {
        draw_inside_blacks    = true;
        draw_outside_whites   = true;
        algorithm             = 0;
    }
}

Filled::~Filled()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting filled";
    white.clear();
    black.clear();
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
    whiteColorSet.resetIndex();
    blackColorSet.resetIndex();
    colorGroup.resetIndex();
    clearFaces();
}


void Filled::createStyleRepresentation()
{
    if (! whiteFaces.isEmpty() || !blackFaces.isEmpty() || !allFaces.isEmpty() || !faceGroup.isEmpty())
    {
        return;     // already created
    }

    MapPtr map = setupStyleMap();
    map->verifyMap("Filled");

    purifyMap(map);

    switch (algorithm)
    {
    case 0:
        if (blackFaces.size() == 0 && whiteFaces.size() == 0)
        {
            buildFacesOriginal(map);
            assignColorsOriginal(map);
            qDebug() << "black=" << blackFaces.size() <<  "white=" << whiteFaces.size();
        }
        break;

    case 1:
        if (blackFaces.size() == 0 && whiteFaces.size() == 0)
        {
            buildFacesOriginal(map);
            assignColorsNew1();
            qDebug() << "black=" << blackFaces.size() <<  "white=" << whiteFaces.size();
        }
        break;

    case 2:
        if (faceGroup.size() == 0)
        {
            buildFacesOriginal(map);
            buildFacesNew23();
            assignColorsNew2(whiteColorSet);
        }
        break;

    case 3:
        if (faceGroup.size() == 0)
        {
            buildFacesOriginal(map);
            buildFacesNew23();
            assignColorsNew3(colorGroup );
        }
        break;
    }
}

void Filled::draw(GeoGraphics * gg)
{
    if (!isVisible())
    {
        return;
    }


    switch (algorithm)
    {
    case 1:
    case 0:
        qDebug() << "Filled::draw() algorithm=" << algorithm;
        drawOriginal(gg);
        break;

    case 2:
        qDebug() << "Filled::draw() algorithm 2 :" << faceGroup.size() << faceGroup.totalSize();
        drawNew2(gg);
        break;

    case 3:
        qDebug() << "Filled::draw() algorithm 3 :" << faceGroup.size() << faceGroup.totalSize();
        drawNew3(gg);
        break;
    }
}

void Filled::drawOriginal(GeoGraphics * gg)
{
    if( whiteFaces.size() != 0 || blackFaces.size() != 0 )
    {
        qDebug()  << "black=" << blackFaces.size() << "white=" << whiteFaces.size();

        if (draw_outside_whites)
        {
            QColor color = whiteColorSet.getFirstColor().color;
            for (auto face : whiteFaces)
            {
                gg->fillEdgePoly(*face.get(), color);
                color = whiteColorSet.getNextColor().color;
            }
        }

        if (draw_inside_blacks)
        {
            QColor color = blackColorSet.getFirstColor().color;
            for (auto face : blackFaces)
            {
                gg->fillEdgePoly(*face.get(), color);
                color = blackColorSet.getNextColor().color;
            }
        }
    }
}

void Filled::drawNew2(GeoGraphics *gg)
{
    // not selected
    for (auto fset : faceGroup)
    {
        if (fset->tpcolor.hidden && !fset->selected)
            continue;

        QColor color = fset->tpcolor.color;
        if (fset->selected)
        {
            color = Qt::red;
        }

        for (auto face : *fset)
        {
            gg->fillEdgePoly(*face.get(),color);
        }
    }
}

void Filled::drawNew3(GeoGraphics *gg)
{
    qDebug() << "Filled::drawNew3";

    for (auto fset : faceGroup)
    {
        qDebug() << "FaceSet size:" << fset->size();
        if (fset->colorSet.isHidden() && !fset->selected)
            continue;

        ColorSet &  cset = fset ->colorSet;
        cset.resetIndex();

        for (auto face : *fset)
        {
            Q_ASSERT(face->isClockwise());
            TPColor tpc = cset.getNextColor();
            if (tpc.hidden && !fset->selected)
                continue;

            QColor color = tpc.color;
            if (fset->selected)
            {
                color = Qt::red;
            }
            gg->fillEdgePoly(*face.get(),color);
        }
    }
}
