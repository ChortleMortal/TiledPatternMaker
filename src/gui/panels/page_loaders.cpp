#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QMultiMap>
#include <QString>

#include "gui/panels/page_loaders.h"
#include "gui/panels/panel_misc.h"
#include "gui/panels/panel_worker.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "gui/widgets/dlg_listnameselect.h"
#include "gui/widgets/dlg_rebase.h"
#include "gui/widgets/dlg_rename.h"
#include "gui/widgets/flash_msgbox.h"
#include "gui/widgets/layout_sliderset.h"
#include "legacy/design.h"
#include "legacy/design_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/settings/configuration.h"
#include "sys/engine/mosaic_bmp_generator.h"
#include "sys/sys/fileservices.h"
#include "sys/sys/load_unit.h"
#include "sys/sys/pugixml.hpp"
#include "sys/tiledpatternmaker.h"

#define LOADER_MAX_HEIGHT   580

using namespace pugi;

Q_DECLARE_METATYPE(eTILM_Event)

page_loaders::page_loaders(ControlPanel * apanel) : panel_page(apanel,PAGE_LOAD,"Load")
{
    selectedDesign = NO_DESIGN;

    QVBoxLayout * vb = new QVBoxLayout;
    vb->addWidget(createGeneralColumn());
    vb->addWidget(createLegacyColumn());

    QHBoxLayout * loaderLayout = new QHBoxLayout;
    loaderLayout->addLayout(vb);
    loaderLayout->addWidget(createMosaicColumn());
    loaderLayout->addWidget(createTilingColumn());
    loaderLayout->addStretch();

    vbox->addLayout(loaderLayout);
    vbox->addStretch();

    cbAutoLoadLast->setChecked(config->autoLoadLast);
    cbShowWithImages->setChecked(config->showWithBkgds);

    mosaicFilter->setText(config->mosaicFilter);
    mosaicFilterCheck->setChecked(config->mosaicFilterCheck);
    mosaicOrigChk->setChecked(config->mosaicOrigCheck);
    mosaicNewChk->setChecked(config->mosaicNewCheck);
    mosaicTestChk->setChecked(config->mosaicTestCheck);
    if (config->insightMode)
        mosaicWorklistCheck->setChecked(config->mosaicWorklistCheck);

    cbMosaicSortChk->setChecked(config->mosaicSortCheck);

    tilingFilter->setText(config->tileFilter);
    tilingFilterCheck->setChecked(config->tileFilterCheck);
    tilingOrigChk->setChecked(config->tilingOrigCheck);
    tilingNewChk->setChecked(config->tilingNewCheck);
    tilingTestChk->setChecked(config->tilingTestCheck);
    if (config->insightMode)
        tilingWorklistCheck->setChecked(config->tilingWorklistCheck);

    // make connections before making selections
    makeConnections();

    // DAC designs
    loadDesignCombo();

    // Tiling
    loadTilingsCombo();

    // Mosaic
    loadMosaicsCombo();

    emit sig_setTilingUses();
}

page_loaders::~page_loaders()
{
    Q_ASSERT(thread);
    if (thread->isRunning())
    {
        thread->quit();
        thread->wait();

        thread->terminate();
        thread->wait();

        delete thread;
    }
}

QGroupBox * page_loaders::createGeneralColumn()
{
    cbAutoLoadLast   = new QCheckBox("Load last on startup");
    cbShowWithImages = new QCheckBox("Show with background images");
    cbMosaicSortChk  = new QCheckBox("Sort all by date written");

    QVBoxLayout * vb = new QVBoxLayout();
    vb->addWidget(cbAutoLoadLast);
    vb->addWidget(cbShowWithImages);
    vb->addWidget(cbMosaicSortChk);

    QGroupBox * gb = new QGroupBox();
    gb->setLayout(vb);

    return gb;
}

// design patterns (shapes)
QGroupBox * page_loaders::createLegacyColumn()
{
    pbLoadShapes = new QPushButton("Load Fixed Design");
    pbLoadShapes->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addStretch();
    hbox->addWidget(pbLoadShapes);

    // second row
    kbdModeCombo = new QComboBox();
    kbdModeCombo->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Ignored);
    kbdModeCombo->setFixedHeight(25);
    setupKbdModeCombo();

    QHBoxLayout * hbox2 = new QHBoxLayout();
    hbox2->addWidget(kbdModeCombo);
    hbox2->addStretch();

    // list widget
    designListWidget = new LoaderListWidget();
    designListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // column layout
    QVBoxLayout * designLayout = new QVBoxLayout;
    designLayout->addLayout(hbox);
    designLayout->addLayout(hbox2);
    designLayout->addWidget(designListWidget);

    QGroupBox * designGroup = new QGroupBox();
    designGroup->setLayout(designLayout);

    connect(kbdModeCombo,  &QComboBox::currentIndexChanged, this, &page_loaders::slot_kbdModeChanged);
    connect(designMaker,   &DesignMaker::sig_LegacyKbdMode, this, &page_loaders::slot_kbdMode);

    return designGroup;
}

// Mosaics
QGroupBox * page_loaders::createMosaicColumn()
{
    // first row
    pbLoadXML = new QPushButton("Load Mosaic");
    pbLoadXML->setFixedWidth(119);
    pbLoadXML->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    if (config->insightMode)
        mosaicWorklistXCheck = new QCheckBox("Exclude WList");

    QHBoxLayout * hbox1 = new QHBoxLayout();
    if (config->insightMode)
        hbox1->addWidget(mosaicWorklistXCheck);
    hbox1->addStretch();
    hbox1->addWidget(pbLoadXML);

    // second row
    mosaicOrigChk = new QCheckBox("Original");
    mosaicNewChk  = new QCheckBox("New");
    mosaicTestChk = new QCheckBox("Test");

    AQHBoxLayout * hbox2 = new AQHBoxLayout();
    hbox2->addWidget(mosaicOrigChk);
    hbox2->addWidget(mosaicNewChk);
    hbox2->addWidget(mosaicTestChk);
    hbox2->addStretch();

    // third row
    if (config->insightMode)
        mosaicWorklistCheck = new QCheckBox("Worklist");

    mosaicFilter = new QLineEdit();
    mosaicFilter->setFixedWidth(81);
    mosaicFilterCheck = new QCheckBox("Filter");

    QHBoxLayout * hbox3 = new QHBoxLayout();
    if (config->insightMode)
        hbox3->addWidget(mosaicWorklistCheck);
    hbox3->addStretch();
    hbox3->addWidget(mosaicFilterCheck);
    hbox3->addWidget(mosaicFilter);

    // list widget
    mosaicListWidget = new VersionedListWidget();
    mosaicListWidget->setFixedHeight(LOADER_MAX_HEIGHT);
    mosaicListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    QVBoxLayout * mosaicLayout = new QVBoxLayout;
    mosaicLayout->addLayout(hbox1);
    mosaicLayout->addLayout(hbox3);
    mosaicLayout->addLayout(hbox2);
    mosaicLayout->addStretch();
    mosaicLayout->addWidget(mosaicListWidget);

    QGroupBox * mosaicGroup = new QGroupBox();
    mosaicGroup->setLayout(mosaicLayout);

    return mosaicGroup;
}

