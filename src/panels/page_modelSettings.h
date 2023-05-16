#pragma once
#ifndef PAGE_MODEL_SETTINGS_H
#define PAGE_MODEL_SETTINGS_H

class QGroupBox;
class QGridLayout;
class QTreeWidgetItem;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;

#include "widgets/panel_page.h"
#include "settings/model_settings.h"

class AQSpinBox;
class SpinSet;
class DoubleSpinSet;

class ClickableLabel;
class AQTableWidget;

class page_modelSettings : public panel_page
{
    Q_OBJECT

enum eSettingsGroup
{
    DESIGN_SETTINGS,    // propagates to CANVAS_SETTINGS
    TILING_SETTINGS,    // propagates to CANVAS_SETTINGS
    FRAME_SETTINGS,     // definitions per view type
    VIEW_STATUS,        // current status
    NUM_SETTINGS
};

#define NUM_BORDER_COLORS 2

public:
    page_modelSettings(ControlPanel * apanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void slot_set_repsDesign(int val);
    void slot_set_repsTiling(int val);

    void designSizeChanged(int);
    void cropSizeChanged(int);
    void viewSizeChanged(int);

    void backgroundColorDesignPick();
    void backgroundColorDesignChanged(const QString & str);

    void slot_tilingSizeChanged(int val);
    void slot_showFrameInfoChanged(bool checked);
    void slot_boundsChanged();

    void singleton_changed_des(bool checked);
    void singleton_changed_tile(bool checked);

    void dummySetup();

protected:
    QGroupBox * createTilingSettings();
    QGroupBox * createDesignSettings();
    QGroupBox * createFrameSettings();
    QGroupBox * createViewStatus();

    QGridLayout * createFillDataRow(eSettingsGroup group);

    ModelSettings & getMosaicOrDesignSettings();

    void displayFillData(const FillData &fd, eSettingsGroup group);

private:
    void removeChildren(QTreeWidgetItem * parent);

    QCheckBox       * chkSingle[NUM_SETTINGS];
    AQSpinBox       * xRepMin[NUM_SETTINGS];
    AQSpinBox       * xRepMax[NUM_SETTINGS];
    AQSpinBox       * yRepMin[NUM_SETTINGS];
    AQSpinBox       * yRepMax[NUM_SETTINGS];

    SpinSet         * sizeW[NUM_SETTINGS];
    SpinSet         * sizeH[NUM_SETTINGS];

    SpinSet         * sizeW2;
    SpinSet         * sizeH2;

    DoubleSpinSet   * ds_left;
    DoubleSpinSet   * ds_top;
    DoubleSpinSet   * ds_width;

    QLabel          * l_xform;

    QLineEdit       * bkColorEdit[NUM_SETTINGS];
    ClickableLabel  * bkgdColorPatch[NUM_SETTINGS];

    DoubleSpinSet   * startEditX[NUM_SETTINGS];
    DoubleSpinSet   * startEditY[NUM_SETTINGS];

    QPushButton     * bkgdImageBtn[NUM_SETTINGS];
    QLineEdit       * bkgdImage[NUM_SETTINGS];

    QCheckBox       * chk_adjustBkgd[NUM_SETTINGS];

    QGroupBox       * viewBox;
    QGroupBox       * frameBox;

    AQTableWidget   * frameTable;
};

#endif
