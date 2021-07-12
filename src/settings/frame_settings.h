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
#include "enums/eviewtype.h"
#include <QTransform>
#include <QMap>

enum eModelAlignment
{
    M_ALIGN_NONE,
    M_ALIGN_MOSAIC,
    M_ALIGN_TILING,
};

class FrameData
{
public:
    FrameData();
    FrameData(Bounds bounds, QSize size);
    FrameData(const FrameData & other);

    FrameData & operator=(const FrameData & other);

    void            reInit();

    void            setDeltaSize(QSize size);
    void            setCropSize( QSize size) { cropSize = size; }
    void            setZoomSize( QSize size) { zoomSize = size; calculateTransform(); }

    QTransform      getTransform()      const { return transform; }
    QTransform      getDeltaTransform() const { return baseInv; }
    QSize           getCropSize()       const { return cropSize; }
    QSize           getZoomSize()       const { return zoomSize; }
    Bounds &        getBounds()               { return bounds; }

    void            calculateTransform();
    void            calculateBaseInv() { baseInv = transform.inverted(); }

protected:
    QSize           cropSize;
    QSize           zoomSize;
    QTransform      transform;
    Bounds          bounds;

    QTransform      baseInv;

private:
    class Configuration * config;

    QSize           initSize;
    Bounds          initBounds;
};


class FrameSettings
{
public:
    FrameSettings();

    void    reInit();

    void    initialise(eViewType e, QSize cropSize, QSize zoomSize);
    void    initialiseCommon(QSize cropSize, QSize  zoomSize);

    void    setDeltaSize(eViewType e, QSize sz);
    void    setCommonDeltaSizes(QSize sz);

    QTransform  getTransform(eViewType e);
    QSize       getCropSize(eViewType e);
    QSize       getZoomSize(eViewType e);

    const QMap<eViewType,FrameData*> & getFrameSettings() { return  settings; }
    FrameData * getFrameData(eViewType evt)               { return settings[evt]; }

    void    setModelAlignment(eModelAlignment mode) { modelAlignment = mode; }
    eModelAlignment getModelAlignment()              { return modelAlignment; }

protected:
    void    add(eViewType evt, Bounds bounds, QSize size);
    void    reInit(eViewType evt);

private:
    QMap<eViewType,FrameData*> settings;
    eModelAlignment modelAlignment;
};

#endif
