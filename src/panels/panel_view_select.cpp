#include <QCheckBox>

#include "panels/controlpanel.h"
#include "panels/panel_view_select.h"
#include "panels/panel_misc.h"
#include "settings/configuration.h"
#include "tiledpatternmaker.h"
#include "viewers/viewcontrol.h"

extern TiledPatternMaker * theApp;

PanelViewSelect::PanelViewSelect(ControlPanel * parent)
{
    setTitle("View Select");

    panel               = parent;
    config              = Configuration::getInstance();
    view                = ViewControl::getInstance();
    exclusiveViews      = true;

    createGUI();

    // restore
    config->lockView = false;
    restoreViewEnables();
    config->lockView = true;
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
    cbGrid               = new QCheckBox("Grid");
    cbMeasure            = new QCheckBox("Measure");
    cbCenter             = new QCheckBox("Center");

    cbMultiSelect        = new QCheckBox("Multi View");    // defaults to off - not persisted
    cbLockView           = new QCheckBox("Lock View");

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
        qWarning() <<"ViewEnables empty - defaulting to mosiac view";
        loadedList.push_back(static_cast<int>(VIEW_MOSAIC));
    }

    for (auto i : loadedList)
    {
        eViewType vt = static_cast<eViewType>(i);
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
    cbMeasure->setChecked(config->measure);
    cbCenter->setChecked(config->showCenterDebug);
    cbBackgroundImage->setChecked(config->showBackgroundImage);
}

void  PanelViewSelect::selectViewer(eViewType vtype)
{
    if (config->lockView)
    {
        return;
    }

    viewerGroup.button(vtype)->setChecked(true);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    emit viewerGroup.buttonClicked(vtype);
#else
    emit viewerGroup.idClicked(vtype);
#endif
}

void  PanelViewSelect::slot_Viewer_pressed(int id, bool enable)
{
    eViewType viewType = static_cast<eViewType>(id);
    qDebug() << sViewerType[viewType] << (enable ? "enabled" : "disabled");

    switch (viewType)
    {
    case VIEW_BKGD_IMG:
        config->showBackgroundImage = enable;
        break;

    case VIEW_GRID:
        config->showGrid = enable;
        break;

    case VIEW_MEASURE:
        config->measure = enable;
        break;

    case VIEW_CENTER:
        config->showCenterDebug = enable;
        break;

    case VIEW_BORDER:
    case VIEW_CROP:
    case VIEW_IMAGE:
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
    {
        // thse are the prime views
        // deal with exclusivity (the standard mode)
        if (enable)
        {
            if (exclusiveViews)
            {
                // uncheck the last exclusive
                viewerGroup.blockSignals(true);
                viewerGroup.button(view->getMostRecent())->setChecked(false);
                viewerGroup.button(id)->setChecked(true);
                viewerGroup.blockSignals(false);
            }
            panel->delegateKeyboardMouse();
        }

        if (exclusiveViews)
        {
            view->disablePrimeViews();
        }
    }   break;
    }

    view->viewEnable(viewType,enable);

    view->slot_refreshView();
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
        for (auto button : vbuttons)
        {
            if (viewerGroup.id(button) == view->getMostRecent())
            {
                continue;
            }
            button->setChecked(false);
        }
        viewerGroup.blockSignals(false);
    }
    view->disablePrimeViews();
    view->viewEnable(view->getMostRecent(),true);
    view->slot_refreshView();
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
