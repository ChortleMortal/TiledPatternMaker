#pragma once

#include <QItemSelection>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include "gui/panels/panel_page.h"

class AQTableWidget;
class AQComboBox;
class SpinSet;
class DoubleSpinSet;

typedef std::shared_ptr<class Prototype>       ProtoPtr;
typedef std::shared_ptr<class StyleEditor>     StyleEditorPtr;
typedef std::shared_ptr<class Style>           StylePtr;
typedef std::weak_ptr<class Style>             WeakStylePtr;

class page_mosaic_maker : public panel_page
{
    Q_OBJECT

    enum eStyleCol
    {
        STYLE_COL_ENABLE,
        STYLE_COL_TILING,
        STYLE_COL_STYLE_TYPE,
        STYLE_COL_LAYER_CONTROL,
        STYLE_COL_INFO,
        STYLE_COL_STYLE_ADDR,
        STYLE_COL_NUM_COLS
    };

public:
    page_mosaic_maker(ControlPanel * apanel);

    void    onEnter() override;
    void    onExit() override {}
    void    onRefresh() override;

private slots:
    void    slot_styleSelected(const QItemSelection &selected, const QItemSelection &deselected);
    void    slot_deleteStyle();
    void    slot_moveStyleUp();
    void    slot_moveStyleDown();
    void    slot_duplicateStyle();
    void    slot_set_reps();
    void    slot_setCleanse();
    void    slot_cleansed();
    void    singleton_changed(bool checked);

    void    slot_noaddr(bool checked);
    void    slot_nolayer(bool checked);

    void    slot_gotoBackgrounds();
    void    slot_gotoCropMaker();
    void    slot_gotoBorderMaker();

    void    slot_showMosaicCrop(bool checked);
    void    slot_showPainterCrop(bool checked);

protected:
    QHBoxLayout *buildDistortionsLayout();

    void     styleChanged(int row);
    void     tilingChanged(int row);
    void     styleVisibilityChanged(int row, bool checked);

    void     reEnter();
    void     displayStyles();
    void     displayStyleParams();
    QPushButton * addInfoRow(int row, QString label, QString info, QString address);

    StylePtr getStyleRow(int row);
    QString  getRowLabel(int row);

    void     updateStyleButtons(int row);

    ProtoPtr getCurrentPrototype();

    QHBoxLayout *createFillDataRow();

    void     setDistortion();


private:
    QWidget       * lowerWidget;
    StyleEditor   * currentEditor;

    QGroupBox     * componentsBox;
    QGroupBox     * styleEditorBox;

    AQTableWidget * styleTable;

    QPushButton * delBtn;
    QPushButton * upBtn;
    QPushButton * downBtn;
    QPushButton * dupBtn;

    QCheckBox  * chkSingle;
    QCheckBox  * chk_noaddr;
    QCheckBox  * chk_nolayer;

    SpinSet    * xRepMin;
    SpinSet    * xRepMax;
    SpinSet    * yRepMin;
    SpinSet    * yRepMax;

    QPushButton * pbClean;

    AQComboBox    * distortCombo;
    DoubleSpinSet * xBox;
    DoubleSpinSet * yBox;

    // set by displayStyles() only when the corresponding info row exists; resynced by onRefresh()
    // since each also has a twin checkbox elsewhere (Crop Maker page, Backgrounds page / LHS panel)
    // that can change the same underlying state
    QCheckBox     * cbShowBkgd        = nullptr;
    QCheckBox     * cbShowCrop        = nullptr;
    QCheckBox     * cbShowPainterCrop = nullptr;

};
