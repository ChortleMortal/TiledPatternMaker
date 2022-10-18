#ifndef PAGE_GRID_H
#define PAGE_GRID_H

class QGroupBox;
class QHBoxLayout;
class QButtonGroup;
class QCheckBox;

#include "widgets/panel_page.h"

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
    void    slot_gridModelWidthChanged(int value);
    void    slot_gridScreenCenteredChanged(int state);
    void    slot_gridModelCenteredChanged(int state);
    void    slot_gridAngleChanged(qreal angle);

//    void    slot_showCenterChanged(int state);

protected:
    QGroupBox   * createGridSection();
    QHBoxLayout * createGridTypeLayout();

private:
    QGroupBox    * gridBox;
    QButtonGroup * gridUnitGroup;
    QButtonGroup * gridTypeGroup;
};

#endif
