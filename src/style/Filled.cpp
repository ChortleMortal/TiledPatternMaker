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
#include "base/configuration.h"
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

Filled::Filled(PrototypePtr proto, int algorithm ) : Style(proto)
{
    draw_inside_blacks    = true;
    draw_outside_whites   = true;
    this->algorithm       = algorithm;
    cleanseLevel          = 0;
}

Filled::Filled(StylePtr other) : Style(other)
{
    shared_ptr<Filled> filled = std::dynamic_pointer_cast<Filled>(other);
    if (filled)
    {
        draw_inside_blacks    = filled->draw_inside_blacks;
        draw_outside_whites   = filled->draw_outside_whites;
        whiteColorSet         = filled->whiteColorSet;
        blackColorSet         = filled->blackColorSet;
        algorithm             = filled->algorithm;
        cleanseLevel          = filled->cleanseLevel;
    }
    else
    {
        draw_inside_blacks    = true;
        draw_outside_whites   = true;
        algorithm             = 0;
        cleanseLevel          = 0;
    }
}

Filled::~Filled()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting filled";
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

void  Filled::setCleanseLevel(int val)
{
    cleanseLevel = val;
    resetStyleRepresentation();
}

// Style overrides.

void Filled::resetStyleRepresentation()
{
    blackFaces.clear();
    whiteFaces.clear();
    faceGroup.clear();
    dcel.reset();
    eraseStyleMap();
    whiteColorSet.resetIndex();
    blackColorSet.resetIndex();
    colorGroup.resetIndex();
}


void Filled::createStyleRepresentation()
{
    if (!dcel)
    {
        MapPtr map = getMap();

        qDebug().noquote() << "Filled pre  map cleanse" << map->summary();
        if (cleanseLevel == 1)
        {
            map->cleanse(divideupIntersectingEdges | badVertices_0 | badVertices_1);
        }
        else if (cleanseLevel == 2)
        {
            map->cleanse(badVertices_0 | badVertices_1);
        }
        else if (cleanseLevel == 3)
        {
            map->cleanse(divideupIntersectingEdges);
        }
        qDebug().noquote() << "Filled post map cleanse" << map->summary();

        dcel = map->getDCEL();
    }

    Q_ASSERT(dcel);
    switch (algorithm)
    {
    case 0:
        if (blackFaces.size() == 0 && whiteFaces.size() == 0)
        {
            assignColorsOriginal();
        }
        break;

    case 1:
        if (blackFaces.size() == 0 && whiteFaces.size() == 0)
        {
            assignColorsNew1();
        }
        break;

    case 2:
        if (faceGroup.size() == 0)
        {
            buildFaceGroups();
            assignColorSets(whiteColorSet);
        }
        break;

    case 3:
        if (faceGroup.size() == 0)
        {
            buildFaceGroups();
            assignColorGroups(colorGroup);
        }
        break;
    }
}

void Filled::updateStyleRepresentation()
{
    Q_ASSERT(dcel);
    switch (algorithm)
    {
    case 0:
        assignColorsOriginal();
        break;

    case 1:
        assignColorsNew1();
        break;

    case 2:
        assignColorSets(whiteColorSet);
        break;

    case 3:
        assignColorGroups(colorGroup);
        break;
    }
}

void Filled::draw(GeoGraphics * gg)
{
    if (!isVisible() || !dcel)
    {
        return;
    }

    switch (algorithm)
    {
    case 0:
        qDebug() << "Filled::draw() algorithm=" << algorithm;
        drawDCEL(gg);
        break;

    case 1:
        qDebug() << "Filled::draw() algorithm=" << algorithm;
        drawDCEL(gg);
        break;

    case 2:
        qDebug() << "Filled::draw() algorithm 2 :" << faceGroup.size() << faceGroup.totalSize();
        drawDCELNew2(gg);
        break;

    case 3:
        qDebug() << "Filled::draw() algorithm 3 :" << faceGroup.size() << faceGroup.totalSize();
        drawDCELNew3(gg);
        break;
    }
}

void Filled::drawDCEL(GeoGraphics * gg)
{
    if (draw_outside_whites)
    {
        QColor color = whiteColorSet.getFirstColor().color;
        for (auto & face : qAsConst(whiteFaces))
        {
            gg->fillEdgePoly(face.get(), color);
            color = whiteColorSet.getNextColor().color;
        }
    }

    if (draw_inside_blacks)
    {
        QColor color = blackColorSet.getFirstColor().color;
        for (auto & face : qAsConst(blackFaces))
        {
            gg->fillEdgePoly(face.get(), color);
            color = blackColorSet.getNextColor().color;
        }
    }
}

void Filled::drawDCELNew2(GeoGraphics *gg)
{
    // not selected
    for (auto& fset : qAsConst(faceGroup))
    {
        if (fset->tpcolor.hidden && !fset->selected)
            continue;

        QColor color = fset->tpcolor.color;
        if (fset->selected)
        {
            color = Qt::red;
        }

        for (auto & face : qAsConst(*fset))
        {
            gg->fillEdgePoly(face.get(),color);
        }
    }
}

void Filled::drawDCELNew3(GeoGraphics *gg)
{
    for (FaceSetPtr & fset : faceGroup)
    {
        if (fset->selected)
        {
            for (FacePtr & face : *fset)
            {
                gg->fillEdgePoly(face.get(),Qt::red);
            }
            continue;
        }

        ColorSet *  cset = fset->pColorSet;

        if (cset->isHidden())
        {
            continue;
        }

        cset->resetIndex();

        for (FacePtr & face : *fset)
        {
            Q_ASSERT(face->isClockwise());
            TPColor tpc = cset->getNextColor();
            if (tpc.hidden)
            {

                continue;
            }
            QColor color = tpc.color;
            gg->fillEdgePoly(face.get(),color);
        }
    }
}