// tiling
QGroupBox * page_loaders::createTilingColumn()
{
    // first row
    pbLoadTiling = new QPushButton("Load Tiling");
    pbLoadTiling->setFixedWidth(119);
    pbLoadTiling->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    if (config->insightMode)
        tilingWorklistXCheck = new QCheckBox("Exclude WList");

    AQHBoxLayout * hbox = new AQHBoxLayout();
    if (config->insightMode)
        hbox->addWidget(tilingWorklistXCheck);
    hbox->addStretch();
    hbox->addWidget(pbLoadTiling);

    // second row
    tilingOrigChk = new QCheckBox("Original");
    tilingNewChk  = new QCheckBox("New");
    tilingTestChk = new QCheckBox("Test");

    QHBoxLayout * hbox2 = new QHBoxLayout();
    hbox2->addWidget(tilingOrigChk);
    hbox2->addWidget(tilingNewChk);
    hbox2->addWidget(tilingTestChk);
    hbox2->addStretch();

    // third row
    if (config->insightMode)
        tilingWorklistCheck = new QCheckBox("Worklist");
    tilingFilterCheck = new QCheckBox("Filter");
    tilingFilter = new QLineEdit();
    tilingFilter->setFixedWidth(81);

    QHBoxLayout * hbox3 = new QHBoxLayout();
    if(config->insightMode)
        hbox3->addWidget(tilingWorklistCheck);
    hbox3->addStretch();
    hbox3->addWidget(tilingFilterCheck);
    hbox3->addWidget(tilingFilter);

    // fourth row
    pbTilingLoadMulti   = new QPushButton(" Load Additonal ");
    pbTilingLoadReplace  = new QPushButton(" Load Replace ");

    QHBoxLayout * hbox4 = new QHBoxLayout();
    hbox4->addWidget(pbTilingLoadReplace);
    hbox4->addStretch();
    hbox4->addWidget(pbTilingLoadMulti);

    // list widget
    tileListWidget = new VersionedListWidget();
    tileListWidget->setFixedHeight(LOADER_MAX_HEIGHT);
    tileListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    QVBoxLayout * tilingLayout = new QVBoxLayout;
    tilingLayout->addLayout(hbox);
    tilingLayout->addLayout(hbox3);
    tilingLayout->addLayout(hbox2);
    tilingLayout->addLayout(hbox4);
    tilingLayout->addWidget(tileListWidget);

    QGroupBox * tilingGroup = new QGroupBox();
    tilingGroup->setLayout(tilingLayout);

    return tilingGroup;
}

void page_loaders::setupKbdModeCombo()
{
    kbdModeCombo->blockSignals(true);
    kbdModeCombo->clear();

    kbdModeCombo->insertItem(100,"Mode Position",   QVariant(LEGACY_MODE_DES_POS));
    kbdModeCombo->insertItem(100,"Mode Layer",      QVariant(LEGACY_MODE_DES_LAYER_SELECT));
    kbdModeCombo->insertItem(100,"Mode Z-level",    QVariant(LEGACY_MODE_DES_ZLEVEL));
    kbdModeCombo->insertItem(100,"Mode Step",       QVariant(LEGACY_MODE_DES_STEP));
    kbdModeCombo->insertItem(100,"Mode Separation", QVariant(LEGACY_MODE_MODE_DES_SEPARATION));
    kbdModeCombo->insertItem(100,"Mode Origin",     QVariant(LEGACY_MODE_DES_ORIGIN));
    kbdModeCombo->insertItem(100,"Mode Offset",     QVariant(LEGACY_MODE_DES_OFFSET));

    kbdModeCombo->blockSignals(false);

    designMaker->resetLegacyKbdMode();
}

void page_loaders::makeConnections()
{
    thread               = new QThread();
    PanelWorker * worker = new PanelWorker();
    worker->moveToThread(thread);

    connect(this,               &page_loaders::sig_sortTilings,     worker, &PanelWorker::sortTilings);
    connect(this,               &page_loaders::sig_sortMosaics,     worker, &PanelWorker::sortMosaics);
    connect(this,               &page_loaders::sig_setTilingUses,   worker, &PanelWorker::getTilingUses);
    connect(worker,             &PanelWorker::tilingsSorted,        this,   &page_loaders::refillTilingsCombo);
    connect(worker,             &PanelWorker::mosaicsSorted,        this,   &page_loaders::refillMosaicsCombo);

    thread->start();

    connect(mosaicMaker,        &MosaicMaker::sig_mosaicWritten,    this,   &page_loaders::slot_newMosaic);
    connect(tilingMaker,        &TilingMaker::sig_tilingWritten,    this,   &page_loaders::slot_newTiling);
    connect(mosaicMaker,        &MosaicMaker::sig_mosaicLoaded,     this,   &page_loaders::slot_mosaicLoaded);
    connect(tilingMaker,        &TilingMaker::sig_tilingLoaded,     this,   &page_loaders::slot_tilingLoaded);
    connect(designMaker,        &DesignMaker::sig_loadedDesign,     this,   &page_loaders::slot_loadedDesign);

    connect(theApp,         &TiledPatternMaker::sig_workListChanged,this,   &page_loaders::loadMosaicsCombo);

    connect(pbLoadXML,          &QPushButton::clicked,              this,   &page_loaders::loadMosaic);
    connect(pbLoadTiling,       &QPushButton::clicked,              this,   &page_loaders::slot_loadTiling);
    connect(pbTilingLoadReplace,&QPushButton::clicked,              this,   &page_loaders::slot_loadTilingReplace);
    connect(pbTilingLoadMulti,  &QPushButton::clicked,              this,   &page_loaders::slot_loadTilingMulti);
    connect(pbLoadShapes,       &QPushButton::clicked,              this,   &page_loaders::loadShapes);
    connect(mosaicFilter,       &QLineEdit::textEdited,             this,   &page_loaders::slot_mosaicFilter);
    connect(tilingFilter,       &QLineEdit::textEdited,             this,   &page_loaders::slot_tilingFilter);
    connect(mosaicFilterCheck,  &QCheckBox::clicked,                this,   &page_loaders::slot_mosaicCheck);
    if (config->insightMode)
    {
        connect(mosaicWorklistCheck,&QCheckBox::clicked,                this,   &page_loaders::slot_mosaicWorklistCheck);
        connect(tilingWorklistCheck,&QCheckBox::clicked,                this,   &page_loaders::slot_tilingWorklistCheck);
        connect(mosaicWorklistXCheck,&QCheckBox::clicked,               this,   &page_loaders::slot_mosaicWorklistXCheck);
        connect(tilingWorklistXCheck,&QCheckBox::clicked,               this,   &page_loaders::slot_tilingWorklistXCheck);
    }
    connect(mosaicOrigChk,      &QCheckBox::clicked,                this,   &page_loaders::slot_mosOrigCheck);
    connect(mosaicNewChk,       &QCheckBox::clicked,                this,   &page_loaders::slot_mosNewCheck);
    connect(mosaicTestChk,      &QCheckBox::clicked,                this,   &page_loaders::slot_mosTestCheck);
    connect(cbMosaicSortChk,    &QCheckBox::clicked,                this,   &page_loaders::slot_mosSortCheck);
    connect(cbShowWithImages,   &QCheckBox::clicked,                this,   &page_loaders::slot_showWithBkgds);
    connect(tilingOrigChk,      &QCheckBox::clicked,                this,   &page_loaders::slot_tilingOrigCheck);
    connect(tilingNewChk,       &QCheckBox::clicked,                this,   &page_loaders::slot_tilingNewCheck);
    connect(tilingTestChk,      &QCheckBox::clicked,                this,   &page_loaders::slot_tilingTestCheck);
    connect(tilingFilterCheck,  &QCheckBox::clicked,                this,   &page_loaders::slot_tilingCheck);

    connect(this,               &page_loaders::sig_loadDesign,      designMaker, &DesignMaker::slot_loadDesign);
    connect(this,               &page_loaders::sig_buildDesign,     designMaker, &DesignMaker::slot_buildDesign);

    connect(designListWidget,   &QListWidget::itemClicked,          this,   &page_loaders::designClicked);
    connect(designListWidget,   &LoaderListWidget::rightClick,      this,   &page_loaders::desRightClick);

    connect(mosaicListWidget,   &QListWidget::itemClicked,          this,   &page_loaders::mosaicClicked);
    connect(mosaicListWidget,   &QListWidget::itemActivated,        this,   &page_loaders::slot_mosaicActivated);
    connect(mosaicListWidget,   &QListWidget::currentTextChanged,   this,   &page_loaders::slot_mosaicTextChanged);
    connect(mosaicListWidget,   &LoaderListWidget::itemEntered,     this,   &page_loaders::slot_mosaicItemEnteredToolTip);
    connect(mosaicListWidget,   &LoaderListWidget::rightClick,      this,   &page_loaders::xmlRightClick);
    connect(mosaicListWidget,   &LoaderListWidget::leftDoubleClick, this,   &page_loaders::loadMosaic);
    connect(mosaicListWidget,   &LoaderListWidget::listEnter,       this,   &page_loaders::loadMosaic);

    connect(tileListWidget,     &LoaderListWidget::leftDoubleClick, this,   &page_loaders::slot_loadTiling);
    connect(tileListWidget,     &LoaderListWidget::rightClick,      this,   &page_loaders::tileRightClick);
    connect(tileListWidget,     &QListWidget::currentItemChanged,   this,   &page_loaders::tilingSelected);
    connect(tileListWidget,     &QListWidget::itemClicked,          this,   &page_loaders::tilingClicked);
    connect(tileListWidget,     &QListWidget::itemActivated,        this,   &page_loaders::slot_tilingActivated);
    connect(tileListWidget,     &QListWidget::currentTextChanged,   this,   &page_loaders::slot_tilingTextChanged);

    connect(cbAutoLoadLast,     &QCheckBox::clicked,                this,   &page_loaders::autoLoadLastClicked);

    Q_ASSERT(thread->isRunning());
}

