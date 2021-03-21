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
#include "panels/page_decoration_maker.h"
#include "panels/page_layers.h"
#include "panels/page_tiling_maker.h"
#include "panels/page_style_figure_info.h"
#include "panels/page_design_elements.h"
#include "panels/page_prototype_info.h"
#include "panels/page_debug.h"
#include "panels/page_image_tools.h"
#include "panels/page_config.h"
#include "panels/page_log.h"
#include "panels/page_save.h"
#include "panels/page_motif_maker.h"
#include "panels/page_system_info.h"
#include "panels/page_modelSettings.h"
#include "panels/page_loaders.h"
#include "panels/page_map_editor.h"
#include "panels/view_panel.h"
#include "designs/design.h"
#include "base/tiledpatternmaker.h"
#include "base/version.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"


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
    QStringList qsl;
    qsl << "branch" << "--show-current";
    process.start("git",qsl);
    process.waitForFinished(-1); // will wait forever until finished

    QByteArray sout  = process.readAllStandardOutput();
    QString branch(sout);
    QString br = branch.trimmed();
    qDebug().noquote() << QDir::currentPath() <<  "branch =" << br;

#if (QT_VERSION >= QT_VERSION_CHECK(5,9,0))
    setWindowFlag(Qt::WindowMinimizeButtonHint,true);
#endif

#ifdef TPMSPLASH
    splash = new TPMSplash();
#endif

    QString title;
#ifdef QT_DEBUG
    title  = "Control Panel - Debug - ";
    title += tpmVersion;
    if  (!br.isEmpty())
    {
        title += "[";
        title += branch.trimmed();
        title += "]  ";
    }
#else
    title = "Control Panel - Release - ";
    title  += tpmVersion;
#endif
    config  = Configuration::getInstance();
    title  += config->rootMediaDir;
    setWindowTitle(title);

    QSettings s;
    move(s.value("panelPos").toPoint());
}

void ControlPanel::init(TiledPatternMaker * parent)
{
    maker   = parent;
    closed  = false;
    view    = View::getInstance();
    vcontrol = ViewControl::getInstance();

    updateLocked = false;

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
    mpTimer->stop();

    if (!config->splitScreen && !closed)
    {
        QSettings s;
        s.setValue("panelPos", pos());
        closed =  true;
    }

    mPages.clear();
    panelPageList->clear();
    delete panelPages;
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
    for (auto page : qAsConst(mPages))
    {
        page->closePage();
    }
}

