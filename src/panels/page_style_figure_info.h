#ifndef PAGE_FIGURES_H
#define PAGE_FIGURES_H

#include "widgets/panel_page.h"

class page_mosaic_info : public panel_page
{
    enum eCols
    {
        COL_STYLE_NAME,
        COL_TILING_NAME,
        COL_FIGURE_TYPE
    };

public:
    page_mosaic_info(ControlPanel * panel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

protected:
    void showFiguresFromStyles();

private:
    class AQTableWidget * figureTable;
};

#endif
