#ifndef PANELVIEWSELECT_H
#define PANELVIEWSELECT_H

#include <QGroupBox>
#include <QButtonGroup>
#include "sys/enums/eviewtype.h"

class QCheckBox;
class Configuration;
class SystemViewController;
class ControlPanel;

class PanelViewSelect : public QGroupBox
{
    Q_OBJECT

public:
    explicit PanelViewSelect();

    void    selectViewer(eViewType vtype, bool enable = true);
    void    deselectGangedViewers();

protected slots:
    void    slot_Viewer_pressed(int id, bool enable);

    void    slot_lockViewClicked(bool enb);
    void    slot_multiSelect(bool enb);
    void    slot_lockStatusChanged();

signals:
    void    sig_reconstructView();

protected:
    void    createGUI();
    void    restoreViewEnables();

    QCheckBox   * cbRawDesignView;
    QCheckBox   * cbMosaicView;
    QCheckBox   * cbPrototypeView;
    QCheckBox   * cbTilingView;
    QCheckBox   * cbProtoMaker;
    QCheckBox   * cbTilingMakerView;
    QCheckBox   * cbMapEditor;
    QCheckBox   * cbBackgroundImage;
    QCheckBox   * cbBMPImage;
    QCheckBox   * cbGrid;
    QCheckBox   * cbDebug;
    QCheckBox   * cbCrop;

    QCheckBox   * cbLockSelect;
    QCheckBox   * cbMultiSelect;

    QButtonGroup  viewerGroup;

private:
    eViewType     soloType;

    Configuration  * config;
    SystemViewController * viewController;
};

#endif // PANELVIEWSELECT_H
