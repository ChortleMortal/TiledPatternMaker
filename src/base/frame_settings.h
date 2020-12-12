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

#ifndef VIEW_SETTINGS_H
#define VIEW_SETTINGS_H

#include "geometry/bounds.h"
#include <QTransform>

class FrameSettings
{
    friend class View;

public:
    FrameSettings();
    FrameSettings(Bounds bounds, QSize size);
    FrameSettings(const FrameSettings & other);

    FrameSettings & operator=(const FrameSettings & other);

    void            reInit();

    QTransform      getDefinedFrameTransform() const { return _definedTransform; }
    QTransform      getActiveFrameTransform() const { return _activeTransform; }

    void            setDefinedFrameSize(QSize size);
    QSize           getDefinedFrameSize() const { return  _definedSize; }

    void            setActiveFrameSize(QSize size);
    QSize           getActiveFrameSize() const;

    QSize           getDefaultFrameSize() const { return _defaultSize; }

protected:
    void            calculateDefinedFrameTransform();
    void            calculateActiveFrameTransform();
    QTransform      calculateTransform(QSize size);

private:
    Bounds          _bounds;
    QSize           _defaultSize;
    QSize           _definedSize;
    QSize           _activeSize;
    QTransform      _definedTransform;      // calculated
    QTransform      _activeTransform;       // calculated
};

#endif
