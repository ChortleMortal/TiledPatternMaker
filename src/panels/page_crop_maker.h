#pragma once
#ifndef PAGE_CROP_MAKER_H
#define PAGE_CROP_MAKER_H

#include "widgets/panel_page.h"
#include "makers/crop_maker/crop_maker.h"

class CropWidget;

class page_crop_maker : public panel_page
{
    Q_OBJECT

#define NUM_BORDER_COLORS 2

public:
    page_crop_maker(ControlPanel * apanel);

    void onRefresh() override;
    void onEnter()   override;
    void onExit()    override;

private slots:
    void display();

    void slot_createCrop();
    void slot_embedCrop(bool checked);
    void slot_applyCrop(bool checked);
    void slot_removeCrop();
    void slot_fetchBorder();

protected:
    QHBoxLayout * createCropControls();

private:
    CropMaker    cropMaker;

    CropViewer * cropViewer;
    CropWidget * cropWidget;

    QCheckBox  * chkEmbed;
    QCheckBox  * chkApply;
    QGroupBox  * cropbox2;
};

#endif
