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

#include "style/Filled.h"
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

Filled::Filled(const Style *other ) : Style(other)
{
    const Filled * filled = dynamic_cast<const Filled*>(other);
    if (filled)
    {
        draw_inside_blacks    = filled->draw_inside_blacks;
        draw_outside_whites   = filled->draw_outside_whites;
        whiteColorSet         = filled->whiteColorSet;
        blackColorSet         = filled->blackColorSet;
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
        return;
    }

    setupStyleMap();

    getReadOnlyMap()->verify("Filled",false,true);

    MapPtr map = getMap();
    purifyMap(map);

    if (algorithm == 3)
    {
        if (faceGroup.size() == 0)
        {
            extractFacesNew23(map);
            assignColorsNew3(colorGroup );
        }
    }
    else if (algorithm == 2)
    {
        if (faceGroup.size() == 0)
        {
            extractFacesNew23(map);
            assignColorsNew2(whiteColorSet);
        }
    }
    else if (algorithm == 1)
    {
        if (blackFaces.size() == 0 && whiteFaces.size() == 0)
        {
            extractFacesNew1(map);
            qDebug() << "black=" << blackFaces.size() <<  "white=" << whiteFaces.size();
        }
    }
    else
    {
        Q_ASSERT(algorithm == 0);
        if (blackFaces.size() == 0 && whiteFaces.size() == 0)
        {
            extractFaces(map);
            qDebug() << "black=" << blackFaces.size() <<  "white=" << whiteFaces.size();
        }
    }

    purifyFaces();

}

void Filled::draw(GeoGraphics * gg)
{
    if (!isVisible())
    {
        return;
    }

    qDebug() << "Filled::draw()  algorithm=" << algorithm;
    switch (algorithm)
    {
    case 3:
        drawNew3(gg);
        break;
    case 2:
        drawNew2(gg);
        break;
    case 1:
    case 0:
        drawOriginal(gg);
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

void Filled::drawOriginal(GeoGraphics * gg)
{
    if( whiteFaces.size() != 0 || blackFaces.size() != 0 )
    {
        qDebug()  << "black=" << blackFaces.size() << "white=" << whiteFaces.size();
        gg->pushAndCompose(*getLayerTransform());

        if (draw_outside_whites)
        {
            QColor color = whiteColorSet.getFirstColor().color;
            for(int i=0; i < whiteFaces.size(); i++ )
            {
                FacePtr fp = whiteFaces[i];
                gg->setColor(color);
                gg->drawPolygon(*fp->getPolygon(),true);

                color = whiteColorSet.getNextColor().color;
            }
        }

        if (draw_inside_blacks)
        {
            QColor color = blackColorSet.getFirstColor().color;
            for(int i=0; i < blackFaces.size(); i++)
            {
                FacePtr fp = blackFaces[i];
                gg->setColor(color);
                gg->drawPolygon(*fp->getPolygon(),true);

                color = blackColorSet.getNextColor().color;
            }
        }
        gg->pop();
    }
}

void Filled::drawNew2(GeoGraphics *gg)
{
    gg->pushAndCompose(*getLayerTransform());

    // not selected
    for (auto it = faceGroup.begin(); it != faceGroup.end(); it++)
    {
        FaceSetPtr fsp = *it;
        if (fsp->selected || fsp->tpcolor.hidden)
        {
            continue;
        }

        TPColor tpcolor = fsp->tpcolor;
        for (auto it2 = fsp->begin(); it2 != fsp->end(); it2++)
        {
            FacePtr fp = *it2;
            gg->setColor(tpcolor.color);
            gg->drawPolygon(*fp->getPolygon(),true);
        }
    }

    // selected
    for (auto it = faceGroup.begin(); it != faceGroup.end(); it++)
    {
        FaceSetPtr fsp = *it;
        if (!fsp->selected)
        {
            continue;
        }

        for (auto it2 = fsp->begin(); it2 != fsp->end(); it2++)
        {
            FacePtr fp = *it2;
            gg->setColor(Qt::red);
            gg->drawPolygon(*fp->getPolygon(),false);
        }
    }

    gg->pop();
}

void Filled::drawNew3(GeoGraphics *gg)
{
    gg->pushAndCompose(*getLayerTransform());

    // not selected
    for (auto it = faceGroup.begin(); it != faceGroup.end(); it++)
    {
        FaceSetPtr fsp = *it;

        if (fsp->selected || fsp->tpcolor.hidden)
        {
            continue;
        }

        ColorSet &  cset = fsp->colorSet;
        if (cset.isHidden())
        {
            continue;
        }
        cset.resetIndex();
        for (auto it2 = fsp->begin(); it2 != fsp->end(); it2++)
        {
            TPColor tpcolor = cset.getNextColor();
            FacePtr fp = *it2;
            gg->setColor(tpcolor.color);
            gg->drawPolygon(*fp->getPolygon(),true);
        }
    }

    // selected
    for (auto it = faceGroup.begin(); it != faceGroup.end(); it++)
    {
        FaceSetPtr fsp = *it;
        if (!fsp->selected)
        {
            continue;
        }

        for (auto it2 = fsp->begin(); it2 != fsp->end(); it2++)
        {
            FacePtr fp = *it2;
            gg->setColor(Qt::red);
            gg->drawPolygon(*fp->getPolygon(),false);
        }
    }

    gg->pop();
}
