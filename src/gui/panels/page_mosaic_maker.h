#pragma once
#ifndef PAGE_MOSAIC_MAKER_H
#define PAGE_MOSAIC_MAKER_H

#include <QItemSelection>
#include "gui/panels/panel_page.h"

class AQTableWidget;
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
        STYLE_COL_TRANSFORM,
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
    void    slot_notrans(bool checked);
    void    slot_nolayer(bool checked);

protected:
    QHBoxLayout *createBackgroundInfo();
    QHBoxLayout *buildDistortionsLayout();

    void     styleChanged(int row);
    void     tilingChanged(int row);
    void     styleVisibilityChanged(int row, bool checked);

    void     reEnter();
    void     displayStyles();
    void     displayStyleParams();

    StylePtr getStyleRow(int row);
    StylePtr getStyleIndex(int index);
    StylePtr copyStyle(const StylePtr style);

    ProtoPtr getCurrentPrototype();

    QHBoxLayout *createFillDataRow();

    void     setDistortion();
    void     enbDistortion(bool checked);


private:
    QWidget       * lowerWidget;
    StyleEditor   * currentEditor;

    AQTableWidget * styleTable;

    QPushButton * delBtn;
    QPushButton * upBtn;
    QPushButton * downBtn;
    QPushButton * dupBtn;

    QCheckBox  * chkSingle;
    QCheckBox  * chk_noaddr;
    QCheckBox  * chk_notrans;
    QCheckBox  * chk_nolayer;

    SpinSet    * xRepMin;
    SpinSet    * xRepMax;
    SpinSet    * yRepMin;
    SpinSet    * yRepMax;

    QPushButton * pbClean;
    QLabel      * cleanseStatus;

    QPushButton * pbExam;

    QCheckBox     * chkDistort;
    DoubleSpinSet * xBox;
    DoubleSpinSet * yBox;

};

#endif