// kbdModeCombo selection
void page_loaders::slot_kbdModeChanged(int row)
{
    Q_UNUSED(row)
    qDebug() << "slot_kbdModeChanged to:"  << kbdModeCombo->currentText();
    QVariant var = kbdModeCombo->currentData();
    eLegacyMode mode = static_cast<eLegacyMode>(var.toInt());
    designMaker->setLegacyKbdMode(mode);
}

// from canvas setKbdMode
void page_loaders::slot_kbdMode(eLegacyMode mode)
{
    QVariant var(mode);
    int index = kbdModeCombo->findData(var);
    kbdModeCombo->blockSignals(true);
    kbdModeCombo->setCurrentIndex(index);
    kbdModeCombo->blockSignals(false);
}
void page_loaders::slot_mosaicFilter(const QString & filter)
{
    config->mosaicFilter = filter;
    if (config->mosaicFilterCheck)
    {
        loadMosaicsCombo();
    }
}

void page_loaders::slot_tilingFilter(const QString & filter)
{
    config->tileFilter = filter;
    if (config->tileFilterCheck)
    {
        loadTilingsCombo();
    }
}

void page_loaders::slot_mosaicCheck(bool check)
{
    config->mosaicFilter      = mosaicFilter->text();
    config->mosaicFilterCheck = check;
    loadMosaicsCombo();
}

void page_loaders::slot_mosaicWorklistCheck(bool check)
{
    config->mosaicWorklistCheck = check;
    if (check)
    {
        mosaicWorklistXCheck->setChecked(false);
        config->mosaicWorklistXCheck = false;
    }
    loadMosaicsCombo();
}

void page_loaders::slot_tilingWorklistCheck(bool check)
{
    config->tilingWorklistCheck = check;
    if (check)
    {
        tilingWorklistXCheck->setChecked(false);
        config->tilingWorklistXCheck = false;
    }
    loadTilingsCombo();
}

void page_loaders::slot_mosaicWorklistXCheck(bool check)
{
    config->mosaicWorklistXCheck = check;
    if (check)
    {
        mosaicWorklistCheck->setChecked(false);
        config->mosaicWorklistCheck = false;
    }
    loadMosaicsCombo();
}

void page_loaders::slot_tilingWorklistXCheck(bool check)
{
    config->tilingWorklistXCheck = check;
    if (check)
    {
        tilingWorklistCheck->setChecked(false);
        config->tilingWorklistCheck = false;
    }
    loadTilingsCombo();
}

void page_loaders::slot_mosOrigCheck(bool check)
{
    config->mosaicOrigCheck = check;
    loadMosaicsCombo();
}

void page_loaders::slot_mosNewCheck(bool check)
{
    config->mosaicNewCheck = check;
    loadMosaicsCombo();
}

void page_loaders::slot_mosTestCheck(bool check)
{
    config->mosaicTestCheck = check;
    loadMosaicsCombo();
}

void page_loaders::slot_mosSortCheck(bool check)
{
    config->mosaicSortCheck = check;
    // unusually do both
    loadTilingsCombo();
    loadMosaicsCombo();
}

void page_loaders::slot_showWithBkgds(bool check)
{
    config->showWithBkgds = check;
    // unusually do both
    loadTilingsCombo();
    loadMosaicsCombo();
}

void page_loaders::slot_tilingOrigCheck(bool check)
{
    config->tilingOrigCheck = check;
    loadTilingsCombo();
}

void page_loaders::slot_tilingNewCheck(bool check)
{
    config->tilingNewCheck = check;
    loadTilingsCombo();
}

void page_loaders::slot_tilingTestCheck(bool check)
{
    config->tilingTestCheck = check;
    loadTilingsCombo();
}

void page_loaders::slot_tilingCheck(bool check)
{
    config->tileFilter      = tilingFilter->text();
    config->tileFilterCheck = check;
    loadTilingsCombo();
}

void page_loaders::slot_newTiling()
{
    loadTilingsCombo();
    emit sig_setTilingUses();
}

