#include <QRadioButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>

#include "panels/panel.h"
#include "legacy/design_maker.h"
#include "makers/map_editor/map_editor.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/runguard.h"
#include "misc/version.h"
#include "mosaic/mosaic.h"
#include "panels/page_background_image.h"
#include "panels/page_borders.h"
#include "panels/page_config.h"
#include "panels/page_crop_maker.h"
#include "panels/page_debug.h"
#include "panels/page_grid.h"
#include "panels/page_image_tools.h"
#include "panels/page_layers.h"
#include "panels/page_loaders.h"
#include "panels/page_log.h"
#include "panels/page_map_editor.h"
#include "panels/page_modelSettings.h"
#include "panels/page_mosaic_info.h"
#include "panels/page_mosaic_maker.h"
#include "panels/page_motif_maker.h"
#include "panels/page_prototype_info.h"
#include "panels/page_save.h"
#include "panels/page_system_info.h"
#include "panels/page_tiling_maker.h"
#include "settings/configuration.h"
#include "tile/tiling.h"
#include "tiledpatternmaker.h"
#include "viewers/viewcontrol.h"
#include "widgets/mouse_mode_widget.h"
#include "widgets/panel_pagesWidget.h"

extern RunGuard * guard;

ControlPanel * ControlPanel::mpThis = nullptr;

ControlPanel * ControlPanel::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new ControlPanel();
    }
    return mpThis;
}

