#pragma once
#ifndef PAGE_PROTOS_H
#define PAGE_PROTOS_H

#include "gui/panels/panel_page.h"
#include "model/makers/prototype_maker.h"
#include "model/settings/configuration.h"

class DoubleSpinSet;

class AQTableWidget;

enum eDELCol
{
    DEL_COL_PROTO,
    DEL_COL_TILING,
    DEL_COL_DEL,
    DEL_COL_TILE,
    DEL_COL_MOTIF,
    DEL_COL_SHOW_DEL
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
    void    slot_widthChanged(int val);
    void    drawProtoClicked(bool enb);
    void    drawMapClicked(bool enb);
    void    drawMotifClicked(bool enb);
    void    drawTileClicked(bool enb);
    void    hiliteMotifClicked(bool enb);
    void    hiliteTileClicked(bool enb);
    void    setDefaultColors();
    void    populateDelTable();
    void    populateProtoTable();

    void    singleClicked(bool enb);
    void    placedClicked(bool enb);
    void    visibleTilesClicked(bool enb);
    void    visibleMotifsClicked(bool enb);

protected:
    QGroupBox * buildPrototypeLayout();
    QGroupBox * buildTilingUnitLayout();
    QGroupBox * buildDistortionsLayout();

    void    protoChecked(int  row,bool checked);
    void    DELChecked(int  row,bool checked);

    void    buildColorGrid();
    void    buildColorGrid2();
    void    populateTables();
    void    setProtoViewMode(eProtoViewMode mode, bool enb);
    void    pickColor(QColor & color);

    void    enbDistortion(bool checked);
    void    setDistortion();

private:
    AQTableWidget       * DELTable;
    AQTableWidget       * protoTable;
    QGridLayout         * showSettings;
    QGridLayout         * showSettings2;
    PrototypeMaker      * protoMaker;

    QCheckBox     * chkDistort;
    DoubleSpinSet * xBox;
    DoubleSpinSet * yBox;
};

#endif