void page_loaders::slot_newMosaic()
{
    loadMosaicsCombo();
    emit sig_setTilingUses();
}

void page_loaders::loadMosaic()
{
    if (selectedMosaicFile.isEmpty())
    {
        return;
    }

    MosaicPtr mosaic = mosaicMaker->loadMosaic(selectedMosaicFile);

    if (!config->lockView)
    {
       panel->delegateView(VIEW_MOSAIC,true);
    }

#ifdef LEGACY_CONVERT_XML
    QVector<TilingPtr> tilings = mosaic->getTilings();
    for (auto & tiling : tilings)
    {

        if (tiling->legacyModelConverted())
        {
            // ask
            QMessageBox box(panel);
            box.setIcon(QMessageBox::Warning);
            box.setWindowTitle("Save Tiling ?");
            box.setText("Overwrite converted tiling XML file?");
            box.setInformativeText("Tiling XML file has legacy code which should be removed");
            box.setStandardButtons(QMessageBox::Ok |QMessageBox::Ignore);
            box.setDefaultButton(QMessageBox::Ok);
            int rv = box.exec();
            if (rv == QMessageBox::Ok)
            {
                // overwrite the file
                tilingMaker->saveTiling(tiling,true);
            }
        }
    }
    if (mosaic->legacyModelConverted())
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle("Save Mosaic ?");
        box.setText("Overwrite converted mosaic XML file?");
        box.setInformativeText("Mosaic XML file has legacy code which should be removed");
        box.setStandardButtons(QMessageBox::Ok |QMessageBox::Ignore);
        box.setDefaultButton(QMessageBox::Ok);
        int rv = box.exec();
        if (rv == QMessageBox::Ok)
        {
            // ask
            mosaicMaker->saveMosaic(mosaic,true);   // force overwrite
        }
    }
#endif
}

void page_loaders::loadTiling(eTILM_Event event)
{
    if (selectedTilingFile.isEmpty())
    {
        return;
    }

    if (event != TILM_RELOAD && !viewControl->isEnabled(VIEW_TILING_MAKER))
    {
        // delegate the view
        panel->delegateView(VIEW_TILING,true);
    }

    TilingPtr tiling = tilingMaker->loadTiling(selectedTilingFile,event);

#ifdef LEGACY_CONVERT_XML
    if (tiling->legacyModelConverted())
    {
        // ask
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle("Save Tiling ?");
        box.setText("Overwrite converted tiling file?");
        box.setInformativeText("Tiling file has legacy code which should be removed");
        box.setStandardButtons(QMessageBox::Ok |QMessageBox::Ignore);
        box.setDefaultButton(QMessageBox::Ok);
        int rv = box.exec();
        if (rv == QMessageBox::Ok)
        {
            // overwrite the file
            tilingMaker->saveTiling(tiling,true);
        }
    }
#endif
}

void page_loaders::loadTiling2()
{
    loadTiling(TILM_LOAD_SINGLE);
}

void page_loaders::loadShapes()
{
    emit sig_buildDesign(selectedDesign);

    if (!config->lockView)
    {
       panel->delegateView(VIEW_LEGACY,true);
    }
    emit sig_reconstructView();
}

void page_loaders::slot_loadTiling()
{
    loadTiling(TILM_LOAD_SINGLE);
}

void page_loaders::slot_loadTilingReplace()
{
    loadTiling(TILM_LOAD_REPLACE);
}

void page_loaders::slot_loadTilingMulti()
{
    loadTiling(TILM_LOAD_MULTI);
}

void page_loaders::slot_mosaicLoaded(VersionedFile file)
{
    VersionedName name = file.getVersionedName();
    qDebug() << "page_loaders:: loaded XML:" << name.get();

    mosaicListWidget->blockSignals(true);
    if (mosaicListWidget->selectItemByName(name.get()))
    {
        selectedMosaicFile = file;
    }
    else
    {
        selectedMosaicFile.clear();
    }
    mosaicListWidget->blockSignals(false);

    if (viewControl->enabledGangCount() == 0)
    {
        config->lockView = false;
        panel->delegateView(VIEW_MOSAIC,true);
    }

    // update the tiling too
    VersionedFile tileFile = FileServices::getTileFileFromMosaicFile(file);
    slot_tilingLoaded(tileFile);
}

void page_loaders::slot_tilingLoaded(VersionedFile file)
{
    VersionedName name = file.getVersionedName();

    qDebug() << "page_loaders:: loaded Tiling:" << name.get();
    tileListWidget->blockSignals(true);
    if (tileListWidget->selectItemByName(name.get()))
    {
        selectedTilingFile = file;
    }
    tileListWidget->blockSignals(false);

    if (viewControl->enabledGangCount() == 0)
    {
        config->lockView = false;
        panel->delegateView(VIEW_TILING,true);
    }
}

void page_loaders::slot_loadedDesign(eDesign design)
{
    qDebug() << "page_loaders:: loaded Design:" << design;
    designListWidget->blockSignals(true);
    if (designListWidget->selectItemByValue(design))
    {
        selectedDesign =  design;
    }
    designListWidget->blockSignals(false);

    if (viewControl->enabledGangCount() == 0)
    {
        config->lockView = false;
        panel->delegateView(VIEW_LEGACY,true);
    }
}

void page_loaders::onEnter()
{
    QListWidgetItem * item;

    item = tileListWidget->currentItem();
    tileListWidget->scrollToItem(item);

    item = mosaicListWidget->currentItem();
    mosaicListWidget->scrollToItem(item);

    item = designListWidget->currentItem();
    designListWidget->scrollToItem(item);
}

void page_loaders::onRefresh()
{}

void page_loaders::loadDesignCombo()
{
    // Legacy (pre-taprats) DAC designs
    designListWidget->blockSignals(true);

    QListWidgetItem * currentItem = nullptr;
    auto designSet = designMaker->getAvailDesigns();
    QMap<eDesign, DesignPtr>::const_iterator it =  designSet.constBegin();
    while (it != designSet.constEnd())
    {
        eDesign id   = it.key();
        DesignPtr dp = it.value();
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(dp->getTitle());
        item->setData(Qt::UserRole,id);
        designListWidget->addItem(item);
        eDesign  des = (eDesign)designs.key(config->lastLoadedLegacy.get());
        if (des == id)
        {
            currentItem = item;
        }
        it++;
    }

    if (currentItem)
    {
        designListWidget->setCurrentItem(currentItem);
        selectedDesign = static_cast<eDesign>(currentItem->data(Qt::UserRole).toInt());
    }

    designListWidget->blockSignals(false);
}

void page_loaders::loadMosaicsCombo()
{
    emit sig_sortMosaics();
}

void page_loaders::refillMosaicsCombo()
{
    mosaicListWidget->blockSignals(true);
    mosaicListWidget->addItemList(Sys::mosaicsList);
    mosaicListWidget->blockSignals(false);

    if (mosaicListWidget->selectItemByName(config->lastLoadedMosaic.get()))
    {
        selectedMosaicFile = FileServices::getFile(config->lastLoadedMosaic,FILE_MOSAIC);
    }
    else
    {
        selectedMosaicFile.clear();
        config->lastLoadedMosaic.clear();
    }
}

