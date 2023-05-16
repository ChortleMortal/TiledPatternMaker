#pragma once
#ifndef PAGE_BACKGROUND_IMAGE_H
#define PAGE_BACKGROUND_IMAGE_H

#include "widgets/panel_page.h"
#include "widgets/layout_transform.h"

typedef std::shared_ptr<class BackgroundImage>   BkgdImgPtr;

class AQPushButton;
class QGroupBox;
class QCheckBox;

class page_background_image : public panel_page
{
    Q_OBJECT

public:
    page_background_image(ControlPanel *apanel);

    QGroupBox     * createBackgroundGroup();

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void slot_loadBackground();
    void slot_startSkewAdjustment(bool checked);
    void slot_adjustBackground();
    void slot_saveAdjustedBackground();
    void slot_setBkgdXform();
    void slot_clearBackground();
    void slot_useAdjustedClicked(bool checked);
    void slot_resetXform();

protected:
    void setupBackground(Xform xform);
    void displayBackgroundStatus(bool force);

private:
    BkgdImgPtr      bip;

    LayoutTransform bkgdLayout;
    AQPushButton  * startAdjustBtn;
    QCheckBox     * chk_useAdjusted;
    QLineEdit     * imageName;
    QGroupBox     * bkgdGroup;
};

#endif // PAGE_BACKGROUND_IMAGE_H
