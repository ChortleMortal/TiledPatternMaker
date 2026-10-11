#pragma once

#include <QGroupBox>
#include "gui/panels/panel_page.h"
#include "gui/viewers/layer.h"
#include "gui/widgets/layout_transform.h"

class AQPushButton;
class AQTableWidget;
class BackgroundImageView;
class FloatableTab;
class MemoryCombo;
class SMXWidget;

class page_backgrounds : public panel_page
{
    Q_OBJECT

public:
    page_backgrounds(ControlPanel *apanel);
    ~page_backgrounds();

    QVBoxLayout * createBackgroundImageLayout();
    QGroupBox * createBackgroundColorGroup();

    void onRefresh() override;
    void onEnter() override;
    void onExit() override;

public slots:
    void slot_detach(int index);

private slots:
    void slot_selectBackground();
    void slot_loadBackground();
    void slot_loadBackgroundReplace();
    void slot_unloadBackground();

    void slot_startKeystoneAdjustment(bool checked);
    void slot_endKeystoneAdjust();

    void slot_startCrop(bool checked);
    void slot_endCrop();

    void slot_setBkgdXform();
    void slot_resetXform();
    void slot_showImageChanged(bool checked);

protected:
    FloatableTab * createImageTab();
    FloatableTab * createColorTab();

    void loadBackground(bool replace);
    void saveModified(bool adjusted);

    void useAdjusted(bool checked);
    void useCropped(bool checked);

    void setupBackgroundPixmap();
    void displayBackgroundImageStatus(bool force);
    void updateImageNameTally();
    void selectColor(Layer::Type vtype);
    bool hasBkgdColor(Layer::Type vtype);
    void reInitBkgdColors(QColor bcolor);

    QString convertJpgToPng(const QString & jpgPath);

private:
    LayoutTransform tformLayout;

    QTabWidget   * tabWidget;
    FloatableTab * imageTab;
    FloatableTab * colorTab;

    AQPushButton * startAdjustBtn;
    AQPushButton * startCropBtn;

    QCheckBox    * chkConvertToPng;
    QCheckBox    * chkShowAdjusted;
    QCheckBox    * chkShowCropped;
    QCheckBox    * chkView;

    MemoryCombo  * imageName;

    AQTableWidget* viewTable;

    SMXWidget    * smxWidget;

};
