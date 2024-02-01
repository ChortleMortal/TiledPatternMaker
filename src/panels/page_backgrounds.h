#pragma once
#ifndef PAGE_BACKGROUNDS
#define PAGE_BACKGROUNDD

#include "panels/panel_page.h"
#include "widgets/layout_transform.h"

class AQPushButton;
class AQTableWidget;
class BackgroundImageView;

class page_backgrounds : public panel_page
{
    Q_OBJECT

public:
    page_backgrounds(ControlPanel *apanel);

    QGroupBox     * createBackgroundImageGroup();
    QGroupBox     * createBackgroundColorGroup();

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void slot_loadBackground();
    void slot_removeBackground();
    void slot_startSkewAdjustment(bool checked);
    void slot_adjustBackground();
    void slot_saveAdjustedBackground();
    void slot_setBkgdXform();
    void slot_clearBackground();
    void slot_useAdjustedClicked(bool checked);
    void slot_resetXform();
    void slot_showImageChanged(bool checked);

protected:
    void setupBackground(Xform xform);
    void displayBackgroundStatus(bool force);
    void selectColor(int row);
    void reInitBkgdColors(QColor bcolor);

private:
    BackgroundImageView * bview;
    LayoutTransform       bkgdLayout;
    AQPushButton        * startAdjustBtn;
    QCheckBox           * chk_useAdjusted;
    QLineEdit           * imageName;
    QCheckBox           * chkShowBkgd;
    AQTableWidget       * viewTable;
};

#endif // PAGE_BACKGROUNDS
