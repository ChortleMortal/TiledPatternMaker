#include "gui/map_editor/map_editor.h"
#include "gui/panels/page_debug.h"
#include "gui/panels/page_map_editor.h"
#include "gui/panels/page_mosaic_maker.h"
#include "gui/panels/page_motif_maker.h"
#include "gui/panels/page_prototype_info.h"
#include "gui/panels/page_tiling_maker.h"
#include "gui/panels/panel_pages_widget.h"
#include "gui/panels/panel_view_select.h"
#include "gui/top/controlpanel.h"
#include "gui/top/split_screen.h"
#include "gui/top/system_view.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/gui_modes.h"
#include "gui/widgets/mouse_mode_widget.h"
#include "legacy/design_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/settings/configuration.h"
#include "model/tilings/tiling.h"
#include "sys/enums/eviewtype.h"
#include "sys/geometry/transform.h"
#include "sys/qt/utilities.h"
#include "sys/sys.h"
#include "sys/sys/debugflags.h"
#include "sys/sys/load_unit.h"
#include "sys/tiledpatternmaker.h"
#include "sys/version.h"

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

    panelStatus = nullptr;
}

ControlPanel::~ControlPanel()
{
    mpTimer->stop();

    if (!Sys::config->splitScreen)
    {
        QPoint pt = pos();
        QSettings s;
        s.setValue(QString("panelPos/%1").arg(Sys::appInstance), pt);
        qInfo() << "ControlPanel::~ControlPanel" << "pos" << pt;
    }
    else
    {
        qInfo() << "ControlPanel::~ControlPanel";
    }
    closePages();
    delete pageController;
}

void ControlPanel::init()
{
    getPanelInfo();
    setupGUI();

    mpTimer = new QTimer(this);
    connect(mpTimer, &QTimer::timeout, this, &ControlPanel::slot_poll);

    mpTimer->start(250);    // always runs
}

void ControlPanel::slot_exit()
{
    // This is the main exit called by pushing the QUIT button
    qInfo() << "ControlPanel::slot_exit";
    emit sig_close();
}

void ControlPanel::closeEvent(QCloseEvent *event)
{
    qInfo() << "ControlPanel::closeEvent";
    QWidget::closeEvent(event);
    emit sig_close();
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
    static bool isShown = false;
    if (!isShown)
    {
        // first time only
        isShown = true;
        if (!Sys::config->splitScreen)
        {
            QSettings s;
            QPoint pt = s.value((QString("panelPos/%1").arg(Sys::appInstance))).toPoint();
            move(pt);
        }
    }
    QWidget::paintEvent(event);
}

void ControlPanel::setupGUI()
{
    QMargins qm0(0,0,0,0);

    // create the common top three lines
    QGroupBox * commonGroup = createTopGroup();
    commonGroup->setFixedWidth(841);
    commonGroup->setContentsMargins(3,0,3,0);

    // LHS
    QLabel * menuSelectLabel  = new QLabel("Menu Select");

    PageListWidget * pageList = new PageListWidget(this);
    pageList->setContentsMargins(qm0);

    viewSelect                = new PanelViewSelect();

    AQVBoxLayout * pageListLayout = new AQVBoxLayout();
    pageListLayout->addWidget(menuSelectLabel);
    pageListLayout->addWidget(pageList);
    Q_ASSERT(pageListLayout->contentsMargins() == qm0);

    QVBoxLayout * leftLayout = new QVBoxLayout;
    leftLayout->addLayout(pageListLayout);                       // left widget contains page list
    leftLayout->addWidget(viewSelect);
    leftLayout->addStretch();
    Q_ASSERT(leftLayout->contentsMargins() == qm0);

    QWidget * leftWidget = new QWidget();
    leftWidget->setMaximumWidth(117);
    leftWidget->setLayout(leftLayout);
    Q_ASSERT(leftWidget->contentsMargins() == qm0);

    // RHS
    PanelPagesWidget * rhs = new PanelPagesWidget();
    rhs->setContentsMargins(qm0);

    // contains left & right
    AQHBoxLayout * lowerLayout = new AQHBoxLayout;
    lowerLayout->setSpacing(0);
    lowerLayout->addWidget(leftWidget, Qt::AlignLeft | Qt::AlignTop);
    lowerLayout->addWidget(rhs,0,Qt::AlignLeft | Qt::AlignTop);
    lowerLayout->addStretch();

    QWidget * lowerWidget = new QWidget();
    lowerWidget->setContentsMargins(qm0);
    lowerWidget->setFixedWidth(841);
    lowerWidget->setLayout(lowerLayout);

    // this puts it all together
    QVBoxLayout * paneLayout = new QVBoxLayout;
    paneLayout->setSizeConstraint(QLayout::SetFixedSize);
    paneLayout->addWidget(commonGroup);
    paneLayout->addWidget(lowerWidget);
    paneLayout->addStretch();

    this->setLayout(paneLayout);

    // connections
    connect(this,   &ControlPanel::sig_reconstructView, Sys::viewController, &SystemViewController::slot_reconstructView);

    // populate pages
    pageController  = new PanelPageController(pageList,rhs);
    pageController->populatePages();                        // create the new pages
    pageController->setCurrentPage(Sys::config->pageName); // this makes the first page selection
}

