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

#ifndef PROTOTYPE_H
#define PROTOTYPE_H

#include "tile/Tiling.h"
#include "tile/Feature.h"
#include "tapp/Figure.h"
#include "tapp/DesignElement.h"
#include "geometry/FillRegion.h"

////////////////////////////////////////////////////////////////////////////
//
// Prototype.java
//
// The complete information needed to build a pattern: the tiling and
// a mapping from features to figures.  Prototype knows how to turn
// this information into a finished design, returned as a Map.

// DAC: The implementation requires 'figures' (now designElements) to be an ordered list.
// Taprats uses a hash based on an algorithm which presumably provided an order.
// Generally I use QMap to replace the hashes but its
// order would be randomly based on an address in memory, so instead I have
// created an array of the feature and figure pointers

class Prototype : public FillRegion
{
public:
    Prototype(TilingPtr t);
    virtual ~Prototype();

    MapPtr  getProtoMap();
    MapPtr  getExistingProtoMap() { return protoMap; }
    MapPtr  createProtoMap();
    void    resetProtoMap();

    void    setTiling(TilingPtr t);
    void    addElement(DesignElementPtr element);
    void    setFeaturesReversed(QList<FeaturePtr> features);

    QString getInfo() const;

    void    walk();

    TilingPtr         getTiling()   { return tiling; }
    QList<FeaturePtr> getFeatures();
    FeaturePtr        getFeature(FigurePtr figure );
    FigurePtr         getFigure(FeaturePtr feature );
    DesignElementPtr  getDesignElement(FeaturePtr feature);
    DesignElementPtr  getDesignElement(int index);
    QTransform        getTransform(int index);

    QVector<DesignElementPtr> &  getDesignElements() { return designElements; }
    QVector<QTransform>       &  getLocations()      { return locations; }

    void receive(GeoGraphics *gg, int h, int v );

    static int refs;

protected:

private:
    TilingPtr                   tiling;
    QVector<DesignElementPtr>   designElements;
    QVector<QTransform>         locations;
    MapPtr                      protoMap;
};
#endif

