#pragma once
#ifndef PAGE_BORDERS_H
#define PAGE_BORDERS_H

#include "gui/panels/panel_page.h"
#include "gui/widgets/layout_qrectf.h"

class AQComboBox;
class ClickableLabel;

typedef std::shared_ptr<class Border>  BorderPtr;
typedef std::shared_ptr<class Crop>    CropPtr;

class page_borders;

class PlainControl : public QWidget
{
    Q_OBJECT

public:
    PlainControl(page_borders * parent);

    DoubleSpinSet   * borderWidth;

    QLabel          * borderColorLabel;
    QLineEdit       * borderColor;
    ClickableLabel  * borderColorPatch;

private:
    page_borders    * parent;
};

class TwoColorControl : public QWidget
{
    Q_OBJECT

public:
    TwoColorControl(page_borders * parent);

    DoubleSpinSet   * borderWidth;
    DoubleSpinSet   * borderLength;

    QLabel          * borderColorLabel;
    QLineEdit       * borderColor;
    ClickableLabel  * borderColorPatch;

    QLabel          * borderColorLabel2;
    QLineEdit       * borderColor2;
    ClickableLabel  * borderColorPatch2;

private:
    page_borders    * parent;
};

class BlockControl : public QWidget
{
    Q_OBJECT

public:
    BlockControl(page_borders * parent);

    DoubleSpinSet   * borderWidth;

    SpinSet         * borderRows;
    SpinSet         * borderColumns;

    QLabel          * borderColorLabel;
    QLineEdit       * borderColor;
    ClickableLabel  * borderColorPatch;

private:
    page_borders    * parent;
};

class page_borders : public panel_page
{
    Q_OBJECT

public:
    page_borders(ControlPanel * apanel);

    void onRefresh()        override;
    void onEnter()          override {}
    void onExit()           override {}
    QString getPageStatus() override;

public slots:
    void slot_removeBorder();
    void slot_loadFromCrop();

    void slot_borderTypeChanged(int row);
    void slot_cropTypeChanged(int row);
    void slot_pickBorderColor();
    void slot_pickBorderColor2();
    void slot_borderColorChanged(const QString & text);
    void slot_borderColor2Changed(const QString & text);
    void slot_borderWidthChanged(qreal width);
    void slot_borderLengthChanged(qreal length);
    void slot_borderRowsChanged(int rows);
    void slot_borderColumnsChanged(int cols);
    void slot_useViewSzChanged(bool checked);
    void slot_rectBoundaryChanged();
    void slot_centreChanged();
    void slot_radiusChanged(qreal r);

protected:
    void        createBorder();
    void        displayRectShape(BorderPtr bp);
    void        refreshBorderType(BorderPtr border);
    void        refreshBorderCrop(BorderPtr border);

    BorderPtr   getMosaicBorder();

    QWidget   * createBorderTypeWidget();
    QWidget   * createUndefinedShapeWidget();
    QWidget   * createRectShapeWidget();
    QWidget   * createCircleShapeWidget();
    QWidget   * createPolyShapeWidget();
    QWidget   * createBorderTypeNone();

private:
    QWidget         * controlNone;
    PlainControl    * controlPlain;
    TwoColorControl * controlTwoColors;
    BlockControl    * controlBlocks;

    AQComboBox      *  borderTypes;
    AQComboBox      *  cropTypes;

    QGroupBox       * borderTypeBox;
    QGroupBox       * cropTypeBox;

    QWidget         * outerBorderWidget;
    QWidget         * nullBorderWidget;
    QWidget         * nullBorderWidget2;

    QStackedLayout  * borderTypeStack;
    QStackedLayout  * cropTypeStack;

    DoubleSpinSet   * radius;

    LayoutQRectF    * rectBoundaryLayout;
    LayoutQPointF   * centre;

    QCheckBox       * chkUseViewSize;
};

#endif
