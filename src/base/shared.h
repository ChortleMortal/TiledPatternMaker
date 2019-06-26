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

#ifndef SHARED_H
#define SHARED_H

#include <memory>

#undef  EXPLICIT_DESTRUCTOR

#define  WARN_BAD
//#define  CRITICAL_BAD

#ifdef WARN_BAD
    #define badness  qWarning
#else
    #define badness  qCritical
#endif

using std::shared_ptr;
using std::weak_ptr;
using std::make_shared;

class Prototype;
class Vertex;
class Edge;
class Neighbour;
class QPolygonF;
class Feature;
class Figure;
class ExplicitFigure;
class Star;
class ExtendedStar;
class Rosette;
class StarConnectFigure;
class RosetteConnectFigure;
class ExtendedRosette;
class RadialFigure;
class Map;
class PlacedFeature;
class GeoLayer;
class Style;
class Tiling;
class Infer;
class DesignElement;
class PlacedDesignElement;
class FigureView;

class Design;
class Pattern;
class Border;
class ShapeFactory;
class FeatureButton;

typedef shared_ptr<Style>           StylePtr;
typedef shared_ptr<Tiling>          TilingPtr;
typedef shared_ptr<GeoLayer>        GeoLayerPtr;
typedef shared_ptr<Vertex>          VertexPtr;
typedef shared_ptr<Map>             MapPtr;
typedef shared_ptr<Prototype>       PrototypePtr;
typedef shared_ptr<Edge>            EdgePtr;
typedef shared_ptr<Neighbour>       NeighbourPtr;
typedef shared_ptr<Feature>         FeaturePtr;
typedef shared_ptr<Figure>          FigurePtr;
typedef shared_ptr<ExplicitFigure>  ExplicitPtr;
typedef shared_ptr<Star>            StarPtr;
typedef shared_ptr<Rosette>         RosettePtr;
typedef shared_ptr<StarConnectFigure>    StarConnectPtr;
typedef shared_ptr<RosetteConnectFigure> RosetteConnectPtr;
typedef shared_ptr<ExtendedRosette> ExtRosettePtr;
typedef shared_ptr<ExtendedStar>    ExtStarPtr;
typedef shared_ptr<RadialFigure>    RadialPtr;
typedef shared_ptr<PlacedFeature>   PlacedFeaturePtr;
typedef shared_ptr<QPolygonF>       PolyPtr;
typedef shared_ptr<Infer>           InferPtr;
typedef shared_ptr<DesignElement>   DesignElementPtr;
typedef shared_ptr<PlacedDesignElement> PlacedDesignElementPtr;

typedef shared_ptr<Design>          DesignPtr;
typedef shared_ptr<Pattern>         PatternPtr;
typedef shared_ptr<Border>          BorderPtr;
typedef shared_ptr<ShapeFactory>    ShapeFPtr;
typedef shared_ptr<FigureView>      FigureViewPtr;
typedef shared_ptr<FeatureButton>   FeatureBtnPtr;

typedef weak_ptr<Style>           WeakStylePtr;
typedef weak_ptr<Tiling>          WeakTilingPtr;
typedef weak_ptr<Vertex>          WeakVertexPtr;
typedef weak_ptr<Map>             WeakMapPtr;
typedef weak_ptr<Prototype>       WeakPrototypePtr;
typedef weak_ptr<Edge>            WeakEdgePtr;
typedef weak_ptr<Neighbour>       WeakNeighbourPtr;
typedef weak_ptr<Feature>         WeakFeaturePtr;
typedef weak_ptr<Figure>          WeakFigurePtr;
typedef weak_ptr<ExplicitFigure>  WeakExplicitPtr;
typedef weak_ptr<Star>            WeakStarPtr;
typedef weak_ptr<Rosette>         WeakRosettePtr;
typedef weak_ptr<StarConnectFigure>    WeakStarConnectPtr;
typedef weak_ptr<RosetteConnectFigure> WeakRosetteConnectPtr;
typedef weak_ptr<RadialFigure>    WeakRadialPtr;
typedef weak_ptr<PlacedFeature>   WeakPlacedFeaturePtr;
typedef weak_ptr<QPolygonF>       WeakPolyPtr;
typedef weak_ptr<Design>          WeakDesignPtr;
typedef weak_ptr<DesignElement>   WeakDesignElementPtr;
typedef weak_ptr<FeatureButton>   WeakFeatureBtnPtr;

typedef shared_ptr<const Map>     constMapPtr;

#endif // SHARED_H
