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
    _evt        = VIEW_UNDEFINED;
    _bounds     = Bounds(-10.0,10.0,20.0);  // default
    _size       = QSize(1500,1100);
    _activeSize = _size;

    calculateFrameTransform();
}

void FrameSettings::init(eViewType evt, Bounds bounds, QSize size)
{
    _evt        = evt;
    _bounds     = bounds;
    _size       = size;
    _activeSize = _size;

    calculateFrameTransform();
}

void FrameSettings::setFrameSize(QSize size)
{
    if (size != _size)
    {
        _size = size;
        calculateFrameTransform();
    }
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
void FrameSettings::calculateFrameTransform()
{
    QSize size   = _size;
    qreal w      = qreal(size.width());
    qreal h      = qreal(size.height());
    qreal aspect = w / h;
    qreal height = _bounds.width / aspect;
    qreal scalex = w /_bounds.width;

    QTransform first  = QTransform().translate(-_bounds.left, - (_bounds.top - height));
    QTransform second = QTransform().scale(scalex,scalex);
    QTransform third  = QTransform().translate(0.0,((w -h)/2.0));
    _t  = first * second * third;
    qDebug().noquote() << "ViewSettings:: viewTransform" << sViewerType[_evt] << _size << Transform::toInfoString(_t);
}





