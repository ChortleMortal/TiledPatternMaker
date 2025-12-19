#pragma once
#ifndef PAGE_PROTOS_H
#define PAGE_PROTOS_H

#include "gui/panels/panel_page.h"
#include "gui/viewers/prototype_view.h"
#include "model/makers/prototype_maker.h"
#include "model/settings/configuration.h"

class AQTableWidget;
class ProtoMakerData;

enum eDELCol
{
    DEL_COL_PROTO,
    DEL_COL_TILING,
    DEL_COL_DEL,
    DEL_COL_TILE,
    DEL_COL_MOTIF,
    DEL_COL_SHOW_MOTIF
};

enum eProtoCol
{
    PROTO_COL_PROTO,
    PROTO_COL_TILING,
    PROTO_COL_SHOW
};

class page_prototype_info : public panel_page
{
    Q_OBJECT

public:

    page_prototype_info(ControlPanel * cpanel);

    void onRefresh()        override;
    void onEnter()          override;
    void onExit()           override;

public slots:

private slots:
    void    slot_DELSelected(int row, int col);
    void    slot_protoSelected(int row, int col);
    void    slot_widthChanged(int val);
    void    drawProtoClicked(bool enb);
    void    drawMapClicked(bool enb);
    void    drawMotifClicked(bool enb);
    void    drawTileClicked(bool enb);
    void    hiliteMotifClicked(bool enb);
    void    hiliteTileClicked(bool enb);
    void    setDefaultColors();
    void    setupDelTable();
    void    setupProtoTable();

    void    allVisibleClicked(bool enb);
    void    visibleTilesClicked(bool enb);
    void    visibleMotifsClicked(bool enb);
    void    slot_hide();
    void    slot_show();

protected:
    void    buildColorGrid();
    void    populateTables();
    void    setProtoViewMode(eProtoViewMode mode, bool enb);
    void    pickColor(QColor & color);

private:
    AQTableWidget       * DELTable;
    AQTableWidget       * protoTable;
    QGridLayout         * showSettings;
    PrototypeMaker      * protoMaker;
};

#endif
