﻿/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
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

#ifndef FEATURE_BUTTON
#define FEATURE_BUTTON

#include "tapp/DesignElement.h"
#include "viewers/GeoGraphics.h"
#include <QtWidgets>

////////////////////////////////////////////////////////////////////////////
//
// FeatureButton.java
//
// A feature button is an (optionally clickable) GeoView that displays
// a DesignElement (perhaps DesignElementButton would have been a better
// name).  It draws the underlying feature with the figure overlaid.
// It includes some optimization for drawing radially symmetric figures
// without building the complete map.
//
// These buttons are meant to function like radio buttons -- they live
// in groups, and one is active at any given time.  If a button is active,
// it gets drawn with a red border.
//
// This class is also used to show the large DesignElement being edited
// in the main editing window.  Nice!

class FeatureButton : public QFrame
{
    Q_OBJECT

public:
    FeatureButton(int index);
    FeatureButton(DesignElementPtr dep, int index );
    ~FeatureButton();

    QSizeF getMinimumSize();
    int    getIndex() { return index; }

    void setSize(QSize d );
    void setSize( int w, int h );

    DesignElementPtr getDesignElement();
    void setDesignElement(DesignElementPtr delp);
    void designElementChanged();

    static Transform resetViewport(DesignElementPtr dep, QRect frameRect);

    void setSelected(bool selected);

    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

protected:
    void    construct(DesignElementPtr dep, int index);
    static Transform lookAt(QRectF rect, QRect frameRect);
    static Transform centerInside(QRectF first, QRectF other);

private:
    DesignElementPtr 	dep;

    bool		selected;
    int         index;

    static QColor feature_interior;
    static QColor feature_border;

    Transform   transform;
};

#endif