QGroupBox * ControlPanel::createTopGroup()
{
    // instantiate controls
    pbLogEvent      = nullptr;
    pbReload        = nullptr;
    pbRefreshView   = nullptr;
    pbUpdateView    = nullptr;
    pbSaveLog       = nullptr;

    radioDefined = new QRadioButton("Full");
    radioPack    = new QRadioButton("Packed");
    radioSingle  = new QRadioButton("Simple");

    repeatRadioGroup.addButton(radioDefined,REPEAT_DEFINED);
    repeatRadioGroup.addButton(radioPack,REPEAT_PACK);
    repeatRadioGroup.addButton(radioSingle,REPEAT_SINGLE);
    repeatRadioGroup.button(Sys::config->repeatMode)->setChecked(true);

    rad_hbox = new AQHBoxLayout;
    rad_hbox->addWidget(radioDefined);
    rad_hbox->addWidget(radioPack);
    rad_hbox->addWidget(radioSingle);

    pbShowMosaic    = new QPushButton("Show Mosaic");
    pbShowTiling    = new QPushButton("Show Tiling");
    pbClearAll      = new QPushButton("Clear All");

    QHBoxLayout * line1 = new QHBoxLayout();
    line1->setContentsMargins(0, 0, 0, 0);
    line1->setSizeConstraint(QLayout::SetFixedSize);

    if (Sys::config->insightMode)
    {
        pbLogEvent      = new  QPushButton("Log Event");
        pbReload        = new  QPushButton("Reload");
        pbRefreshView   = new  QPushButton("Recreate");
        pbUpdateView    = new  QPushButton("Repaint");
        pbSaveLog       = new  QPushButton("Save Log");
        pbEnbDbgView    = new AQPushButton("Debug View");
        pbEnbDbgFlags   = new AQPushButton("Debug Flags");
    }

    mouseModeWidget = new MouseModeWidget;

    pbRaise    = new QPushButton("Raise View");
    pbExit     = new QPushButton("QUIT");
    chkScaleToView  = new QCheckBox("Scale to View");

    pbRaise->setFixedHeight(25);
    pbExit->setFixedSize(QSize(71,25));

    panelStatus = new PanelStatus();

    // layout the controls
    QVBoxLayout * commonLayout;
    if (Sys::config->insightMode)
    {
        commonLayout = getInsightTop();
    }
    else
    {
        commonLayout = getDesignerTop();
    }

    QGroupBox * commonGroup = new QGroupBox();
    commonGroup->setLayout(commonLayout);

    // initial values
    chkScaleToView->setChecked(Sys::config->scaleToView);

    // connections
    connect(pbExit,             &QPushButton::clicked,    this,           &ControlPanel::slot_exit);
    connect(pbShowTiling,       &QPushButton::pressed,    this,           &ControlPanel::showTilingPressed);
    connect(pbClearAll,         &QPushButton::pressed,    Sys::viewController, &SystemViewController::slot_unloadAll, Qt::QueuedConnection);
    connect(pbShowMosaic,       &QPushButton::pressed,    this,           &ControlPanel::showMosaicPressed);
    connect(chkScaleToView,     &QCheckBox::clicked,      this,           &ControlPanel::slot_scaleToView);
    connect(&repeatRadioGroup,  &QButtonGroup::idClicked, this,           &ControlPanel::repeatChanged);

    if (Sys::config->insightMode)
    {
        // initial values
        pbEnbDbgView->setChecked(Sys::viewController->isEnabled(VIEW_DEBUG));
        pbEnbDbgFlags->setChecked(Sys::flags->enabled());

        // connections
        connect(pbUpdateView,   &QPushButton::clicked, Sys::viewController, &SystemViewController::slot_updateView);
        connect(pbRefreshView,  &QPushButton::clicked, Sys::viewController, &SystemViewController::slot_reconstructView);
        connect(pbRaise,        &QPushButton::clicked,  this, []    { Sys::viewController->raiseView();});
        connect(pbLogEvent,     &QPushButton::clicked,  this,       &ControlPanel::slot_logEvent);
        connect(pbSaveLog,      &QPushButton::clicked,  this,       &ControlPanel::sig_saveLog);
        connect(pbReload,       &QPushButton::clicked,  this,       &ControlPanel::slot_reload);
        connect(pbEnbDbgView,   &AQPushButton::clicked, this,       &ControlPanel::slot_dbgViewClicked);
        connect(pbEnbDbgFlags,  &AQPushButton::clicked, this,       &ControlPanel::slot_dbgFlagsClicked);
    }

    return commonGroup;
}

