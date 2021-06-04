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
#include <QtGlobal>

#undef  EXPLICIT_DESTRUCTOR

#define  WARN_BAD
//#define  CRITICAL_BAD

#ifdef WARN_BAD
    #define badness  qWarning
#else
    #define badness  qCritical
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
#include <QTextStream>
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
using Qt::endl;
#else
#define endl Qt::endl
#endif
#endif

#define GOLDEN_RATIO  1.61803398874989484820

#define DAC_DEPRECATED QT_DEPRECATED

extern class TiledPatternMaker * theApp;

using std::shared_ptr;
using std::weak_ptr;
using std::make_shared;

class BackgroundImage;
class Border;
class ColorMaker;
class Design;
class DesignElement;
class DCEL;
class Edge;
class ExplicitFigure;
class ExtendedRosette;
class ExtendedStar;
class Face;
class FaceSet;
class Faces;
class Feature;
class FeatureButton;
class Figure;
class FigureView;
class Filled;
class Grid;
class Infer;
class Layer;
class Map;
class MapEditor;
class MarkX;
class ModelSettings;
class Mosaic;
class Neighbours;
class Pattern;
class PlacedDesignElement;
class PlacedFeature;
class Prototype;
class PrototypeView;
class QPolygonF;
class RadialFigure;
class Rosette;
class RosetteConnectFigure;
class ShapeFactory;
class Star;
class StarConnectFigure;
class Style;
class StyleEditor;
class Thick;
class Thread;
class Tiling;
class TilingMaker;
class TilingSelector;
class TilingView;
class Vertex;

typedef shared_ptr<BackgroundImage> BkgdImgPtr;
typedef shared_ptr<Border>          BorderPtr;
typedef shared_ptr<ColorMaker>      ColorMakerPtr;
typedef shared_ptr<Design>          DesignPtr;
typedef shared_ptr<DesignElement>   DesignElementPtr;
typedef shared_ptr<DCEL>            DCELPtr;
typedef shared_ptr<Edge>            EdgePtr;
typedef shared_ptr<ExplicitFigure>  ExplicitPtr;
typedef shared_ptr<ExtendedRosette> ExtRosettePtr;
typedef shared_ptr<ExtendedStar>    ExtStarPtr;
typedef shared_ptr<Face>            FacePtr;
typedef shared_ptr<FaceSet>         FaceSetPtr;
typedef shared_ptr<Faces>           FacesPtr;
typedef shared_ptr<Feature>         FeaturePtr;
typedef shared_ptr<FeatureButton>   FeatureBtnPtr;
typedef shared_ptr<Figure>          FigurePtr;
typedef shared_ptr<Filled>          FilledPtr;
typedef shared_ptr<FigureView>      FigureViewPtr;
typedef shared_ptr<Grid>            GridPtr;
typedef shared_ptr<Infer>           InferPtr;
typedef shared_ptr<Layer>           LayerPtr;
typedef shared_ptr<Map>             MapPtr;
typedef shared_ptr<MapEditor>       MapEditorPtr;
typedef shared_ptr<MarkX>           MarkXPtr;
typedef shared_ptr<ModelSettings>   ModelSettingsPtr;
typedef shared_ptr<Mosaic>          MosaicPtr;
typedef shared_ptr<Neighbours>      NeighboursPtr;
typedef shared_ptr<Pattern>         PatternPtr;
typedef shared_ptr<PlacedDesignElement> PlacedDesignElementPtr;
typedef shared_ptr<PlacedFeature>   PlacedFeaturePtr;
typedef shared_ptr<Prototype>       PrototypePtr;
typedef shared_ptr<PrototypeView>   PrototypeViewPtr;
typedef shared_ptr<QPolygonF>       PolyPtr;
typedef shared_ptr<RadialFigure>    RadialPtr;
typedef shared_ptr<Rosette>         RosettePtr;
typedef shared_ptr<RosetteConnectFigure> RosetteConnectPtr;
typedef shared_ptr<ShapeFactory>    ShapeFPtr;
typedef shared_ptr<Star>            StarPtr;
typedef shared_ptr<StarConnectFigure>    StarConnectPtr;
typedef shared_ptr<Style>           StylePtr;
typedef shared_ptr<StyleEditor>     StyleEditorPtr;
typedef shared_ptr<Thick>           ThickPtr;
typedef shared_ptr<Thread>          ThreadPtr;
typedef shared_ptr<Tiling>          TilingPtr;
typedef shared_ptr<TilingMaker>     TilingMakerPtr;
typedef shared_ptr<TilingSelector>  TilingSelectorPtr;
typedef shared_ptr<TilingView>      TilingViewPtr;
typedef shared_ptr<Vertex>          VertexPtr;
typedef shared_ptr<const Map>       constMapPtr;

typedef weak_ptr<ColorMaker>      WeakColorMakerPtr;
typedef weak_ptr<DCEL>            WeakDCELPtr;
typedef weak_ptr<DesignElement>   WeakDesignElementPtr;
typedef weak_ptr<Face>            WeakFacePtr;
typedef weak_ptr<Feature>         WeakFeaturePtr;
typedef weak_ptr<FeatureButton>   WeakFeatureBtnPtr;
typedef weak_ptr<Layer>           WeakLayerPtr;
typedef weak_ptr<Mosaic>          WeakMosaicPtr;
typedef weak_ptr<PlacedFeature>   WeakPlacedFeaturePtr;
typedef weak_ptr<Prototype>       WeakPrototypePtr;
typedef weak_ptr<Style>           WeakStylePtr;
typedef weak_ptr<Tiling>          WeakTilingPtr;
typedef weak_ptr<Edge>            WeakEdgePtr;
typedef weak_ptr<Vertex>          WeakVertexPtr;

class Tristate
{
public:

    enum eTristate
    {
        False,
        True,
        Unknown
    };

    Tristate() { state = Unknown; }

    void        set(bool b) { if (b) state = True; else state = False; }
    void        reset() { state = Unknown; }
    eTristate   get() { return state; }

private:
    eTristate state;
};

#endif // SHARED_H
