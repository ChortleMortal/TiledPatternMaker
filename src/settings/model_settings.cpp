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

#include "settings/model_settings.h"
#include "tile/backgroundimage.h"

ModelSettings::ModelSettings()
{
    _size       = QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT);   // default
    _zsize      = QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT);   // default
    _bkgdColor  = QColor(Qt::white);
    _startTile  = QPoint(0,0);
}

ModelSettings::ModelSettings(const ModelSettings & other)
{
    _size      = other._size;
    _zsize     = other._zsize;
    _bkgdColor = other._bkgdColor;
    _startTile = other._startTile;
    _fillData  = other._fillData;
}

ModelSettings::~ModelSettings()
{}

void ModelSettings::clear()
{
    _size        = QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT);
    _zsize       = QSize(DEFAULT_WIDTH,DEFAULT_HEIGHT);
    _bkgdColor   = QColor(Qt::black);
    _startTile   = QPoint(0,0);
}

void ModelSettings::setSize(QSize size)
{
    _size = size;
}

QPointF ModelSettings::getStartTile()
{
    return _startTile;
}

void ModelSettings::setStartTile(QPointF pt)
{
    _startTile = pt;
}

QColor ModelSettings::getBackgroundColor()
{
    return _bkgdColor;
}

void  ModelSettings::setBackgroundColor(QColor color)
{
    qDebug() << "CanvasSettings::setBackgroundColor()"  << color.name();
    _bkgdColor = color;
}

QPointF ModelSettings::getCenter()
{
    QRectF r(QPoint(0,0),_size);
    return r.center();
}

void  ModelSettings::setFillData(FillData & fd)
{
    _fillData = fd;
}

FillData & ModelSettings::getFillData()
{
    return _fillData;
}




