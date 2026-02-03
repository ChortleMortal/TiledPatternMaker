#include <QCheckBox>

#include "gui/panels/panel_misc.h"
#include "gui/panels/panel_view_select.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "model/settings/configuration.h"
#include "sys/sys.h"
#include "sys/tiledpatternmaker.h"

extern TiledPatternMaker * theApp;

PanelViewSelect::PanelViewSelect()
{
    setTitle("Layer Enable");

    config              = Sys::config;
    viewController      = Sys::viewController;

    createGUI();

    connect(this, &PanelViewSelect::sig_reconstructView, viewController, &SystemViewController::slot_reconstructView);
}

void PanelViewSelect::createGUI()
{
    cbMosaicView         = new QCheckBox("Mosaic");
    cbMosaicView->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbPrototypeView      = new QCheckBox("Prototype");
    cbPrototypeView->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbMapEditor          = new QCheckBox("Map Editor");
    cbMapEditor->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbProtoMaker         = new QCheckBox("Motif Maker");
    cbProtoMaker->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbTilingView         = new QCheckBox("Tiling");
    cbTilingView->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbTilingMakerView    = new QCheckBox("Tiling Maker");
    cbTilingMakerView->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbRawDesignView      = new QCheckBox("Fixed Design");
    cbRawDesignView->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbBackgroundImage    = new QCheckBox("Bkgd Image");
    cbBackgroundImage->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbBMPImage          = new QCheckBox("BMP Image");
    cbBMPImage->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbGrid               = new QCheckBox("Grid");
    cbGrid->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbDebug              = new QCheckBox("Debug");
    cbDebug->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbCrop             = new QCheckBox("Crop");
    cbCrop->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbMultiSelect          = new QCheckBox("Multi Layer");
    cbMultiSelect->setStyleSheet("QCheckBox {padding-left: 3px;}");
    cbMultiSelect->setChecked(config->multiView);

    cbLockSelect           = new QCheckBox("Lock Layers");
    cbLockSelect->setStyleSheet("QCheckBox {padding-left: 3px;}");
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
    if (config->insightMode)
        avbox->addWidget(cbDebug);
    avbox->addSpacing(7);
    avbox->addWidget(cbLockSelect);
    avbox->addWidget(cbMultiSelect);

    setLayout(avbox);

    // viewer group
    viewerGroup.addButton(cbRawDesignView,      VIEW_LEGACY);
    viewerGroup.addButton(cbMosaicView,         VIEW_MOSAIC);
    viewerGroup.addButton(cbPrototypeView,      VIEW_PROTOTYPE);
    viewerGroup.addButton(cbProtoMaker,         VIEW_MOTIF_MAKER);
    viewerGroup.addButton(cbTilingView,         VIEW_TILING);
    viewerGroup.addButton(cbTilingMakerView,    VIEW_TILING_MAKER);
    if (config->insightMode)
        viewerGroup.addButton(cbMapEditor,      VIEW_MAP_EDITOR);
    viewerGroup.addButton(cbBackgroundImage,    VIEW_BKGD_IMG);
    viewerGroup.addButton(cbCrop,               VIEW_CROP);
    if (config->insightMode)
        viewerGroup.addButton(cbBMPImage,       VIEW_BMP_IMAGE);
    viewerGroup.addButton(cbGrid,               VIEW_GRID);
    if (config->insightMode)
        viewerGroup.addButton(cbDebug,          VIEW_DEBUG);

    // this class handles the exclusivity, not the QButtonGroup,
    // because an exclusive button group cannot have no buttons enabled
    viewerGroup.setExclusive(false);

    connect(&viewerGroup, &QButtonGroup::idToggled,           this, &PanelViewSelect::slot_Viewer_pressed);
    connect(cbLockSelect, &QCheckBox::clicked,                this, &PanelViewSelect::slot_lockViewClicked);
    connect(cbMultiSelect,&QCheckBox::clicked,                this, &PanelViewSelect::slot_multiSelect);
    connect(theApp,       &TiledPatternMaker::sig_lockStatus, this, &PanelViewSelect::slot_lockStatusChanged);
}

void PanelViewSelect::restoreViewEnables()
{
    QSettings settings;
    QList<int> loadedList = settings.value("viewEnables2").value<QList<int>>();
    if (loadedList.isEmpty())
    {
        qDebug() <<"ViewEnables empty - defaulting to mosiac view";
        loadedList.push_back(static_cast<int>(VIEW_MOSAIC));
    }

    for (auto i : std::as_const(loadedList))
    {
        eViewType vt = static_cast<eViewType>(i);
        if (!config->insightMode)
        {
            switch (vt)
            {
            case VIEW_MAP_EDITOR:
            case VIEW_GRID:
            case VIEW_CROP:
            case VIEW_DEBUG:
                vt = VIEW_MOSAIC;
                break;

            default:
                break;
            }

        }
        else if (vt == VIEW_BMP_IMAGE)
        {
            continue;
        }
        selectViewer(vt);
    }
}

// the front  door
void  PanelViewSelect::selectViewer(eViewType vtype, bool enable)
{
    if (config->lockView)
    {
        return;
    }

    if (enable)
    {
        if (!viewerGroup.button(vtype)->isChecked())
        {
            viewerGroup.button(vtype)->setChecked(true);
            emit viewerGroup.idClicked(vtype);
        }
        else
        {
            slot_Viewer_pressed(vtype,true);
        }
    }
    else
    {
        if (viewerGroup.button(vtype)->isChecked())
        {
            viewerGroup.button(vtype)->setChecked(false);
            emit viewerGroup.idClicked(vtype);
        }
        else
        {
            slot_Viewer_pressed(vtype,false);
        }
    }
}

void PanelViewSelect::deselectGangedViewers()
{
    auto vec = Sys::viewController->getGangMembers();
    for (auto & view : vec)
    {
        selectViewer(view,false);
    }
}

void  PanelViewSelect::slot_Viewer_pressed(int id, bool enable)
{
    eViewType viewType = static_cast<eViewType>(id);
    qDebug().noquote() << sViewerType[viewType] << (enable ? "enabled" : "disabled");

    if (Sys::viewController->isGangMember(viewType))
    {
        // thse are the prime views
        // deal with exclusivity (the standard mode)
        if (enable)
        {
            if (!config->multiView)
            {
                // uncheck the last exclusive
                viewerGroup.blockSignals(true);
                viewerGroup.button(viewController->getMostRecentGangEnable())->setChecked(false);
                viewerGroup.button(id)->setChecked(true);
                viewerGroup.blockSignals(false);
            }
        }

        if (!config->multiView)
        {
            viewController->disableGang();
        }
    }

    viewController->viewEnable(viewType,enable);

    emit sig_reconstructView();
}

void PanelViewSelect::slot_multiSelect(bool enb)
{
    if (!enb)
    {
        auto vbuttons          = viewerGroup.buttons();
        eViewType lastSelected = viewController->getMostRecentGangEnable();
        for (auto button : std::as_const(vbuttons))
        {
            eViewType vtype = (eViewType)viewerGroup.id(button);
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

void PanelViewSelect::slot_lockStatusChanged()
{
    cbLockSelect->blockSignals(true);
    cbLockSelect->setChecked(config->lockView);
    cbLockSelect->blockSignals(false);
    restoreViewEnables();
}
