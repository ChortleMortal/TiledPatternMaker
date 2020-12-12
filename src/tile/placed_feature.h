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

////////////////////////////////////////////////////////////////////////////
//
// PlacedFeature.java
//
// A PlacedFeature is a Feature together with a transform matrix.
// It allows us to share an underlying feature while putting several
// copies together into a tiling.  A tiling is represented as a
// collection of PlacedFeatures (that may share Features) that together
// make up a translational unit.

#ifndef PLACED
#define PLACED

#include "tile/feature.h"
#include "base/shared.h"
#include "base/pugixml.hpp"

class PlacedFeature
{
public:
    // Creation.
    PlacedFeature();
    PlacedFeature(FeaturePtr feature, QTransform T );
    PlacedFeature(PlacedFeaturePtr other);
    ~PlacedFeature() {}

    // Data.
    void             setTransform(QTransform newT);
    void             setFeature(FeaturePtr feature);
    FeaturePtr       getFeature();
    QTransform       getTransform();
    EdgePoly         getFeatureEdgePoly(){ return feature->getEdgePoly(); }
    QPolygonF        getFeaturePolygon() { return feature->getPolygon(); }
    EdgePoly         getPlacedEdgePoly();
    QPolygonF        getPlacedPolygon()  { return T.map(feature->getPolygon()); }

    bool saveAsGirihShape(QString name);
    bool loadFromGirihShape(QString name);

    bool isGirihShape() { return !girihShapeName.isEmpty(); }
    QString getGirihShapeName() { return girihShapeName; }

    bool show() { return _show; }
    void setShow(bool show) { _show = show; }

protected:
    void saveGirihShape(QTextStream & out, QString name);
    void loadGirihShape(pugi::xml_node & poly_node);
    void loadGirihShape(int sides, pugi::xml_node & poly_node);
    void loadGirihShapeOld(pugi::xml_node & poly_node);

private:
    QString      girihShapeName;
    FeaturePtr   feature;
    QTransform   T;
    bool         _show;  // used by tiling maker view
};
#endif
