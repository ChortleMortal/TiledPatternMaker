#ifndef PAGE_CONFIG_H
#define PAGE_CONFIG_H

class QPushButton;
class QCheckBox;
class QGroupBox;
class QLineEdit;

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

    void    slot_rootDesignChanged(QString txt);
    void    slot_rootImageChanged(QString txt);
    void    slot_designDefaultChanged(bool checked);
    void    slot_imageDefaultChanged(bool checked);
    void    slot_darkThemeChanged(bool checked);

    void    slot_reconfigurePaths();
    void    slot_mode(int id);
    void    slot_about();
    void    slot_showCenterChanged(int state);

protected:
    void    updatePaths();
    void    restartApp();

    QGroupBox   * createViewControl();

private:
    QPushButton * rootMediaBtn;
    QPushButton * rootImagesBtn;
    QPushButton * xmlToolBtn;

    QLineEdit   * le_rootMedia;
    QLineEdit   * le_rootImages;
    QLineEdit   * le_xmlTool;

    QCheckBox   * defaultDesigns;
    QCheckBox   * defaultImages;
};

#endif
