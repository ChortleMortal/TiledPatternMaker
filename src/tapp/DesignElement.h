﻿/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
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

#ifndef DESIGN_ELEMENT_H
#define DESIGN_ELEMENT_H

#include "tapp/Figure.h"
#include "tile/Feature.h"
#include "base/shared.h"

////////////////////////////////////////////////////////////////////////////
//
// DesignElement.java
//
// A DesignElement is the core of the process of building a finished design.
// It's a Feature together with a Figure.  The Feature comes from the
// tile library and will be used to determine where to place copies of the
// Figure, which is designed by the user.

class DesignElement
{
public:

    DesignElement(FeaturePtr feature, FigurePtr figure );
    DesignElement(FeaturePtr feature );
    DesignElement(DesignElementPtr dep);
    DesignElement();
    ~DesignElement();

    FeaturePtr getFeature();
    FigurePtr  getFigure();
    void       setFigure(FigurePtr afigure);

    QString toString();

    static int refs;

protected:
    FeaturePtr	feature;
    FigurePtr	figure;
};

// added by DAC - not in taprats
class PlacedDesignElement : public DesignElement
{
public:
    PlacedDesignElement();
    PlacedDesignElement(DesignElementPtr del, Transform T);
    PlacedDesignElement(FigurePtr figp, FeaturePtr featp, Transform T);

    Transform   getTransform() { return trans; }

    QString toString();

protected:
    Transform           trans;
};
#endif