void page_loaders::loadTilingsCombo()
{
    emit sig_sortTilings();
}

void page_loaders::refillTilingsCombo()
{
    tileListWidget->blockSignals(true);
    tileListWidget->addItemList(Sys::tilingsList);
    tileListWidget->blockSignals(false);

    for(int i = 0; i < tileListWidget->count(); i++)
    {
        QListWidgetItem * item = tileListWidget->item(i);
        QString displayName = item->text();
        bool used = false;
        for (TilingUse & tu : Sys::tilingUses)
        {
            if (tu.first.getVersionedName().get() == displayName)
            {
                used = true;
            }
        }
        if (!used)
        {
            if (Sys::isDarkTheme)
                tileListWidget->item(i)->setForeground(Qt::lightGray);
            else
                tileListWidget->item(i)->setForeground(Qt::gray);
        }
    }

    if (tileListWidget->selectItemByName(config->lastLoadedTiling.get()))
    {
        selectedTilingFile = FileServices::getFile(config->lastLoadedTiling,FILE_TILING);
    }
    else
    {
        selectedTilingFile.clear();
        config->lastLoadedTiling.clear();
    }
}

void page_loaders::mosaicSelected(QListWidgetItem *item, QListWidgetItem *oldItem)
{
    Q_UNUSED(oldItem)
    selectedMosaicFile = FileServices::getFile(item->text(),FILE_MOSAIC);
}

void page_loaders::mosaicClicked(QListWidgetItem *item)
{
    selectedMosaicFile = FileServices::getFile(item->text(),FILE_MOSAIC);
}

void page_loaders::slot_mosaicActivated(QListWidgetItem * item)
{
    Q_UNUSED(item)
    loadMosaic();
}

void page_loaders::slot_mosaicTextChanged(const QString & currentText)
{
    selectedMosaicFile = FileServices::getFile(currentText,FILE_MOSAIC);
}

void page_loaders::tilingSelected(QListWidgetItem *item, QListWidgetItem *oldItem)
{
    Q_UNUSED(oldItem)
    selectedTilingFile = FileServices::getFile(item->text(),FILE_TILING);
}

void page_loaders::tilingClicked(QListWidgetItem *item)
{
    selectedTilingFile = FileServices::getFile(item->text(),FILE_TILING);
}

void page_loaders::slot_tilingActivated(QListWidgetItem * item)
{
    Q_UNUSED(item)
    loadTiling(TILM_LOAD_SINGLE);
}

void page_loaders::slot_tilingTextChanged(const QString & currentText)
{
    selectedTilingFile = FileServices::getFile(currentText,FILE_TILING);
}

void page_loaders::designSelected(QListWidgetItem * item, QListWidgetItem* oldItem)
{
    Q_UNUSED(oldItem)
    qDebug() << "page_loaders::designSelected";
    selectedDesign = static_cast<eDesign>(item->data(Qt::UserRole).toInt());
    emit sig_loadDesign(selectedDesign);
}

void page_loaders::designClicked(QListWidgetItem * item)
{
    qDebug() << "page_loaders::designSelected";
    selectedDesign = static_cast<eDesign>(item->data(Qt::UserRole).toInt());
    emit sig_loadDesign(selectedDesign);
    panel->delegateView(VIEW_LEGACY,true);
}

void page_loaders::slot_mosaicItemEnteredToolTip(QListWidgetItem * item)
{
    QString s = item->text();

    VersionedName vn(s);
    VersionedFile file = FileServices::getFile(vn,FILE_MOSAIC);
    if (file.isEmpty())
    {
        return;
    }

    QFile afile(file.getPathedName());
    if (!afile.exists())
    {
        item->setToolTip(s);
        return;
    }

    xml_document doc;
    xml_parse_result result = doc.load_file(file.getPathedName().toStdString().c_str());
    if (result)
    {
        xml_node n = doc.child("vector");
        if (n)
        {
            n = n.child("designNotes");
            if (n)
            {
                s = item->text() + " : " + n.child_value();
                item->setToolTip(s);
                return;
            }
        }
    }
    item->setToolTip(s);
}

void page_loaders::desRightClick(QPoint pos)
{
    Q_UNUSED(pos)
}

void page_loaders::xmlRightClick(QPoint pos)
{
    QString mosaic   = "Mosaic : " + selectedMosaicFile.getVersionedName().get();
    QString tiling   = "Tiling used : " + FileServices::getTileFileFromMosaicFile(selectedMosaicFile).getVersionedName().get();
    QString bkgd     = FileServices::getBackgroundFilenameFromMosaicFile(selectedMosaicFile);

    QMenu myMenu;
    myMenu.addSection(mosaic);
    myMenu.addSection(tiling);
    if (!bkgd.isEmpty())
    {
        bkgd = "Background : " + bkgd;
        myMenu.addSection(bkgd);
    }
    myMenu.addSeparator();
    myMenu.addAction("Load",            this, SLOT(loadMosaic()));
    myMenu.addAction("View XML",        this, SLOT(openXML()));
#ifdef Q_OS_WINDOWS
    myMenu.addAction("Show in Dir",     this, SLOT(showXMLDir()));
#endif
    myMenu.addAction("Tiling used",     this, SLOT(showTilings()));
    myMenu.addAction("Rename",          this, SLOT(renameMosaic()));
    myMenu.addAction("Rebase",          this, SLOT(rebaseMosaic()));
    myMenu.addAction("Delete",          this, SLOT(deleteMosaic()));
    myMenu.addAction("Reformat",        this, SLOT(reformatMosaic()));
    myMenu.addAction("Add to Worklist", this, SLOT(addToWorklist()));
    myMenu.addAction("Remove from Worklist", this, SLOT(removeFromWorklist()));
    myMenu.addAction("Generate BMP",    this, SLOT(genMosaicBMP()));

    myMenu.exec(mosaicListWidget->mapToGlobal(pos));
}

void page_loaders::tileRightClick(QPoint pos)
{
    QMenu myMenu;
    myMenu.addSection(selectedTilingFile.getVersionedName().get());
    myMenu.addAction("Load",        this, SLOT(loadTiling2()));
    myMenu.addAction("View XML",    this, SLOT(openTiling()));
    myMenu.addAction("Rename",      this, SLOT(renameTiling()));
    myMenu.addAction("Rebase",      this, SLOT(rebaseTiling()));
    myMenu.addAction("Delete",      this, SLOT(deleteTiling()));
    myMenu.addAction("Reformat",    this, SLOT(reformatTiling()));
    myMenu.addAction("Add to Worklist", this, SLOT(addTilingToWorklist()));
    myMenu.addAction("Remove from Worklist", this, SLOT(removeTilingFromWorklist()));
    myMenu.addAction("Where used",  this, SLOT(slot_whereTilingUsed()));

    myMenu.exec(tileListWidget->mapToGlobal(pos));
}

