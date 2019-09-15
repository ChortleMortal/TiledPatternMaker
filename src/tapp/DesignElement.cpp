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

#include "tapp/DesignElement.h"
#include "base/utilities.h"
#include "tapp/Rosette.h"
#include "tapp/ExplicitFigure.h"
#include "geometry/Transform.h"

int DesignElement::refs = 0;

////////////////////////////////////////////////////////////////////////////
//
// DesignElement.java
//
// A DesignElement is the core of the process of building a finished design.
// It's a Feature together with a Figure.  The Feature comes from the
// tile library and will be used to determine where to place copies of the
// Figure, which is designed by the user.

DesignElement::DesignElement( FeaturePtr feature, FigurePtr figure )
{
    this->feature = feature;
    this->figure = figure;
    refs++;
}

DesignElement::DesignElement( FeaturePtr feature )
{
    this->feature = feature;
    if( feature->isRegular() && (feature->numPoints() >= 4) )  // DAC was > 4
    {
        figure = make_shared<Rosette>(feature->numPoints(), 0.0, 3 );
    }
    else
    {
        figure = make_shared<ExplicitFigure>(make_shared<Map>(),FIG_TYPE_EXPLICIT);
    }
    refs++;
}

DesignElement::DesignElement(DesignElementPtr dep)
{
    feature = dep->feature;
    figure  = dep->figure;
    refs++;
}

DesignElement::DesignElement()
{
    refs++;
}

DesignElement::~DesignElement()
{
    refs--;
}

FeaturePtr DesignElement::getFeature()
{
    return feature;
}

FigurePtr DesignElement::getFigure()
{
    return figure;
}

void DesignElement::setFigure(FigurePtr afigure )
{
    qDebug() << "old=" << figure.get() << "new=" << afigure.get();
    figure = afigure;
}

QString DesignElement::toString()
{
    return QString("this=%1 feature=%2 figure=%3").arg(Utils::addr(this)).arg(Utils::addr(feature.get())).arg(Utils::addr(figure.get()));
}

/////////////////////////////////////////////////////////
///
///
/// /////////////////////////////////////////////////////

PlacedDesignElement::PlacedDesignElement()
{
}

PlacedDesignElement::PlacedDesignElement(DesignElementPtr del, QTransform T)
{
    feature = del->getFeature();
    figure  = del->getFigure();
    trans   = T;
}

PlacedDesignElement::PlacedDesignElement(FeaturePtr featp, FigurePtr figp, QTransform T)
{
    feature = featp;
    figure  = figp;
    trans   = T;
}

QString PlacedDesignElement::toString()
{
    return QString("feature=%1 figure=%2 T=%3").arg(Utils::addr(feature.get())).arg(Utils::addr(figure.get())).arg(Transform::toInfoString(trans));
}
