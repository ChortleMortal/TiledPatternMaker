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
#include "panels/page_prototype_info.h"
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
#include "panels/view_panel.h"
#include "designs/design.h"
#include "base/tiledpatternmaker.h"
#include "base/version.h"
#include "viewers/workspace_viewer.h"

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
    splash = new TPMSplash();
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
    closed = false;
    viewer = WorkspaceViewer::getInstance();
    view   = View::getInstance();

    setupGUI();

    mpTimer = new QTimer(this);
    connect(mpTimer, &QTimer::timeout, this, &ControlPanel::slot_poll);

    connect (mpPanelPageList, &PanelListWidget::detachWidget, this,	&ControlPanel::slot_detachWidget, Qt::QueuedConnection);

    // create the new pages
    populatePages();
    mpPanelPageList->establishHeight();

    if (config->enableDetachedPages)
    {
        floatPages();
    }

    mpTimer->start(100);    // always runs

    // select page
    mpPanelPageList->setCurrentRow(config->panelName);

    //adjustSize();
}

ControlPanel::~ControlPanel()
{
    mpTimer->stop();

    if (!config->splitScreen && !closed)
    {
        QSettings s;
        s.setValue("panelPos", pos());
        closed =  true;
    }

    mAttachedPages.clear();
    mDetachedPages.clear();
    mpPanelPageList->clear();
    delete panelPagesWidget;
}

