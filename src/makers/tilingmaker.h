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

#ifndef TILING_MAKER_H
#define TILING_MAKER_H

#include "base/view.h"
#include "makers/tilingmouseactions.h"
#include "viewers/tilingmakerview.h"
#include "tile/Tiling.h"

class Canvas;
class MouseAction;


class TilingMaker : public TilingMakerView
{
    Q_OBJECT

public:
    static TilingMaker * getInstance();

    void      setTiling(TilingPtr tiling);
    TilingPtr getTiling() { return _tiling; }

    void draw(GeoGraphics * g2d) override;
    void viewRectChanged();

    void clearDesignerData();
    void updatePlacedFeaturesFromData();
    QString  getStatus();

    bool verifyTiling();
    void removeFeature(PlacedFeaturePtr pf);
    bool accumHasPoint(QPointF wpt);

    // Internal tiling creation.
    QPointF getTrans1();
    QPointF getTrans2();
    void    fixupTranslate();

    eMouseMode getMouseMode();
    int        getPolygonSides() { return poly_side_count; }

    // Mouse interaction underway..
    MouseActionPtr mouse_interaction;
    QPointF     sMousePos;   // screen points DAC added
    QPointF     wTrans1_start;
    QPointF     wTrans1_end;
    QPointF     featureEditPoint;

    // Feature management.
    int         addToAllPlacedFeatures( PlacedFeaturePtr pf );
    TilingSelectionPtr addFeatureSelectionPointer(TilingSelectionPtr sel );
    void        removeFeature(TilingSelectionPtr sel);
    void        addToTranslate(QPointF wpt, bool ending);
    void        setCurrentFeature(PlacedFeaturePtr pfp) { currentFeature = pfp; }
    void        toggleInclusion(TilingSelectionPtr sel);

    // global modifications to features
    void allDeltaX(int delta);
    void allDeltaY(int delta);
    void allDeltaScale(int delta);
    void allDeltaRotate(int delta);

signals:
    void sig_buildMenu();
    void sig_refreshMenu();
    void sig_current_feature(int fIndex);

public slots:
    void updatePolygonSides(int number);
    void setMouseMode(eMouseMode mode);
    void addRegularPolygon();
    void fillUsingTranslations();
    void removeExcluded();
    void excludeAll();
    void clearTranslation();
    void hide(bool state);
    void setFeatureEditPoint(QPointF pt);
    void setConvexEdge(bool convex);
    void slot_showOverlaps(bool checked);
    void slot_xformMode_changed(int row);

    void slot_procKeyEvent(QKeyEvent *k);
    void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn);
    void slot_mouseDragged(QPointF spt);
    void slot_mouseReleased(QPointF spt);
    void slot_mouseMoved(QPointF spt);

protected slots:
    void slot_deleteFeature();
    void slot_includeFeature();
    void slot_excludeFeature();
    void slot_copyMoveFeature();

protected:
    void setupDesigner(TilingPtr tiling);
    void addInTiling(PlacedFeaturePtr pf);
    void removeFromInTiling(PlacedFeaturePtr pf);

    void drawTiling(GeoGraphics * g2d);

    void createFillCopies();

    bool isTranslationInvalid();

    // Mouse mode handling.
    void setMousePos(QPointF spt);
    void updateUnderMouse(QPointF  spt);

    // Possible user actions.
    void deleteFeature(TilingSelectionPtr sel);
    void copyPolygon( TilingSelectionPtr sel );
    void mirrorPolygonX(TilingSelectionPtr sel);
    void mirrorPolygonY(TilingSelectionPtr sel);

    // Mouse tracking.
    TilingSelectionPtr findFeatureUnderMouse();

    // Mouse interactions.
    void startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton);

private:
    TilingMaker();
    ~TilingMaker() override;

    static TilingMaker * mpThis;

    TilingPtr   _tiling;                    // DAC added

    TilingSelectionPtr  currentSelection;   // Current mouse selection.
    PlacedFeaturePtr    editFeature;        // Feature in DlgFeatureEdit
    PlacedFeaturePtr    currentFeature;     // current menu row selection too

    TilingSelectionPtr  menuSelection;
    QPointF             menuSpt;

    int         poly_side_count;            // number of selected vertices when drawing polygons.
    bool        convex;                     // used when creating a curved edge

    // Translation vector so that the tiling tiles the plane.
    QPointF     wTrans2_start;
    QPointF     wTrans2_end;

    Canvas          * canvas;
    View            * view;
};

#endif
