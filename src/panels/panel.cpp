/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "panels/panel.h"
#include "panels/page_style_maker.h"
#include "panels/page_layers.h"
#include "panels/page_tiling_maker.h"
#include "panels/page_style_figure_info.h"
#include "panels/page_design_elements.h"
#include "panels/page_protos.h"
#include "panels/page_position.h"
#include "panels/page_debug.h"
#include "panels/page_config.h"
#include "panels/page_log.h"
#include "panels/page_save.h"
#include "panels/page_prototype_maker.h"
#include "panels/page_workspace.h"
#include "panels/page_canvasSettings.h"
#include "panels/page_views.h"
#include "panels/page_loaders.h"
#include "panels/page_map_editor.h"
#include "designs/design.h"
#include "base/canvas.h"
#include "base/tiledpatternmaker.h"
#include "viewers/workspace_viewer.h"
#include "base/version.h"

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
    currentPage = nullptr;

    setObjectName("ControlPanel");

#if (QT_VERSION >= QT_VERSION_CHECK(5,9,0))
    setWindowFlag(Qt::WindowMinimizeButtonHint,true);
#endif

#ifdef TPMSPLASH
    splash = new TPMSplash(nullptr);    // null because don't want to loose focus when using
#endif

    QString title;
#ifdef QT_DEBUG
    title = "Control Panel - Debug - ";
#else
    title = "Control Panel - Release - ";
#endif
    title    += tpmVersion;
    config    = Configuration::getInstance();
    title    += config->rootMediaDir;
    setWindowTitle(title);

    QSettings s;
    move(s.value("panelPos").toPoint());
}

void ControlPanel::init(TiledPatternMaker * parent)
{
    maker  = parent;
    canvas = Canvas::getInstance();

    lastViewType = VIEW_MOSAIC;

    setupGUI();

    mpTimer = new QTimer(this);
    connect(mpTimer, &QTimer::timeout, this, &ControlPanel::slot_poll);

    connect (mpPanelPageList, &PanelListWidget::detachWidget, this,	&ControlPanel::slot_detachWidget, Qt::QueuedConnection);

    // create the new pages
    populatePages();

    if (config->enableDetachedPages)
    {
        floatPages();
    }

    mpTimer->start(100);    // always runs


    // select page
    mpPanelPageList->setCurrentRow(config->panelName);

    adjustSize();
}

ControlPanel::~ControlPanel()
{
    mpTimer->stop();

    mAttachedPages.clear();
    mDetachedPages.clear();
    mpPanelPageList->clear();
    delete panelPagesWidget;
}

void ControlPanel::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    if (!config->screenIsSplit)
    {
        QSettings s;
        s.setValue("panelPos", pos());
    }

    QWidget::closeEvent(event);

    qApp->quit();
}

void ControlPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    emit sig_panelResized();
}

void ControlPanel::closePages()
{
    for (auto it= mAttachedPages.begin(); it != mAttachedPages.end(); it++)
    {
        panel_page * pp = *it;
        pp->closePage();
    }

    for (auto it= mDetachedPages.begin(); it != mDetachedPages.end(); it++)
    {
        panel_page * pp = *it;
        pp->closePage();
    }
}

