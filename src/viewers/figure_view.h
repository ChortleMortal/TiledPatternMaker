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

#ifndef FIGUREVIEW_H
#define FIGUREVIEW_H

#include "base/shared.h"
#include "base/layer.h"
#include "base/canvas.h"
#include "viewers/geo_graphics.h"

class FigureView : public Layer
{
public:
    FigureView(DesignElementPtr dep);
    virtual ~FigureView() override;

    virtual void   paint(QPainter *painter) override;
    virtual void   receive(GeoGraphics * gg, int h, int v );
    virtual void   draw( GeoGraphics * gg );

protected:
    void paintExplicitFigureMap(QPainter *painter, QPen pen);
    void paintRadialFigureMap(QPainter *painter, QPen pen);
    void paintFeatureBoundary(QPainter *painter);
    void paintRadialFigureBoundary(QPainter *painter);
    void paintExtendedBoundary(QPainter *painter);

private:
    void paintMap(QPainter * painter, MapPtr map, QPen pen);

    DesignElementPtr _dep;
    FigurePtr        _fig;
    FeaturePtr       _feat;
    QTransform       _T;
};

#endif // FIGUREVIEW_H