void page_loaders::openXML()
{
    VersionedFile path = selectedMosaicFile;
    if (path.isEmpty())
    {
        return;
    }
    if (!QFile::exists(path.getPathedName()))
    {
        return;
    }

    QStringList qsl;
    qsl << path.getPathedName();
    QString cmd = config->xmlTool;
    if (!QFile::exists(cmd))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Information);
        box.setText("Please configure an XML viewet/editor");
        box.exec();
        return;
    }
    qDebug() << "starting"  << cmd  << qsl;
    QProcess::startDetached(cmd,qsl);
}

#ifdef Q_OS_WINDOWS
void page_loaders::showXMLDir()
{
    VersionedFile file = selectedMosaicFile;
    if (file.isEmpty())
    {
        return;
    }

    if (!QFile::exists(file.getPathedName()))
    {
        return;
    }

    QString path  = file.getPathOnly();
    QString path2 = QDir::toNativeSeparators(path);
    qDebug() <<  "Path2:" << path2;

    QStringList args;
    args << path2;

    QProcess::startDetached("explorer",args);
}
#endif

void page_loaders::rebaseMosaic()
{
    VersionedFile xfile  = selectedMosaicFile;
    VersionedFile xdata  = FileServices::getFile(selectedMosaicFile.getVersionedName(),FILE_TEMPLATE);
    if (xfile.isEmpty())
    {
        QMessageBox box(this);
        box.setText(QString("File <%1>not found").arg(selectedMosaicFile.getVersionedName().get()));
        box.exec();
        return;
    }

    if (!QFile::exists(xfile.getPathedName()))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Could not find file to rebase");
        box.exec();
        return;
    }

    VersionedName vname = xfile.getVersionedName();
    uint iOldVer        = vname.getVersion();

    DlgRebase dlg(this);
    dlg.oldVersion->setValue(iOldVer);
    dlg.newVersion->setValue(iOldVer);
    int retval = dlg.exec();

    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    uint iNewVer = dlg.newVersion->value();

    if (iNewVer >= iOldVer)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Information);
        box.setText("New version invalid - cannot rebase");
        box.exec();
        return;
    }

    vname.setVersion(iNewVer);

    // this is the new file
    VersionedFile newXML = xfile;
    newXML.updateFromVersionedName(vname);
    VersionedFile newDat = xdata;
    newDat.updateFromVersionedName(vname);

    // purge other versions
    for (uint i= iNewVer; i < iOldVer; i++)
    {
        QString aname;
        if (i == 0)
            aname = vname.getUnversioned();
        else
            aname = vname.getUnversioned() + ".v" + QString::number(i);

        VersionedName vn(aname);

        VersionedFile afile = FileServices::getFile(vn,FILE_MOSAIC);
        if (!afile.isEmpty())
        {
            QFile::remove(afile.getPathedName());
        }
    }

    for (uint i = iNewVer; i <iOldVer; i++)
    {
        QString aname;
        if (i == 0)
            aname = vname.getUnversioned();
        else
            aname = vname.getUnversioned() + ".v" + QString::number(i);

        VersionedName vn(aname);

        VersionedFile afile = FileServices::getFile(vn,FILE_TEMPLATE);
        if (!afile.isEmpty())
        {
            QFile::remove(afile.getPathedName());
        }
    }

    // rename XML to new file
    QString from = xfile.getPathedName();
    QString to   = newXML.getPathedName();
    qDebug() << "copy from" << from << "to" << to;
    bool rv = QFile::rename(from,to);
    if (!rv)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText("Rebase - FAILED during copy");
        box.exec();
        return;
    }

    // rename template .dat to new file
    from = xdata.getPathedName();
    to   = newDat.getPathedName();
    qDebug() << "copy from" << from << "to" << to;
    QFile::rename(from,to);  // it is ok for this to fail

    // setup
    if (selectedMosaicFile.getVersionedName().get() == config->lastLoadedMosaic.get())
    {
        mosaicMaker->getLoadUnit()->declareLoaded(selectedMosaicFile);
    }

    selectedMosaicFile = newXML;

    slot_newMosaic();
}

void page_loaders::renameMosaic()
{
    DlgRename dlg(this);

    dlg.oldEdit->setText(selectedMosaicFile.getVersionedName().get());
    dlg.newEdit->setText(selectedMosaicFile.getVersionedName().get());

    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    QString newFileName = dlg.newEdit->text();
    if (newFileName == selectedMosaicFile.getVersionedName().get())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText("Name not changed in dlg.  Must have a different name");
        box.exec();
        return;
    }

    bool rv = false;
    VersionedFile oldXML = selectedMosaicFile;
    VersionedFile oldDat = FileServices::getFile(selectedMosaicFile.getVersionedName(),FILE_TEMPLATE);

    if (oldXML.isEmpty())
    {
        return;
    }

    QString oldFileName = oldXML.getVersionedName().get();

    QString oldFile     = oldXML.getPathedName();
    QString oldDatFile  = oldDat.getPathedName();

    if (!QFile::exists(oldFile))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText("Could not find existing file to rename");
        box.exec();
        return;
    }

    QString newFile = oldFile;
    newFile.replace(oldFileName ,newFileName);

    QString newDatFile = oldDatFile;
    newDatFile.replace(oldFileName ,newFileName);

    rv = QFile::rename(oldFile,newFile);
    QFile::rename(oldDatFile,newDatFile);   // it is ok for this to fail

    slot_newMosaic();

    // report satus
    QMessageBox box(this);
    if (rv)
    {
        VersionedFile vf(newFile);

        if (selectedMosaicFile.getVersionedName().get() == config->lastLoadedMosaic.get())
        {
            mosaicMaker->getLoadUnit()->end(LS_LOADED);
        }

        selectedMosaicFile = vf;;

        slot_mosaicLoaded(vf);
        box.setText("Rename OK");
    }
    else
    {
        box.setIcon(QMessageBox::Critical);
        box.setText("Rename - FAILED");
    }
    box.exec();
}

void page_loaders::addToWorklist()
{
    if (selectedMosaicFile.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select a mosaic - then try again");
        box.exec();
        return;
    }

    qDebug() << "Adding to worklist" << selectedMosaicFile.getVersionedName().get();
    config->worklist.add(selectedMosaicFile.getVersionedName());
}

void page_loaders::removeFromWorklist()
{
    if (selectedMosaicFile.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select a mosaic - then try again");
        box.exec();
        return;
    }

    qDebug() << "Removing from worklist" << selectedMosaicFile.getVersionedName().get();
    config->worklist.remove(selectedMosaicFile.getVersionedName());
}

void page_loaders::deleteMosaic()
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setText(QString("Delete file: %1.  Are you sure?").arg(selectedMosaicFile.getVersionedName().get()));
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }

    VersionedFile mosaic = selectedMosaicFile;
    VersionedFile data   = FileServices::getFile(selectedMosaicFile.getVersionedName(),FILE_TEMPLATE);

    if (mosaic.isEmpty())
    {
        QMessageBox box(this);
        box.setText(QString("File <%1>not found").arg(selectedMosaicFile.getVersionedName().get()));
        box.exec();
        return;
    }

    if (!QFile::exists(mosaic.getPathedName()))
    {
       QMessageBox box(this);
       box.setIcon(QMessageBox::Critical);
       box.setText(QString("File <%1>not found").arg(mosaic.getPathedName()));
       box.exec();
       return;
    }

    bool rv = QFile::remove(mosaic.getPathedName());
    QFile::remove(data.getPathedName());     // it is ok for this to fail

    slot_newMosaic();

    // report satus
    QMessageBox box2(this);
    if (rv)
    {
        selectedMosaicFile.clear();
        box2.setText("Deleted OK");
    }
    else
    {
        box2.setIcon(QMessageBox::Critical);
        box2.setText("Deletion- FAILED");
    }
    box2.exec();
}

