#pragma once
#ifndef PAGE_CROP_MAKER_H
#define PAGE_CROP_MAKER_H

#include "panels/panel_page.h"
#include "makers/crop_maker/crop_maker.h"


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
    QString getPageStatus() override;

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
    void slot_clip(bool checked);
    void slot_showPainterCrop(bool checked);

protected:
    QWidget * createMosaicCropControls();
    QWidget * createPainterCropControls();

private:
    MosaicCropMaker mosaicCropMaker;
    CropWidget    * mosaicWidget;

    PainterCropMaker painterCropMaker;
    CropWidget     * painterWidget;

    QCheckBox  * chkEmbed;
    QCheckBox  * chkCropOutside;
    QCheckBox  * chkClip;
    QCheckBox  * chkShowMosaic;
    QCheckBox  * chkShowPainter;
};

#endif