void ControlPanel::setupGUI()
{
    // hlayout - top row
    QHBoxLayout * hlayout = new QHBoxLayout();
    hlayout->setSizeConstraint(QLayout::SetFixedSize);

    cbShowDesign      = new QCheckBox("Show Design");

    radioDefined = new QRadioButton("Defined");
    radioPack    = new QRadioButton("Pack");
    radioSingle  = new QRadioButton("Single");


    QPushButton * pbLogEvent      = new QPushButton("Log Event");
    QPushButton * pbViewWorkspace = new QPushButton("View Workspace");
    QPushButton * pbUpdateView    = new QPushButton("UpdateView");

    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->addWidget(pbLogEvent);
    hlayout->addStretch();
    hlayout->addWidget(pbViewWorkspace);
    hlayout->addWidget(pbUpdateView);
    hlayout->addStretch();
    hlayout->addWidget(radioDefined);
    hlayout->addWidget(radioPack);
    hlayout->addWidget(radioSingle);
    hlayout->addStretch();
    hlayout->addWidget(cbShowDesign);

    repeatRadioGroup.addButton(radioDefined,REPEAT_DEFINED);
    repeatRadioGroup.addButton(radioPack,REPEAT_PACK);
    repeatRadioGroup.addButton(radioSingle,REPEAT_SINGLE);
    repeatRadioGroup.button(config->repeatMode)->setChecked(true);

    // hbox - second row

    statusLabel = new QLabel("Status");
    statusLabel->setFixedWidth(401);

    xformModeCombo = new QComboBox();
    xformModeCombo->insertItem(KBD_MODE_XFORM_VIEW,     "XForm View");
    xformModeCombo->insertItem(KBD_MODE_XFORM_BKGD,     "XForm Background");
    xformModeCombo->insertItem(KBD_MODE_XFORM_TILING,   "XForm Tiling");
    xformModeCombo->insertItem(KBD_MODE_XFORM_FEATURE,  "XForm Feature");
    xformModeCombo->insertItem(KBD_MODE_LAYER,          "Mode Layer");
    xformModeCombo->insertItem(KBD_MODE_ZLEVEL,         "Mode Z-level");
    xformModeCombo->insertItem(KBD_MODE_STEP,           "Mode Step");
    xformModeCombo->insertItem(KBD_MODE_SEPARATION,     "Mode separation");
    xformModeCombo->insertItem(KBD_MODE_ORIGIN,         "Mode Origin");
    xformModeCombo->insertItem(KBD_MODE_OFFSET,         "Mode Offset");
    xformModeCombo->insertItem(KBD_MODE_CENTER,         "Set Center");
    xformModeCombo->setCurrentIndex(config->kbdMode);

    cbUpdate                = new QCheckBox("Update Pages");
    QPushButton *pbRaise    = new QPushButton("Raise Picture");
    QPushButton *pbExit     = new QPushButton("QUIT");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(statusLabel);
    hbox->addStretch();
    hbox->addWidget(cbUpdate);
    hbox->addWidget(xformModeCombo);
    hbox->addWidget(pbRaise);
    hbox->addWidget(pbExit);

    // left widget
    mpPanelPageList = new PanelListWidget;      // left - list of pages
    mpPanelPageList->setFixedWidth(105);
    mpPanelPageList->setMinimumHeight(401);

    // right widget contains the pages
    panelPagesWidget = new PanelPagesWidget();

    // main box : contains left & right
    QHBoxLayout * mainBox = new QHBoxLayout;
    mainBox->setSizeConstraint(QLayout::SetFixedSize);
    mainBox->addWidget(mpPanelPageList);
    mainBox->addWidget(panelPagesWidget);
    mainBox->addStretch();
    mainBox->setAlignment(panelPagesWidget,Qt::AlignTop);

    status = new PanelStatus();

    // panel box - top level
    QVBoxLayout * panelBox = new QVBoxLayout;
    panelBox->setSizeConstraint(QLayout::SetFixedSize);

    panelBox->addWidget(status);
    panelBox->addLayout(hlayout);
    panelBox->addLayout(hbox);
    panelBox->addLayout(mainBox);
    panelBox->addStretch();
    // this puts it all together
    this->setLayout(panelBox);

    cbUpdate->setChecked(config->updatePanel);

    // connections
    WorkspaceViewer * viewer = WorkspaceViewer::getInstance();

    connect(pbExit,             &QPushButton::clicked,              this,    &ControlPanel::slot_exit);
    connect(pbLogEvent,         &QPushButton::clicked,              this,    &ControlPanel::slot_logEvent);
    connect(pbRaise,            &QPushButton::clicked,              this,    &ControlPanel::slot_raise);
    connect(pbViewWorkspace,    &QPushButton::clicked,              viewer,  &WorkspaceViewer::slot_viewWorkspace);
    connect(pbUpdateView,       &QPushButton::clicked,              this,    &ControlPanel::updateView);
    connect(&repeatRadioGroup,  SIGNAL(buttonClicked(int)),         this,    SLOT(repeatChanged(int)));
    connect(this,               &ControlPanel::sig_viewWS,          viewer,  &WorkspaceViewer::slot_viewWorkspace);
    connect(this,               &ControlPanel::sig_render,          maker,   &TiledPatternMaker::slot_render);
    connect(cbUpdate,           SIGNAL(clicked(bool)),              this,    SLOT(updateClicked(bool)));
    connect(xformModeCombo,     SIGNAL(currentIndexChanged(int)),   this,    SLOT(slot_xformModeChanged(int)));
    connect(canvas,             &Canvas::sig_kbdMode,               this,    &ControlPanel::slot_kbdMode);
    connect(cbShowDesign,       &QCheckBox::clicked,                this,    &ControlPanel::showDesignClicked);
}

