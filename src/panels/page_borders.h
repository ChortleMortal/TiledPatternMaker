#ifndef PAGE_BORDERS_H
#define PAGE_BORDERS_H

#include "widgets/panel_page.h"
#include "widgets/layout_qrectf.h"

class AQComboBox;
class QGroupBox;
class QStackedLayout;

class SpinSet;
class ClickableLabel;

typedef std::shared_ptr<class Border>  BorderPtr;
typedef std::shared_ptr<class Crop>    CropPtr;
typedef std::shared_ptr<class BorderView>   BorderViewPtr;

class page_borders : public panel_page
{
    Q_OBJECT

public:
    page_borders(ControlPanel * apanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

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
    void slot_borderRowsChanged(int rows);
    void slot_borderColsChanged(int cols);
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
    BorderViewPtr      brview;
    AQComboBox      *  borderTypes;
    AQComboBox      *  cropTypes;

    QGroupBox       * borderTypeBox;
    QGroupBox       * cropTypeBox;

    QWidget         * outerBorderWidget;
    QWidget         * nullBorderWidget;
    QWidget         * nullBorderWidget2;

    QStackedLayout  * borderTypeStack;
    QStackedLayout  * cropTypeStack;

    DoubleSpinSet   * borderWidth[3];
    DoubleSpinSet   * radius;
    SpinSet         * borderRows;
    SpinSet         * borderCols;
    QLabel          * borderColorLabel[4];
    QLineEdit       * borderColor[4];
    ClickableLabel  * borderColorPatch[4];

    LayoutQRectF    * rectBoundaryLayout;
    LayoutQPointF   * centre;

};

#endif
