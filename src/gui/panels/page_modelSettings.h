#pragma once
#ifndef PAGE_MODEL_SETTINGS_H
#define PAGE_MODEL_SETTINGS_H

#include "gui/panels/panel_page.h"
#include "model/settings/canvas_settings.h"
#include "model/settings/filldata.h"

class AQSpinBox;
class AQTableWidget;
class SpinSet;
class DoubleSpinSet;
class ClickableLabel;

class page_modelSettings : public panel_page
{
    Q_OBJECT

enum eSettingsGroup
{
    MOSAIC_SETTINGS,    // propagates to Canvas
    TILING_SETTINGS,    // propagates to Canvas
    CANVAS,             // the canvas
    VIEW_STATUS        // the view
};

#define NUM_SETTINGS        (VIEW_STATUS + 1)
#define NUM_BORDER_COLORS   2

public:
    page_modelSettings(ControlPanel * apanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void slot_set_repsDesign(int val);
    void slot_set_repsTiling(int val);

    void slot_canvasSizeChanged(int);
    void slot_viewerSizeChanged(int);
    void slot_windowSizeChanged(int);

    void backgroundColorDesignPick();
    void backgroundColorDesignChanged(const QString & str);

    void slot_tilingSizeChanged(int val);
    void slot_boundsChanged();

    void singleton_changed_des(bool checked);
    void singleton_changed_tile(bool checked);

    void dummySetup();

protected:
    QGroupBox * createTilingSettings();
    QGroupBox * createMosaicSettings();
    QGroupBox * createCanvasStatus();
    QGroupBox * createViewStatus();

    QGridLayout * createFillDataRow(eSettingsGroup group);
    
    CanvasSettings &getMosaicOrDesignModelSettings();
    void setMosaicOrDesignModelSettings(CanvasSettings &ms);

    void displayFillData(const FillData &fd, eSettingsGroup group);

private:
    void removeChildren(QTreeWidgetItem * parent);

    QCheckBox       * chkSingle[NUM_SETTINGS];
    AQSpinBox       * xRepMin[NUM_SETTINGS];
    AQSpinBox       * xRepMax[NUM_SETTINGS];
    AQSpinBox       * yRepMin[NUM_SETTINGS];
    AQSpinBox       * yRepMax[NUM_SETTINGS];

    SpinSet         * sizeW[NUM_SETTINGS];  // FIXME canvas is double
    SpinSet         * sizeH[NUM_SETTINGS];

    DoubleSpinSet   * ds_left;
    DoubleSpinSet   * ds_top;
    DoubleSpinSet   * ds_width;

    QLabel          * l_xform;
    QLabel          * l_canvas;
    QLabel          * l_layer;

    QLineEdit       * bkColorEdit[NUM_SETTINGS];
    ClickableLabel  * bkgdColorPatch[NUM_SETTINGS];

    DoubleSpinSet   * startEditX[NUM_SETTINGS];
    DoubleSpinSet   * startEditY[NUM_SETTINGS];

    QPushButton     * bkgdImageBtn[NUM_SETTINGS];
    QLineEdit       * bkgdImage[NUM_SETTINGS];

    QCheckBox       * chk_adjustBkgd[NUM_SETTINGS];

    QGroupBox       * canvasBox;
    QGroupBox       * viewBox;
    QGroupBox       * frameBox;
};

#endif