void ControlPanel::populatePages()
{
    // qDebug() << "populateDevice";
    panel_page * wp;

    mpPanelPageList->addSeparator();

    wp = new page_loaders(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    wp = new page_save(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    mpPanelPageList->addSeparator();

    wp = new page_views(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());
    page_views * wp_cnt = dynamic_cast<page_views*>(wp);

    mpPanelPageList->addSeparator();

    wp = new page_workspace(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    wp = new page_protos(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    wp = new page_style_figure_info(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    wp = new page_design_elements(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    wp = new page_layers(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    mpPanelPageList->addSeparator();

    wp = new page_style_maker(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    wp = new page_map_editor(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());
    page_map_editor * wp_med = dynamic_cast<page_map_editor*>(wp);

    wp = new page_prototype_maker(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());
    page_prototype_maker * wp_fm = dynamic_cast<page_prototype_maker*>(wp);

    wp = new page_tiling_maker(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());
    page_tiling_maker * wp_td = dynamic_cast<page_tiling_maker*>(wp);

    wp = new page_canvasSettings(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    wp = new page_position(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    mpPanelPageList->addSeparator();

    wp = new page_config(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    wp = new page_debug(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());
    page_debug * wp_db = dynamic_cast<page_debug*>(wp);

    wp = new page_log(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    // connect page selection
    connect(mpPanelPageList, &PanelListWidget::currentRowChanged, this, &ControlPanel::slot_selectWidget);

    // make inter-page connections
    connect(wp_td,  &page_tiling_maker::sig_tilingChanged,   wp_fm,  &page_prototype_maker::slot_tilingChanged);
    connect(wp_td,  &page_tiling_maker::sig_reload,          wp_fm,  &page_prototype_maker::slot_reload);

    connect(this,   &ControlPanel::sig_selectViewer,         wp_cnt, &page_views::slot_selectViewer);

    connect(wp_med, &page_map_editor::sig_stylesReplaceProto,wp_fm,  &page_prototype_maker::slot_replaceInStyle);
    connect(wp_med, &page_map_editor::sig_stylesAddProto,    wp_fm,  &page_prototype_maker::slot_addToStyle);

    connect(wp_cnt, &page_views::sig_mapEdSelection,        wp_med, &page_map_editor::slot_reload);
    connect(this,   &ControlPanel::sig_reload,              wp_med, &page_map_editor::slot_reload);

    connect(maker, &TiledPatternMaker::sig_image0,          wp_db, &page_debug::slot_setImage0);
    connect(maker, &TiledPatternMaker::sig_image1,          wp_db, &page_debug::slot_setImage1);
}

void ControlPanel::floatPages()
{
    QVector<QString> names;
    for (auto it = mAttachedPages.begin(); it != mAttachedPages.end(); it++)
    {
        panel_page * pp = *it;
        if (pp->wasFloated())
        {
            qDebug() << "floating:" << pp->getName();
            names.push_back(pp->getName());
        }
    }

    for (auto it = names.begin(); it != names.end(); it++)
    {
        slot_detachWidget(*it);
    }
}

void ControlPanel::slot_selectWidget(int index)
{
    qDebug() << "select panel Widget" << index;
    if (index == -1)  return;

    // exiting previous page
    if (currentPage)
    {
        if (currentPage->canExit())
        {
            currentPage->onExit();
        }
        else
        {
            mpPanelPageList->blockSignals(true);
            mpPanelPageList->setCurrentRow(currentPage->getName());
            mpPanelPageList->blockSignals(false);
            return;
        }
    }

    // select new page
    QListWidgetItem * item =  mpPanelPageList->item(index);
    Q_ASSERT(item);
    QString name = item->text();
    qDebug().noquote() << "new panel selection:" << name;

    currentPage = panelPagesWidget->setPanelPage(name);
    Q_ASSERT(currentPage);
    currentPage->setNewlySelected(true);
    config->panelName = name;

    adjustSize();
    delegateView();
}

void ControlPanel::slot_detachWidget(QString name)
{
    qDebug() << "slot_detachWidget" << name;

    QMutexLocker ml(&mUpdateMutex);

    panel_page * page = nullptr;

    for (auto it = mAttachedPages.begin(); it != mAttachedPages.end(); it++)
    {
        panel_page * pp = *it;
        if (pp->getName() == name)
        {
            page = pp;
            break;
        }
    }
    if (page == nullptr)
    {
        qWarning("Page not found to detach");
        return;
    }

    // remove from QListWidget
    mpPanelPageList->removeItem(name);

    // remove from mAttachedPages...
    mAttachedPages.removeAll(page);

    // add to detached pages
    mDetachedPages.push_back(page);
    qDebug() << "slot_detachWidget: DETACHED PAGE:" << name;

    // deal with the page
    page->setParent(this,Qt::Window);
    page->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    page->setWindowTitle(name);
    if (page->wasFloated())
    {
        page->floatMe();
    }
    else
    {
        QPoint pt = QCursor::pos();
        qDebug() << "floating to cursor pos =" << pt;
        page->move(pt);
    }

    page->adjustSize();
    page->setFloated(true);
    page->setNewlySelected(true);
    refreshPage(page);
}

void ControlPanel::slot_attachWidget(QString name)
{
    qDebug() << "slot_attachWidget" << name;

    QMutexLocker ml(&mUpdateMutex);

    mpPanelPageList->addItem(name);

    for (auto it = mDetachedPages.begin();  it != mDetachedPages.end();  ++it)
    {
        panel_page * wp = *it;
        if (wp->windowTitle() == name)
        {
            mAttachedPages.push_back(wp);
            mDetachedPages.erase(it);

            QList<QListWidgetItem*> items = mpPanelPageList->findItems(name, Qt::MatchFixedString);
            Q_ASSERT(items.size());
            mpPanelPageList->setCurrentItem(items[0]);

            return;
        }
    }
}

// called by mpTimer
void ControlPanel::slot_poll()
{
    cbUpdate->setChecked(config->updatePanel);

    if (!config->updatePanel)
        return;

    QMutexLocker ml(&mUpdateMutex);

    QString astring = canvas->getKbdModeStr() + " : " + sViewerType[config->viewerType];
    statusLabel->setText(astring);

    //	Update active page...
    panel_page * pp = panelPagesWidget->currentPage;
    if (pp)
    {
        refreshPage(pp);
    }

    //	Update detached pages...
    for (auto it = mDetachedPages.begin(); it != mDetachedPages.end(); ++it)
    {
        pp = *it;
        refreshPage(pp);
    }
    //mpStackedWidget->updateGeometry();
    updateGeometry();
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

    wp->refreshPage();
    wp->show();
}

void ControlPanel::repeatChanged(int mode)
{
    config->repeatMode = static_cast<eRepeatType>(mode);

    emit sig_render();

    if (config->viewerType == VIEW_MAP_EDITOR)
    {
        emit sig_reload();
    }
}

void ControlPanel::slot_xformModeChanged(int row)
{
    qDebug() << "slot_xformMode_changed"  << row;
    eKbdMode mode = static_cast<eKbdMode>(row);
    config->kbdMode = mode;
}

void ControlPanel::slot_kbdMode(eKbdMode mode)
{
    xformModeCombo->setCurrentIndex(mode);
}

void ControlPanel::slot_logEvent()
{
    qInfo() << "** EVENT MARK **";
    View * view = View::getInstance();
    view->dump(true);
}

void ControlPanel::slot_raise()
{
    View * view = View::getInstance();
    view->setWindowState( (view->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    view->raise();
    view->activateWindow();
}

void ControlPanel::slot_exit()
{
    qApp->exit();
}

void  ControlPanel::updateClicked(bool enb)
{
    config->updatePanel = enb;
}

void  ControlPanel::updateView(bool enb)
{
    Q_UNUSED(enb);
    View * view = View::getInstance();
    view->update();
}

void ControlPanel::delegateView()
{
    if (config->lockView)
    {
        return;
    }

    page_map_editor * pfe = dynamic_cast<page_map_editor*>(currentPage);
    if (pfe)
    {
        if (cbShowDesign->isChecked())
            lastViewType = VIEW_MAP_EDITOR;
        else
            emit sig_selectViewer(VIEW_MAP_EDITOR);
        return;
    }

    page_prototype_maker * pfm = dynamic_cast<page_prototype_maker*>(currentPage);
    if (pfm)
    {
        if (cbShowDesign->isChecked())
            lastViewType = VIEW_FIGURE_MAKER;
        else
            emit sig_selectViewer(VIEW_FIGURE_MAKER);
        return;
    }

    page_tiling_maker * ptm = dynamic_cast<page_tiling_maker*>(currentPage);
    if (ptm)
    {
        if (cbShowDesign->isChecked())
            lastViewType = VIEW_TILING_MAKER;
        else
            emit sig_selectViewer(VIEW_TILING_MAKER);
        return;
    }

    page_style_figure_info * sfi = dynamic_cast<page_style_figure_info*>(currentPage);
    if (sfi)
    {
        if (cbShowDesign->isChecked())
            lastViewType = VIEW_DEL;
        else
            emit sig_selectViewer(VIEW_DEL);
        return;
    }

    page_style_maker * psm = dynamic_cast<page_style_maker*>(currentPage);
    if (psm)
    {
        if (cbShowDesign->isChecked())
            lastViewType = VIEW_MOSAIC;
        else
            emit sig_selectViewer(VIEW_MOSAIC);
        return;
    }
}

void  ControlPanel::showDesignClicked(bool state)
{
    if (state)
    {
        lastViewType = config->viewerType;
        emit sig_selectViewer(VIEW_MOSAIC);
    }
    else
    {
        emit sig_selectViewer(lastViewType);
    }
    emit sig_viewWS();
}

void  page_views::slot_selectViewer(int id)
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
