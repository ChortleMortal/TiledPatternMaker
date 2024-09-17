#pragma once
#ifndef PAGE_MOSAIC_INFO_H
#define PAGE_MOSAIC_INFO_H

#include "gui/panels/panel_page.h"

class page_mosaic_info : public panel_page
{
    enum eCols
    {
        COL_STYLE_NAME,
        COL_PROTO,
        COL_TILING_NAME,
        COL_TILE_TYPE
    };

public:
    page_mosaic_info(ControlPanel * panel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

protected:
    void showMotifsFromStyles();

private:
    class AQTableWidget * motifTable;
};

#endif
