#pragma once

#include <QGroupBox>
#include "gui/panels/panel_page.h"
#include "model/makers/motif_maker.h"
#include "model/settings/configuration.h"

class AQTableWidget;



class page_prototype_info : public panel_page
{
    Q_OBJECT

    enum class ProtoCol
    {
        Proto = 0,
        Tiling,
        Show
    };

    enum class MotifCol
    {
        Proto = 0,
        Tiling,
        TileMotif,
        Tile,
        Motif,
        Show
    };

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
    void    drawExtBoundaryClicked(bool enb);
    void    drawTileClicked(bool enb);
    void    hiliteMotifClicked(bool enb);
    void    hiliteTileClicked(bool enb);
    void    setDefaultColors();
    void    setDefaultTUColors();
    void    populateTileMotifTable();
    void    populateProtoTable();

    void    singleClicked(bool enb);
    void    placedClicked(bool enb);
    void    visibleTilesClicked(bool enb);
    void    visibleMotifsClicked(bool enb);

protected:
    QGroupBox * buildPrototypeLayout();
    QGroupBox * buildTilingUnitLayout();

    void    protoChecked(int  row,bool checked);
    void    tileMotifChecked(int  row,bool checked);

    void    buildColorGrid();
    void    buildColorGrid2();
    void    addSwatch(QGridLayout * grid, int row, int col, QColor & color);
    void    refreshSwatches();
    void    populateTables();
    void    setProtoViewMode(eProtoViewMode mode, bool enb);
    void    pickColor(QColor & color);

private:
    AQTableWidget       * tileMotifTable;
    AQTableWidget       * protoTable;
    QGridLayout         * showSettings;
    QGridLayout         * showSettings2;
    MotifMaker          * motifMaker;

    QVector<std::pair<class ClickableLabel*,QColor*>> swatches;

};