QVBoxLayout * ControlPanel::getInsightTop()
{
    QHBoxLayout * line1 = new QHBoxLayout;
    line1->addWidget(pbLogEvent);
    line1->addWidget(pbSaveLog);
    line1->addWidget(pbReload);
    line1->addWidget(pbRefreshView);
    line1->addWidget(pbUpdateView);

    line1->addStretch();

    line1->addWidget(radioDefined);
    line1->addWidget(radioPack);
    line1->addWidget(radioSingle);

    line1->addStretch();

    line1->addWidget(pbClearAll);
    line1->addWidget(pbShowTiling);
    line1->addWidget(pbShowMosaic);

    QHBoxLayout * line2 = new QHBoxLayout;
    line2->addWidget(pbRaise);
    line2->addSpacing(2);
    line2->addWidget(chkScaleToView);

    line2->addLayout(new Separator());

    line2->addWidget(pbEnbDbgView);
    line2->addWidget(pbEnbDbgFlags);
    line2->addSpacing(2);

    line2->addLayout(new Separator());

    line2->addWidget(mouseModeWidget);
    line2->addSpacing(2);

    line2->addLayout(new Separator());

    line2->addSpacing(2);
    line2->addWidget(pbExit);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(line1);
    vbox->addLayout(line2);
    vbox->addWidget(panelStatus);

    return vbox;
}

QVBoxLayout * ControlPanel::getDesignerTop()
{
    QHBoxLayout * line = new QHBoxLayout;
    line->addWidget(chkScaleToView);
    line->addLayout(rad_hbox);
    line->addWidget(pbClearAll);
    line->addWidget(pbShowTiling);
    line->addWidget(pbShowMosaic);
    line->addWidget(mouseModeWidget);
    line->addWidget(pbExit);

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(line);
    vbox->addWidget(panelStatus);

    return vbox;
}

