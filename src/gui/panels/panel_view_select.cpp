#include <QCheckBox>

#include "gui/top/controlpanel.h"
#include "gui/panels/panel_view_select.h"
#include "gui/panels/panel_misc.h"
#include "model/settings/configuration.h"
#include "sys/tiledpatternmaker.h"
#include "gui/viewers/measure_view.h"
#include "gui/top/view_controller.h"
#include "sys/sys.h"

extern TiledPatternMaker * theApp;

PanelViewSelect::PanelViewSelect(ControlPanel * parent)
{
    setTitle("View Select");

    panel               = parent;
    config              = Sys::config;
    viewController      = Sys::viewController;
    exclusiveViews      = true;

    createGUI();

    // restore
    config->lockView = false;
    restoreViewEnables();
    config->lockView = true;

    connect(this, &PanelViewSelect::sig_reconstructView, viewController, &ViewController::slot_reconstructView);
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

    cbMeasure            = new QCheckBox("Measure");
    cbMeasure->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbCenter             = new QCheckBox("Center");
    cbCenter->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbMultiSelect        = new QCheckBox("Multi View");    // defaults to off - not persisted
    cbMultiSelect->setStyleSheet("QCheckBox {padding-left: 3px;}");

    cbLockView           = new QCheckBox("Lock View");
    cbLockView->setStyleSheet("QCheckBox {padding-left: 3px;}");
    cbLockView->setChecked(config->lockView);

    AQVBoxLayout * avbox = new AQVBoxLayout();

    avbox->addWidget(cbMosaicView);
    avbox->addWidget(cbPrototypeView);
    if (config->insightMode)
    {
        avbox->addWidget(cbMapEditor);
    }
    avbox->addWidget(cbProtoMaker);
    avbox->addWidget(cbTilingView);
    avbox->addWidget(cbTilingMakerView);
    avbox->addWidget(cbRawDesignView);
    avbox->addSpacing(7);
    avbox->addWidget(cbBackgroundImage);
    avbox->addWidget(cbBMPImage);
    avbox->addWidget(cbGrid);
    avbox->addWidget(cbMeasure);
    avbox->addWidget(cbCenter);
    avbox->addSpacing(7);
    avbox->addWidget(cbLockView);
    avbox->addWidget(cbMultiSelect);

    setLayout(avbox);

    // viewer group
    viewerGroup.addButton(cbRawDesignView,      VIEW_DESIGN);
    viewerGroup.addButton(cbMosaicView,         VIEW_MOSAIC);
    viewerGroup.addButton(cbPrototypeView,      VIEW_PROTOTYPE);
    viewerGroup.addButton(cbProtoMaker,         VIEW_MOTIF_MAKER);
    viewerGroup.addButton(cbTilingView,         VIEW_TILING);
    viewerGroup.addButton(cbTilingMakerView,    VIEW_TILING_MAKER);
    if (config->insightMode)
    {
        viewerGroup.addButton(cbMapEditor,      VIEW_MAP_EDITOR);
    }
    viewerGroup.addButton(cbBackgroundImage,    VIEW_BKGD_IMG);
    viewerGroup.addButton(cbBMPImage,           VIEW_IMAGE);
    viewerGroup.addButton(cbGrid,               VIEW_GRID);
    viewerGroup.addButton(cbMeasure,            VIEW_MEASURE);
    viewerGroup.addButton(cbCenter,             VIEW_CENTER);

    // this class handles the exclusivity, not the QButtonGroup
    viewerGroup.setExclusive(false);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(&viewerGroup,       SIGNAL(buttonToggled(int, bool)),  this, SLOT(slot_Viewer_pressed(int,bool)));
#else
    connect(&viewerGroup,       &QButtonGroup::idToggled,       this, &PanelViewSelect::slot_Viewer_pressed);
#endif
    connect(cbLockView,         &QCheckBox::clicked,  this, &PanelViewSelect::slot_lockViewClicked);
    connect(cbMultiSelect,      &QCheckBox::clicked,  this, &PanelViewSelect::slot_multiSelect);
    connect(theApp,             &TiledPatternMaker::sig_lockStatus, this,&PanelViewSelect::slot_lockStatusChanged);
}

