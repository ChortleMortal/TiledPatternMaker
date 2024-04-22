#include "panels/controlpanel.h"
#include "enums/eviewtype.h"
#include "geometry/transform.h"
#include "legacy/design_maker.h"
#include "makers/map_editor/map_editor.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/sys.h"
#include "misc/utilities.h"
#include "misc/version.h"
#include "mosaic/mosaic.h"
#include "panels/page_debug.h"
#include "panels/page_map_editor.h"
#include "panels/page_mosaic_maker.h"
#include "panels/page_motif_maker.h"
#include "panels/page_prototype_info.h"
#include "panels/page_tiling_maker.h"
#include "panels/panel_pages_widget.h"
#include "panels/panel_view_select.h"
#include "panels/splitscreen.h"
#include "settings/configuration.h"
#include "tile/tiling.h"
#include "tiledpatternmaker.h"
#include "viewers/view_controller.h"
#include "widgets/mouse_mode_widget.h"

ControlPanel::ControlPanel()
{
    setObjectName("ControlPanel");

    setFocusPolicy(Qt::ClickFocus);

#if defined(Q_OS_WIN32)
    setWindowFlag(Qt::Window);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
    setWindowFlag(Qt::WindowMinimizeButtonHint);
    setWindowFlag(Qt::WindowCloseButtonHint);
#endif

    setContentsMargins(0,0,0,0);

    isShown = false;
}

void ControlPanel::init()
{
    view           = Sys::view;
    viewControl    = Sys::viewController;
    mosaicMaker    = Sys::mosaicMaker;
    tilingMaker    = Sys::tilingMaker;
    config         = Sys::config;

    getPanelInfo();
    setupGUI();

    mpTimer = new QTimer(this);
    connect(mpTimer, &QTimer::timeout, this, &ControlPanel::slot_poll);

    mpTimer->start(250);    // always runs
}

ControlPanel::~ControlPanel()
{
    qDebug() << "ControlPanel destructor";
    mpTimer->stop();
    delete pageController;
}

void ControlPanel::closeEvent(QCloseEvent *event)
{
    qDebug() << "ControlPanel::closeEvent";
    if (!config->splitScreen)
    {
        QPoint pt = pos();
        QSettings s;
        s.setValue(QString("panelPos/%1").arg(Sys::appInstance), pt);
    }

    QWidget::closeEvent(event);
    qApp->quit();
}

void ControlPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    emit sig_panelResized();
}

#if 0
// every resize event has a move event afterwards
// but every move event does not necessarily have a resize event
void ControlPanel::moveEvent(QMoveEvent *event)
{
    qDebug().noquote() << "ControlPanel::move from =" << event->oldPos() << "to =" << event->pos();
    QWidget::moveEvent(event);
}
#endif

void ControlPanel::paintEvent(QPaintEvent *event)
{
    if (!isShown)
    {
        // first time only
        QSettings s;
        QPoint pt = s.value((QString("panelPos/%1").arg(Sys::appInstance))).toPoint();
        move(pt);
        isShown = true;
    }
    QWidget::paintEvent(event);
}

