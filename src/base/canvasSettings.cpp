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
#include "base/utilities.h"
#include "geometry/Loose.h"
#include "viewers/workspaceviewer.h"

CanvasSettings::CanvasSettings()
{
    _sceneSize = QSizeF(1500.0,1000.0);   // default
    _bkgdColor = QColor(Qt::white);
    _startTile = QPointF(0,0);
}

CanvasSettings::CanvasSettings(const CanvasSettings & other)
{
    _sceneSize = other._sceneSize;
    _bkgdColor = other._bkgdColor;
    _startTile = other._startTile;
    _bkgdImage = other._bkgdImage;
    _border    = other._border;
}

CanvasSettings::~CanvasSettings()
{}



void CanvasSettings::clear()
{
    _sceneSize  = QSize(1500.0,1100.0);
    _bkgdColor  = QColor(Qt::black);
    _startTile  = QPointF(0.0,0.0);
    _border.reset();
    _bkgdImage.reset();
}

void CanvasSettings::setBorder(BorderPtr border)
{
    qDebug() << "CanvasSettings::setBorder" << Utils::addr(border.get());
    _border = border;
}

void CanvasSettings::setCanvasSize(QSizeF size)
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
    qDebug() << "CanvasSettings::setBackgroundColor()"  << color.name();
    _bkgdColor = color;
}

//
// View Settings
//

ViewSettings::ViewSettings()
{
    _bounds   =  Bounds(-10.0,10.0,20.0);  // default
    _viewSize = QSize(1500,1100);
    calculateViewTransform();
}

ViewSettings::ViewSettings(eViewType evt, Bounds bounds, QSize size)
{
    _evt      = evt;
    _bounds   = bounds;
    _viewSize = size;
    calculateViewTransform();
}

void ViewSettings::init(eViewType evt, Bounds bounds, QSize size)
{
    _evt      = evt;
    _bounds   = bounds;
    _viewSize = size;
    calculateViewTransform();
}

void ViewSettings::setViewSize(QSize size)
{
    _viewSize = size;
    calculateViewTransform();
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
void ViewSettings::calculateViewTransform()
{
    QSizeF size  = _viewSize;
    //QSizeF size  = GetCanvasSettings().getCanvasSize();
    qreal aspect = size.width() / size.height();
    qreal height = _bounds.width / aspect;
    qreal scalex = size.width()/_bounds.width;

    QTransform first  = QTransform::fromTranslate(-_bounds.left, - (_bounds.top - height));
    QTransform second = QTransform().scale(scalex,scalex);
    QTransform third  = QTransform().translate(0.0,(size.width() - size.height())/2.0);
    _t  = first * second * third;
}





