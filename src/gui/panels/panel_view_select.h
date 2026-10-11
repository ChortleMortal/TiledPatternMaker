#pragma once

#include <QGroupBox>
#include <QButtonGroup>
#include <QCheckBox>
#include "gui/viewers/layer.h"
#include "model/settings/configuration.h"

class ViewController;
class ControlPanel;

class PanelViewSelect : public QGroupBox
{
    Q_OBJECT

public:
    explicit PanelViewSelect();

    void    selectViewer(Layer::Type vtype, bool enable);
    void    syncLockCheckbox()    { cbLockSelect->setChecked(config->lockView); }

    // for checkboxes elsewhere (e.g. page_mosaic_maker, page_backgrounds, page_grid, page_debug) that
    // mirror one of these layer enables - a direct user click, so unlike selectViewer() it is not
    // subject to Lock Layers, which is only meant to stop automatic/programmatic view switching
    void    userSelectViewer(Layer::Type vtype, bool enable)    { slot_Viewer_pressed((int)vtype, enable); }

protected slots:
    void    slot_Viewer_pressed(int id, bool enable);
    void    slot_lockViewClicked(bool enb);
    void    slot_multiSelect(bool enb);

signals:
    void    sig_reconstructView();

private:
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
    QCheckBox   * cbConstructionLines;
    QCheckBox   * cbDebug;
    QCheckBox   * cbCrop;

    QCheckBox   * cbLockSelect;
    QCheckBox   * cbMultiSelect;

    QButtonGroup  viewerGroup;

    Layer::Type      soloType;

    Configuration  * config;
    ViewController * viewController;
};