void ControlPanel::setupGUI()
{
    // create the common top three lines
    QGroupBox * commonGroup = createTopGroup();
    commonGroup->setFixedWidth(841);

    // LHS
    QLabel * menuSelectLabel = new QLabel("Menu Select");
    auto pageList            = new PageListWidget(this);
    viewSelect               = new PanelViewSelect(this);

    QVBoxLayout * leftBox = new QVBoxLayout;
    leftBox->addSpacing(7);
    leftBox->addWidget(menuSelectLabel);
    leftBox->addWidget(pageList);                       // left widget contains page list
    leftBox->addWidget(viewSelect);
    leftBox->addStretch();

    // RHS
    auto panelPages = new PanelPagesWidget();
    pageController  = new PanelPageController(pageList,panelPages);
    pageController->populatePages();                    // create the new pages
    pageController->setCurrentPage(config->panelName);  // this makes the first page selection

    // main box : contains left & right
    QHBoxLayout * mainBox = new QHBoxLayout;
    mainBox->addLayout(leftBox, Qt::AlignLeft);
    mainBox->addWidget(panelPages,0,Qt::AlignLeft | Qt::AlignTop);      // right widget contains the pages

    QWidget * mainWidget = new QWidget();
    mainWidget->setContentsMargins(0,0,0,0);
    mainWidget->setFixedWidth(841);
    mainWidget->setLayout(mainBox);

    // this puts it all together
    QVBoxLayout * panelBox = new QVBoxLayout;
    panelBox->setSizeConstraint(QLayout::SetFixedSize);
    panelBox->addWidget(commonGroup);
    panelBox->addWidget(mainWidget);
    panelBox->addStretch();

    this->setLayout(panelBox);

    // connections
    connect(this,   &ControlPanel::sig_refreshView, viewControl,    &ViewController::slot_reconstructView);
    connect(this,   &ControlPanel::sig_render,      theApp,         &TiledPatternMaker::slot_render);
}

