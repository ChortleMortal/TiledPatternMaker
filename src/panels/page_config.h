#pragma once
#ifndef PAGE_CONFIG_H
#define PAGE_CONFIG_H

class QPushButton;
class QCheckBox;
class QGroupBox;
class QLineEdit;
class QButtonGroup;

#include "widgets/panel_page.h"

class page_config : public panel_page
{
    Q_OBJECT

public:
    page_config(ControlPanel * cpanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void    slot_selectRootMediaDir();
    void    slot_selectRootImageDir();
    void    slot_selectXMLTool();
    void    slot_selectDiffTool();

    void    slot_rootDesignChanged(QString txt);
    void    slot_rootImageChanged(QString txt);
    void    slot_designDefaultChanged(bool checked);
    void    slot_imageDefaultChanged(bool checked);

    void    slot_reconfigurePaths();
    void    slot_mode(int id);
    void    slot_darkThemeChanged(int id);
    void    slot_about();
    void    slot_showCenterChanged(int state);
    void    slot_updateClicked(bool enb);

protected:
    void    updatePaths();
    void    restartApp();

    QGroupBox   * createViewControl();

private:
    QPushButton * rootMediaBtn;
    QPushButton * rootImagesBtn;

    QLineEdit   * le_rootMedia;
    QLineEdit   * le_rootImages;
    QLineEdit   * le_xmlTool;
    QLineEdit   * le_diffTool;
    QLineEdit   * le_logName;

    QCheckBox   * defaultDesigns;
    QCheckBox   * defaultImages;

    QButtonGroup * btnGroup2;
};

#endif
