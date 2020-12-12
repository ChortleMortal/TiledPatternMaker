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

#include "base/frame_settings.h"
#include "geometry/transform.h"

FrameSettings::FrameSettings()
{
}

FrameSettings::FrameSettings(Bounds bounds, QSize size)
{
    _bounds      = bounds;
    _defaultSize = size;
    _definedSize = size;
    _activeSize  = size;

    calculateDefinedFrameTransform();
}

FrameSettings::FrameSettings(const FrameSettings & other)
{
    _bounds      = other._bounds;
    _defaultSize = other._defaultSize;
    _definedSize = other._definedSize;
    _activeSize  = other._activeSize;
    _definedTransform           = other._definedTransform;
}

FrameSettings & FrameSettings::operator=(const FrameSettings & other)
{
    _bounds      = other._bounds;
    _defaultSize = other._defaultSize;
    _definedSize = other._definedSize;
    _activeSize  = other._activeSize;
    _definedTransform           = other._definedTransform;
    return *this;
}

void FrameSettings::reInit()
{
    setDefinedFrameSize(_defaultSize);
    _activeSize = _defaultSize;
}

void FrameSettings::setDefinedFrameSize(QSize size)
{
    if (size != _definedSize)
    {
        _definedSize = size;
        calculateDefinedFrameTransform();
    }
}

void FrameSettings::setActiveFrameSize(QSize size)
{
    if (size != _activeSize)
    {
        _activeSize = size;
        calculateActiveFrameTransform();
    }
}

QSize FrameSettings::getActiveFrameSize() const
{
    return  _activeSize;
}

/*
    Results:
    VIEW_DESIGN         scale=75 rot=0 (0) trans=750 550
    VIEW_PROTO          scale=75 rot=0 (0) trans=750 550
    VIEW_PROTO_FEATURE  scale=75 rot=0 (0) trans=750 550
    VIEW_DEL            scale=75 rot=0 (0) trans=750 550
    VIEW_FIGURE_MAKER   scale=45 rot=0 (0) trans=450 450
    VIEW_TILING         scale=75 rot=0 (0) trans=750 550
    VIEW_TILIING_MAKER  scale=100 rot=0 (0)trans=500 500
    VIEW_MAP_EDITOR     scale=45 rot=0 (0) trans=450 450
*/

void FrameSettings::calculateDefinedFrameTransform()
{
    _definedTransform = calculateTransform(_definedSize);
}

void FrameSettings::calculateActiveFrameTransform()
{
    _activeTransform = calculateTransform(_activeSize);
}

QTransform FrameSettings::calculateTransform(QSize size)
{
    qreal w      = qreal(size.width());
    qreal h      = qreal(size.height());
    qreal aspect = w / h;
    qreal height = _bounds.width / aspect;
    qreal scalex = w /_bounds.width;

    QTransform first  = QTransform().translate(-_bounds.left, - (_bounds.top - height));
    QTransform second = QTransform().scale(scalex,scalex);
    QTransform third  = QTransform().translate(0.0,((w -h)/2.0));
    QTransform transf = first * second * third;
    return transf;
}




