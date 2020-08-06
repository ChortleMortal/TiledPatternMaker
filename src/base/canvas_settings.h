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
#include "base/filldata.h"
#include "tile/backgroundimage.h"
#include "geometry/bounds.h"
#include "configuration.h"

class CanvasSettings
{
public:
    CanvasSettings();
    CanvasSettings(const CanvasSettings & other);
    ~CanvasSettings();

    void            clear();

    void            setBorder(BorderPtr border);
    BorderPtr       getBorder() { return _border; }

    QColor          getBackgroundColor();
    void            setBackgroundColor(QColor color);

    void            setBkgdImage(BkgdImgPtr bkImage) { _bkgdImage = bkImage; }
    BkgdImgPtr      getBkgdImage() { return  _bkgdImage; }

    void            setCanvasSize(QSize size);
    QSize           getCanvasSize() { return _canvasSize; }

    QPointF         getStartTile();
    void            setStartTile(QPointF pt);

    void            setFillData(FillData & fd);
    FillData &      getFillData();

    QPointF         getCenter();

protected:

private:
    QSize           _canvasSize;
    QColor          _bkgdColor;
    QPointF         _startTile;
    BkgdImgPtr      _bkgdImage;
    BorderPtr       _border;
    FillData        _fillData;
};


class ViewSettings
{
public:
    ViewSettings();
    ViewSettings(eViewType evt, Bounds bounds, QSize size);
    void    init(eViewType evt, Bounds bounds, QSize size);

    QTransform      getViewTransform() { return _t; }

    void            setViewSize(QSize size);
    QSize           getViewSize() { return  _viewSize; }

protected:
    void            calculateViewTransform();

private:
    eViewType       _evt;
    Bounds          _bounds;
    QSize           _viewSize;
    QTransform      _t;             // calculated
};

#endif
