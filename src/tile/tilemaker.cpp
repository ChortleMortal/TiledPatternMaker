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
#include "tilemaker.h"
#include "Tiling.h"

FillData::FillData()
{
    minX = -4;
    minY = -4;
    maxX = 4;
    maxY = 4;
}

void FillData::set(int minX, int maxX, int minY, int maxY)
{
    this->minX = minX;
    this->minY = minY;
    this->maxX = maxX;
    this->maxY = maxY;
}

void FillData::set(FillData & fdata)
{
    minX = fdata.minX;
    minY = fdata.minY;
    maxX = fdata.maxX;
    maxY = fdata.maxY;
}

void FillData::get(int & minX, int & maxX, int & minY, int & maxY)
{
    minX = this->minX;
    minY = this->minY;
    maxX = this->maxX;
    maxY = this->maxY;
}

void TileMaker::beginTiling( QString name )
{
    b_name = name;

    b_desc.clear();
    b_author.clear();
    b_pfs.clear();
    b_pts.clear();
    b_t1 = QPointF();
    b_t2 = QPointF();
    b_f  = nullptr;
}

TilingPtr TileMaker::EndTiling()
{
    TilingPtr tiling = make_shared<Tiling>(b_name, b_t1, b_t2);
    for( int idx = 0; idx < b_pfs.size(); ++idx )
    {
        tiling->add( b_pfs[idx]);
    }

    tiling->setDescription( b_desc );
    tiling->setAuthor( b_author );
    tiling->setFillData(b_fillData);
    //tiling->dump();
    return tiling;
}

void TileMaker::setTranslations( QPointF t1, QPointF t2 )
{
    b_t1 = t1;
    b_t2 = t2;
}

void TileMaker::setFillData(FillData fd)
{
    b_fillData = fd;
}

void TileMaker::beginPolygonFeature( int n )
{
    Q_UNUSED(n);
    b_pts.clear();
}

void TileMaker::addPoint( QPointF pt )
{
    b_pts << pt;
}

void TileMaker::commitPolygonFeature()
{
    b_f = make_shared<Feature>(b_pts);
}

void TileMaker::addPlacement( Transform T )
{
    b_pfs.push_back(make_shared<PlacedFeature>(b_f, T));
}

void TileMaker::beginRegularFeature(int n)
{
    b_f = make_shared<Feature>(n);
}

void TileMaker::b_setDescription( QString t )
{
    b_desc = t;
}

void TileMaker::b_setAuthor( QString t )
{
    b_author = t;
}

void TileMaker::addColors(QVector<TPColor> & colors)
{
    ColorSet & bkgds = b_f->getBkgdColors();
    bkgds.setColors(colors);
}

void TileMaker::getFill(QString txt)
{
    QStringList qsl;
    qsl = txt.split(',');
    b_fillData.set(qsl[0].toInt(),qsl[1].toInt(),qsl[2].toInt(),qsl[3].toInt());
}