void PanelViewSelect::restoreViewEnables()
{
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");
#endif

    QSettings settings;
    QList<int> loadedList = settings.value("viewEnables").value<QList<int>>();
    if (loadedList.isEmpty())
    {
        qDebug() <<"ViewEnables empty - defaulting to mosiac view";
        loadedList.push_back(static_cast<int>(VIEW_MOSAIC));
    }

    for (auto i : std::as_const(loadedList))
    {
        eViewType vt = static_cast<eViewType>(i);
        if (vt == VIEW_IMAGE)
        {
            continue;
        }
        if (vt == VIEW_MAP_EDITOR && !config->insightMode)
        {
            vt = VIEW_MOSAIC;
        }
        selectViewer(vt);
    }
}

void PanelViewSelect::refresh()
{
    // refresh panel
    cbGrid->setChecked(config->showGrid);
    cbMeasure->setChecked(Sys::measure);
    cbCenter->setChecked(config->showCenterDebug);
    cbBackgroundImage->setChecked(config->showBackgroundImage);
}

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
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            emit viewerGroup.buttonClicked(vtype);
#else
            emit viewerGroup.idClicked(vtype);
#endif
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
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            emit viewerGroup.buttonClicked(vtype);
#else
            emit viewerGroup.idClicked(vtype);
#endif
        }
        else
        {
            slot_Viewer_pressed(vtype,false);
        }
    }
}

void  PanelViewSelect::slot_Viewer_pressed(int id, bool enable)
{
    eViewType viewType = static_cast<eViewType>(id);
    qDebug().noquote() << sViewerType[viewType] << (enable ? "enabled" : "disabled");

    switch (viewType)
    {
    case VIEW_BKGD_IMG:
        config->showBackgroundImage = enable;
        break;

    case VIEW_GRID:
        config->showGrid = enable;
        break;

    case VIEW_MEASURE:
        Sys::measureView->clear();
        Sys::measure = enable;
        break;

    case VIEW_CENTER:
        config->showCenterDebug = enable;
        break;

    case VIEW_BORDER:
    case VIEW_CROP:
        // not added separately but part of mosaic (or legacy design)
        // there are no panel sdelect buttons for these
        return;

    case VIEW_DESIGN:
    case VIEW_MOSAIC:
    case VIEW_PROTOTYPE:
    case VIEW_MOTIF_MAKER:
    case VIEW_TILING:
    case VIEW_TILING_MAKER:
    case VIEW_MAP_EDITOR:
    case VIEW_IMAGE:
    {
        // thse are the prime views
        // deal with exclusivity (the standard mode)
        if (enable)
        {
            if (exclusiveViews)
            {
                // uncheck the last exclusive
                viewerGroup.blockSignals(true);
                viewerGroup.button(viewController->getMostRecent())->setChecked(false);
                viewerGroup.button(id)->setChecked(true);
                viewerGroup.blockSignals(false);
            }
            panel->delegateKeyboardMouse();
        }

        if (exclusiveViews)
        {
            viewController->disablePrimeViews();
        }
    }   break;
    }

    viewController->viewEnable(viewType,enable);
    emit sig_reconstructView();
}

void PanelViewSelect::slot_multiSelect(bool enb)
{
    // the multiselect checkbox has been clicked
    auto vbuttons = viewerGroup.buttons();
    // this is a mirror of the page views control
    exclusiveViews = !enb;
    if (!enb)
    {
        viewerGroup.blockSignals(true);
        for (auto button : std::as_const(vbuttons))
        {
            if (viewerGroup.id(button) == viewController->getMostRecent())
            {
                continue;
            }
            button->setChecked(false);
        }
        viewerGroup.blockSignals(false);
    }
    viewController->disablePrimeViews();
    viewController->viewEnable(viewController->getMostRecent(),true);
    emit sig_reconstructView();
}

void  PanelViewSelect::slot_lockViewClicked(bool enb)
{
    config->lockView = enb;
}

void PanelViewSelect::slot_lockStatusChanged()
{
    cbLockView->blockSignals(true);
    cbLockView->setChecked(config->lockView);
    cbLockView->blockSignals(false);
}