QGroupBox * ControlPanel::createTopGroup()
{
    // top row
    panelStatus = new PanelStatus();

    // hlayout - second row
    QHBoxLayout * hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->setSizeConstraint(QLayout::SetFixedSize);

    radioDefined = new QRadioButton("Full");
    radioPack    = new QRadioButton("Packed");
    radioSingle  = new QRadioButton("Simple");

    QPushButton * pbShowMosaic    = new QPushButton("Show Mosaic");
    QPushButton * pbShowTiling    = new QPushButton("Show Tiling");
    QPushButton * pbClearAll      = new QPushButton("Clear All");

    if (config->insightMode)
    {
        QPushButton * pbLogEvent      = new QPushButton("Log Event");
        QPushButton * pbReload        = new QPushButton("Reload");
        QPushButton * pbRefreshView   = new QPushButton("Recreate");
        QPushButton * pbUpdateView    = new QPushButton("Repaint");
        QPushButton * pbSaveLog       = new QPushButton("Save Log");

#define BWIDTH 69
        pbLogEvent->setMaximumWidth(BWIDTH);
        pbReload->setMaximumWidth(BWIDTH);
        pbRefreshView->setMaximumWidth(BWIDTH);
        pbUpdateView->setMaximumWidth(BWIDTH);
        pbSaveLog->setMaximumWidth(BWIDTH);

        hlayout->addWidget(pbLogEvent);
        hlayout->addWidget(pbSaveLog);
        hlayout->addWidget(pbReload);
        hlayout->addWidget(pbRefreshView);
        hlayout->addWidget(pbUpdateView);

        connect(pbUpdateView,       &QPushButton::clicked,  this,       &ControlPanel::updateView);
        connect(pbLogEvent,         &QPushButton::clicked,  this,       &ControlPanel::slot_logEvent);
        connect(pbRefreshView,      &QPushButton::clicked,  viewControl,&ViewController::slot_reconstructView);
        connect(pbSaveLog,          &QPushButton::clicked,  this,       &ControlPanel::sig_saveLog);
        connect(pbReload,           &QPushButton::clicked,  this,       &ControlPanel::slot_reload);
    }

    hlayout->addStretch();
    hlayout->addWidget(radioDefined);
    hlayout->addWidget(radioPack);
    hlayout->addWidget(radioSingle);
    hlayout->addStretch();
    hlayout->addWidget(pbClearAll);
    hlayout->addStretch();
    hlayout->addWidget(pbShowTiling);
    hlayout->addWidget(pbShowMosaic);

    repeatRadioGroup.addButton(radioDefined,REPEAT_DEFINED);
    repeatRadioGroup.addButton(radioPack,REPEAT_PACK);
    repeatRadioGroup.addButton(radioSingle,REPEAT_SINGLE);
    repeatRadioGroup.button(config->repeatMode)->setChecked(true);

    // hbox - third row
    kbdModeCombo = new QComboBox();
    kbdModeCombo->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    kbdModeCombo->setFixedHeight(25);
    delegateKeyboardMouse();

    mouseModeWidget = new MouseModeWidget;

    QPushButton *pbRaise    = new QPushButton("Raise View");
    pbRaise->setFixedHeight(25);
    //pbRaise->setStyleSheet("QPushButton{ border-radius: 3px; padding-left: 9px; padding-right: 9px; }");

    QPushButton *pbExit     = new QPushButton("QUIT");
    pbExit->setFixedSize(QSize(71,25));
    //pbExit->setStyleSheet("QPushButton{ border-radius: 3px; }");

    QCheckBox * chkScaleToView  = new QCheckBox("Scale to View");

    QLabel    * l_kbdModes      = new QLabel("Keyboard:");

    QFrame    * line           = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setLineWidth(1);
    if (!Sys::isDarkTheme) line->setStyleSheet("QFrame {background-color: LightGray;}");

    QFrame    * line1           = new QFrame();
    line1->setFrameShape(QFrame::VLine);
    line1->setFrameShadow(QFrame::Sunken);
    line1->setLineWidth(1);
    if (!Sys::isDarkTheme) line1->setStyleSheet("QFrame {background-color: LightGray;}");

    QFrame    * line2           = new QFrame();
    line2->setFrameShape(QFrame::VLine);
    line2->setFrameShadow(QFrame::Sunken);
    line2->setLineWidth(1);
    if (!Sys::isDarkTheme) line2->setStyleSheet("QFrame {background-color: LightGray;}");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(pbRaise);
    hbox->addSpacing(2);
    hbox->addWidget(chkScaleToView);
    //hbox->addSpacing(2);
    hbox->addWidget(line);
    hbox->addSpacing(2);
    hbox->addWidget(l_kbdModes);
    hbox->addWidget(kbdModeCombo);
    hbox->addSpacing(2);
    hbox->addWidget(line1);
    hbox->addSpacing(2);
    hbox->addWidget(mouseModeWidget);
    hbox->addSpacing(2);
    hbox->addWidget(line2);
    hbox->addSpacing(2);
    hbox->addWidget(pbExit);

    // common group box
    QVBoxLayout * commonLayout    = new QVBoxLayout();
    commonLayout->addWidget(panelStatus);
    commonLayout->addLayout(hlayout);
    commonLayout->addLayout(hbox);

    QGroupBox * commonGroup       = new QGroupBox();
    commonGroup->setLayout(commonLayout);
    commonGroup->setContentsMargins(3,3,3,3);

    // initial values
    chkScaleToView->setChecked(config->scaleToView);

    // connections
    connect(pbExit,             &QPushButton::clicked,      this,           &ControlPanel::slot_exit);
    connect(pbRaise,            &QPushButton::clicked,      this,           &ControlPanel::slot_raise);
    connect(pbShowTiling,       &QPushButton::pressed,      this,           &ControlPanel::showTilingPressed);
    connect(pbClearAll,         &QPushButton::pressed,      viewControl,    &ViewController::slot_unloadAll, Qt::QueuedConnection);
    connect(pbShowMosaic,       &QPushButton::pressed,      this,           &ControlPanel::showMosaicPressed);
    connect(chkScaleToView,     &QCheckBox::clicked,        this,           &ControlPanel::slot_scaleToView);
    connect(&repeatRadioGroup,  &QButtonGroup::idClicked,   this,           &ControlPanel::repeatChanged);

    connect(kbdModeCombo,       SIGNAL(currentIndexChanged(int)),   this,   SLOT(slot_kbdModeChanged(int)));
    connect(view,               &View::sig_kbdMode,                 this,   &ControlPanel::slot_kbdMode);

    return commonGroup;
}

// called by mpTimer
void ControlPanel::slot_poll()
{
    viewSelect->refresh();

    if (config->insightMode && !Sys::updatePanel)
    {
        return;
    }

    // refresh visible pages
    pageController->refreshPages();

    updateGeometry();
    setWindowTitles();
}

