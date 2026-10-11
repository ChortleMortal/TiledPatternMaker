#pragma once

#include "gui/panels/panel_page.h"

class FloatableTab;
class page_layer_algnment;
class page_layers;

class page_layers_tabs : public panel_page
{
    Q_OBJECT

public:
    page_layers_tabs(ControlPanel * cpanel);
    ~page_layers_tabs();

    void onEnter()          override;
    void onExit()           override;
    void onRefresh()        override;

public slots:
    void slot_detach(int index);

protected:
    FloatableTab * createTab(panel_page * page);

private:
    QTabWidget          * tabWidget;
    FloatableTab        * alignTab;
    FloatableTab        * infoTab;

    page_layer_algnment * alignPage;
    page_layers         * infoPage;
};
