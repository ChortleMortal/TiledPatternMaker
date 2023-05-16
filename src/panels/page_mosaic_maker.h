#pragma once
#ifndef PAGE_MOSAIC_MAKER_H
#define PAGE_MOSAIC_MAKER_H

#include <QItemSelection>
#include "widgets/panel_page.h"

class AQTableWidget;
class SpinSet;
class QHBoxLayout;
class QPushButton;
class QCheckBox;
class QLineEdit;

typedef std::shared_ptr<class StyleEditor>     StyleEditorPtr;
typedef std::shared_ptr<class Style>           StylePtr;
typedef std::weak_ptr<class Style>             WeakStylePtr;

class page_mosaic_maker : public panel_page
{
    Q_OBJECT

    enum eStyleCol
    {
        STYLE_COL_CHECK_SHOW,
        STYLE_COL_TILING,
        STYLE_COL_STYLE,
        STYLE_COL_ADDR,
        STYLE_COL_STYLE_DATA = STYLE_COL_ADDR,
        STYLE_COL_TRANS,
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
    void    slot_analyzeStyleMap();
    void    slot_set_reps();
    void    slot_setCleanse();
    void    singleton_changed(bool checked);

protected:
    void     setCurrentEditor(StylePtr style);
    void     styleChanged(int row);
    void     tilingChanged(int row);
    void     styleVisibilityChanged(int row);

    void     reEnter();
    void     displayStyles();
    void     displayStyleParams();

    StylePtr getStyleRow(int row);
    StylePtr getStyleIndex(int index);
    StylePtr copyStyle(const StylePtr style);

    QHBoxLayout *createFillDataRow();

private:
    StyleEditorPtr  currentStyleEditor;

    AQTableWidget * styleTable;
    AQTableWidget * parmsTable;
    QVBoxLayout   * parmsLayout;

    QPushButton * delBtn;
    QPushButton * upBtn;
    QPushButton * downBtn;
    QPushButton * dupBtn;
    QPushButton * analyzeBtn;

    QCheckBox  * chkSingle;
    SpinSet    * xRepMin;
    SpinSet    * xRepMax;
    SpinSet    * yRepMin;
    SpinSet    * yRepMax;

    QLineEdit * cleanseLevel;
};

#endif
