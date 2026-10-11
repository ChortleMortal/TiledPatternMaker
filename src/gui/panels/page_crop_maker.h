#pragma once

#include <QCheckBox>
#include "gui/panels/panel_page.h"
#include "model/makers/crop_maker.h"

class CropWidget;

class page_crop_maker : public panel_page
{
    Q_OBJECT

#define NUM_BORDER_COLORS 2

public:
    page_crop_maker(ControlPanel * apanel);

    void onRefresh()        override;
    void onEnter()          override;
    void onExit()           override {}

    // reused by page_mosaic_maker's crop-visibility checkboxes so both pages
    // stay in sync through the same chkShowMosaic/chkShowPainter checkboxes
    void setShowMosaicCrop(bool show)  { chkShowMosaic->setChecked(show); }
    void setShowPainterCrop(bool show) { chkShowPainter->setChecked(show); }

private slots:
    void display();

    void slot_createMosaicCrop();
    void slot_removeMosaicCrop();
    void slot_applyCrop();
    void slot_fetchBorder();
    void slot_embedCrop(bool checked);
    void slot_cropOutside(bool checked);
    void slot_showMosaicCrop(bool checked);

    void slot_createPainterCrop();
    void slot_removePainterCrop();
    void slot_fetchBorderForPainter();
    void slot_fetchMosaicCrop();
    void slot_showPainterCrop(bool checked);

protected:
    QGridLayout * createMosaicCropControls();
    QGridLayout * createPainterCropControls();

private:
    MosaicCropMaker mosaicCropMaker;
    CropWidget    * mosaicWidget;

    PainterCropMaker painterCropMaker;
    CropWidget     * painterWidget;

    QCheckBox  * chkEmbed;
    QCheckBox  * chkCropOutside;
    QCheckBox  * chkShowMosaic;
    QCheckBox  * chkShowPainter;
};
