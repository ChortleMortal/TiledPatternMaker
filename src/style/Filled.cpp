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
#include "geometry/map_cleanser.h"
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
    white.clear();
    black.clear();
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
    eraseStyleMap();
    cm.reset();
    whiteColorSet.resetIndex();
    blackColorSet.resetIndex();
    colorGroup.resetIndex();
}


void Filled::createStyleRepresentation()
{
    if (!cm)
    {
        MapPtr map = getMap();

        MapCleanser cleanser(map);
        qDebug().noquote() << "Filled pre  map cleanse" << map->summary();
        if (cleanseLevel == 1)
        {
            cleanser.cleanse(divideupIntersectingEdges | badVertices_0 | badVertices_1);
        }
        else if (cleanseLevel == 2)
        {
            cleanser.cleanse(badVertices_0 | badVertices_1);
        }
        else if (cleanseLevel == 3)
        {
            cleanser.cleanse(divideupIntersectingEdges);
        }
        qDebug().noquote() << "Filled post map cleanse" << map->summary();

        DCELPtr dp = map->getDCEL();
        cm = make_shared<ColorMaker>(dp);
        config->colorMaker = WeakColorMakerPtr(cm);
    }

    switch (algorithm)
    {
    case 0:
        if (cm->getBlackFaces().size() == 0 && cm->getWhiteFaces().size() == 0)
        {
            cm->assignColorsOriginal();
        }
        break;

    case 1:
        if (cm->getBlackFaces().size() == 0 && cm->getWhiteFaces().size() == 0)
        {
            cm->assignColorsNew1();
        }
        break;

    case 2:
        if (cm->getFaceGroup().size() == 0)
        {
            cm->buildFaceGroups();
            cm->assignColorSets(whiteColorSet);
        }
        break;

    case 3:
        if (cm->getFaceGroup().size() == 0)
        {
            cm->buildFaceGroups();
            cm->assignColorGroups(colorGroup);
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

    if (!cm)
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
        qDebug() << "Filled::draw() algorithm 2 :" << cm->getFaceGroup().size() << cm->getFaceGroup().totalSize();
        drawDCELNew2(gg);
        break;

    case 3:
        qDebug() << "Filled::draw() algorithm 3 :" << cm->getFaceGroup().size() << cm->getFaceGroup().totalSize();
        drawDCELNew3(gg);
        break;
    }
}

void Filled::drawDCEL(GeoGraphics * gg)
{
    if( cm->getWhiteFaces().size() != 0 || cm->getBlackFaces().size() != 0 )
    {
        qDebug()  << "black=" << cm->getWhiteFaces().size() << "white=" <<  cm->getBlackFaces().size();

        if (draw_outside_whites)
        {
            QColor color = whiteColorSet.getFirstColor().color;
            for (auto & face : qAsConst(cm->getWhiteFaces()))
            {
                gg->fillEdgePoly(face.get(), color);
                color = whiteColorSet.getNextColor().color;
            }
        }

        if (draw_inside_blacks)
        {
            QColor color = blackColorSet.getFirstColor().color;
            for (auto & face : qAsConst(cm->getBlackFaces()))
            {
                gg->fillEdgePoly(face.get(), color);
                color = blackColorSet.getNextColor().color;
            }
        }
    }
}

void Filled::drawDCELNew2(GeoGraphics *gg)
{
    // not selected
    for (auto& fset : qAsConst(cm->getFaceGroup()))
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
    qDebug() << "Filled::drawNew3";

    for (auto& fset : qAsConst(cm->getFaceGroup()))
    {
        if (fset->colorSet.isHidden() && !fset->selected)
            continue;

        ColorSet &  cset = fset ->colorSet;
        cset.resetIndex();

        for (auto & face : qAsConst(*fset))
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
            gg->fillEdgePoly(face.get(),color);
        }
    }
}
