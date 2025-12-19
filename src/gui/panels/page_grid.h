#pragma once
#ifndef PAGE_GRID_H
#define PAGE_GRID_H

#include "gui/panels/panel_page.h"

class ClickableLabel;
class SMXWidget;

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
    void    slot_gridScreenCenteredChanged(bool checked);
    void    slot_gridModelCenteredChanged(bool checked);
    void    slot_gridAngleChanged(qreal angle);
    void    slot_zValueChanged(int value);

    void    slot_drawModelCenterChanged(bool checked);
    void    slot_drawViewCenterChanged(bool checked);

    void    slot_pickColorTiling();
    void    slot_pickColorModel();
    void    slot_pickColorScreen();

//  void    slot_showCenterChanged(int state);

protected:

private:
    QGroupBox    * viewBox;
    QGroupBox    * centerBox;

    QButtonGroup * gridUnitGroup;
    QButtonGroup * gridTypeGroup;

    SMXWidget    * smxWidget;

    ClickableLabel * labelT;
    ClickableLabel * labelM;
    ClickableLabel * labelS;
};

#endif
