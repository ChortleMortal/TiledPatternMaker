#ifndef PAGE_CROP_MAKER_H
#define PAGE_CROP_MAKER_H

#include "widgets/panel_page.h"

class QStackedLayout;
class QGroupBox;
class QHBoxLayout;
class QPushButton;

class CropWidget;
class ClickableLabel;
class CropMaker;
class CropView;

typedef std::shared_ptr<class Crop> CropPtr;
typedef std::shared_ptr<class CropView>   CropViewPtr;

class page_crop_maker : public panel_page
{
    Q_OBJECT

#define NUM_BORDER_COLORS 2

public:
    page_crop_maker(ControlPanel * apanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void display();

    void slot_createCrop();
    void slot_embedCrop();
    void slot_applyCrop();
    void slot_editCrop();
    void slot_removeCrop();
    void slot_fetchBorder();
    void slot_centerCrop();

protected:
    QHBoxLayout * createCropControls();

    CropPtr  getMosaicCrop();

private:
    CropMaker       * cropMaker;
    CropViewPtr       crview;

    QPushButton     * pbEditing;
    CropWidget      * cropWidget;
    QStackedLayout  * stack;
    QGroupBox       * cropbox2;

    CropPtr           lastCrop;

};

#endif
