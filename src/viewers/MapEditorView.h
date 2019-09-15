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

#include "makers/mapselection.h"
#include "tile/PlacedFeature.h"
#include "base/layer.h"


enum eMapEdInput
{
    ME_INPUT_UNDEFINED,
    ME_INPUT_FIGURE,
    ME_INPUT_PROTO,
    ME_INPUT_STYLE
};

#define E2STR(x) #x

static QString sMapEdInput[] = {
    E2STR(ME_INPUT_UNDEFINED),
    E2STR(ME_INPUT_FIGURE),
    E2STR(ME_INPUT_PROTO),
    E2STR(ME_INPUT_STYLE)
};

class MapEditorView : public Layer
{
public:
    MapEditorView();

    virtual QRectF  boundingRect() const Q_DECL_OVERRIDE;
    virtual void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    virtual void    draw(QPainter *) = 0;

    void            unload();
    eMapEdInput     getInputMode() { return inputMode; }
    MapPtr          getMap()  { return map; }

    void            drawMap(QPainter * painer);
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
    eMapEdInput       inputMode;
    StylePtr          styp;     // set
    PrototypePtr      prop;     // set
    DesignElementPtr  delp;     // set
    FigurePtr         figp;     // derived
    FeaturePtr        feap;     // derived

    MapPtr             map;     // derived

    QTransform        viewT;
    QTransform        viewTinv; // inverted

    qreal             selectionWidth;
};

#endif