void ControlPanel::repeatChanged(int id)
{
    config->repeatMode = static_cast<eRepeatType>(id);

    emit sig_render();

    if (viewControl->isEnabled(VIEW_MAP_EDITOR))
    {
        emit sig_reload();
    }
}

void ControlPanel::slot_reload()
{
    LoadUnit & loadUnit = view->getLoadUnit();
    QString name = loadUnit.getLastLoadName();

    switch (loadUnit.getLastLoadState())
    {
    case LOADING_MOSAIC:
        mosaicMaker->loadMosaic(name);
        break;

    case LOADING_TILING:
        tilingMaker->loadTiling(name,TILM_LOAD_SINGLE);
        break;

    case LOADING_LEGACY:
    {
        eDesign design = (eDesign)designs.key(name);
        Sys::designMaker->slot_loadDesign(design);
    }   break;

    case LOADING_NONE:
        break;
    }
}

void ControlPanel::slot_logEvent()
{
    qInfo() << "** EVENT MARK **";
    Sys::dumpRefs();
    const QVector<Layer*> & layers = Sys::view->getActiveLayers();
    if (layers.size())
    {
        Layer * layer = layers.first();
        if (layer)
        {
            auto vsize = Sys::view->size();
            auto csize = Sys::viewController->getCanvas().getSize();
            qInfo().noquote() << "ViewSize" << vsize << "aspect" <<  Utils::aspect(vsize) << "CanvasSize" << csize << "aspect" << Utils::aspect(csize);
            qInfo().noquote() << layer->getLayerName() << "Canvas" << Transform::info(layer->getCanvasTransform());
            qInfo().noquote() << layer->getLayerName() << "Model " << Transform::info(layer->getModelTransform());
        }
    }
    //emit sig_id_layers();
}

