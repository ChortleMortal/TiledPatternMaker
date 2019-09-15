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

#ifndef TILING_LOADER_H
#define TILING_LOADER_H

#include <QtCore>
#include "geometry/Transform.h"
#include "tile/PlacedFeature.h"
#include "base/pugixml.hpp"

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

class TilingLoader
{
public:
    TilingLoader() {}

    TilingPtr   readTiling(QString file);
    TilingPtr   readTiling(QTextStream & st);
    TilingPtr   readTilingXML(QString file);
    TilingPtr   readTilingXML(pugi::xml_node & tiling_node);

    void        beginTiling( QString name );
    TilingPtr   endTiling();

    void        setFillData(FillData fd);
    void        setTranslations( QPointF t1, QPointF t2 );
    void        beginRegularFeature(int n);
    void        addPlacement(QTransform T );
    void        endFeature() {}
    void        b_setDescription( QString t );
    void        b_setAuthor( QString t );

protected:
    void        beginPolygonFeature( int n );
    void        addPoint( QPointF pt );
    void        commitPolygonFeature();
    void        commitEdgePolyFeature(EdgePoly & ep);
    void        commitGirihShapeFeature(QString name);
    void        getFill(QString txt);
    void        addColors(QVector<TPColor> &colors);

    QString     readQuotedString(QTextStream &str);
    QPointF     getPoint(QString txt);
    Transform   getTransform(QString txt);
    QTransform  getQTransform(QString txt);

private:
    int            version;
    QString        b_name;
    QString        b_desc;
    QString        b_author;
    QPointF        b_t1;
    QPointF        b_t2;
    QVector<PlacedFeaturePtr> b_pfs;
    FeaturePtr     b_f;
    QPolygonF      b_pts;

    QVector<QColor> b_colors;
    FillData       b_fillData;

    QString        b_bkName;
    qreal          b_bkScale;
    qreal          b_bkRot;
    qreal          b_bkX;
    qreal          b_bkY;
    QTransform     b_bkgdAdjust;
};

#endif
