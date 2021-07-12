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

#ifndef FIGURE_EDITOR_VIEW_H
#define FIGURE_EDITOR_VIEW_H

#include "base/layer.h"

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Figure>           FigurePtr;
typedef std::shared_ptr<class Prototype>        PrototypePtr;
typedef std::shared_ptr<class Border>           BorderPtr;
typedef std::shared_ptr<class Feature>          FeaturePtr;
typedef std::shared_ptr<class DCEL>             DCELPtr;
typedef std::shared_ptr<class Crop>             CropPtr;
typedef std::shared_ptr<class Circle>           CirclePtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;
typedef std::shared_ptr<class Style>            StylePtr;

class MapEditorView : public Layer
{
public:
    MapEditorView();

    virtual void    paint(QPainter * painter) override;
    virtual void    draw(QPainter *) = 0;

    void            unload();
    TilingPtr       getTiling()  { return tiling; }
    MapPtr          getMap()     { return map; }
    FigurePtr       getFigure()  { return figp; }
    DCELPtr         getDCEL()    { return dcel; }

    CropPtr         getCrop(){ return cropRect; }

    void            drawMap(QPainter * painer);
    void            drawDCEL(QPainter * painer);
    void            drawCropMap(QPainter * painer);
    void            drawFeature(QPainter * painter);
    void            drawBoundaries(QPainter * painter);
    void            drawPoints(QPainter * painter, QVector<class pointInfo> & points);
    void            drawConstructionLines(QPainter * painter);
    void            drawConstructionCircles(QPainter * painter);

    bool              hideConstructionLines;
    bool              hideMap;
    bool              hidePoints;
    bool              hideMidPoints;
    qreal             mapLineWidth;
    qreal             constructionLineWidth;

    QVector<QLineF>    constructionLines;
    QVector<CirclePtr> constructionCircles;

protected:
    StylePtr          styp;      // set
    PrototypePtr      prototype; // set
    DesignElementPtr  delp;      // set
    TilingPtr         tiling;    // set
    DCELPtr           dcel;      // set
    BorderPtr         border;    // set

    FigurePtr         figp;      // derived
    FeaturePtr        feap;      // derived

    MapPtr            map;       // derived
    CropPtr           cropRect;

    QTransform        viewT;
    QTransform        viewTinv;  // inverted

    qreal             selectionWidth;
};

#endif