void ControlPanel::setupGUI()
{
    // top row
    panelStatus = new PanelStatus();
    panelStatus->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    // hlayout - second row
    QHBoxLayout * hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->setSizeConstraint(QLayout::SetFixedSize);

    radioDefined = new QRadioButton("Full Mosiac");
    radioPack    = new QRadioButton("Packed Mosiac");
    radioSingle  = new QRadioButton("Simple Mosaic");

    QPushButton * pbShowMosaic    = new QPushButton("Show Mosaic");
    QPushButton * pbShowTiling    = new QPushButton("Show Tiling");
    QCheckBox   * chkScaleToView  = new QCheckBox("Scale to View");
    QCheckBox   * showBackImage   = new QCheckBox("Show Background Images");

    if (config->insightMode)
    {
        QPushButton * pbLogEvent      = new QPushButton("Log Event");
        QPushButton * pbUpdateView    = new QPushButton("Update View");
        QPushButton * pbRefreshView   = new QPushButton("Refresh Current View");

        hlayout->addWidget(pbLogEvent);
        hlayout->addStretch();
        hlayout->addWidget(pbRefreshView  );
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

        connect(pbUpdateView,       &QPushButton::clicked,  this,     &ControlPanel::updateView);
        connect(pbLogEvent,         &QPushButton::clicked,  this,     &ControlPanel::slot_logEvent);
        connect(pbRefreshView  ,    &QPushButton::clicked,  vcontrol, &ViewControl::slot_refreshView);
    }
    else
    {
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

    // hbox - third row
    kbdModeCombo = new QComboBox();
    kbdModeCombo->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    kbdModeCombo->setFixedHeight(29);
    delegateKeyboardMouse(config->getViewerType());

    ViewPanel * vpanel = new ViewPanel;
    vpanel->setButtonSize(QSize(79,29));
    vpanel->setContentsMargins(0,0,0,0);

    QPushButton *pbRaise    = new QPushButton("Raise Picture to Top");
    pbRaise->setFixedHeight(29);
    pbRaise->setStyleSheet("QPushButton{ background-color: white; border: 1px solid black; border-radius: 3px; padding-left: 9px; padding-right: 9px; }");

    QPushButton *pbExit     = new QPushButton("QUIT");
    pbExit->setFixedSize(QSize(79,29));
    pbExit->setStyleSheet("QPushButton{ background-color: white; border: 1px solid black; border-radius: 3px; }");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(pbRaise);
    hbox->addStretch();

    if (config->insightMode)
    {
        cbUpdate    = new QCheckBox("Update Pages");
        cbUpdate->setChecked(config->updatePanel);

        hbox->addWidget(cbUpdate);
        hbox->addWidget(kbdModeCombo);
        hbox->addStretch();
        hbox->addWidget(vpanel);

        connect(cbUpdate, SIGNAL(clicked(bool)), this, SLOT(updateClicked(bool)));
    }
    else
    {
        hbox->addWidget(kbdModeCombo);
        hbox->addStretch();
        hbox->addWidget(vpanel);
    }

    hbox->addStretch();
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
    panelPageList                 = new PanelListWidget;
    QHBoxLayout * panelPageLayout = new QHBoxLayout();
    panelPageLayout->addWidget(panelPageList, Qt::AlignTop);
    QGroupBox * menuSelectGroup   = new QGroupBox("Menu select");
    menuSelectGroup->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    menuSelectGroup->setLayout(panelPageLayout);

    QGroupBox * viewersGroup = createViewersBox();

    QVBoxLayout * leftBox = new QVBoxLayout;
    leftBox->addSpacing(7);
    leftBox->addWidget(menuSelectGroup);
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
    showBackImage->setChecked(config->showBackgroundImage);

    // connections
    connect(pbExit,             &QPushButton::clicked,              this,     &ControlPanel::slot_exit);
    connect(pbRaise,            &QPushButton::clicked,              this,     &ControlPanel::slot_raise);
    connect(pbShowTiling,       &QPushButton::pressed,              this,     &ControlPanel::showTilingPressed);
    connect(pbShowMosaic,       &QPushButton::pressed,              this,     &ControlPanel::showMosaicPressed);
    connect(&repeatRadioGroup,  &QButtonGroup::idClicked,           this,     &ControlPanel::repeatChanged);
    connect(this,               &ControlPanel::sig_refreshView,     vcontrol, &ViewControl::slot_refreshView);
    connect(this,               &ControlPanel::sig_render,          theApp,   &TiledPatternMaker::slot_render);
    connect(kbdModeCombo,       SIGNAL(currentIndexChanged(int)),   this,     SLOT(slot_kbdModeChanged(int)));
    connect(view,               &View::sig_kbdMode,                 this,     &ControlPanel::slot_kbdMode);
    connect(chkScaleToView,     &QCheckBox::clicked,                this,     &ControlPanel::slot_scaleToView);
    connect(showBackImage,      &QCheckBox::clicked,                this,     &ControlPanel::slot_showBackChanged);

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

    wp = new page_tiling_maker(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    wp = new page_motif_maker(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    wp = new page_decoration_maker(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    wp = new page_modelSettings(this);
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

        panelPageList->addSeparator();

        wp = new page_layers(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());

        wp = new page_design_elements(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());

        wp = new page_prototype_info(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());

        wp = new page_style_figure_info(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());

        wp = new page_system_info(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());

        // make inter-page connections
        connect(this,   &ControlPanel::sig_reload,               wp_med, &page_map_editor::slot_reload);
    }

    panelPageList->addSeparator();

    wp = new page_config(this);
    mPages.push_back(wp);
    panelPages->addWidget(wp);
    panelPageList->addItem(wp->getName());

    if (config->insightMode)
    {
        wp = new page_debug(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());

        wp = new page_image_tools(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());
        page_image_tools * wp_im = dynamic_cast<page_image_tools*>(wp);

        wp = new page_log(this);
        mPages.push_back(wp);
        panelPages->addWidget(wp);
        panelPageList->addItem(wp->getName());

        connect(maker, &TiledPatternMaker::sig_image0, wp_im, &page_image_tools::slot_setImage0);
        connect(maker, &TiledPatternMaker::sig_image1, wp_im, &page_image_tools::slot_setImage1);
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
    qDebug() << "ControlPanel::slot_selectPanelPage:" << index;
    if (index == -1)  return;

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
    updateLocked = false;
}

// called by mpTimer
void ControlPanel::slot_poll()
{
    static volatile bool updateLocked = false;

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
        if (isVisiblePage(page))
        {
            refreshPage(page);
        }
    }

    updateGeometry();
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

    wp->refreshPage();
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
    view->dump(true);
}

void ControlPanel::slot_raise()
{
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

    page_decoration_maker * psm = dynamic_cast<page_decoration_maker*>(currentPage);
    if (psm)
    {
        selectViewer(VIEW_MOSAIC);
        return;
    }

    page_modelSettings * pcs = dynamic_cast<page_modelSettings*>(currentPage);
    if (pcs)
    {
        //selectViewer(VIEW_MOSAIC);
        return;
    }

    if (config->insightMode)
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
    }
    else
    {
        kbdModeCombo->insertItem(100,"Adjust View",      QVariant(KBD_MODE_XFORM_VIEW));
        kbdModeCombo->insertItem(100,"Adjust Background",QVariant(KBD_MODE_XFORM_BKGD));
        kbdModeCombo->insertItem(100,"Adjust Tiling",    QVariant(KBD_MODE_XFORM_TILING));
        kbdModeCombo->insertItem(100,"Adjust Unique Feature", QVariant(KBD_MODE_XFORM_UNIQUE_FEATURE));
        kbdModeCombo->insertItem(100,"Adjust Placed Feature", QVariant(KBD_MODE_XFORM_PLACED_FEATURE));
        kbdModeCombo->insertItem(100,"Set Center",       QVariant(KBD_MODE_CENTER));
    }

    view->setKbdMode(config->kbdMode); // converts if necessry

    kbdModeCombo->blockSignals(false);

    slot_kbdMode(config->kbdMode);  // selects
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
    case KBD_MODE_XFORM_UNIQUE_FEATURE:
    case KBD_MODE_XFORM_PLACED_FEATURE:
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
    emit sig_refreshView();
}


QGroupBox *  ControlPanel::createViewersBox()
{
    QGroupBox * viewersBox  = new QGroupBox("View Select");

    cbMosaicView         = new QCheckBox("Mosaic");
    cbPrototypeView      = new QCheckBox("Prototype");
    cbDELView            = new QCheckBox("Design Elements");
    cbMapEditor          = new QCheckBox("Map Editor");
    cbProtoMaker         = new QCheckBox("Motif Maker");
    cbTilingView         = new QCheckBox("Tiling");
    cbTilingMakerView    = new QCheckBox("Tiling Maker");
    cbFaceSetView        = new QCheckBox("FaceSet");
    cbRawDesignView      = new QCheckBox("Fixed Design");

    cbMultiSelect        = new QCheckBox("Multi View");    // defaults to off - not persisted
    cbLockView           = new QCheckBox("Lock View");
    cbLockView->setChecked(config->lockView);

    AQVBoxLayout * avbox = new AQVBoxLayout();

    avbox->addWidget(cbMosaicView);
    avbox->addWidget(cbPrototypeView);
    avbox->addWidget(cbDELView);
    if (config->insightMode)
    {
        avbox->addWidget(cbMapEditor);
    }
    avbox->addWidget(cbProtoMaker);
    avbox->addWidget(cbTilingView);
    avbox->addWidget(cbTilingMakerView);
    avbox->addWidget(cbFaceSetView);
    avbox->addWidget(cbRawDesignView);
    avbox->addSpacing(13);
    avbox->addWidget(cbLockView);
    avbox->addWidget(cbMultiSelect);

    viewersBox->setLayout(avbox);

    // viewer group
    viewerGroup.addButton(cbRawDesignView,VIEW_DESIGN);
    viewerGroup.addButton(cbMosaicView,VIEW_MOSAIC);
    viewerGroup.addButton(cbPrototypeView,VIEW_PROTOTYPE);
    viewerGroup.addButton(cbTilingView,VIEW_TILING);
    viewerGroup.addButton(cbProtoMaker,VIEW_MOTIF_MAKER);
    viewerGroup.addButton(cbDELView,VIEW_DESIGN_ELEMENT);
    viewerGroup.addButton(cbTilingMakerView,VIEW_TILING_MAKER);
    if (config->insightMode)
    {
        viewerGroup.addButton(cbMapEditor,VIEW_MAP_EDITOR);
    }
    viewerGroup.addButton(cbFaceSetView,VIEW_FACE_SET);
    viewerGroup.button(config->getViewerType())->setChecked(true);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(&viewerGroup,   SIGNAL(buttonToggled(int, bool)),  this, SLOT(slot_Viewer_pressed(int,bool)));
#else
    connect(&viewerGroup,   &QButtonGroup::idToggled,      this, &ControlPanel::slot_Viewer_pressed);
#endif
    connect(this, &ControlPanel::sig_view_synch,  this, &ControlPanel::slot_view_synch);
    connect(cbLockView,     &QCheckBox::clicked,  this, &ControlPanel::slot_lockViewClicked);
    connect(cbMultiSelect,  &QCheckBox::clicked,  this, &ControlPanel::slot_multiSelect);
    connect(theApp,         &TiledPatternMaker::sig_lockStatus, this,&ControlPanel::slot_lockStatusChanged);

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

void  ControlPanel::slot_Viewer_pressed(int id, bool enable)
{
    eViewType viewType = static_cast<eViewType>(id);
    if (enable)
    {
        // allways do this - the user has asked for it
        config->setViewerType(viewType);
        delegateKeyboardMouse(viewType);
    }

    if (viewerGroup.exclusive())
    {
        vcontrol->disableAllViews();
    }
    vcontrol->viewEnable(viewType,enable);

    emit sig_view_synch(id, enable);

    emit sig_refreshView();
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
    viewerGroup.setExclusive(!enb);
    if (!enb)
    {
        viewerGroup.blockSignals(true);
        for (int i=0; i <= VIEW_MAX; i++ )
        {
            if (i == config->getViewerType())
            {
                continue;
            }
            viewerGroup.button(i)->setChecked(false);
        }
        viewerGroup.blockSignals(false);
    }
    vcontrol->disableAllViews();
    vcontrol->viewEnable(config->getViewerType(),true);
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
