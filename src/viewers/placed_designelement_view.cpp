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

// Casper: this is derived from the tapratty FeatureButton

// This class is also used to show the large DesignElement being edited
// in the main editing window.  Nice!

#include "viewers/placed_designelement_view.h"
#include "viewers/geo_graphics.h"
#include "base/shared.h"
#include "base/workspace.h"
#include "viewers/workspace_viewer.h"
#include "viewers/viewerbase.h"


PlacedDesignElementView::PlacedDesignElementView(PlacedDesignElementPtr pde, bool selected) : Layer("PlacedDesignElementView")
{
    Q_ASSERT(pde);

    feature_interior = QColor(255, 217, 217, 127);
    feature_border   = QColor(140, 140, 140);

    this->pde = pde;
    this->selected = selected;
}

void PlacedDesignElementView::paint(QPainter *painter)
{
    qDebug() << "PlacedDesignElementView::paint:" << pde->toString();

    if(!pde->getFeature())
    {
        qWarning() << "Can't draw empty design element";
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing ,true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform,true);

    QTransform tr = getLayerTransform();
    GeoGraphics gg(painter,tr);

    // We could keep the figure's map cached until the figure changes.
    // On the other hand, it's really fast enough already.
    drawPlacedDesignElement(&gg, pde, QPen(Qt::blue,3), QBrush(feature_interior), QPen(feature_border,3));

    drawCenter(painter);
}

void PlacedDesignElementView::drawPlacedDesignElement(GeoGraphics * gg, PlacedDesignElementPtr pde, QPen linePen, QBrush interiorBrush, QPen borderPen)
{
    QTransform T = pde->getTransform();
    gg->pushAndCompose(T);

    FeaturePtr fp = pde->getFeature();
    QPen pen;
    if (selected)
        pen = QPen(Qt::red,3);
    else
        pen = borderPen;

    ViewerBase::drawFeature(gg,fp,interiorBrush,pen);

    // Draw the figure
    FigurePtr fig = pde->getFigure();

    if (!fig)
    {
        qWarning() << "DesignElementView::drawDesignElement - figure is NULL";
        gg->pop();
        return;
    }
    if (fig->getFigType() == FIG_TYPE_EXPLICIT_FEATURE)
    {
        // figure is same as feature
        gg->pop();
        return;
    }

    ViewerBase::drawFigure(gg,fig,linePen);

    gg->pop();
}
