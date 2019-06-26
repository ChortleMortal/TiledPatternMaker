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
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
// DesignerPanel.java
//
// It's used to design the tilings that are used as skeletons for the Islamic
// construction process.  It's fairly featureful, much more rapid and accurate
// than expressing the tilings directly as code, which is what I did in a
// previous version.

#ifndef TILING_DESIGNER_H
#define TILING_DESIGNER_H

#include "base/view.h"
#include "makers/tilingmouseactions.h"
#include "viewers/TilingDesignerView.h"
#include "tile/Tiling.h"

class Canvas;
class MouseAction;


class TilingDesigner : public TilingDesignerView
{
    Q_OBJECT

public:
    static TilingDesigner * getInstance();

    void setTiling(TilingPtr tiling);
    void clearDesigner();

    void updateTilingFromData(TilingPtr tiling);
    void updateStatus();

    bool verifyTiling();

    QVector<PlacedFeaturePtr> & getFeatures() { return placed_features; }
    QVector<PlacedFeaturePtr> & getInTiling() { return in_tiling; }// DAC was hash

    void draw(GeoGraphics * g2d) override;

    void removeFeature(PlacedFeaturePtr pf);

    void procKey(QKeyEvent *k);

    bool accumHasPoint(QPointF spt);

    void updateOverlaps();
    void setDrawAllOverlaps(bool set) { drawAllOverlaps = set; }

    // Internal tiling creation.
    QPointF getTrans1();
    QPointF getTrans2();

    eMouseMode getMouseMode();
    int        getPolygonSides() { return poly_side_count; }

    // Mouse interaction underway..
    MouseActionPtr mouse_interaction;
    QPointF     mousePos;   // DAC added
    QPointF     trans1_start;
    QPointF     trans1_end;

    QVector<QPointF>       accum;
    QVector<Measurement>   measurements;

    // Feature management.
    int          addFeaturePF( PlacedFeaturePtr pf );
    TilingSelectionPtr addFeatureSP(TilingSelectionPtr sel );
    void         removeFeature(TilingSelectionPtr sel);
    void         addToTranslate(QPointF pt, bool ending);
    void         forgetPolygon();

signals:
    void hasChanged();
    void sig_current_feature(int fIndex);

public slots:
    void updatePolygonSides(int number);
    void setMouseMode(eMouseMode mode);
    void addRegularPolygon();
    void addIrregularPolygon();
    void fillUsingTranslations();
    void removeExcluded();
    void excludeAll();
    void clearTranslation();

    void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn);
    void slot_mouseDragged(QPointF spt);
    void slot_mouseReleased(QPointF spt);
    void slot_mouseMoved(QPointF spt);

protected:
    void setupDesigner(TilingPtr tiling);
    void addInTiling(PlacedFeaturePtr pf);
    void removeFromTiling(PlacedFeaturePtr pf);

    void createCopies();

    bool isTranslationInvalid();

    // Mouse mode handling.
    void setMousePos(QPointF spt);
    void updateUnderMouse(QPointF  spt);

    // Possible user actions.
    void toggleInclusion(TilingSelectionPtr sel);
    void deletePolygon(TilingSelectionPtr sel);
    void copyPolygon( TilingSelectionPtr sel );

    // Mouse tracking.
    TilingSelectionPtr findFeatureUnderMouse();

    // Mouse interactions.
    void startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton);

private:
    TilingDesigner();
    ~TilingDesigner() override;

    static TilingDesigner * mpThis;

    QVector<PlacedFeaturePtr> in_tiling;    // DAC was hash
    QVector<PlacedFeaturePtr> overlapping;  // DAC was hash
    QVector<PlacedFeaturePtr> touching;

    // Current mouse selection.
    TilingSelectionPtr currentSelection;
    bool               drawAllOverlaps;

    // Mouse mode, triggered by the toolbar.
    eMouseMode  mouse_mode;

    // Accumulation of selected vertices when drawing polygons.
    int         poly_side_count;

    // Translation vector so that the tiling tiles the plane.
    QPointF     trans2_start;
    QPointF     trans2_end;

    Canvas          * canvas;
    Configuration   * config;
    View            * view;
};

#endif