void ControlPanel::releaseInstance()
{
    if (mpThis)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

ControlPanel::ControlPanel() : AQWidget()
{
    setObjectName("ControlPanel");

    QProcess process;
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    process.startCommand("git branch --show-current");
#else
    QStringList qsl;
    qsl << "branch" << "--show-current";
    process.start("git",qsl);
#endif
    process.waitForFinished(-1); // will wait forever until finished

    QByteArray sout  = process.readAllStandardOutput();
    QString br(sout);
    this->gitBranch = br.trimmed();
    qInfo().noquote() << "git branch      :" << gitBranch;

#if (QT_VERSION >= QT_VERSION_CHECK(5,9,0))
    setWindowFlag(Qt::WindowMinimizeButtonHint,true);
#endif

    loadState = LOADING_NONE;

    config  = Configuration::getInstance();

    getPanelInfo();

    splash = new TPMSplash();

    isShown = false;
#if 0
    QSettings s;
    move(s.value((QString("panelPos/%1").arg(config->appInstance))).toPoint());
#endif
}

void ControlPanel::init(TiledPatternMaker * parent)
{
    maker   = parent;
    view    = ViewControl::getInstance();
    mosaicMaker = MosaicMaker::getInstance();
    tilingMaker = TilingMaker::getInstance();

    exclusiveViews = true;
    updateLocked   = false;

    setupGUI();

    mpTimer = new QTimer(this);
    connect(mpTimer, &QTimer::timeout, this, &ControlPanel::slot_poll);

    connect (panelPageList, &PanelListWidget::sig_detachWidget, this,	&ControlPanel::slot_detachWidget, Qt::QueuedConnection);

    // create the new pages
    populatePages();

    // select page
    panelPageList->setCurrentRow(config->panelName);
    panelPageList->establishSize();

    if (config->enableDetachedPages)
    {
        floatPages();
    }

    mpTimer->start(100);    // always runs
}

ControlPanel::~ControlPanel()
{
    qDebug() << "ControlPanel destructor";
    mpTimer->stop();
#if 0
    if (!config->splitScreen && !closed)
    {
        QSettings s;
        s.setValue(QString("panelPos/%1").arg(config->appInstance), pos());
        closed =  true;
    }
#endif
    mPages.clear();
    panelPageList->clear();
    delete panelPages;
}

void ControlPanel::showEvent(QShowEvent * event)
{
    if (!isShown)
    {
        // first time only
        QSettings s;
        QPoint pt = s.value((QString("panelPos/%1").arg(config->appInstance))).toPoint();
        qDebug() << "ControlPanel::showEvent moving to:" << pt;
        move(pt);
        isShown = true;
    }
    QWidget::showEvent(event);
}

void ControlPanel::closeEvent(QCloseEvent *event)
{
    qDebug() << "ControlPanel::closeEvent";
    if (!config->splitScreen)
    {
        QPoint pt = pos();
        QSettings s;
        s.setValue(QString("panelPos/%1").arg(config->appInstance), pt);
        qDebug() << "ControlPanel::closeEvent pos" << pt;
    }

    guard->release();
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

void ControlPanel::closePages()
{
    for (auto page : qAsConst(mPages))
    {
        page->closePage();
    }
}

void ControlPanel::setCurrentPage(QString name)
{
    panelPageList->setCurrentRow(name);
}

panel_page *  ControlPanel::getCurrentPage()
{
    return panelPages->getCurrentPage();
}

void ControlPanel::setupGUI()
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
        QPushButton * pbUpdateView    = new QPushButton("Repaint View");
        QPushButton * pbRefreshView   = new QPushButton("Recreate View");
        QPushButton * pbSaveLog       = new QPushButton("Save Log");


        hlayout->addWidget(pbLogEvent);
        hlayout->addWidget(pbSaveLog);
        hlayout->addStretch();
        hlayout->addWidget(pbRefreshView  );
        hlayout->addWidget(pbUpdateView);
        hlayout->addStretch();

        connect(pbUpdateView,       &QPushButton::clicked,  this,     &ControlPanel::updateView);
        connect(pbLogEvent,         &QPushButton::clicked,  this,     &ControlPanel::slot_logEvent);
        connect(pbRefreshView,      &QPushButton::clicked,  view,     &ViewControl::slot_refreshView);
        connect(pbSaveLog,          &QPushButton::clicked,  this,     &ControlPanel::sig_saveLog);
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
    delegateKeyboardMouse(config->getViewerType());

    mouseModeWidget = new MouseModeWidget;

    QPushButton *pbRaise    = new QPushButton("Raise View");
    pbRaise->setFixedHeight(25);
    //pbRaise->setStyleSheet("QPushButton{ border-radius: 3px; padding-left: 9px; padding-right: 9px; }");

    QPushButton *pbExit     = new QPushButton("QUIT");
    pbExit->setFixedSize(QSize(71,25));
    //pbExit->setStyleSheet("QPushButton{ border-radius: 3px; }");

    QCheckBox * chkScaleToView  = new QCheckBox("Scale to View");

    QLabel    * l_kbdModes      = new QLabel("Keyboard control:");

    QFrame    * line           = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setLineWidth(1);
    QFrame    * line1           = new QFrame();
    line1->setFrameShape(QFrame::VLine);
    line1->setFrameShadow(QFrame::Sunken);
    line1->setLineWidth(1);
    QFrame    * line2           = new QFrame();
    line2->setFrameShape(QFrame::VLine);
    line2->setFrameShadow(QFrame::Sunken);
    line2->setLineWidth(1);

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

    // left widget
    panelPageList = new PanelListWidget;

    QLabel * menuSelect = new QLabel("Menu Select");

    QGroupBox * viewersGroup = createViewersBox();

    QVBoxLayout * leftBox = new QVBoxLayout;
    leftBox->addSpacing(7);
    leftBox->addWidget(menuSelect);
    leftBox->addWidget(panelPageList);
    leftBox->addWidget(viewersGroup);
    leftBox->addStretch();

    // right widget contains the pages
    panelPages = new PanelPagesWidget();

    // main box : contains left & right
    QHBoxLayout * mainBox = new QHBoxLayout;
    mainBox->setSizeConstraint(QLayout::SetFixedSize);
    mainBox->addLayout(leftBox);
    mainBox->addWidget(panelPages);
    mainBox->addStretch(2);
    mainBox->setAlignment(panelPages,Qt::AlignTop);

    // this puts it all together
    QVBoxLayout * panelBox = new QVBoxLayout;
    panelBox->setSizeConstraint(QLayout::SetFixedSize);
    panelBox->addWidget(commonGroup);
    panelBox->addLayout(mainBox);
    panelBox->addStretch();
    this->setLayout(panelBox);

    // initial values
    chkScaleToView->setChecked(config->scaleToView);

    // connections
    connect(pbExit,             &QPushButton::clicked,              this,     &ControlPanel::slot_exit);
    connect(pbRaise,            &QPushButton::clicked,              this,     &ControlPanel::slot_raise);
    connect(pbShowTiling,       &QPushButton::pressed,              this,     &ControlPanel::showTilingPressed);
    connect(pbClearAll,         &QPushButton::pressed,              view,     &ViewControl::slot_unloadAll, Qt::QueuedConnection);
    connect(pbShowMosaic,       &QPushButton::pressed,              this,     &ControlPanel::showMosaicPressed);
    connect(&repeatRadioGroup,  &QButtonGroup::idClicked,           this,     &ControlPanel::repeatChanged);
    connect(this,               &ControlPanel::sig_refreshView,     view,     &ViewControl::slot_refreshView);
    connect(this,               &ControlPanel::sig_render,          theApp,   &TiledPatternMaker::slot_render);
    connect(kbdModeCombo,       SIGNAL(currentIndexChanged(int)),   this,     SLOT(slot_kbdModeChanged(int)));
    connect(view,               &View::sig_kbdMode,                 this,     &ControlPanel::slot_kbdMode);
    connect(chkScaleToView,     &QCheckBox::clicked,                this,     &ControlPanel::slot_scaleToView);
}

void ControlPanel::populatePages()
{
    // qDebug() << "populateDevice";
    panel_page * wp;

    wp = new page_loaders(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    wp = new page_save(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    panelPageList->addSeparator();

    wp = new page_mosaic_maker(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    panelPageList->addSeparator();

    wp = new page_motif_maker(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    panelPageList->addSeparator();

    wp = new page_tiling_maker(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    panelPageList->addSeparator();

    wp = new page_borders(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    wp = new page_crop_maker(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    wp = new page_background_image(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    wp = new page_modelSettings(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    wp = new page_grid(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    if (config->insightMode)
    {
        panelPageList->addSeparator();

        wp = new page_map_editor(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());

        page_map_editor * wp_med = dynamic_cast<page_map_editor*>(wp);
        connect(this,   &ControlPanel::sig_reload,  wp_med,    &page_map_editor::slot_mosaicChanged);

        panelPageList->addSeparator();

        wp = new page_mosaic_info(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());

        wp = new page_prototype_info(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());

        wp = new page_layers(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());

        wp = new page_system_info(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());
    }

    panelPageList->addSeparator();

    wp = new page_config(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    if (config->insightMode)
    {
        wp = new page_image_tools(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());

        page_image_tools * wp_im = dynamic_cast<page_image_tools*>(wp);
        connect(maker, &TiledPatternMaker::sig_image0, wp_im, &page_image_tools::slot_setImageLeftCombo);
        connect(maker, &TiledPatternMaker::sig_image1, wp_im, &page_image_tools::slot_setImageRightCombo);

        wp = new page_log(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());

        page_log * pageLog = dynamic_cast<page_log*>(wp);
        connect(this, &ControlPanel::sig_saveLog, pageLog, &page_log::slot_copyLog);

        wp = new page_debug(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());
    }

    panelPageList->adjustSize();

    // connect page selection
    connect(panelPageList, &PanelListWidget::currentRowChanged, this, &ControlPanel::slot_selectPanelPage);
    connect(panelPageList, &PanelListWidget::itemClicked,       this, &ControlPanel::slot_itemPanelPage);
}

void ControlPanel::floatPages()
{
    QVector<QString> names;
    for (auto page :  qAsConst(mPages))
    {
        if (page->wasFloated())
        {
            qDebug() << "floating:" << page->getName();
            names.push_back(page->getName());
        }
    }

    for (auto& pagename : names)
    {
        slot_detachWidget(pagename);
    }
}

void ControlPanel::slot_selectPanelPage(int index)
{
    if (index == -1)  return;

    qDebug() << "ControlPanel::slot_selectPanelPage:" << index;

    // exiting previous page
    panel_page * currentPage = panelPages->getCurrentPage();
    if (currentPage)
    {
        if (currentPage->canExit())
        {
            currentPage->onExit();
        }
        else
        {
            panelPageList->blockSignals(true);
            panelPageList->setCurrentRow(currentPage->getName());
            panelPageList->blockSignals(false);
            return;
        }
    }

    // select new page
    QListWidgetItem * item =  panelPageList->item(index);
    Q_ASSERT(item);
    QString name = item->text();
    qDebug().noquote() << "slot_selectPanelPage:" << name;

    currentPage = panelPages->setCurrentPage(name);
    Q_ASSERT(currentPage);
    currentPage->setNewlySelected(true);
    config->panelName = name;

    delegateView();
    delegateKeyboardMouse(config->getViewerType());

    adjustSize();
}

void  ControlPanel::slot_itemPanelPage(QListWidgetItem * item)
{
    // this code duplicates slot_selectPanelPage but picks up when
    // view is different from panel selection (e.g. after a render)
    qDebug() << "ControlPanel::slot_itemPanelPage";
    QString name = item->text();
    if (name.isEmpty())
        return;

    delegateView();
    delegateKeyboardMouse(config->getViewerType());
}


void ControlPanel::slot_detachWidget(QString name)
{
    qDebug() << "slot_detachWidget" << name;

    panel_page * page = nullptr;

    for (auto panelPage : qAsConst(mPages))
    {
        if (panelPage->getName() == name)
        {
            page = panelPage;
            break;
        }
    }
    if (page == nullptr)
    {
        qWarning("Page not found to detach");
        return;
    }

    updateLocked = true;

    // hide in QListWidget
    int row = panelPageList->currentRow();
    panelPageList->hide(name);
    panelPageList->setCurrentRow(row);

    // change status
    page->setFloated(true);
    page->setNewlySelected(true);

    // deal with the page
    QDialog * dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    page->setParent(dlg,Qt::Window);
    page->setWindowTitle(name);
    page->floatMe();
    //page->adjustSize();
    refreshPage(page);

    updateLocked = false;
}

void ControlPanel::slot_attachWidget(QString name)
{
    qDebug() << "slot_attachWidget" << name;

    updateLocked = true;

    panelPageList->show(name);

    for (auto page : qAsConst(mPages))
    {
        if (page->windowTitle() == name)
        {
            page->setFloated(false);
            break;
        }
    }

    panelPageList->setCurrentRow(name);

    updateLocked = false;
}

// called by mpTimer
void ControlPanel::slot_poll()
{
    // refresh panel
    cbGrid->setChecked(config->showGrid);
    cbMeasure->setChecked(config->measure);
    cbCenter->setChecked(config->showCenterDebug);
    cbBackgroundImage->setChecked(config->showBackgroundImage);

    // refresh pages
    if (config->insightMode && !config->updatePanel)
    {
        return;
    }

    if (updateLocked)
    {
        return;
    }

    updateLocked = true;

    //	Update pages
    for (auto page : qAsConst(mPages))
    {
        if (isVisiblePage(page) && !page->pageBlocked())
        {
            refreshPage(page);
        }
    }

    updateGeometry();

    setWindowTitles();

    updateLocked = false;
}

bool ControlPanel::isVisiblePage(panel_page * page)
{
    if (page == getCurrentPage())
    {
        return true;
    }
    else if (page->isFloated())
    {
        return true;
    }
    return false;
}

void ControlPanel::refreshPage(panel_page * wp)
{
    Q_ASSERT(wp != nullptr);

    if (wp->isNewlySelected())
    {
        wp->setNewlySelected(false);
        wp->onEnter();
        wp->repaint();
    }

    wp->onRefresh();
    wp->show();
}

void ControlPanel::repeatChanged(int id)
{
    config->repeatMode = static_cast<eRepeatType>(id);

    emit sig_render();

    if (config->getViewerType() == VIEW_MAP_EDITOR)
    {
        emit sig_reload();
    }
}

void ControlPanel::slot_logEvent()
{
    qInfo() << "** EVENT MARK **";
    view->dumpRefs();
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

void  ControlPanel::updateView(bool enb)
{
    Q_UNUSED(enb);
    view->update();
}

void ControlPanel::delegateView()
{
    panel_page * currentPage = panelPages->getCurrentPage();
    if (!currentPage)
    {
        return;
    }

    qDebug() << "delegate view based on:" << currentPage->getName();

    page_prototype_info * ppi = dynamic_cast<page_prototype_info*>(currentPage);
    if (ppi)
    {
        selectViewer(VIEW_PROTOTYPE);
        return;
    }

    page_map_editor * pfe = dynamic_cast<page_map_editor*>(currentPage);
    if (pfe)
    {
        selectViewer(VIEW_MAP_EDITOR);
        return;
    }

    page_motif_maker * pfm = dynamic_cast<page_motif_maker*>(currentPage);
    if (pfm)
    {
        selectViewer(VIEW_MOTIF_MAKER);
        return;
    }

    page_tiling_maker * ptm = dynamic_cast<page_tiling_maker*>(currentPage);
    if (ptm)
    {
        selectViewer(VIEW_TILING_MAKER);
        return;
    }

    page_mosaic_maker * psm = dynamic_cast<page_mosaic_maker*>(currentPage);
    if (psm)
    {
        selectViewer(VIEW_MOSAIC);
        return;
    }
}

void ControlPanel::delegateKeyboardMouse(eViewType viewType)
{
    kbdModeCombo->blockSignals(true);
    kbdModeCombo->clear();
    if (viewType == VIEW_DESIGN)
    {
        kbdModeCombo->insertItem(100,"Mode Position",   QVariant(KBD_MODE_DES_POS));
        kbdModeCombo->insertItem(100,"Mode Layer",      QVariant(KBD_MODE_DES_LAYER_SELECT));
        kbdModeCombo->insertItem(100,"Mode Z-level",    QVariant(KBD_MODE_DES_ZLEVEL));
        kbdModeCombo->insertItem(100,"Mode Step",       QVariant(KBD_MODE_DES_STEP));
        kbdModeCombo->insertItem(100,"Mode Separation", QVariant(KBD_MODE_DES_SEPARATION));
        kbdModeCombo->insertItem(100,"Mode Origin",     QVariant(KBD_MODE_DES_ORIGIN));
        kbdModeCombo->insertItem(100,"Mode Offset",     QVariant(KBD_MODE_DES_OFFSET));
    }
    else
    {
        kbdModeCombo->insertItem(100,"Adjust View",      QVariant(KBD_MODE_XFORM_VIEW));
        kbdModeCombo->insertItem(100,"Adjust Selected",  QVariant(KBD_MODE_XFORM_SELECTED));
        kbdModeCombo->insertItem(100,"Adjust Background",QVariant(KBD_MODE_XFORM_BKGD));
        kbdModeCombo->insertItem(100,"Adjust Tiling",    QVariant(KBD_MODE_XFORM_TILING));
        kbdModeCombo->insertItem(100,"Adjust Unique Tile", QVariant(KBD_MODE_XFORM_UNIQUE_TILE));
        kbdModeCombo->insertItem(100,"Adjust Placed Tile", QVariant(KBD_MODE_XFORM_PLACED_TILE));
    }

    kbdModeCombo->blockSignals(false);

    view->resetKbdMode(); // converts if necessry
}

eKbdMode ControlPanel::getValidKbdMode(eKbdMode mode)
{
    Configuration * conf = Configuration::getInstance();
    if (conf->getViewerType() == VIEW_DESIGN)
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
        return mode;
    case KBD_MODE_XFORM_VIEW:
    case KBD_MODE_XFORM_SELECTED:
    case KBD_MODE_XFORM_BKGD:
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_UNIQUE_TILE:
    case KBD_MODE_XFORM_PLACED_TILE:
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
    selectViewer(VIEW_MOSAIC);
}

void ControlPanel::showTilingPressed()
{
    panel_page * currentPage = panelPages->getCurrentPage();

    page_tiling_maker * ptm  = dynamic_cast<page_tiling_maker*>(currentPage);
    if (ptm && (config->getViewerType() == VIEW_TILING))
    {
        selectViewer(VIEW_TILING_MAKER);
    }
    else
    {
        selectViewer(VIEW_TILING);
    }
}

void ControlPanel::slot_scaleToView(bool enb)
{
    config->scaleToView = enb;
}

void ControlPanel::slot_showBackChanged(bool enb)
{
    config->showBackgroundImage = enb;
    view->update();
}

void ControlPanel::slot_showGridChanged(bool enb)
{
    config->showGrid = enb;
    emit sig_refreshView();
}

void ControlPanel::slot_showMeasureChanged(bool enb)
{
    config->measure = enb;
    emit sig_refreshView();
}

void ControlPanel::slot_showCenterChanged(bool enb)
{
    config->showCenterDebug = enb;
    view->update();
}

QGroupBox *  ControlPanel::createViewersBox()
{
    QGroupBox * viewersBox  = new QGroupBox("View Select");

    cbMosaicView         = new QCheckBox("Mosaic");
    cbPrototypeView      = new QCheckBox("Prototype");
    cbMapEditor          = new QCheckBox("Map Editor");
    cbProtoMaker         = new QCheckBox("Motif Maker");
    cbTilingView         = new QCheckBox("Tiling");
    cbTilingMakerView    = new QCheckBox("Tiling Maker");
    cbRawDesignView      = new QCheckBox("Fixed Design");

    cbBackgroundImage    = new QCheckBox("Background Image");
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

    viewersBox->setLayout(avbox);

    // viewer group
    viewerGroup.addButton(cbRawDesignView,VIEW_DESIGN);
    viewerGroup.addButton(cbMosaicView,VIEW_MOSAIC);
    viewerGroup.addButton(cbPrototypeView,VIEW_PROTOTYPE);
    viewerGroup.addButton(cbTilingView,VIEW_TILING);
    viewerGroup.addButton(cbProtoMaker,VIEW_MOTIF_MAKER);
    viewerGroup.addButton(cbTilingMakerView,VIEW_TILING_MAKER);
    if (config->insightMode)
    {
        viewerGroup.addButton(cbMapEditor,VIEW_MAP_EDITOR);
    }
    else
    {
        // designer mode
        if (config->getViewerType() == VIEW_MAP_EDITOR)
        {
            config->setViewerType(VIEW_DESIGN);
        }
    }

    viewerGroup.setExclusive(false); // always false

    viewerGroup.button(config->getViewerType())->setChecked(true);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(&viewerGroup,       SIGNAL(buttonToggled(int, bool)),  this, SLOT(slot_Viewer_pressed(int,bool)));
#else
    connect(&viewerGroup,       &QButtonGroup::idToggled,       this, &ControlPanel::slot_Viewer_pressed);
#endif
    connect(cbLockView,         &QCheckBox::clicked,  this, &ControlPanel::slot_lockViewClicked);
    connect(cbMultiSelect,      &QCheckBox::clicked,  this, &ControlPanel::slot_multiSelect);
    connect(cbBackgroundImage,  &QCheckBox::clicked,  this, &ControlPanel::slot_showBackChanged);
    connect(cbGrid,             &QCheckBox::clicked,  this, &ControlPanel::slot_showGridChanged);
    connect(cbMeasure,          &QCheckBox::clicked,  this, &ControlPanel::slot_showMeasureChanged);
    connect(cbCenter,           &QCheckBox::clicked,  this, &ControlPanel::slot_showCenterChanged);
    connect(theApp,             &TiledPatternMaker::sig_lockStatus, this,&ControlPanel::slot_lockStatusChanged);

    return viewersBox;
}

void  ControlPanel::selectViewer(int id)
{
    if (config->lockView)
    {
        return;
    }

    viewerGroup.button(id)->setChecked(true);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    emit viewerGroup.buttonClicked(id);
#else
    emit viewerGroup.idClicked(id);
#endif
}

void ControlPanel::getPanelInfo()
{
#ifdef QT_DEBUG
    panelInfo  = "Control Panel - Debug - Version ";
#else
    panelInfo = "Control Panel - Release - Version ";
#endif
    panelInfo += tpmVersion;
    if  (!gitBranch.isEmpty())
    {
        panelInfo += " [";
        panelInfo += gitBranch;
        panelInfo += "]";
    }
    panelInfo  += "  ";
    config  = Configuration::getInstance();
    panelInfo  += config->rootMediaDir;
}

void ControlPanel::setWindowTitles()
{
    // The view
    QString viewTitle;
    if (!view->isEnabled(VIEW_DESIGN))
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
            sTiling = tiling->getName();
        }

        viewTitle  = QString("  Mosaic <%1>  Tiling <%2>  ").arg(sMosaic).arg(sTiling);

        if (view->isEnabled(VIEW_MAP_EDITOR))
        {
            viewTitle += MapEditor::getInstance()->getStatus();
        }
    }
    else
    {
        // legacy case
        QString sDesign = DesignMaker::getInstance()->getDesignName();
        viewTitle       = QString("  Design: %1").arg(sDesign);
    }

    // The control panel show transient status. The view shows whats loaded
    QString transient;
    switch (loadState)
    {
    case LOADING_NONE:
        transient = viewTitle;
        break;

    case LOADING_MOSAIC:
        transient = QString("  Loading Mosaic: %1").arg(loadName);
        break;

    case LOADING_TILING:
        transient = QString("  Loading Tiling: %1").arg(loadName);
        break;
    }

    QString panelTitle = panelInfo + transient;

    this->setWindowTitle(panelTitle);
    view->setViewTitle(viewTitle);
}

void ControlPanel::setLoadState(eLoadState state, QString name)
{
    loadState = state;
    loadName  = name;
}

void  ControlPanel::slot_Viewer_pressed(int id, bool enable)
{
    eViewType viewType = static_cast<eViewType>(id);
    if (enable)
    {
        if (exclusiveViews)
        {
            viewerGroup.blockSignals(true);
            viewerGroup.button(config->getViewerType())->setChecked(false);
            viewerGroup.button(id)->setChecked(true);
            viewerGroup.blockSignals(false);
        }

        // allways do this - the user has asked for it
        config->setViewerType(viewType);
        delegateKeyboardMouse(viewType);
    }

    if (exclusiveViews)
    {
        view->disableAllViews();
    }
    view->viewEnable(viewType,enable);

    emit sig_refreshView();
}

void ControlPanel::slot_multiSelect(bool enb)
{
    auto vbuttons = viewerGroup.buttons();
    // this is a mirror of the page views control
    exclusiveViews = !enb;
    if (!enb)
    {
        viewerGroup.blockSignals(true);
        for (auto button : vbuttons)
        {
            if (viewerGroup.id(button) == config->getViewerType())
            {
                continue;
            }
            button->setChecked(false);
        }
        viewerGroup.blockSignals(false);
    }
    view->disableAllViews();
    view->viewEnable(config->getViewerType(),true);
    emit sig_refreshView();
}

void  ControlPanel::slot_lockViewClicked(bool enb)
{
    config->lockView = enb;
}

void ControlPanel::slot_lockStatusChanged()
{
    cbLockView->blockSignals(true);
    cbLockView->setChecked(config->lockView);
    cbLockView->blockSignals(false);
}
