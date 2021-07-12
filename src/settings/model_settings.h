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

#ifndef MODEL_SETTINGS_H
#define MODEL_SETTINGS_H

#include <QtCore>
#include <QColor>
#include "base/shared.h"
#include "settings/filldata.h"

class ModelSettings
{
public:
    ModelSettings();
    ModelSettings(const ModelSettings & other);
    ~ModelSettings();

    void            clear();

    QColor          getBackgroundColor();
    void            setBackgroundColor(QColor color);

    void            setSize(QSize size);
    QSize           getSize() { return _size; }

    void            setZSize(QSize size) { _zsize = size; }
    QSize           getZSize() { return _zsize; }

    QPointF         getStartTile();
    void            setStartTile(QPointF pt);

    void            setFillData(FillData & fd);
    FillData &      getFillData();

    QPointF         getCenter();

protected:

private:
    QSize           _size;          // crop size
    QSize           _zsize;         // zoom size
    QColor          _bkgdColor;
    QPointF         _startTile;
    FillData        _fillData;
};

#endif
