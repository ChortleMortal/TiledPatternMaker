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

#include "base/canvasSettings.h"
#include "base/border.h"
#include "base/configuration.h"
#include "geometry/Loose.h"

CanvasSettings::CanvasSettings()
{
    init();
}

CanvasSettings::~CanvasSettings()
{
}

void CanvasSettings::init()
{
    _sceneSize  = QSizeF(1500.0,1100.0);
    _startTile  = QPointF(0.0,0.0);
    _bkgdColor  = QColor(Qt::black);
    _scale      = 1.0;
    _diameter   = 400;

   Configuration * config = Configuration::getInstance();
   if (config->autoClear)
   {
       if (_border)
       {
           _border.reset();
       }
   }
}

bool CanvasSettings::isDifferent(CanvasSettings & other)
{
    if (_sceneSize != other.getSizeF())
        return true;
    if (_bkgdColor != other.getBackgroundColor())
        return true;
    if (!Loose::equals(_scale,other.getScale()))
        return true;
    if (_border != other.getBorder())
        return true;
    if (!Loose::equals(_diameter,other.getDiameter()))
        return true;
    if (_startTile != other.getStartTile())
        return true;
    return false;
}

void CanvasSettings::set(CanvasSettings & other)
{
    _sceneSize  = other.getSizeF();
    _bkgdColor  = other.getBackgroundColor();
    _scale      = other.getScale();
    _border     = other.getBorder();
    _diameter   = other.getDiameter();
    _startTile  = other.getStartTile();
}

void CanvasSettings::setBorder(BorderPtr border)
{
    qDebug() << "DesignIn::setBorder" << (void*)border.get();
    _border = border;
}

void CanvasSettings::setSizeF(QSizeF size)
{
    _sceneSize = size;
}

QPointF CanvasSettings::getStartTile()
{
    return _startTile;
}

void CanvasSettings::setStartTile(QPointF pt)
{
    _startTile = pt;
}

QColor CanvasSettings::getBackgroundColor()
{
    return _bkgdColor;
}

void  CanvasSettings::setBackgroundColor(QColor color)
{
    qDebug() << "DesignInfo::setBackgroundColor()"  << color.name();
    _bkgdColor = color;
}