void page_loaders::reformatMosaic()
{
    VersionedFile mosaic = selectedMosaicFile;

    if (mosaic.isEmpty())
    {
        QMessageBox box(this);
        box.setText(QString("File <%1>not found").arg(selectedMosaicFile.getVersionedName().get()));
        box.exec();
        return;
    }

    if (!QFile::exists(mosaic.getPathedName()))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("File <%1>not found").arg(mosaic.getPathedName()));
        box.exec();
        return;
    }

    bool rv = FileServices::reformatXML(mosaic);
    if (rv)
    {
        loadMosaicsCombo();

        QMessageBox box(this);
        box.setIcon(QMessageBox::Information);
        box.setText("Reformat OK");
        box.exec();
    }
    else
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Reformat FAILED");
        box.exec();
        return;
    }
}

void page_loaders::genMosaicBMP()
{
    VersionedFile mosaic = selectedMosaicFile;
    VersionedName name   = mosaic.getVersionedName();
    QString path = Sys::config->rootImageDir + "snaps/";

    MosaicBMPGenerator engine;
    engine.saveBitmap(name,path);

    FlashMsgBox box(5000,this);
    box.setIcon(QMessageBox::Warning);
    QString str = QString("BMP %1 generated in %2").arg(name.get()).arg(path);
    box.setText(str);
    box.exec();
}

void page_loaders::showTilings()
{
    VersionedFile tileFile = FileServices::getTileFileFromMosaicFile(selectedMosaicFile);

    QString results = "<pre>";
    results += tileFile.getVersionedName().get();
    results += "<br>";
    results += "</pre>";

    QMessageBox box(this);
    box.setWindowTitle("Tiling used");
    box.setText(results);
    box.exec();

}

void page_loaders::openTiling()
{
    VersionedFile tiling = selectedTilingFile;
    if (!QFile::exists(tiling.getPathedName()))
    {
        return;
    }

    QStringList qsl;
    qsl << tiling.getPathedName();
    QString cmd = config->xmlTool;
    if (!QFile::exists(cmd))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Information);
        box.setText("Please configure an XML viewet/editor");
        box.exec();
        return;
    }
    qDebug() << "starting"  << cmd  << qsl;
    QProcess::startDetached(cmd,qsl);
}

void page_loaders::rebaseTiling()
{
    // find where used
    VersionFileList mosaics = FileServices::whereTilingUsed(selectedTilingFile.getVersionedName());

    // validate file to rebase exists
    VersionedFile xfile = selectedTilingFile;
    if (xfile.isEmpty())
    {
        QMessageBox box;
        box.setText(QString("File <%1>not found").arg(selectedTilingFile.getVersionedName().get()));
        box.exec();
        return;
    }

    if (!QFile::exists(xfile.getPathedName()))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Could not find file to rebase");
        box.exec();
        return;
    }

    VersionedName vname  = xfile.getVersionedName();
    uint iOldVer       = vname.getVersion();

    if (iOldVer ==0)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Information);
        box.setText("Cannot rebase");
        box.exec();
        return;
    }

    DlgRebase dlg(this);
    dlg.oldVersion->setValue(iOldVer);
    dlg.newVersion->setValue(iOldVer);
    int retval = dlg.exec();

    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    uint iNewVer = dlg.newVersion->value();

    if (iNewVer >= iOldVer)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Information);
        box.setText("Cannot rebase");
        box.exec();
        return;
    }

    vname.setVersion(iNewVer);

    // this is the new file
    VersionedFile newTiling = xfile;
    newTiling.updateFromVersionedName(vname);

    // purge other versions
    for (uint i = iNewVer; i < iOldVer; i++)
    {
        QString aname;
        if (i == 0)
            aname = vname.getUnversioned();
        else
            aname = vname.getUnversioned() + ".v" + QString::number(i);

        VersionedName vn(aname);

        VersionedFile afile = FileServices::getFile(vn,FILE_TILING);
        if (!afile.isEmpty())
        {
            VersionFileList xfiles = FileServices::whereTilingUsed(vn);
            mosaics += xfiles;

            QFile::remove(afile.getPathedName());
        }
    }

    // rename to new file
    QString from = xfile.getPathedName();
    QString to   = newTiling.getPathedName();
    qDebug() << "copy from" << from << "to" << to;
    bool rv = QFile::rename(from,to);
    if (!rv)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText("Rebase - FAILED during copy");
        box.exec();
        return;
    }

    // fix the name in the tiling file
    putNewTilingNameIntoTiling(newTiling,vname);

    // fixup tiling name in designs
    putNewTilingNameIntoMosaic(mosaics, vname);

    slot_newTiling();
}

void page_loaders::renameTiling()
{
    // find where used
    VersionFileList used =  FileServices::whereTilingUsed(selectedTilingFile.getVersionedName());

    // select new name
    QString oldname = selectedTilingFile.getVersionedName().get();
    bool fixupName = false;
    if (config->lastLoadedTiling.get() == oldname)
        fixupName = true;

    DlgRename dlg(this);

    dlg.oldEdit->setText(oldname);
    dlg.newEdit->setText(oldname);

    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    // validate new name
    VersionedName newName;
    newName.set(dlg.newEdit->text());
    if (oldname == newName.get())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText("Name not changed in dlg.  Must have a different name");
        box.exec();
        return;
    }

    // validate old file exists
    VersionedFile oldTiling = selectedTilingFile;
    if (oldTiling.isEmpty())
    {
        return;
    }

    QFile theFile(oldTiling.getPathedName());
    if (!theFile.exists())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText("Could not find existing file to rename");
        box.exec();
        return;
    }

    VersionedFile newTiling = oldTiling;
    newTiling.updateFromVersionedName(newName);

    // rename the the file
    bool rv = theFile.rename(newTiling.getPathedName());
    if (!rv)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setText("Could not rename file");
        box.exec();
        return;
    }
    theFile.close();        // sp ot can be opened again;

    // fix the name in the tiling file
    rv = putNewTilingNameIntoTiling(newTiling,newName);

    // fixup tiling name in  XML design files
    putNewTilingNameIntoMosaic(used, newName);

    // report status and remove old file
    QMessageBox box(this);
    if (rv)
    {
        QFile afile(oldTiling.getPathedName());
        afile.remove();     // delete old file

        if (fixupName)
        {
            tilingMaker->getLoadUnit()->declareLoaded(newTiling);
        }

        slot_newTiling();

        box.setText("Rename OK");
    }
    else
    {
        box.setIcon(QMessageBox::Critical);
        box.setText("Rename - FAILED");
    }
    box.exec();
}

