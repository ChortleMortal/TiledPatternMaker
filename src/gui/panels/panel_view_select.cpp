#include <QCheckBox>

#include "gui/widgets/panel_misc.h"
#include "gui/panels/panel_view_select.h"
#include "gui/top/controlpanel.h"
#include "gui/top/view_controller.h"
#include "model/settings/configuration.h"
#include "sys/sys.h"
#include "sys/tiledpatternmaker.h"

extern TiledPatternMaker * theApp;

PanelViewSelect::PanelViewSelect()
{
    setTitle("Layer Enables");
    setStyleSheet("QGroupBox { border: none; margin-top: 1.2em; } QGroupBox::title { subcontrol-origin: margin; left: 0px; } QCheckBox { padding-left: 0px; }");

    config              = Sys::config;
    viewController      = Sys::systemViewController;

    createGUI();

    connect(this, &PanelViewSelect::sig_reconstructView, viewController, &ViewController::slot_reconstructView);
}

void PanelViewSelect::createGUI()
{
    cbMosaicView         = new QCheckBox("Mosaic");

    cbPrototypeView      = new QCheckBox("Prototype");

    cbMapEditor          = new QCheckBox("Map Editor");

    cbProtoMaker         = new QCheckBox("Motif Maker");

    cbTilingView         = new QCheckBox("Tiling");

    cbTilingMakerView    = new QCheckBox("Tiling Maker");

    cbRawDesignView      = new QCheckBox("Fixed Design");

    cbBackgroundImage    = new QCheckBox("Bkgd Image");

    cbBMPImage           = new QCheckBox("Image Viewer");

    cbGrid               = new QCheckBox("Grid");

    cbConstructionLines  = new QCheckBox("C-Lines");

    cbDebug              = new QCheckBox("Debug");

    cbCrop               = new QCheckBox("Crop");

    cbMultiSelect        = new QCheckBox("Multi Layer");
    cbMultiSelect->setChecked(config->multiView);

    cbLockSelect         = new QCheckBox("Lock Layers");
    cbLockSelect->setChecked(config->lockView);

    AQVBoxLayout * avbox = new AQVBoxLayout();

    avbox->addWidget(cbMosaicView);
    avbox->addWidget(cbPrototypeView);
    if (config->insightMode)
        avbox->addWidget(cbMapEditor);
    avbox->addWidget(cbProtoMaker);
    avbox->addWidget(cbTilingView);
    avbox->addWidget(cbTilingMakerView);
    avbox->addWidget(cbRawDesignView);
    if (config->insightMode)
        avbox->addWidget(cbBMPImage);
    avbox->addSpacing(7);
    avbox->addWidget(cbBackgroundImage);
    avbox->addWidget(cbCrop);
    avbox->addWidget(cbGrid);
    avbox->addWidget(cbConstructionLines);
    if (config->insightMode)
        avbox->addWidget(cbDebug);
    avbox->addSpacing(7);
    avbox->addWidget(cbLockSelect);
    avbox->addWidget(cbMultiSelect);

    setLayout(avbox);

    // viewer group
    viewerGroup.addButton(cbRawDesignView,      (int)Layer::Type::LegacyDesign);
    viewerGroup.addButton(cbMosaicView,         (int)Layer::Type::Mosaic);
    viewerGroup.addButton(cbPrototypeView,      (int)Layer::Type::Prototype);
    viewerGroup.addButton(cbProtoMaker,         (int)Layer::Type::MotifMaker);
    viewerGroup.addButton(cbTilingView,         (int)Layer::Type::Tiling);
    viewerGroup.addButton(cbTilingMakerView,    (int)Layer::Type::TilingMaker);
    if (config->insightMode)
        viewerGroup.addButton(cbMapEditor,      (int)Layer::Type::MapEditor);
    viewerGroup.addButton(cbBackgroundImage,    (int)Layer::Type::BkgdImage);
    viewerGroup.addButton(cbCrop,               (int)Layer::Type::Crop);
    if (config->insightMode)
        viewerGroup.addButton(cbBMPImage,       (int)Layer::Type::BMPImage);
    viewerGroup.addButton(cbGrid,               (int)Layer::Type::Grid);
    viewerGroup.addButton(cbConstructionLines,  (int)Layer::Type::ConstructionLines);
    if (config->insightMode)
        viewerGroup.addButton(cbDebug,          (int)Layer::Type::DebugLayer);

    // this class handles the exclusivity, not the QButtonGroup,
    // because an exclusive button group cannot have no buttons enabled
    viewerGroup.setExclusive(false);

    connect(&viewerGroup, &QButtonGroup::idToggled,           this, &PanelViewSelect::slot_Viewer_pressed);
    connect(cbLockSelect, &QCheckBox::clicked,                this, &PanelViewSelect::slot_lockViewClicked);
    connect(cbMultiSelect,&QCheckBox::clicked,                this, &PanelViewSelect::slot_multiSelect);
}

