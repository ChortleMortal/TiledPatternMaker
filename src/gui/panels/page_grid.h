#pragma once
#ifndef PAGE_GRID_H
#define PAGE_GRID_H

#include "gui/panels/panel_page.h"

class ClickableLabel;

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
    void    slot_gridScreenCenteredChanged(int state);
    void    slot_gridModelCenteredChanged(int state);
    void    slot_gridAngleChanged(qreal angle);
    void    slot_zValueChanged(int value);

    void    slot_gridLayerCenterChanged(int state);
    void    slot_drawModelCenterChanged(int state);
    void    slot_drawViewCenterChanged(int state);
    void    slot_lockToViewChanged(bool enb);

    void    slot_pickColorTiling();
    void    slot_pickColorModel();
    void    slot_pickColorScreen();

    void    slot_resetPos();
    void    slot_reAlign();
//  void    slot_showCenterChanged(int state);

protected:

private:
    QGroupBox    * groupBox;
    QButtonGroup * gridUnitGroup;
    QButtonGroup * gridTypeGroup;

    ClickableLabel * labelT;
    ClickableLabel * labelM;
    ClickableLabel * labelS;
};

#endif
