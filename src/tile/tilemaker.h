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

#ifndef TILEMAKER_H
#define TILEMAKER_H

#include <QtCore>
#include "geometry/Transform.h"
#include "tile/PlacedFeature.h"

class Tiling;

class FillData
{
public:
    FillData();
    void set(int minX, int minY, int maxX, int maxY);
    void set(FillData & fdata);
    void get(int & minX, int & maxX, int & minY, int & maxY);

protected:
    int minX;
    int minY;
    int maxX;
    int maxY;
};

class TileMaker
{
public:
    TileMaker() {}

    void beginTiling( QString name );
    TilingPtr EndTiling();

    void setFillData(FillData fd);
    void setTranslations( QPointF t1, QPointF t2 );
    void beginPolygonFeature( int n );
    void addPoint( QPointF pt );
    void commitPolygonFeature();
    void addPlacement( Transform T );
    void endFeature() {}
    void beginRegularFeature(int n);
    void b_setDescription( QString t );
    void b_setAuthor( QString t );
    void getFill(QString txt);
    void addColors(QVector<TPColor> &colors);

private:
    QString        b_name;
    QString        b_desc;
    QString        b_author;
    QPointF        b_t1;
    QPointF        b_t2;
    QVector<PlacedFeaturePtr> b_pfs;
    QVector<QColor> b_colors;
    FeaturePtr     b_f;
    QPolygonF      b_pts;
    FillData       b_fillData;
};

#endif // TILEMAKER_H
