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

#ifndef DESIGN_INFO_H
#define DESIGN_INFO_H

#include <QtCore>
#include <QtGui>
#include "base/shared.h"
#include "tile/backgroundimage.h"

class Configuration;
class Workspace;
class Pattern;
class Border;
class Canvas;
struct ViewDefinition;

class CanvasSettings
{
public:
    CanvasSettings();
    CanvasSettings(const CanvasSettings & other);
    ~CanvasSettings();

    void            init2();
    void            set(ViewDefinition * viewDef);

    void            setBorder(BorderPtr border);
    BorderPtr       getBorder() { return _border; }

    QColor          getBackgroundColor();
    void            setBackgroundColor(QColor color);

    void            setBkgdImage(BkgdImgPtr bkImage) { _bkgdImage = bkImage; }
    BkgdImgPtr      getBkgdImage() { return  _bkgdImage; }

    void            setSizeF(QSizeF size);
    QSizeF          getSizeF() { return _sceneSize; }
    QRectF          getRectF() { return QRectF(QPoint(0,0),_sceneSize); }
    QRect           getRect()  { return getRectF().toAlignedRect(); }

    QPointF         getStartTile();
    void            setStartTile(QPointF pt);

    QPointF         getCenter() { return getRectF().center(); }

    void            dump();

private:
    QColor          _bkgdColor;     // used by canvas
    BkgdImgPtr      _bkgdImage;
    QSizeF          _sceneSize;     // used by canvas

    BorderPtr       _border;
    QPointF         _startTile;
};

#endif