// the front  door
void  PanelViewSelect::selectViewer(Layer::Type vtype, bool enable)
{
    int type = (int)vtype;

    if (config->lockView)
    {
        return;
    }

    if (enable)
    {
        if (!viewerGroup.button(type)->isChecked())
        {
            viewerGroup.button(type)->setChecked(true);
            emit viewerGroup.idClicked(type);
        }
        else
        {
            slot_Viewer_pressed(type,true);
        }
    }
    else
    {
        if (viewerGroup.button(type)->isChecked())
        {
            viewerGroup.button(type)->setChecked(false);
            emit viewerGroup.idClicked(type);
        }
        else
        {
            slot_Viewer_pressed(type,false);
        }
    }
}

void  PanelViewSelect::slot_Viewer_pressed(int id, bool enable)
{
    qDebug().noquote() << sLayerType[id] << (enable ? "enabled" : "disabled");

    Layer::Type viewType = static_cast<Layer::Type>(id);

    if (Sys::systemViewController->isGangMember(viewType))
    {
        // thse are the prime views
        // deal with exclusivity (the standard mode)
        if (enable)
        {
            if (!config->multiView)
            {
                // uncheck the last exclusive
                viewerGroup.blockSignals(true);
                viewerGroup.button((int)viewController->getMostRecentGangEnable())->setChecked(false);
                viewerGroup.button(id)->setChecked(true);
                viewerGroup.blockSignals(false);
            }
        }

        if (!config->multiView)
        {
            viewController->disableGang();
        }

        if (viewerGroup.button(id)->isChecked() != enable)
        {
            // a gang member changed via something other than this checkbox's own click
            // (e.g. userSelectViewer() called programmatically, or session restore in multi-layer
            // mode, where the exclusivity block above is skipped) - keep the LHS checkbox in sync
            viewerGroup.blockSignals(true);
            viewerGroup.button(id)->setChecked(enable);
            viewerGroup.blockSignals(false);
        }
    }
    else if (viewerGroup.button(id)->isChecked() != enable)
    {
        // an overlay/additive layer changed via something other than this checkbox's own click
        // (e.g. userSelectViewer() called from another page's checkbox, or session restore) -
        // keep the LHS checkbox in sync
        viewerGroup.blockSignals(true);
        viewerGroup.button(id)->setChecked(enable);
        viewerGroup.blockSignals(false);
    }

    viewController->viewEnable(viewType,enable);

    emit sig_reconstructView();
}

void PanelViewSelect::slot_multiSelect(bool enb)
{
    if (!enb)
    {
        auto vbuttons          = viewerGroup.buttons();
        Layer::Type lastSelected = viewController->getMostRecentGangEnable();
        for (auto button : std::as_const(vbuttons))
        {
            Layer::Type vtype = (Layer::Type)viewerGroup.id(button);
            if (vtype == lastSelected)
            {
                continue;
            }
            if (viewController->isGangMember(vtype))
            {
                button->setChecked(false);
            }
        }
        // sometimes this results in the gang being empty - but this is OK
        emit sig_reconstructView();
    }
    config->multiView = enb;
}

void  PanelViewSelect::slot_lockViewClicked(bool enb)
{
    config->lockView = enb;
}
