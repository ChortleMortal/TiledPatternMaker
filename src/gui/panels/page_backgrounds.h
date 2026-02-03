#pragma once
#ifndef PAGE_BACKGROUNDS
#define PAGE_BACKGROUNDD

#include "gui/panels/panel_page.h"
#include "gui/widgets/layout_transform.h"

class AQPushButton;
class AQTableWidget;
class BackgroundImageView;
class SMXWidget;

class page_backgrounds : public panel_page
{
    Q_OBJECT

public:
    page_backgrounds(ControlPanel *apanel);

    QGroupBox * createBackgroundImageGroup();
    QGroupBox * createBackgroundColorGroup();

    void onRefresh() override;
    void onEnter() override;
    void onExit() override;

private slots:
    void slot_loadBackground();
    void slot_removeBackground();
    void slot_clearBackground();

    void slot_startKeystoneAdjustment(bool checked);
    void slot_endKeystoneAdjust();

    void slot_startCrop(bool checked);
    void slot_endCrop();

    void slot_setBkgdXform();
    void slot_resetXform();
    void slot_showImageChanged(bool checked);

protected:
    void saveModified(bool adjusted);

    void useAdjusted(bool checked);
    void useCropped(bool checked);

    void setupBackgroundPixmap();
    void displayBackgroundImageStatus(bool force);
    void selectColor(int row);
    void reInitBkgdColors(QColor bcolor);

private:
    LayoutTransform tformLayout;

    AQPushButton * startAdjustBtn;
    AQPushButton * startCropBtn;

    QCheckBox    * chkShowAdjusted;
    QCheckBox    * chkShowCropped;
    QCheckBox    * chkView;

    QLineEdit    * imageName;

    AQTableWidget* viewTable;

    SMXWidget    * smxWidget;

    QRadioButton * btnNone;
    QRadioButton * btnMosaic;
    QRadioButton * btnTiling;
    QRadioButton * btnMaped;
    QRadioButton * btnDefine;
};

#endif // PAGE_BACKGROUNDS