void ControlPanel::closeEvent(QCloseEvent *event)
{
    if (!config->splitScreen && !closed)
    {
        QSettings s;
        s.setValue("panelPos", pos());
        closed =  true;
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

    radioDefined = new QRadioButton("Full Mosiac");
    radioPack    = new QRadioButton("Packed Mosiac");
    radioSingle  = new QRadioButton("Simple Mosaic");

    QPushButton * pbShowMosaic    = new QPushButton("Show Mosaic");
    QPushButton * pbShowTiling    = new QPushButton("Show Tiling");
    QCheckBox   * chkScaleToView  = new QCheckBox("Scale to View");
    QCheckBox   * showBackImage   = new QCheckBox("Show background images");

    if (config->nerdMode)
    {
        QPushButton * pbLogEvent      = new QPushButton("Log Event");
        QPushButton * pbUpdateView    = new QPushButton("Updatev View");
        QPushButton * pbViewWorkspace = new QPushButton("View Workspace");

        hlayout->setContentsMargins(0, 0, 0, 0);
        hlayout->addWidget(pbLogEvent);
        hlayout->addStretch();
        hlayout->addWidget(pbViewWorkspace);
        hlayout->addWidget(pbUpdateView);
        hlayout->addStretch();
        hlayout->addWidget(chkScaleToView);
        hlayout->addWidget(showBackImage);
        hlayout->addStretch();
        hlayout->addWidget(radioDefined);
        hlayout->addWidget(radioPack);
        hlayout->addWidget(radioSingle);
        hlayout->addStretch();
        hlayout->addWidget(pbShowTiling);
        hlayout->addWidget(pbShowMosaic);

        connect(pbUpdateView,       &QPushButton::clicked,              this,    &ControlPanel::updateView);
        connect(pbLogEvent,         &QPushButton::clicked,              this,    &ControlPanel::slot_logEvent);
        connect(pbViewWorkspace,    &QPushButton::clicked,              viewer,  &WorkspaceViewer::slot_viewWorkspace);
    }
    else
    {
        hlayout->setContentsMargins(0, 0, 0, 0);
        hlayout->addWidget(chkScaleToView);
        hlayout->addWidget(showBackImage);
        hlayout->addStretch();
        hlayout->addWidget(radioDefined);
        hlayout->addWidget(radioPack);
        hlayout->addWidget(radioSingle);
        hlayout->addStretch();
        hlayout->addWidget(pbShowTiling);
        hlayout->addWidget(pbShowMosaic);
    }

    repeatRadioGroup.addButton(radioDefined,REPEAT_DEFINED);
    repeatRadioGroup.addButton(radioPack,REPEAT_PACK);
    repeatRadioGroup.addButton(radioSingle,REPEAT_SINGLE);
    repeatRadioGroup.button(config->repeatMode)->setChecked(true);

    // hbox - second row
    kbdModeCombo = new QComboBox();
    delegateKeyboardMouse(config->viewerType);

    QPushButton *pbRaise    = new QPushButton("Raise Picture to Top");
    QPushButton *pbExit     = new QPushButton("QUIT");

    QHBoxLayout * hbox = new QHBoxLayout;

    if (config->nerdMode)
    {
        statusLabel = new QLabel("Status");
        statusLabel->setFixedWidth(401);
        cbUpdate    = new QCheckBox("Update Pages");
        cbUpdate->setChecked(config->updatePanel);

        hbox->addWidget(statusLabel);
        hbox->addStretch();
        hbox->addWidget(cbUpdate);
        hbox->addWidget(kbdModeCombo);
        hbox->addWidget(pbRaise);
        hbox->addWidget(pbExit);

        connect(cbUpdate, SIGNAL(clicked(bool)), this, SLOT(updateClicked(bool)));
    }
    else
    {
        hbox->addStretch();
        hbox->addWidget(kbdModeCombo);
        hbox->addWidget(pbRaise);
        hbox->addWidget(pbExit);
    }

    // left widget
    mpPanelPageList = new PanelListWidget;
    QGroupBox * gb  =  createWorkspaceViewers();

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addWidget(mpPanelPageList);
    vb->addWidget(gb);
    vb->addStretch();

    // right widget contains the pages
    panelPagesWidget = new PanelPagesWidget();

    // main box : contains left & right
    QHBoxLayout * mainBox = new QHBoxLayout;
    mainBox->setSizeConstraint(QLayout::SetFixedSize);
    mainBox->addLayout(vb);
    mainBox->addWidget(panelPagesWidget);
    mainBox->addStretch(2);
    mainBox->setAlignment(panelPagesWidget,Qt::AlignTop);

    panelStatus = new PanelStatus();
    panelStatus->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    ViewPanel * vpanel = new ViewPanel;
    vpanel->setButtonSize(QSize(89,29));
    vpanel->setContentsMargins(0,0,0,0);

    QHBoxLayout * statusHbox = new QHBoxLayout;
    statusHbox->addWidget(panelStatus);
    statusHbox->addWidget(vpanel);

    // panel box - top level
    QVBoxLayout * panelBox = new QVBoxLayout;
    panelBox->setSizeConstraint(QLayout::SetFixedSize);

    // this puts it all together
    panelBox->addLayout(statusHbox);
    panelBox->addLayout(hlayout);
    panelBox->addLayout(hbox);
    panelBox->addLayout(mainBox);
    panelBox->addStretch();
    this->setLayout(panelBox);

    // initial values
    chkScaleToView->setChecked(config->scaleToView);
    showBackImage->setChecked(config->showBackgroundImage);

    // connections
    connect(pbExit,             &QPushButton::clicked,              this,    &ControlPanel::slot_exit);
    connect(pbRaise,            &QPushButton::clicked,              this,    &ControlPanel::slot_raise);
    connect(pbShowTiling,       &QPushButton::pressed,              this,    &ControlPanel::showTilingPressed);
    connect(pbShowMosaic,       &QPushButton::pressed,              this,    &ControlPanel::showMosaicPressed);
    connect(&repeatRadioGroup,  SIGNAL(buttonClicked(int)),         this,    SLOT(repeatChanged(int)));
    connect(this,               &ControlPanel::sig_viewWS,          viewer,  &WorkspaceViewer::slot_viewWorkspace);
    connect(this,               &ControlPanel::sig_render,          maker,   &TiledPatternMaker::slot_render);
    connect(kbdModeCombo,       SIGNAL(currentIndexChanged(int)),   this,    SLOT(slot_kbdModeChanged(int)));
    connect(view,               &View::sig_kbdMode,                 this,    &ControlPanel::slot_kbdMode);
    connect(chkScaleToView,     &QCheckBox::clicked,                this,    &ControlPanel::slot_scaleToView);
    connect(showBackImage,      &QCheckBox::clicked,                this,    &ControlPanel::slot_showBackChanged);

}

void ControlPanel::populatePages()
{
    // qDebug() << "populateDevice";
    panel_page * wp;

    wp = new page_loaders(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    wp = new page_save(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    if (config->nerdMode)
    {
        mpPanelPageList->addSeparator();

        wp = new page_views(this);
        mAttachedPages.push_back(wp);
        panelPagesWidget->addWidget(wp);
        mpPanelPageList->addItem(wp->getName());
    }

    mpPanelPageList->addSeparator();

    wp = new page_tiling_maker(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());
    page_tiling_maker * wp_td = dynamic_cast<page_tiling_maker*>(wp);

    wp = new page_prototype_maker(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());
    page_prototype_maker * wp_fm = dynamic_cast<page_prototype_maker*>(wp);

    wp = new page_map_editor(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());
    page_map_editor * wp_med = dynamic_cast<page_map_editor*>(wp);

    wp = new page_style_maker(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    wp = new page_canvasSettings(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    if (config->nerdMode)
    {
        mpPanelPageList->addSeparator();

        wp = new page_layers(this);
        mAttachedPages.push_back(wp);
        panelPagesWidget->addWidget(wp);
        mpPanelPageList->addItem(wp->getName());

        wp = new page_design_elements(this);
        mAttachedPages.push_back(wp);
        panelPagesWidget->addWidget(wp);
        mpPanelPageList->addItem(wp->getName());

        wp = new page_prototype_info(this);
        mAttachedPages.push_back(wp);
        panelPagesWidget->addWidget(wp);
        mpPanelPageList->addItem(wp->getName());

        wp = new page_style_figure_info(this);
        mAttachedPages.push_back(wp);
        panelPagesWidget->addWidget(wp);
        mpPanelPageList->addItem(wp->getName());

        wp = new page_workspace(this);
        mAttachedPages.push_back(wp);
        panelPagesWidget->addWidget(wp);
        mpPanelPageList->addItem(wp->getName());
    }

    mpPanelPageList->addSeparator();

    wp = new page_config(this);
    mAttachedPages.push_back(wp);
    panelPagesWidget->addWidget(wp);
    mpPanelPageList->addItem(wp->getName());

    if (config->nerdMode)
    {
        wp = new page_debug(this);
        mAttachedPages.push_back(wp);
        panelPagesWidget->addWidget(wp);
        mpPanelPageList->addItem(wp->getName());
        page_debug * wp_db = dynamic_cast<page_debug*>(wp);

        wp = new page_log(this);
        mAttachedPages.push_back(wp);
        panelPagesWidget->addWidget(wp);
        mpPanelPageList->addItem(wp->getName());

        connect(maker, &TiledPatternMaker::sig_image0, wp_db, &page_debug::slot_setImage0);
        connect(maker, &TiledPatternMaker::sig_image1, wp_db, &page_debug::slot_setImage1);
    }

    mpPanelPageList->adjustSize();


    // connect page selection
    connect(mpPanelPageList, &PanelListWidget::currentRowChanged, this, &ControlPanel::slot_selectPanelPage);

    // make inter-page connections
    connect(wp_td,  &page_tiling_maker::sig_tilingChanged,   wp_fm,  &page_prototype_maker::slot_tilingChanged);
    connect(wp_td,  &page_tiling_maker::sig_reload,          wp_fm,  &page_prototype_maker::slot_reload);

    connect(wp_med, &page_map_editor::sig_stylesReplaceProto,wp_fm,  &page_prototype_maker::slot_replaceInStyle);
    connect(wp_med, &page_map_editor::sig_stylesAddProto,    wp_fm,  &page_prototype_maker::slot_addToStyle);

    connect(this,   &ControlPanel::sig_reload,              wp_med, &page_map_editor::slot_reload);
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

void ControlPanel::slot_selectPanelPage(int index)
{
    qDebug() << "slot_selectPanelPage:" << index;
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
    qDebug().noquote() << "slot_selectPanelPage:" << name;

    currentPage = panelPagesWidget->setPanelPage(name);
    Q_ASSERT(currentPage);
    currentPage->setNewlySelected(true);
    config->panelName = name;

    delegateView();
    delegateKeyboardMouse(config->viewerType);

    adjustSize();
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
    if (config->nerdMode && !config->updatePanel)
    {
        return;
    }

    QMutexLocker ml(&mUpdateMutex);

    if (config->nerdMode)
    {
        QString astring = view->getKbdModeStr() + " : " + sViewerType[config->viewerType];
        statusLabel->setText(astring);
    }

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
    if (!currentPage)
    {
        return;
    }

    qDebug() << "delegate view based on:" << currentPage->getName();

    if (config->lockView)
    {
        return;
    }

    page_map_editor * pfe = dynamic_cast<page_map_editor*>(currentPage);
    if (pfe)
    {
        selectViewer(VIEW_MAP_EDITOR);
        return;
    }

    page_prototype_maker * pfm = dynamic_cast<page_prototype_maker*>(currentPage);
    if (pfm)
    {
        selectViewer(VIEW_FROTOTYPE_MAKER);
        return;
    }

    page_tiling_maker * ptm = dynamic_cast<page_tiling_maker*>(currentPage);
    if (ptm)
    {
        selectViewer(VIEW_TILING_MAKER);
        return;
    }

    page_style_maker * psm = dynamic_cast<page_style_maker*>(currentPage);
    if (psm)
    {
        selectViewer(VIEW_MOSAIC);
        return;
    }

    page_canvasSettings * pcs = dynamic_cast<page_canvasSettings*>(currentPage);
    if (pcs)
    {
        selectViewer(VIEW_MOSAIC);
        return;
    }

    if (config->nerdMode)
    {
        page_style_figure_info * sfi = dynamic_cast<page_style_figure_info*>(currentPage);
        if (sfi)
        {
            //selectViewer(VIEW_DESIGN_ELEMENT);
            return;
        }

        page_design_elements * pdes = dynamic_cast<page_design_elements*>(currentPage);
        if (pdes)
        {
            selectViewer(VIEW_DESIGN_ELEMENT);
            return;
        }

        page_prototype_info * ppi = dynamic_cast<page_prototype_info*>(currentPage);
        if (ppi)
        {
            selectViewer(VIEW_PROTOTYPE);
            return;
        }
    }
}

void ControlPanel::delegateKeyboardMouse(eViewType viewType)
{
    kbdModeCombo->blockSignals(true);
    kbdModeCombo->clear();
    if (viewType == VIEW_DESIGN)
    {
        kbdModeCombo->insertItem(100,"Mode Position",   QVariant(KBD_MODE_POS));
        kbdModeCombo->insertItem(100,"Mode Layer",      QVariant(KBD_MODE_LAYER));
        kbdModeCombo->insertItem(100,"Mode Z-level",    QVariant(KBD_MODE_ZLEVEL));
        kbdModeCombo->insertItem(100,"Mode Step",       QVariant(KBD_MODE_STEP));
        kbdModeCombo->insertItem(100,"Mode Separation", QVariant(KBD_MODE_SEPARATION));
        kbdModeCombo->insertItem(100,"Mode Origin",     QVariant(KBD_MODE_ORIGIN));
        kbdModeCombo->insertItem(100,"Mode Offset",     QVariant(KBD_MODE_OFFSET));

        config->kbdMode = getValidDesignMode(config->kbdMode);
    }
    else
    {
        kbdModeCombo->insertItem(100,"XForm View",      QVariant(KBD_MODE_XFORM_VIEW));
        kbdModeCombo->insertItem(100,"XForm Background",QVariant(KBD_MODE_XFORM_BKGD));
        kbdModeCombo->insertItem(100,"XForm Tiling",    QVariant(KBD_MODE_XFORM_TILING));
        kbdModeCombo->insertItem(100,"XForm Feature",   QVariant(KBD_MODE_XFORM_FEATURE));
        kbdModeCombo->insertItem(100,"Set Center",      QVariant(KBD_MODE_CENTER));

        config->kbdMode = getValidMosaicMode(config->kbdMode);
    }
    kbdModeCombo->blockSignals(false);

    slot_kbdMode(config->kbdMode);  // selects
}


eKbdMode ControlPanel::getValidKbdMode(eKbdMode mode)
{
    Configuration * conf = Configuration::getInstance();
    if (conf->viewerType == VIEW_DESIGN)
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
    case KBD_MODE_LAYER:
    case KBD_MODE_ZLEVEL:
    case KBD_MODE_STEP:
    case KBD_MODE_SEPARATION:
    case KBD_MODE_ORIGIN:
    case KBD_MODE_OFFSET:
        return mode;
    default:
        return KBD_MODE_LAYER;
    }
}

eKbdMode ControlPanel::getValidMosaicMode(eKbdMode mode)
{
    switch (mode)
    {
    case KBD_MODE_XFORM_VIEW:
    case KBD_MODE_XFORM_BKGD:
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_FEATURE:
    case KBD_MODE_CENTER:
        return mode;
    default:
        return KBD_MODE_XFORM_VIEW;
    }
}

// kbdModeCombo selection
void ControlPanel::slot_kbdModeChanged(int row)
{
    Q_UNUSED(row)
    qDebug() << "slot_kbdModeChanged to:"  << kbdModeCombo->currentText();
    QVariant var = kbdModeCombo->currentData();
    eKbdMode mode = static_cast<eKbdMode>(var.toInt());
    config->kbdMode = getValidKbdMode(mode);
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
    page_tiling_maker * ptm = dynamic_cast<page_tiling_maker*>(currentPage);
    if (ptm && (config->viewerType == VIEW_TILING))
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
    emit sig_viewWS();
}


QGroupBox *  ControlPanel::createWorkspaceViewers()
{
    QGroupBox * workspaceViewersBox  = new QGroupBox("View Select");

    cbRawDesignView      = new QCheckBox("Raw Design");
    cbMosaicView         = new QCheckBox("Mosaic");
    cbPrototypeView      = new QCheckBox("Prototype");
    cbDELView            = new QCheckBox("Design Elements");
    cbFigMapView         = new QCheckBox("Map Editor");
    cbProtoMaker         = new QCheckBox("Prototype Maker");
    cbTilingView         = new QCheckBox("Tiling");
    cbTilingMakerView    = new QCheckBox("Tiling Maker");
    cbFaceSetView        = new QCheckBox("FaceSet");

    AQVBoxLayout * avbox = new AQVBoxLayout();

    avbox->addWidget(cbRawDesignView);
    avbox->addWidget(cbMosaicView);
    avbox->addWidget(cbPrototypeView);
    avbox->addWidget(cbDELView);
    avbox->addWidget(cbFigMapView);
    avbox->addWidget(cbProtoMaker);
    avbox->addWidget(cbTilingView);
    avbox->addWidget(cbTilingMakerView);
    avbox->addWidget(cbFaceSetView);

    workspaceViewersBox->setLayout(avbox);

    // viewer group
    viewerGroup.addButton(cbRawDesignView,VIEW_DESIGN);
    viewerGroup.addButton(cbMosaicView,VIEW_MOSAIC);
    viewerGroup.addButton(cbPrototypeView,VIEW_PROTOTYPE);
    viewerGroup.addButton(cbTilingView,VIEW_TILING);
    viewerGroup.addButton(cbProtoMaker,VIEW_FROTOTYPE_MAKER);
    viewerGroup.addButton(cbDELView,VIEW_DESIGN_ELEMENT);
    viewerGroup.addButton(cbTilingMakerView,VIEW_TILING_MAKER);
    viewerGroup.addButton(cbFigMapView,VIEW_MAP_EDITOR);
    viewerGroup.addButton(cbFaceSetView,VIEW_FACE_SET);
    viewerGroup.button(config->viewerType)->setChecked(true);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(&viewerGroup,   SIGNAL(buttonToggled(int, bool)),  this, SLOT(slot_Viewer_pressed(int,bool)));
#else
    connect(&viewerGroup,   &QButtonGroup::idToggled,      this, &ControlPanel::slot_Viewer_pressed);
#endif
    connect(this, &ControlPanel::sig_view_synch,  this, &ControlPanel::slot_view_synch);

    return workspaceViewersBox;
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

void  ControlPanel::slot_Viewer_pressed(int id, bool enb)
{
    eViewType evt = static_cast<eViewType>(id);
    if (enb)
    {
        config->viewerType = evt;
        delegateKeyboardMouse(evt);
    }

    if (viewerGroup.exclusive())
    {
        viewer->disableAll();
    }
    viewer->viewEnable(evt,enb);

    emit sig_view_synch(id, enb);
    emit sig_viewWS();
}

void ControlPanel::slot_view_synch(int id, int enb)
{
    viewerGroup.blockSignals(true);
    viewerGroup.button(id)->setChecked(enb);
    viewerGroup.blockSignals(false);
}

void ControlPanel::slot_multiSelect(bool enb)
{
    // this is a mirror of the page views control
    // so does not touch the workspace
    viewerGroup.setExclusive(!enb);
    if (!enb)
    {
        viewerGroup.blockSignals(true);
        for (int i=0; i <= VIEW_MAX; i++ )
        {
            if (i == config->viewerType)
            {
                continue;
            }
            viewerGroup.button(i)->setChecked(false);
        }
        viewerGroup.blockSignals(false);
    }
}