// called by mpTimer
void ControlPanel::slot_poll()
{
#if defined(Q_OS_WINDOWS)
    //Sys::sysview->testSize();
#endif
    if (Sys::config->insightMode && !Sys::updatePanel)
    {
        return;
    }

    // sync status background to view backgroun
    QColor color = Sys::viewController->getViewBackgroundColor();
    panelStatus->setBacgroundColor(color);

    // refresh visible pages
    pageController->refreshPages();

    if (Sys::config->insightMode)
    {
        pbEnbDbgView->blockSignals(true);
        pbEnbDbgView->setChecked(Sys::viewController->isEnabled(VIEW_DEBUG));
        pbEnbDbgView->blockSignals(false);

        pbEnbDbgFlags->blockSignals(true);
        pbEnbDbgFlags->setChecked(Sys::flags->enabled());
        pbEnbDbgFlags->blockSignals(false);
    }

    updateGeometry();
    setWindowTitles();
}

void ControlPanel::slot_raisePanel()
{
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    raise();
    activateWindow();
}

void ControlPanel::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ActivationChange)
    {
        emit sig_raiseDetached();
    }
    QWidget::changeEvent(event);
}

void  ControlPanel::slot_delegateView(eViewType vtype, bool delegate)
{
    delegateView(vtype,delegate);
}

void ControlPanel::repeatChanged(int id)
{
    Sys::config->repeatMode = static_cast<eRepeatType>(id);

    Sys::render(RENDER_RESET_PROTOTYPES);

    if (Sys::viewController->isEnabled(VIEW_MAP_EDITOR))
    {
        emit sig_reload();
    }
}

void ControlPanel::slot_reload()
{
    switch (Sys::config->lastLoadedType)
    {
    case LT_MOSAIC:
        Sys::mosaicMaker->reload();
        break;

    case LT_TILING:
        Sys::tilingMaker->reload();
        break;

    case LT_LEGACY:
        Sys::designMaker->reload();
        break;

    case LT_UNDEFINED:
        break;
    }
}

void ControlPanel::overridePagelStatus(QString str)
{
    if (panelStatus && Sys::isGuiThread())
        panelStatus->overridePanelStatus(str);
}

void ControlPanel::restorePageStatus()
{
    if (panelStatus && Sys::isGuiThread())
        panelStatus->restorePanelStatus();
}

void ControlPanel::slot_logEvent()
{
    qInfo() << "** EVENT MARK **";
    Sys::dumpRefs();
    const QVector<Layer*> & layers = Sys::viewController->getActiveLayers();
    if (layers.size())
    {
        Layer * layer = layers.first();
        if (layer)
        {
            auto vsize = Sys::viewController->viewSize();
            auto csize = Sys::viewController->getCanvas().getCanvasSize();
            qInfo().noquote() << "ViewSize" << vsize << "aspect ratio" <<  Utils::aspectRatio(vsize) << "CanvasSize" << csize << "aspect ratio" << Utils::aspectRatio(csize);
            qInfo().noquote() << layer->layerName() << "Canvas" << Transform::info(layer->getCanvasTransform());
            qInfo().noquote() << layer->layerName() << "Model " << Transform::info(layer->getModelTransform());
        }
    }
    //emit sig_id_layers();
}

void ControlPanel::delegateViewByPage()
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
        delegateView(VIEW_PROTOTYPE,true);
        return;
    }

    page_map_editor * pfe = dynamic_cast<page_map_editor*>(currentPage);
    if (pfe)
    {
        delegateView(VIEW_MAP_EDITOR,true);
        return;
    }

    page_motif_maker * pfm = dynamic_cast<page_motif_maker*>(currentPage);
    if (pfm)
    {
        delegateView(VIEW_MOTIF_MAKER,true);
        return;
    }

    page_tiling_maker * ptm = dynamic_cast<page_tiling_maker*>(currentPage);
    if (ptm)
    {
        delegateView(VIEW_TILING_MAKER,true);
        return;
    }

    page_mosaic_maker * psm = dynamic_cast<page_mosaic_maker*>(currentPage);
    if (psm)
    {
        delegateView(VIEW_MOSAIC,true);
        return;
    }
}

