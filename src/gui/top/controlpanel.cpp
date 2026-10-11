#include <QMessageBox>
#include <QRadioButton>
#include <QTimer>
#include "gui/panels/page_debug.h"
#include "gui/panels/page_map_editor.h"
#include "gui/panels/page_mosaic_maker.h"
#include "gui/panels/page_motif_maker.h"
#include "gui/panels/page_prototype_info.h"
#include "gui/panels/page_tiling_maker.h"
#include "gui/panels/panel_page_controller.h"
#include "gui/panels/panel_pages_widget.h"
#include "gui/panels/panel_view_select.h"
#include "gui/top/controlpanel.h"
#include "gui/top/split_screen.h"
#include "gui/top/system_view.h"
#include "gui/top/view_controller.h"
#include "gui/widgets/mouse_mode_widget.h"
#include "legacy/design_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/settings/configuration.h"
#include "model/styles/interlace.h"
#include "model/tilings/tiling.h"
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
    unwovenBox  = nullptr;
}

ControlPanel::~ControlPanel()
{
    mpTimer->stop();

    if (!Sys::config->integratedScreen)
    {
        QPoint pt = pos();
        QSettings s = Sys::settings();
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
        if (!Sys::config->integratedScreen)
        {
            QSettings s = Sys::settings();
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
    commonGroup->setFixedWidth(PANEL_WIDTH);
    commonGroup->setContentsMargins(3,0,3,0);

    // LHS
    QLabel * menuSelectLabel  = new QLabel("Menu Selection");

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
    leftWidget->setMaximumWidth(PANEL_LHS_WIDTH);
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
    lowerWidget->setFixedWidth(PANEL_WIDTH);
    lowerWidget->setLayout(lowerLayout);

    // this puts it all together
    AQVBoxLayout * paneLayout = new AQVBoxLayout;
    paneLayout->setSizeConstraint(QLayout::SetFixedSize);
    paneLayout->addWidget(commonGroup);
    paneLayout->addWidget(lowerWidget);
    paneLayout->addStretch();

    this->setLayout(paneLayout);

    // connections
    connect(this,   &ControlPanel::sig_reconstructView, Sys::systemViewController, &ViewController::slot_reconstructView);
    connect(Sys::mosaicMaker, &MosaicMaker::sig_messageBox, this, &ControlPanel::slot_messageBox);
    connect(Sys::mosaicMaker, &MosaicMaker::sig_interlaceUnwoven, this, &ControlPanel::slot_interlaceUnwoven);
    connect(Sys::tilingMaker, &TilingMaker::sig_messageBox, this, &ControlPanel::slot_messageBox);

    // populate pages
    pageController  = new PanelPageController(pageList,rhs);
    pageController->populatePages();                        // create the new pages
    rhs->setFixedHeight(PANEL_RHS_HEIGHT);
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
    connect(pbClearAll,         &QPushButton::pressed,    Sys::systemViewController, &ViewController::slot_unloadAll, Qt::QueuedConnection);
    connect(pbShowMosaic,       &QPushButton::pressed,    this,           &ControlPanel::showMosaicPressed);
    connect(chkScaleToView,     &QCheckBox::clicked,      this,           &ControlPanel::slot_scaleToView);
    connect(&repeatRadioGroup,  &QButtonGroup::idClicked, this,           &ControlPanel::repeatChanged);

    if (Sys::config->insightMode)
    {
        // initial values
        pbEnbDbgView->setChecked(Sys::systemViewController->isEnabled(Layer::Type::DebugLayer));
        pbEnbDbgFlags->setChecked(Sys::flags->enabled());

        // connections
        connect(pbUpdateView,   &QPushButton::clicked, Sys::systemViewController, &ViewController::slot_updateView);
        connect(pbRefreshView,  &QPushButton::clicked, Sys::systemViewController, &ViewController::slot_reconstructView);
        connect(pbRaise,        &QPushButton::clicked,  this, []    { Sys::systemViewController->raiseView();});
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
    if (!Sys::updatePanel)
    {
        return;
    }

    // sync status background to view backgroun
    QColor color = Sys::systemViewController->getViewBackgroundColor();
    panelStatus->setBacgroundColor(color);

    // refresh visible pages
    pageController->refreshPages();

    if (Sys::config->insightMode)
    {
        pbEnbDbgView->blockSignals(true);
        pbEnbDbgView->setChecked(Sys::systemViewController->isEnabled(Layer::Type::DebugLayer));
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
    raise();            // for MacOs
    activateWindow();   // for Windows
}

void ControlPanel::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ActivationChange)
    {
        emit sig_raiseDetached();
    }
    QWidget::changeEvent(event);
}

void  ControlPanel::slot_delegateView(Layer::Type vtype, bool delegate)
{
    delegateView(vtype,delegate);
}

void ControlPanel::repeatChanged(int id)
{
    Sys::config->repeatMode = static_cast<eRepeatType>(id);

    Sys::render(RENDER_RESET_PROTOTYPES);

    if (Sys::systemViewController->isEnabled(Layer::Type::MapEditor))
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
    const QVector<Layer*> & layers = Sys::systemViewController->getActiveLayers();
    if (layers.size())
    {
        Layer * layer = layers.first();
        if (layer)
        {
            auto vsize = Sys::systemViewController->viewSize();
            auto csize = Sys::systemViewController->getCanvas()->getCanvasSize();
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
        delegateView(Layer::Type::Prototype,true);
        return;
    }

    page_map_editor * pfe = dynamic_cast<page_map_editor*>(currentPage);
    if (pfe)
    {
        delegateView(Layer::Type::MapEditor,true);
        return;
    }

    page_motif_maker * pfm = dynamic_cast<page_motif_maker*>(currentPage);
    if (pfm)
    {
        delegateView(Layer::Type::MotifMaker,true);
        return;
    }

    page_tiling_maker * ptm = dynamic_cast<page_tiling_maker*>(currentPage);
    if (ptm)
    {
        delegateView(Layer::Type::TilingMaker,true);
        return;
    }

    page_mosaic_maker * psm = dynamic_cast<page_mosaic_maker*>(currentPage);
    if (psm)
    {
        delegateView(Layer::Type::Mosaic,true);
        return;
    }
}

void ControlPanel::showMosaicPressed()
{
    delegateView(Layer::Type::Mosaic,true);

    Sys::systemViewController->raiseView();
}

void ControlPanel::showTilingPressed()
{
    panel_page * currentPage = pageController->getCurrentPage();

    page_tiling_maker * ptm  = dynamic_cast<page_tiling_maker*>(currentPage);
    if (ptm && Sys::systemViewController->isEnabled(Layer::Type::Tiling))
    {
        delegateView(Layer::Type::TilingMaker,true);
    }
    else
    {
        delegateView(Layer::Type::Tiling,true);
    }

    Sys::systemViewController->raiseView();
}

void ControlPanel::slot_scaleToView(bool enb)
{
    Sys::config->scaleToView = enb;
}

// This is the way to delegate a view
void ControlPanel::delegateView(Layer::Type vtype, bool delegate)
{
    viewSelect->selectViewer(vtype,delegate);
}

// for a checkbox click elsewhere that mirrors a layer enable - bypasses Lock Layers,
// see PanelViewSelect::userSelectViewer()
void ControlPanel::userDelegateView(Layer::Type vtype, bool delegate)
{
    viewSelect->userSelectViewer(vtype,delegate);
}

void ControlPanel::deselectGangedViewers()
{
    auto vec = Sys::systemViewController->getGangMembers();
    for (Layer::Type & vtype : vec)
    {
        delegateView(vtype,false);
    }
}

void ControlPanel::reEnableViewers()
{
    // the Lock Layers checkbox was initialized while lockView was forced on during startup
    // init/autoload (see TiledPatternMaker::slot_start()) - now that the real persisted value
    // has been restored, bring the checkbox back in sync with it
    viewSelect->syncLockCheckbox();

    QSettings settings = Sys::settings();
    QList<int> loadedList = settings.value("viewEnables2").value<QList<int>>();
    if (loadedList.isEmpty())
    {
        qDebug() <<"ViewEnables empty - defaulting to mosiac view";
        loadedList.push_back(static_cast<int>(Layer::Type::Mosaic));
    }

    for (auto i : std::as_const(loadedList))
    {
        Layer::Type vtype = static_cast<Layer::Type>(i);
        if (!Sys::config->insightMode)
        {
            switch (vtype)
            {
            case Layer::Type::MapEditor:
            case Layer::Type::Grid:
            case Layer::Type::Crop:
            case Layer::Type::DebugLayer:
                vtype = Layer::Type::Mosaic;
                break;

            default:
                break;
            }

        }
        else if (vtype == Layer::Type::BMPImage)
        {
            continue;
        }
        userDelegateView(vtype,true);
    }
}

void ControlPanel::getPanelInfo()
{
#ifdef QT_DEBUG
    panelInfo  = "Control Panel - Debug - Version ";
#else
    panelInfo = "Control Panel - Version ";
#endif
    panelInfo += TpmVersion::getVersion();
    if  (!Sys::gitBranch.isEmpty())
    {
        panelInfo += " [";
        panelInfo += Sys::gitBranch;
        panelInfo += "]";
    }
    panelInfo  += "  ";
    panelInfo  += Sys::getRootImageDir();
}

void ControlPanel::setWindowTitles()
{
    // The view
    QString viewTitle;

    if (!Sys::systemViewController->isEnabled(Layer::Type::LegacyDesign))
    {
        // regular case
        QString sMosaic;
        auto mosaic = Sys::mosaicMaker->getMosaic();
        if (mosaic)
        {
            sMosaic = mosaic->getName().get();
        }

        QString sTiling;
        auto tiling = Sys::tilingMaker->getSelectedTiling();
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
        Sys::systemViewController->setWindowTitle(viewTitle);
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

void ControlPanel::closePages()
{
    pageController->closePages();
}

bool ControlPanel::isVisiblePage(panel_page * page)
{
    return pageController->isVisiblePage(page);
}

bool ControlPanel::isVisiblePage(ePanelPage page)
{
    return pageController->isVisiblePage(page);
}

void ControlPanel::setCurrentPage(QString name)
{
    pageController->setCurrentPage(name);
}

panel_page * ControlPanel::getCurrentPage()
{
    return pageController->getCurrentPage();
}

panel_page * ControlPanel::getPage(QString name)
{
    return pageController->getPage(name);
}

void ControlPanel::slot_reattachPage(panel_page * page)
{
    pageController->reAttachPage(page);
}

void ControlPanel::slot_messageBox(QString txt, QString txt2, QMessageBox::Icon icon, bool modal)
{
    qInfo() << "Message Box:" << txt << txt2;
    QMessageBox * box = new QMessageBox(this);
    box->setIcon(icon);
    box->setAttribute(Qt::WA_DeleteOnClose, true);
    box->setText(txt);
    if (!txt2.isEmpty())
    {
        box->setInformativeText(txt2);
    }
    if (modal)
        box->exec();
    else
    {
        box->setModal(false);
        box->setAttribute(Qt::WA_ShowWithoutActivating);
        box->show();
    }
}

void ControlPanel::slot_interlaceUnwoven()
{
    int unwoven = 0;
    MosaicPtr mosaic = Sys::mosaicMaker->getMosaic();
    if (mosaic)
    {
        for (const StylePtr & style : mosaic->getStyleSet())
        {
            if (style->getStyleType() == STYLE_INTERLACED)
            {
                unwoven += std::static_pointer_cast<Interlace>(style)->getUnwovenVertices();
            }
        }
    }

    if (unwoven == 0)
    {
        if (unwovenBox)
            unwovenBox->hide();
        return;
    }

    if (!unwovenBox)
    {
        unwovenBox = new QMessageBox(this);
        unwovenBox->setIcon(QMessageBox::Warning);
        unwovenBox->setInformativeText("The map may need cleansing, or a different cleanse sensitivity");
        unwovenBox->setModal(false);
        unwovenBox->setAttribute(Qt::WA_ShowWithoutActivating);
    }
    unwovenBox->setText(QString("Interlace: %1 vertices with more than 4 edges were not woven").arg(unwoven));
    unwovenBox->show();
}

void  ControlPanel::slot_dbgViewClicked(bool checked)
{
    userDelegateView(Layer::Type::DebugLayer,checked);
    Sys::systemViewController->slot_reconstructView();
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
    Sys::systemViewController->slot_updateView();
}

void ControlPanel::bringToPrimaryScreen()
{
    QScreen * primary = qApp->primaryScreen();

    move(primary->geometry().center());
    slot_raisePanel();
}