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

#include <QString>
#include "viewers/tiling_maker_view.h"
#include "enums/etilingmakermousemode.h"
#include "enums/estatemachineevent.h"

class QKeyEvent;

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Prototype>        PrototypePtr;
typedef std::shared_ptr<class TilingMaker>      TilingMakerPtr;
typedef std::shared_ptr<class TilingMouseAction>MouseActionPtr;
typedef std::shared_ptr<class Feature>          FeaturePtr;

#define E2STR(x) #x

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
    int         numExcluded() { return  allPlacedFeatures.count() - in_tiling.count(); }

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

    void        addInTiling(PlacedFeaturePtr pf);
    void        addInTilings(QVector<PlacedFeaturePtr> & pfs);
    void        removeFromInTiling(PlacedFeaturePtr pf);

    QVector<FeaturePtr> getUniqueFeatures();
    TilingSelectorPtr   getCurrentSelection() { return featureSelector; }

    bool        verifyTiling();
    void        deleteFeature(PlacedFeaturePtr pf);
    bool        accumHasPoint(QPointF wpt);

    eTilingMakerMouseMode getTilingMakerMouseMode();
    QString    getStatus();
    int        getPolygonSides() { return poly_side_count; }

    // Mouse interaction underway..
    void       drawMouseInteraction(GeoGraphics * g2d);

    // Feature management.
    void        addNewPlacedFeature(PlacedFeaturePtr pf);
    void        addNewPlacedFeatures(QVector<PlacedFeaturePtr> & pfs);
    void        deleteFeature(TilingSelectorPtr sel);
    TilingSelectorPtr addFeatureSelectionPointer(TilingSelectorPtr sel );
    void        addToTranslate(QLineF mLine);
    void        setCurrentFeature(PlacedFeaturePtr pfp) { currentPlacedFeature = pfp; }
    void        toggleInclusion(TilingSelectorPtr sel);
    bool        isIncluded(PlacedFeaturePtr pfp)  { return in_tiling.contains(pfp); }
    bool        procKeyEvent(QKeyEvent * k);

    void        clearConstructionLines() { constructionLines.clear(); }

    virtual void iamaLayer() override {}
    virtual void iamaLayerController() override {}

signals:
    void        sig_buildMenu();
    void        sig_refreshMenu();
    void        sig_current_feature(int fIndex);

public slots:
    void        updatePolygonSides(int number);
    void        updatePolygonRot(qreal angle);
    void        setTilingMakerMouseMode(eTilingMakerMouseMode mode);
    void        addRegularPolygon();
    void        fillUsingTranslations();
    void        removeExcluded();
    void        excludeAll();
    void        clearTranslationVectors();
    void        setFeatureEditPoint(QPointF pt);

    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseTranslate(QPointF pt)      override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;

    virtual void slot_setCenter(QPointF spt) override;

    virtual void slot_wheel_scale(qreal delta)  override;
    virtual void slot_wheel_rotate(qreal delta) override;

    virtual void slot_scale(int amount)  override;
    virtual void slot_rotate(int amount) override;
    virtual void slot_moveX(int amount)  override;
    virtual void slot_moveY(int amount)  override;

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

    void updateVisibleVectors();
    void createFillCopies();
    void refillUsingTranslations();

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

    class ViewControl         * view;
    class MotifMaker          * motifMaker;
    class MapEditor           * maped;
};

#endif
