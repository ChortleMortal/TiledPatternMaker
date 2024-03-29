﻿#pragma once
#ifndef PAGE_BORDERS_H
#define PAGE_BORDERS_H

#include "panels/panel_page.h"
#include "enums/eborder.h"
#include "widgets/layout_qrectf.h"

class AQComboBox;
class ClickableLabel;

typedef std::shared_ptr<class Border>  BorderPtr;
typedef std::shared_ptr<class Crop>    CropPtr;

class page_borders : public panel_page
{
    Q_OBJECT

public:
    page_borders(ControlPanel * apanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override;

private slots:
    void slot_removeBorder();
    void slot_loadFromCrop();

    void slot_borderTypeChanged(int row);
    void slot_cropTypeChanged(int row);
    void slot_pickBorderColor();
    void slot_pickBorderColor2();
    void slot_borderColorChanged(QLineEdit * le);
    void slot_borderColor2Changed(QLineEdit * le);
    void slot_borderWidthChanged(qreal width);
    void slot_borderLengthChanged(qreal length);
    void slot_borderRowsChanged(int rows);
    void slot_borderColsChanged(int cols);
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
    QWidget   * createBorderTypePlain();
    QWidget   * createBorderType2Color();
    QWidget   * createBorderTypeBlocks();

private:
    class BorderView*  borderView;
    AQComboBox      *  borderTypes;
    AQComboBox      *  cropTypes;

    QGroupBox       * borderTypeBox;
    QGroupBox       * cropTypeBox;

    QWidget         * outerBorderWidget;
    QWidget         * nullBorderWidget;
    QWidget         * nullBorderWidget2;

    QStackedLayout  * borderTypeStack;
    QStackedLayout  * cropTypeStack;

    DoubleSpinSet   * borderWidth[BORDER_BLOCKS+1];
    DoubleSpinSet   * borderLength;
    DoubleSpinSet   * radius;
    SpinSet         * borderRows;
    SpinSet         * borderCols;
    QLabel          * borderColorLabel[BORDER_BLOCKS+1];
    QLineEdit       * borderColor[BORDER_BLOCKS+1];
    ClickableLabel  * borderColorPatch[BORDER_BLOCKS+1];

    LayoutQRectF    * rectBoundaryLayout;
    LayoutQPointF   * centre;

    QCheckBox       * chkUseViewSize;
};

#endif
