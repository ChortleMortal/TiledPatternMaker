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

#include "base/workspace_settings.h"
#include "base/utilities.h"

WorkspaceSettings::WorkspaceSettings()
{
    _size = QSize(1500,1100);   // default
    _bkgdColor  = QColor(Qt::white);
    _startTile  = QPoint(0,0);
}

WorkspaceSettings::WorkspaceSettings(const WorkspaceSettings & other)
{
    _size = other._size;
    _bkgdColor = other._bkgdColor;
    _startTile = other._startTile;
    _bkgdImage = other._bkgdImage;
    _border    = other._border;
    _fillData  = other._fillData;
}

WorkspaceSettings::~WorkspaceSettings()
{}



void WorkspaceSettings::clear()
{
    _size  = QSize(1500,1100);
    _bkgdColor   = QColor(Qt::black);
    _startTile   = QPoint(0,0);
    _border.reset();
    _bkgdImage.reset();
}

void WorkspaceSettings::setBorder(BorderPtr border)
{
    qDebug() << "CanvasSettings::setBorder" << Utils::addr(border.get());
    _border = border;
}

void WorkspaceSettings::setSize(QSize size)
{
    _size = size;
}

QPointF WorkspaceSettings::getStartTile()
{
    return _startTile;
}

void WorkspaceSettings::setStartTile(QPointF pt)
{
    _startTile = pt;
}

QColor WorkspaceSettings::getBackgroundColor()
{
    return _bkgdColor;
}

void  WorkspaceSettings::setBackgroundColor(QColor color)
{
    qDebug() << "CanvasSettings::setBackgroundColor()"  << color.name();
    _bkgdColor = color;
}

QPointF WorkspaceSettings::getCenter()
{
    QRectF r(QPoint(0,0),_size);
    return r.center();
}

void  WorkspaceSettings::setFillData(FillData & fd)
{
    _fillData = fd;
}

FillData & WorkspaceSettings::getFillData()
{
    return _fillData;
}