void page_loaders::deleteTiling()
{
    QString msg;
    VersionFileList used = FileServices::whereTilingUsed(selectedTilingFile.getVersionedName());
    if (used.size())
    {
        msg = "<pre>";
        msg += "This tiling is used in:<br>";
        for (int i=0; i < used.size(); i++)
        {
            msg += used.at(i).getVersionedName().get();
            msg += "<br>";
        }
        msg += "</pre>";
    }
    msg += QString("Delete file: %1.  Are you sure?").arg(selectedTilingFile.getVersionedName().get());

    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setText(msg);
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }

    VersionedFile tiling = selectedTilingFile;
    if (!QFile::exists(tiling.getPathedName()))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("File <%1>not found").arg(tiling.getPathedName()));
        box.exec();
        return;
    }

    bool rv = QFile::remove(tiling.getPathedName());

    // report status
    QMessageBox box2(this);
    if (rv)
    {
        loadTilingsCombo();
        emit sig_setTilingUses();
        box2.setText("Deleted OK");
    }
    else
    {
        box2.setIcon(QMessageBox::Critical);
        box2.setText("Deletion- FAILED");
    }
    box2.exec();
}

void page_loaders::reformatTiling()
{
    VersionedFile tiling = selectedTilingFile;
    if (!QFile::exists(tiling.getPathedName()))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("File <%1>not found").arg(tiling.getPathedName()));
        box.exec();
        return;
    }

    bool rv = FileServices::reformatXML(tiling);
    if (rv)
    {
        loadTilingsCombo();

        QMessageBox box(this);
        box.setIcon(QMessageBox::Information);
        box.setText("Reformat OK");
        box.exec();
    }
    else
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Reformat FAILED");
        box.exec();
        return;
    }
}

void page_loaders::addTilingToWorklist()
{
    if (selectedTilingFile.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select a tiling - then try again");
        box.exec();
        return;
    }

    qDebug() << "Adding to worklist" << selectedTilingFile.getVersionedName().get();
    config->worklist.add(selectedTilingFile.getVersionedName());
}

void page_loaders::removeTilingFromWorklist()
{
    if (selectedTilingFile.isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select a tiling - then try again");
        box.exec();
        return;
    }

    qDebug() << "Removing from worklist" << selectedTilingFile.getVersionedName().get();
    config->worklist.remove(selectedTilingFile.getVersionedName());
}

void  page_loaders::slot_whereTilingUsed()
{
    VersionFileList used = FileServices::whereTilingUsed(selectedTilingFile.getVersionedName());

    if (used.size() == 0)
    {
        QMessageBox box(this);
        box.setWindowTitle(QString("Where tiling %1 is used ").arg(selectedTilingFile.getVersionedName().get()));
        box.setText("Tiling NOT used");
        box.exec();
        return;
    }

    DlgListNameSelect dlg(used,1);
    int rv = dlg.exec();

    if (rv == QDialog::Accepted)
    {
        VersionedName name;
        name.set(dlg.newEdit->text());
        VersionedFile vf = FileServices::getFile(name,FILE_MOSAIC);
        mosaicMaker->loadMosaic(vf);

        if (!config->lockView)
        {
            panel->delegateView(VIEW_MOSAIC,true);
        }
        emit sig_reconstructView();
    }
}

void page_loaders::putNewTilingNameIntoMosaic(VersionFileList & mosaics, VersionedName newTilingName)
{
    for (const VersionedFile & mosaic : std::as_const(mosaics))
    {
        // load file
        xml_document doc;
        xml_parse_result result = doc.load_file(mosaic.getPathedName().toStdString().c_str());
        if (result == false)
        {
            qWarning().noquote() << result.description();
            continue;
        }

        // edit
        xml_node node1 = doc.child("vector");
        if (!node1) continue;

        uint version = 0;
        xml_attribute attr = node1.attribute("version");
        if (attr)
        {
            QString str = attr.value();
            version = str.toUInt();
        }

        xml_node node2;
        for (node2 = node1.first_child(); node2; node2 = node2.next_sibling())
        {
            QString str = node2.name();
            if (str == "style.Thick")
                break;
            else if (str == "style.Filled")
                break;
            else if (str == "style.Interlace")
                break;
            else if (str == "style.Outline")
                break;
            else if (str == "style.Plain")
                break;
            else if (str == "style.Sketch")
                break;
            else if (str == "style.Emboss")
                break;
            else if (str == "style.TileColors")
                break;
        }
        if (!node2) continue;
        xml_node node3 =  node2.child("style.Style");
        if (!node3) continue;
        xml_node node4 =  node3.child("prototype");
        if (!node4) continue;
        xml_node node5 =  node4.child("app.Prototype");
        if (!node5) continue;
        xml_node node6;
        if (version >= 21)
           node6 = node5.child("Tiling");
        else
            node6 = node5.child("string");
        if (!node6) continue;

        std::string  namestr = newTilingName.get().toStdString();
        node5.remove_child(node6);
        qDebug() << "old name =" << node6.child_value() << " new name = " << namestr.c_str();
        xml_node node7;
        if (version >= 21)
            node7 = node5.prepend_child("Tiling");
        else
            node7 = node5.prepend_child("string");
        node7.append_child(pugi::node_pcdata).set_value(namestr.c_str());

        // save file
        bool rv = doc.save_file(mosaic.getPathedName().toStdString().c_str());
        if (rv)
        {
            FileServices::reformatXML(mosaic);   // also timestamps
        }
        else
        {
            qWarning().noquote() << "Failed to reformat:" << mosaic.getPathedName();
        }
    }
}

bool  page_loaders::putNewTilingNameIntoTiling(VersionedFile tiling, VersionedName newTilingName)
{
    // fix name in file and save to new name
    bool rv = false;
    xml_document doc;
    xml_parse_result result = doc.load_file(tiling.getPathedName().toStdString().c_str());  // load file
    if (result == false)
    {
        qWarning() << result.description();
        return false;
    }

    xml_node node1 = doc.child("Tiling");
    if (node1)
    {
        xml_node node2 =  node1.child("Name");
        if (node2)
        {
            std::string  namestr = newTilingName.get().toStdString();

            node1.remove_child(node2);

            qDebug() << "old name =" << node2.child_value() << " new name = " << namestr.c_str();
            xml_node node3 = node1.prepend_child("Name");
            node3.append_child(pugi::node_pcdata).set_value(namestr.c_str());

            rv = doc.save_file(tiling.getPathedName().toStdString().c_str());  // save file
            if (!rv)
                qWarning() << "Failed to re-write modified tiling";
        }
        else
        {
            qWarning() << "Filed to change <Name> content in tiling";
            rv = false;
        }
    }
    else
    {
        qWarning() << "Filed to change <Tiling> content in tiling";
        rv = false;
    }
    if (rv)
    {
        rv = FileServices::reformatXML(tiling);   // also timestamps
    }
    return rv;
}

void  page_loaders::autoLoadLastClicked(bool enb)
{
    config->autoLoadLast = enb;
}
