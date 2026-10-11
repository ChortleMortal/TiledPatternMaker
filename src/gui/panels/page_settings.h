#pragma once

#include <QActionGroup>
#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include "gui/panels/panel_page.h"

class qtAppLog;
class MemoryCombo;

class page_settings : public panel_page
{
    Q_OBJECT

public:
    page_settings(ControlPanel * cpanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void    slot_selectRootMediaDir();
    void    slot_selectRootImageDir();
    void    slot_selectRootLogDir();


    void    slot_rootMediaIndexChanged(int index);
    void    slot_rootMediaEditChanged();
    void    slot_rootLogIndexChanged(int index);
    void    slot_rootLogEditChanged();
    void    slot_rootImagesIndexChanged(int index);
    void    slot_rootImagesEditChanged();

    void    slot_selectXMLTool();
    void    slot_selectDiffTool();

    void    slot_mediaDefaultChanged(bool checked);
    void    slot_imageDefaultChanged(bool checked);
    void    slot_logDefaultChanged(bool checked);

    void    slot_mode(int id);
    void    slot_darkThemeChanged(int id);
    void    slot_bigScreen(bool enb);

    void    slot_firstBirthday();
    void    slot_eraseDebugFlagHistory();

    void    slot_start_options();
    void    slot_about();
    void    slot_showSupport();
    void    slot_showShortcuts();

    void    slot_XMLToolChanged1();
    void    slot_DiffToolChanged2();

protected:
    void    rootMediaChanged(QString dir);
    void    rootImagesChanged(QString dir);
    void    logPathChanged(QString dir);
    void    updatePaths();
    void    restartApp();

    QGroupBox   * createAppSetup();
    QGroupBox   * createMediaPaths();
    QGroupBox   * createAppControl();
    QGroupBox   * createAppInfo();

private:
    qtAppLog    * log;

    QPushButton * rootMediaBtn;
    QPushButton * rootImagesBtn;
    QPushButton * rootLogBtn;

    MemoryCombo * rootMediaCombo;
    MemoryCombo * rootImagesCombo;
    MemoryCombo * xmlToolCombo;
    MemoryCombo * diffToolCombo;
    MemoryCombo * logPathCombo;
    QLineEdit   * le_logName;

    QCheckBox   * defaultMedia;
    QCheckBox   * defaultImages;
    QCheckBox   * defaultLogs;

    QButtonGroup * btnGroup2;
};