void ControlPanel::slot_raise()
{
    view->setWindowState( (view->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    view->raise();
    view->activateWindow();
}

void ControlPanel::slot_exit()
{
    view->close();
    qApp->exit();
}

void  ControlPanel::updateView()
{
    view->update();
}

void ControlPanel::delegateView()
{
    panel_page * currentPage = pageController->getCurrentPage();
    if (!currentPage)
    {
        return;
    }

    qDebug().noquote() << "delegate view based on:" << currentPage->getName();

    page_prototype_info * ppi = dynamic_cast<page_prototype_info*>(currentPage);
    if (ppi)
    {
        delegateView(VIEW_PROTOTYPE);
        return;
    }

    page_map_editor * pfe = dynamic_cast<page_map_editor*>(currentPage);
    if (pfe)
    {
        delegateView(VIEW_MAP_EDITOR);
        return;
    }

    page_motif_maker * pfm = dynamic_cast<page_motif_maker*>(currentPage);
    if (pfm)
    {
        delegateView(VIEW_MOTIF_MAKER);
        return;
    }

    page_tiling_maker * ptm = dynamic_cast<page_tiling_maker*>(currentPage);
    if (ptm)
    {
        delegateView(VIEW_TILING_MAKER);
        return;
    }

    page_mosaic_maker * psm = dynamic_cast<page_mosaic_maker*>(currentPage);
    if (psm)
    {
        delegateView(VIEW_MOSAIC);
        return;
    }
}

void ControlPanel::delegateKeyboardMouse()
{
    kbdModeCombo->blockSignals(true);
    kbdModeCombo->clear();
    if (viewControl->isEnabled(VIEW_DESIGN))
    {
        kbdModeCombo->insertItem(100,"Mode Position",   QVariant(KBD_MODE_DES_POS));
        kbdModeCombo->insertItem(100,"Mode Layer",      QVariant(KBD_MODE_DES_LAYER_SELECT));
        kbdModeCombo->insertItem(100,"Mode Z-level",    QVariant(KBD_MODE_DES_ZLEVEL));
        kbdModeCombo->insertItem(100,"Mode Step",       QVariant(KBD_MODE_DES_STEP));
        kbdModeCombo->insertItem(100,"Mode Separation", QVariant(KBD_MODE_DES_SEPARATION));
        kbdModeCombo->insertItem(100,"Mode Origin",     QVariant(KBD_MODE_DES_ORIGIN));
        kbdModeCombo->insertItem(100,"Mode Offset",     QVariant(KBD_MODE_DES_OFFSET));
        kbdModeCombo->insertItem(100,"Move Window",     QVariant(KBD_MODE_MOVE));
    }
    else
    {
        kbdModeCombo->insertItem(100,"Adjust View",             QVariant(KBD_MODE_XFORM_VIEW));
        kbdModeCombo->insertItem(100,"Adjust Selected Layer",   QVariant(KBD_MODE_XFORM_SELECTED));
        kbdModeCombo->insertItem(100,"Adjust Background",       QVariant(KBD_MODE_XFORM_BKGD));
        kbdModeCombo->insertItem(100,"Adjust Tiling",           QVariant(KBD_MODE_XFORM_TILING));
        kbdModeCombo->insertItem(100,"Adjust Unique Tile",      QVariant(KBD_MODE_XFORM_UNIQUE_TILE));
        kbdModeCombo->insertItem(100,"Adjust Placed Tile",      QVariant(KBD_MODE_XFORM_PLACED_TILE));
        kbdModeCombo->insertItem(100,"Adjust Grid",             QVariant(KBD_MODE_XFORM_GRID));
        kbdModeCombo->insertItem(100,"Move Window",             QVariant(KBD_MODE_MOVE));
    }

    kbdModeCombo->blockSignals(false);

    view->resetKbdMode(); // converts if necessry
}

eKbdMode ControlPanel::getValidKbdMode(eKbdMode mode)
{
    if (viewControl->isEnabled(VIEW_DESIGN))
    {
        return getValidDesignMode(mode);
    }
    else
    {
        return getValidMosaicMode(mode);
    }
}

eKbdMode ControlPanel::getValidDesignMode(eKbdMode mode)
{
    switch (mode)
    {
    case KBD_MODE_DES_POS:
    case KBD_MODE_DES_LAYER_SELECT:
    case KBD_MODE_DES_ZLEVEL:
    case KBD_MODE_DES_STEP:
    case KBD_MODE_DES_SEPARATION:
    case KBD_MODE_DES_ORIGIN:
    case KBD_MODE_DES_OFFSET:
    case KBD_MODE_MOVE:
        return mode;
    case KBD_MODE_XFORM_VIEW:
    case KBD_MODE_XFORM_SELECTED:
    case KBD_MODE_XFORM_BKGD:
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_UNIQUE_TILE:
    case KBD_MODE_XFORM_PLACED_TILE:
    case KBD_MODE_XFORM_GRID:
        break;
    }
    return KBD_MODE_DES_LAYER_SELECT;
}

eKbdMode ControlPanel::getValidMosaicMode(eKbdMode mode)
{
    switch (mode)
    {
    case KBD_MODE_XFORM_VIEW:
    case KBD_MODE_XFORM_SELECTED:
    case KBD_MODE_XFORM_BKGD:
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_UNIQUE_TILE:
    case KBD_MODE_XFORM_PLACED_TILE:
    case KBD_MODE_XFORM_GRID:
    case KBD_MODE_MOVE:
        return mode;
    case KBD_MODE_DES_POS:
    case KBD_MODE_DES_LAYER_SELECT:
    case KBD_MODE_DES_ZLEVEL:
    case KBD_MODE_DES_STEP:
    case KBD_MODE_DES_SEPARATION:
    case KBD_MODE_DES_ORIGIN:
    case KBD_MODE_DES_OFFSET:
        break;
    }
    return KBD_MODE_XFORM_VIEW;
}

// kbdModeCombo selection
void ControlPanel::slot_kbdModeChanged(int row)
{
    Q_UNUSED(row)
    qDebug() << "slot_kbdModeChanged to:"  << kbdModeCombo->currentText();
    QVariant var = kbdModeCombo->currentData();
    eKbdMode mode = static_cast<eKbdMode>(var.toInt());
    view->setKbdMode(mode);
}

// from canvas setKbdMode
void ControlPanel::slot_kbdMode(eKbdMode mode)
{
    QVariant var(mode);
    int index = kbdModeCombo->findData(var);
    kbdModeCombo->blockSignals(true);
    kbdModeCombo->setCurrentIndex(index);
    kbdModeCombo->blockSignals(false);
}

void ControlPanel::showMosaicPressed()
{
    delegateView(VIEW_MOSAIC);
}

void ControlPanel::showTilingPressed()
{
    panel_page * currentPage = pageController->getCurrentPage();

    page_tiling_maker * ptm  = dynamic_cast<page_tiling_maker*>(currentPage);
    if (ptm && viewControl->isEnabled(VIEW_TILING))
    {
        delegateView(VIEW_TILING_MAKER);
    }
    else
    {
        delegateView(VIEW_TILING);
    }
}

void ControlPanel::slot_scaleToView(bool enb)
{
    config->scaleToView = enb;
}

void  ControlPanel::delegateView(eViewType vtype)
{
    viewSelect->selectViewer(vtype);
}

void ControlPanel::getPanelInfo()
{
#ifdef QT_DEBUG
    panelInfo  = "Control Panel - Debug - Version ";
#else
    panelInfo = "Control Panel - Version ";
#endif
    panelInfo += tpmVersion;
    if  (!Sys::gitBranch.isEmpty())
    {
        panelInfo += " [";
        panelInfo += Sys::gitBranch;
        panelInfo += "]";
    }
    panelInfo  += "  ";
    panelInfo  += config->rootMediaDir;
}

void ControlPanel::setWindowTitles()
{
    static QString lastPanelTitle;
    static QString lastViewTitle;

    // The view
    QString viewTitle;
    if (!viewControl->isEnabled(VIEW_DESIGN))
    {
        // regular case
        QString sMosaic;
        auto mosaic = mosaicMaker->getMosaic();
        if (mosaic)
        {
            sMosaic = mosaic->getName();
        }

        QString sTiling;
        auto tiling = tilingMaker->getSelected();
        if (tiling)
        {
            sTiling = tiling->getTitle();
        }

        viewTitle  = QString("  Mosaic <%1>  Tiling <%2>  ").arg(sMosaic).arg(sTiling);
    }
    else
    {
        // legacy case
        QString sDesign = Sys::designMaker->getDesignName();
        viewTitle       = QString("  Design: %1").arg(sDesign);
    }

    // The control panel show transient status. The view shows whats loaded
    QString transient;
    LoadUnit & loadUnit = view->getLoadUnit();
    switch (loadUnit.getLoadState())
    {
    case LOADING_NONE:
        transient = viewTitle;
        break;

    case LOADING_MOSAIC:
        transient = QString("  Loading Mosaic: %1").arg(loadUnit.getLoadName());
        break;

    case LOADING_TILING:
        transient = QString("  Loading Tiling: %1").arg(loadUnit.getLoadName());
        break;

    case LOADING_LEGACY:
        transient = QString("  Loading Legacy Design: %1").arg(loadUnit.getLoadName());
        break;
    }

    QString panelTitle = panelInfo + transient;

    if ((lastPanelTitle != panelTitle) || (lastViewTitle != viewTitle))
    {
        setWindowTitle(panelTitle);
        update();
        view->setViewTitle(viewTitle);
        if (config->splitScreen)
        {
            theApp->getSplitter()->setWindowTitle(panelTitle);
        }

        lastPanelTitle = panelTitle;
        lastViewTitle  = viewTitle;
    }
}

void ControlPanel::completePageSelection()
{
    delegateView();
    delegateKeyboardMouse();
    adjustSize();
}


