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

#include "viewers/tiling_maker_view.h"
#include "base/misc.h"

enum eTMState
{
    TM_EMPTY,
    TM_SINGLE,
    TM_MULTI
};

static QString tm_states[]
{
    E2STR(TM_EMPTY),
    E2STR(TM_SINGLE),
    E2STR(TM_MULTI)
};

class TilingMaker : public TilingMakerView
{
    Q_OBJECT

public:
    static TilingMakerPtr  getSharedInstance();
    TilingMaker();  // don't call this

    void        init();

    void        sm_take(TilingPtr tiling, eSM_Event mode);

    void        select(TilingPtr tiling);
    void        select(PrototypePtr prototype);
    TilingPtr   getSelected() { return selectedTiling; }
    int         numTilings() { return tilings.size(); }

    void        eraseTilings();
    void        removeTiling(TilingPtr tp);
    TilingPtr   findTilingByName(QString name);
    bool        isLoaded(QString name);
    bool        isValidTiling(TilingPtr tiling);

    void        unload();

    const QVector<TilingPtr> & getTilings() { return tilings; }

    void        clearMakerData();
    void        updateTilingPlacedFeatures();
    void        updateVectors();
    void        updateReps();

    TilingSelectorPtr getCurrentSelection() { return featureSelector; }

    bool        verifyTiling();
    void        deleteFeature(PlacedFeaturePtr pf);
    bool        accumHasPoint(QPointF wpt);

    eTMMouseMode getTilingMakerMouseMode();
    QString    getStatus();
    int        getPolygonSides() { return poly_side_count; }

    // Mouse interaction underway..
    void       drawMouseInteraction(GeoGraphics * g2d);

    // Feature management.
    void        addNewPlacedFeature(PlacedFeaturePtr pf);
    void        deleteFeature(TilingSelectorPtr sel);
    TilingSelectorPtr addFeatureSelectionPointer(TilingSelectorPtr sel );
    void        addToTranslate(QLineF mLine);
    void        setCurrentFeature(PlacedFeaturePtr pfp) { currentPlacedFeature = pfp; }
    void        toggleInclusion(TilingSelectorPtr sel);
    bool        isIncluded(PlacedFeaturePtr pfp)  { return in_tiling.contains(pfp); }
    bool        procKeyEvent(QKeyEvent * k);

    void        clearConstructionLines() { constructionLines.clear(); }
signals:
    void sig_buildMenu();
    void sig_refreshMenu();
    void sig_current_feature(int fIndex);

public slots:
    void updatePolygonSides(int number);
    void updatePolygonRot(qreal angle);
    void setTilingMakerMouseMode(eTMMouseMode mode);
    void addRegularPolygon();
    void fillUsingTranslations();
    void removeExcluded();
    void excludeAll();
    void clearTranslationVectors();
    void setFeatureEditPoint(QPointF pt);
    void slot_showOverlaps(bool checked);
    void slot_snapTo(bool checked);

    void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt);
    void slot_mouseReleased(QPointF spt);
    void slot_mouseMoved(QPointF spt);

    void slot_moveX(int amount) override;
    void slot_moveY(int amount) override;
    void slot_rotate(int amount) override;
    void slot_scale(int amount) override;

    void slot_mouseTranslate(QPointF spt) override;
    void slot_wheel_rotate(qreal delta) override;
    void slot_wheel_scale(qreal delta) override;

protected slots:
    void slot_deleteFeature();
    void slot_includeFeature();
    void slot_excludeFeature();
    void slot_editFeature();
    void slot_copyMoveFeature();
    void slot_uniquifyFeature();
    void slot_flatenCurve();
    void slot_makeConvex();
    void slot_makeConcave();
    void slot_moveArcCenter();
    void slot_editMagnitude();

protected:
    void setupMaker(TilingPtr tp);

    void addInTiling(PlacedFeaturePtr pf);
    void removeFromInTiling(PlacedFeaturePtr pf);

    void updateVisibleVectors();
    void createFillCopies();
    void refillUsingTranslations();

    bool isTranslationInvalid();

    // state machine
    void     sm_resetAllAndAdd(TilingPtr tiling);
    void     sm_resetCurrentAndAdd(TilingPtr tiling);
    void     sm_add(TilingPtr tiling);
    eTMState sm_getState();
    bool     sm_askAdd();
    void     sm_title(TilingPtr tiling);

    // Mouse mode handling.
    void setMousePos(QPointF spt);
    void updateUnderMouse(QPointF  spt);

    // Possible user actions.
    void copyPolygon(TilingSelectorPtr sel );
    void mirrorPolygonX(TilingSelectorPtr sel);
    void mirrorPolygonY(TilingSelectorPtr sel);

    // Mouse tracking.
    TilingSelectorPtr findFeatureUnderMouse();

    // Mouse interactions.
    void startMouseInteraction(QPointF spt, enum Qt::MouseButton mouseButton);

    // global modifications to features
    void tilingDeltaX(int delta);
    void tilingDeltaY(int delta);
    void tilingDeltaScale(int delta);
    void tilingDeltaRotate(int delta);

    void placedFeatureDeltaX(int delta);
    void placedFeatureDeltaY(int delta);
    void placedFeatureDeltaScale(int delta);
    void placedFeatureDeltaScale(qreal scale);
    void placedFeatureDeltaRotate(int delta);
    void placedFeatureDeltaRotate(qreal rotate);

    void uniqueFeatureDeltaScale(int delta);
    void uniqueFeatureDeltaScale(qreal scale);
    void uniqueFeatureDeltaRotate(int delta);
    void uniqueFeatureDeltaRotate(qreal rotate);

private:
    static TilingMakerPtr       spThis;

    MouseActionPtr              mouse_interaction;

    UniqueQVector<TilingPtr>    tilings;
    TilingPtr                   selectedTiling;

    TilingSelectorPtr           clickedSelector;
    QPointF                     clickedSpt;

    int                         poly_side_count;            // number of selected vertices when drawing polygons.
    qreal                       poly_rotation;              // regular polygon feature rotation
    bool                        filled;                     // state - currently filled or not

    class View                * view;
    class MotifMaker          * motifMaker;
    class DecorationMaker     * decorationMaker;
    MapEditorPtr                maped;
};

#endif