void ControlPanel::showMosaicPressed()
{
    delegateView(VIEW_MOSAIC,true);

    if (!Sys::config->insightMode)
    {
        Sys::viewController->raiseView();
    }
}

void ControlPanel::showTilingPressed()
{
    panel_page * currentPage = pageController->getCurrentPage();

    page_tiling_maker * ptm  = dynamic_cast<page_tiling_maker*>(currentPage);
    if (ptm && Sys::viewController->isEnabled(VIEW_TILING))
    {
        delegateView(VIEW_TILING_MAKER,true);
    }
    else
    {
        delegateView(VIEW_TILING,true);
    }

    if (!Sys::config->insightMode)
    {
        Sys::viewController->raiseView();
    }
}

void ControlPanel::slot_scaleToView(bool enb)
{
    Sys::config->scaleToView = enb;
}

// This is the way to delegate a view
void ControlPanel::delegateView(eViewType vtype, bool delegate)
{
    viewSelect->selectViewer(vtype,delegate);
}

void ControlPanel::deselectGangedViewers()
{
    viewSelect->deselectGangedViewers();
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
    panelInfo  += Sys::config->rootMediaDir;
}

void ControlPanel::setWindowTitles()
{
    // The view
    QString viewTitle;

    if (!Sys::viewController->isEnabled(VIEW_LEGACY))
    {
        // regular case
        QString sMosaic;
        auto mosaic = Sys::mosaicMaker->getMosaic();
        if (mosaic)
        {
            sMosaic = mosaic->getName().get();
        }

        QString sTiling;
        auto tiling = Sys::tilingMaker->getSelected();
        if (tiling)
        {
            sTiling = tiling->getVName().get();
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
    if (Sys::designMaker->getLoadUnit()->getLoadState() == LS_LOADING)
    {
        transient = QString("  Loading Legacy Design: %1").arg(Sys::designMaker->getLoadUnit()->getLoadFile().getVersionedName().get());
    }
    else if (Sys::tilingMaker->getLoadUnit()->getLoadState() == LS_LOADING)
    {
        transient = QString("  Loading Tiling: %1").arg(Sys::tilingMaker->getLoadUnit()->getLoadFile().getVersionedName().get());
    }
    else if (Sys::mosaicMaker->getLoadUnit()->getLoadState() == LS_LOADING)
    {
        transient = QString("  Loading Mosaic: %1").arg(Sys::mosaicMaker->getLoadUnit()->getLoadFile().getVersionedName().get());
    }
    else
    {
        transient = viewTitle;
    }

    QString panelTitle = panelInfo + transient;

    if ((Sys::lastPanelTitle != panelTitle) || (Sys::lastViewTitle != viewTitle))
    {
        this->setWindowTitle(panelTitle);
        update();
        Sys::viewController->setWindowTitle(viewTitle);
        if (Sys::splitter)
        {
            Sys::splitter->setWindowTitle(panelTitle);
        }

        Sys::lastPanelTitle = panelTitle;
        Sys::lastViewTitle  = viewTitle;
    }
}

void ControlPanel::completePageSelection()
{
    delegateViewByPage();
    adjustSize();
}

void ControlPanel::slot_messageBox(QString txt, QString txt2)
{
    QMessageBox * box = new QMessageBox(this);
    box->setIcon(QMessageBox::Information);
    box->setAttribute(Qt::WA_DeleteOnClose, true);
    box->setText(txt);
    if (!txt2.isEmpty())
    {
        box->setInformativeText(txt2);
    }
    box->show();
}

void  ControlPanel::slot_dbgViewClicked(bool checked)
{
    delegateView(VIEW_DEBUG,checked);
    Sys::viewController->slot_reconstructView();
}

void  ControlPanel::slot_dbgFlagsClicked(bool checked)
{
    Sys::flags->enable(checked);
    if (checked == false)
    {
        Sys::debugMapCreate->wipeout();
        Sys::debugMapPaint->wipeout();
    }
    Sys::render(RENDER_RESET_ALL);
    Sys::viewController->slot_updateView();
}
