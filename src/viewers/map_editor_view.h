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

#include "makers/map_editor/map_selection.h"
#include "tile/placed_feature.h"
#include "base/layer.h"

enum eMapType
{
    MAP_TYPE_UNDEFINED,
    MAP_TYPE_FIGURE,
    MAP_TYPE_PROTO,
    MAP_TYPE_STYLE,
    MAP_TYPE_LOCAL,
    MAP_TYPE_TILING,
    MAP_TYPE_DCEL
};

#define E2STR(x) #x

static QString sMapEdInput[] = {
    E2STR(MAP_TYPE_UNDEFINED),
    E2STR(MAP_TYPE_FIGURE),
    E2STR(MAP_TYPE_PROTO),
    E2STR(MAP_TYPE_STYLE),
    E2STR(MAP_TYPE_LOCAL),
    E2STR(MAP_TYPE_TILING),
    E2STR(MAP_TYPE_DCEL)
};

class MapEditorView : public Layer
{
public:
    MapEditorView();

    virtual void    paint(QPainter * painter) override;
    virtual void    draw(QPainter *) = 0;

    void            unload();
    eMapType        getMapType() { return mapType; }
    MapPtr          getMap()     { return map; }
    FigurePtr       getFigure()  { return figp; }
    WeakDCELPtr     getDCEL()    { return dcel; }

    QRectF          getCropRect(){ return cropRect; }
    void            setCropRect(QRectF & rect) { cropRect = rect; }

    void            drawMap(QPainter * painer);
    void            drawDCEL(QPainter * painer);
    void            drawCropMap(QPainter * painer);
    void            drawFeature(QPainter * painter);
    void            drawBoundaries(QPainter * painter);
    void            drawPoints(QPainter * painter, QVector<pointInfo> & points);
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
    eMapType          mapType;
    StylePtr          styp;     // set
    PrototypePtr      prop;     // set
    DesignElementPtr  delp;     // set
    TilingPtr         tiling;   // set
    WeakDCELPtr       dcel;     // set

    FigurePtr         figp;     // derived
    FeaturePtr        feap;     // derived

    MapPtr             map;     // derived
    QRectF             cropRect;

    QTransform        viewT;
    QTransform        viewTinv; // inverted

    qreal             selectionWidth;
};

#endif
