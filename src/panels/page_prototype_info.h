#ifndef PAGE_PROTOS_H
#define PAGE_PROTOS_H

#include <QGridLayout>
#include "widgets/panel_page.h"
#include "settings/configuration.h"
#include "viewers/prototype_view.h"



enum eProtoCol
{
    PROTO_ROW_PROTO,
    PROTO_ROW_SHOW,
    PROTO_ROW_TILING,
    PROTO_ROW_DEL,
    PROTO_ROW_TILE,
    PROTO_ROW_MOTIF
};



class page_prototype_info : public panel_page
{
    Q_OBJECT

public:

    page_prototype_info(ControlPanel * cpanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override;

public slots:

private slots:
    void    slot_prototypeSelected(int row, int col);
    void    slot_showMotifChanged(int col, bool checked);
    void    slot_widthChanged(int val);
    void    drawDELClicked(bool enb);
    void    drawMapClicked(bool enb);
    void    drawMotifClicked(bool enb);
    void    drawTileClicked(bool enb);
    void    hiliteMotifClicked(bool enb);
    void    hiliteTileClicked(bool enb);
    void    setDefaultColors();
    void    setup();
    void    slot_deselect();

protected:
    void    buildColorGrid();
    void    setProtoViewMode(eProtoViewMode mode, bool enb);
    void    pickColor(QColor & color);

private:
    class AQTableWidget * protoTable;
    QGridLayout         * showSettings;
    PrototypeViewPtr      pview;

    QVector<sColData>     data;

};

#endif
