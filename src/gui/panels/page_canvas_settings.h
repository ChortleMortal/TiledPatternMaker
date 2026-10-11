#pragma once

#include <QGroupBox>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include "gui/panels/panel_page.h"
#include "gui/viewers/layer.h"
#include "model/settings/canvas.h"
#include "model/settings/filldata.h"

class AQSpinBox;
class AQTableWidget;
class AQComboBox;
class SpinSet;
class DoubleSpinSet;
class ClickableLabel;

class page_canvas_settings : public panel_page
{
    Q_OBJECT

enum eSettingsGroup
{
    MOSAIC_SETTINGS,    // propagates to Canvas
    TILING_SETTINGS,    // propagates to Canvas
    CANVAS,             // the canvas
    TAKER,              // the canvas
};

#define NUM_SETTINGS        (TAKER + 1)
#define NUM_BORDER_COLORS   2

public:
    page_canvas_settings(ControlPanel * apanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void slot_set_repsMosaic(int val);
    void slot_set_repsTiling(int val);

    void slot_mosaicViewChanged(int);
    void slot_mosaicCanvasChanged(int);
    void slot_mosaicDeltaChanged(int);

    void slot_tilingCanvasChanged(int val);
    void slot_tilingDeltaChanged(int val);
    void slot_tilingViewChanged(int val);

    void slot_tilingDistortionChanged();
    void slot_mosaicDistortionChanged();

    void backgroundColorDesignPick();
    void backgroundColorDesignPickTiling();

    void backgroundColorDesignChanged(const QString & str);
    void backgroundColorDesignChangedTiling(const QString & str);

#ifdef VARIABLE_BOUNDS
    void slot_boundsChanged();
#endif

    void singleton_changed_des(bool checked);
    void singleton_changed_tile(bool checked);

    void sysViewChanged(QSize oldSize, QSize newSize);
    void slot_chkAspectRatio(bool checked);
    void slot_unlockAspectRatio();

protected:
    void refreshTaker();
    void refreshSystemView();
    void refreshTiling();
    void refreshMosaic();
    void tallyAspectRatio();

    QGroupBox * createTakerStatus();
    QGroupBox * createCanvasStatus();
    QGroupBox * createMosaicSettings();
    QGroupBox * createTilingSettings();

    QGridLayout * createFillDataRow(eSettingsGroup group);
    
    void displayFillData(const FillData &fd, eSettingsGroup group);

    QColor       getMosaicOrDesignColor();
    void         setMosaicOrDesignColor(QColor color);
    Canvas *    getMosaicOrDesignCanvas();

private:
    void removeChildren(QTreeWidgetItem * parent);

    QCheckBox       * chkLockAspectRatio;

    QCheckBox       * chkSingle[NUM_SETTINGS];
    AQSpinBox       * xRepMin[NUM_SETTINGS];
    AQSpinBox       * xRepMax[NUM_SETTINGS];
    AQSpinBox       * yRepMin[NUM_SETTINGS];
    AQSpinBox       * yRepMax[NUM_SETTINGS];

    SpinSet         * sizeMosaicCanvasW;
    SpinSet         * sizeMosaicCanvasH;
    SpinSet         * sizeMosaicDeltaH;
    SpinSet         * sizeMosaicDeltaW;
    SpinSet         * sizeMosaicViewW;
    SpinSet         * sizeMosaicViewH;

    AQComboBox      * distortMosaicCombo;
    DoubleSpinSet   * distortMosaicX;
    DoubleSpinSet   * distortMosaicY;

    SpinSet         * sizeTilingCanvasW;
    SpinSet         * sizeTilingCanvasH;
    SpinSet         * sizeTilingDeltaH;
    SpinSet         * sizeTilingDeltaW;
    SpinSet         * sizeTilingViewW;
    SpinSet         * sizeTilingViewH;

    AQComboBox      * distortTilingCombo;
    DoubleSpinSet   * distortTilingX;
    DoubleSpinSet   * distortTilingY;

    QLineEdit       * actualSizeW;
    QLineEdit       * actualSizeH;
    QLineEdit       * aspectRatioVal;

    QLineEdit       * takerSizeW;
    QLineEdit       * takerSizeH;

#ifdef VARIABLE_BOUNDS
    DoubleSpinSet   * ds_left;
    DoubleSpinSet   * ds_top;
    DoubleSpinSet   * ds_width;
#endif

    QLabel          * l_xform;
    QLabel          * l_canvas;
    QLabel          * l_layer;
    QLabel          * l_primaryLayer;

    QLineEdit       * bkgdColorEdit[NUM_SETTINGS];
    ClickableLabel  * bkgdColorPatch[NUM_SETTINGS];
    QLineEdit       * takerColorSource;

    QPushButton     * bkgdImageBtn[NUM_SETTINGS];
    QLineEdit       * bkgdImage[NUM_SETTINGS];

    QCheckBox       * chk_adjustBkgd[NUM_SETTINGS];

    QGroupBox       * canvasBox;
    QGroupBox       * viewBox;
    QGroupBox       * frameBox;
    QGroupBox       * tilingCanvasBox;
    QGroupBox       * mosaicCanvasBox;
};
