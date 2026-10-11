#pragma once

#include <QActionGroup>
#include <QButtonGroup>
#include <QGroupBox>
#include "gui/panels/panel_page.h"
#include "model/settings/configuration.h"

class ClickableLabel;
class SMXWidget;
class DoubleSpinSet;

class page_grid : public panel_page
{
    Q_OBJECT

public:
    page_grid(ControlPanel * cpanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void    slot_showGridChanged(bool checked);
    void    slot_gridTypeSelected(int);
    void    slot_gridUnitsChanged(int idx);
    void    slot_gridScreenSpacingChanged(int value);
    void    slot_gridModelSpacingChanged(qreal value);
    void    slot_gridScreenWidthChanged(int value);
    void    slot_gridTilingWidthChanged(int value);
    void    slot_gridModelWidthChanged(int value);
    void    slot_gridAngleChanged(qreal angle);
    void    slot_zValueChanged(int value);

    void    slot_drawModelCenterChanged(bool checked);
    void    slot_drawViewCenterChanged(bool checked);


//  void    slot_showCenterChanged(int state);

protected:

private:
    QHBoxLayout  * createPositionButtons(QButtonGroup * group, eGridUnits units);
    ClickableLabel * createColorPatch(QString & color);
    void           pickColor(QString & color, ClickableLabel * patch);

    QGroupBox    * viewBox;
    QGroupBox    * centerBox;

    QButtonGroup * gridUnitGroup;
    QButtonGroup * gridTypeGroup;
    QButtonGroup * screenPosGroup;
    QButtonGroup * modelPosGroup;
    QPushButton  * btnMove;

    SMXWidget    * smxWidget;
    DoubleSpinSet* angleSpin;

    ClickableLabel * labelT;
    ClickableLabel * labelM;
    ClickableLabel * labelS;
};
