#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QMultiMap>

#include "panels/page_loaders.h"
#include "panels/page_loaders_worker.h"
#include "legacy/design.h"
#include "legacy/design_maker.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/fileservices.h"
#include "misc/pugixml.hpp"
#include "panels/controlpanel.h"
#include "panels/panel_misc.h"
#include "settings/configuration.h"
#include "tiledpatternmaker.h"
#include "viewers/view_controller.h"
#include "widgets/dlg_listnameselect.h"
#include "widgets/dlg_rebase.h"
#include "widgets/dlg_rename.h"
#include "widgets/layout_sliderset.h"

#define LOADER_MAX_HEIGHT   580

using namespace pugi;

Q_DECLARE_METATYPE(eTILM_Event)

page_loaders::page_loaders(ControlPanel * apanel) : panel_page(apanel,PAGE_LOAD,"Load")
{
    selectedDesign = NO_DESIGN;

    QHBoxLayout * loaderLayout = new QHBoxLayout;

    loaderLayout->addWidget(createLegacyColumn());
    loaderLayout->addWidget(createMosaicColumn());
    loaderLayout->addWidget(createTilingColumn());
    loaderLayout->addStretch();

    vbox->addLayout(loaderLayout);
    vbox->addStretch();

    cbAutoLoadDesigns->setChecked(config->autoLoadDesigns);
    cbShowWithImages->setChecked(config->showWithBkgds);

    mosaicFilter->setText(config->mosaicFilter);
    mosaicFilterCheck->setChecked(config->mosaicFilterCheck);
    mosaicWorklistCheck->setChecked(config->mosaicWorklistCheck);
    mosaicOrigChk->setChecked(config->mosaicOrigCheck);
    mosaicNewChk->setChecked(config->mosaicNewCheck);
    mosaicTestChk->setChecked(config->mosaicTestCheck);
    cbAutoLoadMosaics->setChecked(config->autoLoadStyles);

    mosaicSortChk->setChecked(config->mosaicSortCheck);

    tilingFilter->setText(config->tileFilter);
    tilingFilterCheck->setChecked(config->tileFilterCheck);
    tilingOrigChk->setChecked(config->tilingOrigCheck);
    tilingNewChk->setChecked(config->tilingNewCheck);
    tilingTestChk->setChecked(config->tilingTestCheck);
    tilingWorklistCheck->setChecked(config->tilingWorklistCheck);
    cbAutoLoadTiling->setChecked(config->autoLoadTiling);

    // make connections before making selections
    makeConnections();

    // DAC designs
    loadDesignCombo();

    // Tiling
    loadTilingsCombo();

    // Mosaic
    loadMosaicsCombo();
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

// design patterns (shapes)
QGroupBox * page_loaders::createLegacyColumn()
{
    // first row
    cbAutoLoadDesigns = new QCheckBox("Auto-load");

    pbLoadShapes = new QPushButton("Load Fixed Design");
    pbLoadShapes->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(cbAutoLoadDesigns);
    hbox->addStretch();
    hbox->addWidget(pbLoadShapes);

    // second row
    QLabel * dummy2 = new QLabel(" ");

    // 3rd row
    QLabel * dummy3 = new QLabel(" ");

    // 4th row
    cbShowWithImages = new QCheckBox("Show with background images");

    // list widget
    designListWidget = new LoaderListWidget();
    designListWidget->setFixedHeight(LOADER_MAX_HEIGHT);
    designListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // column layout
    QVBoxLayout * designLayout = new QVBoxLayout;
    designLayout->addLayout(hbox);
    designLayout->addWidget(dummy2);
    designLayout->addWidget(dummy3);
    designLayout->addWidget(cbShowWithImages);
    designLayout->addWidget(designListWidget);

    QGroupBox * designGroup = new QGroupBox();
    designGroup->setLayout(designLayout);

    return designGroup;
}

// Mosaics
QGroupBox * page_loaders::createMosaicColumn()
{
    // first row
    cbAutoLoadMosaics = new QCheckBox("Auto-load");

    pbLoadXML = new QPushButton("Load Mosaic");
    pbLoadXML->setFixedWidth(119);
    pbLoadXML->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    QHBoxLayout * hbox1 = new QHBoxLayout();
    hbox1->addWidget(cbAutoLoadMosaics);
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
    mosaicWorklistCheck = new QCheckBox("Worklist");

    mosaicFilter = new QLineEdit();
    mosaicFilter->setFixedWidth(81);
    mosaicFilterCheck = new QCheckBox("Filter");

    QHBoxLayout * hbox3 = new QHBoxLayout();
    hbox3->addWidget(mosaicWorklistCheck);
    hbox3->addWidget(mosaicFilterCheck);
    hbox3->addWidget(mosaicFilter);
    hbox3->addStretch();

    // fourth row
    mosaicSortChk = new QCheckBox("Sort all by date written");
    AQHBoxLayout * hbox4 = new AQHBoxLayout();
    hbox4->addWidget(mosaicSortChk);
    hbox4->addStretch();

    // list widget
    mosaicListWidget = new VersionedListWidget();
    mosaicListWidget->setFixedHeight(LOADER_MAX_HEIGHT);
    mosaicListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    QVBoxLayout * mosaicLayout = new QVBoxLayout;
    mosaicLayout->addLayout(hbox1);
    mosaicLayout->addLayout(hbox2);
    mosaicLayout->addLayout(hbox3);
    mosaicLayout->addLayout(hbox4);
    mosaicLayout->addWidget(mosaicListWidget);

    QGroupBox * mosaicGroup = new QGroupBox();
    mosaicGroup->setLayout(mosaicLayout);

    return mosaicGroup;
}

// tiling
QGroupBox * page_loaders::createTilingColumn()
{
    // first row
    cbAutoLoadTiling  = new QCheckBox("Auto-load");

    pbLoadTiling = new QPushButton("Load Tiling");
    pbLoadTiling->setFixedWidth(119);
    pbLoadTiling->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    AQHBoxLayout * hbox = new AQHBoxLayout();
    hbox->addWidget(cbAutoLoadTiling);
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
    tilingWorklistCheck = new QCheckBox("Worklist");
    tilingFilterCheck = new QCheckBox("Filter");
    tilingFilter = new QLineEdit();
    tilingFilter->setFixedWidth(81);

    QHBoxLayout * hbox3 = new QHBoxLayout();
    hbox3->addWidget(tilingWorklistCheck);
    hbox3->addWidget(tilingFilterCheck);
    hbox3->addWidget(tilingFilter);
    hbox3->addStretch();

    // fourth row
    pbTilingLoadMulti   = new QPushButton(" Load Additonal ");
    pbTilingLoadModify  = new QPushButton(" Load Replace ");

    QHBoxLayout * hbox4 = new QHBoxLayout();
    hbox4->addWidget(pbTilingLoadModify);
    hbox4->addStretch();
    hbox4->addWidget(pbTilingLoadMulti);

    // list widget
    tileListWidget = new VersionedListWidget();
    tileListWidget->setFixedHeight(LOADER_MAX_HEIGHT);
    tileListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    QVBoxLayout * tilingLayout = new QVBoxLayout;
    tilingLayout->addLayout(hbox);
    tilingLayout->addLayout(hbox2);
    tilingLayout->addLayout(hbox3);
    tilingLayout->addLayout(hbox4);
    tilingLayout->addWidget(tileListWidget);

    QGroupBox * tilingGroup = new QGroupBox();
    tilingGroup->setLayout(tilingLayout);

    return tilingGroup;
}

void page_loaders::makeConnections()
{
    thread = new QThread(this);
    PageLoadersWorker * worker = new PageLoadersWorker();
    worker->moveToThread(thread);

    connect(this,               &page_loaders::sig_sortTilings,     worker, &PageLoadersWorker::sortTilings);
    connect(this,               &page_loaders::sig_sortMosaics,     worker, &PageLoadersWorker::sortMosaics);
    connect(worker,             &PageLoadersWorker::tilingsSorted,  this,   &page_loaders::refillTilingsCombo);
    connect(worker,             &PageLoadersWorker::mosaicsSorted,  this,   &page_loaders::refillMosaicsCombo);
    thread->start();

    connect(mosaicMaker,        &MosaicMaker::sig_mosaicLoaded,     this,   &page_loaders::slot_mosaicLoaded);
    connect(mosaicMaker,        &MosaicMaker::sig_mosaicWritten,    this,   &page_loaders::slot_newXML);
    connect(tilingMaker,        &TilingMaker::sig_tilingLoaded,     this,   &page_loaders::slot_tilingLoaded);
    connect(tilingMaker,        &TilingMaker::sig_tilingWritten,    this,   &page_loaders::slot_newTile);
    connect(designMaker,        &DesignMaker::sig_loadedDesign,     this,   &page_loaders::slot_loadedDesign);

    connect(theApp,         &TiledPatternMaker::sig_workListChanged,this,   &page_loaders::loadMosaicsCombo);

    connect(pbLoadXML,          &QPushButton::clicked,              this,   &page_loaders::loadMosaic);
    connect(pbLoadTiling,       &QPushButton::clicked,              this,   &page_loaders::slot_loadTiling);
    connect(pbTilingLoadModify, &QPushButton::clicked,              this,   &page_loaders::slot_loadTilingModify);
    connect(pbTilingLoadMulti,  &QPushButton::clicked,              this,   &page_loaders::slot_loadTilingMulti);
    connect(pbLoadShapes,       &QPushButton::clicked,              this,   &page_loaders::loadShapes);
    connect(mosaicFilter,       &QLineEdit::textEdited,             this,   &page_loaders::slot_mosaicFilter);
    connect(tilingFilter,       &QLineEdit::textEdited,             this,   &page_loaders::slot_tilingFilter);
    connect(mosaicFilterCheck,  &QCheckBox::clicked,                this,   &page_loaders::slot_mosaicCheck);
    connect(mosaicWorklistCheck,&QCheckBox::clicked,                this,   &page_loaders::slot_mosaicWorklistCheck);
    connect(tilingWorklistCheck,&QCheckBox::clicked,                this,   &page_loaders::slot_tilingWorklistCheck);

    connect(mosaicOrigChk,      &QCheckBox::clicked,                this,   &page_loaders::slot_mosOrigCheck);
    connect(mosaicNewChk,       &QCheckBox::clicked,                this,   &page_loaders::slot_mosNewCheck);
    connect(mosaicTestChk,      &QCheckBox::clicked,                this,   &page_loaders::slot_mosTestCheck);
    connect(mosaicSortChk,      &QCheckBox::clicked,                this,   &page_loaders::slot_mosSortCheck);
    connect(cbShowWithImages,   &QCheckBox::clicked,                this,   &page_loaders::slot_showWithBkgds);
    connect(tilingOrigChk,      &QCheckBox::clicked,                this,   &page_loaders::slot_tilingOrigCheck);
    connect(tilingNewChk,       &QCheckBox::clicked,                this,   &page_loaders::slot_tilingNewCheck);
    connect(tilingTestChk,      &QCheckBox::clicked,                this,   &page_loaders::slot_tilingTestCheck);
    connect(tilingFilterCheck,  &QCheckBox::clicked,                this,   &page_loaders::slot_tilingCheck);

    connect(this,               &page_loaders::sig_loadDesign,      designMaker, &DesignMaker::slot_loadDesign);
    connect(this,               &page_loaders::sig_buildDesign,     designMaker, &DesignMaker::slot_buildDesign);

    connect(designListWidget,   &QListWidget::itemClicked,          this,   &page_loaders::designClicked);
    connect(designListWidget,   SIGNAL(rightClick(QPoint)),         this,   SLOT(desRightClick(QPoint)));

    connect(mosaicListWidget,   &QListWidget::itemClicked,          this,   &page_loaders::mosaicClicked);
    connect(mosaicListWidget,   &LoaderListWidget::itemEntered,     this,   &page_loaders::slot_mosaicItemEnteredToolTip);
    connect(mosaicListWidget,   SIGNAL(rightClick(QPoint)),         this,   SLOT(xmlRightClick(QPoint)));
    connect(mosaicListWidget,   SIGNAL(leftDoubleClick(QPoint)),    this,   SLOT(loadMosaic()));
    connect(mosaicListWidget,   SIGNAL(listEnter()),                this,   SLOT(loadMosaic()));

    connect(tileListWidget,     &LoaderListWidget::leftDoubleClick, this,   &page_loaders::slot_loadTiling);
    connect(tileListWidget,     &QListWidget::currentItemChanged,   this,   &page_loaders::tilingSelected);
    connect(tileListWidget,     &QListWidget::itemClicked,          this,   &page_loaders::tilingClicked);
    connect(tileListWidget,     SIGNAL(rightClick(QPoint)),         this,   SLOT(tileRightClick(QPoint)));

    connect(cbAutoLoadMosaics,  &QCheckBox::clicked,                this,   &page_loaders::autoLoadStylesClicked);
    connect(cbAutoLoadTiling,   &QCheckBox::clicked,                this,   &page_loaders::autoLoadTilingClicked);
    connect(cbAutoLoadDesigns,  &QCheckBox::clicked,                this,   &page_loaders::autoLoadDesignsClicked);

    Q_ASSERT(thread->isRunning());
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
    loadMosaicsCombo();
}

void page_loaders::slot_tilingWorklistCheck(bool check)
{
    config->tilingWorklistCheck = check;
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

void page_loaders::slot_newTile(QString name)
{
    loadTilingsCombo();
}

void page_loaders::slot_newXML()
{
    loadMosaicsCombo();
}

void page_loaders::loadMosaic()
{
    if (selecteMosaicName.isEmpty())
    {
        return;
    }

    mosaicMaker->loadMosaic(selecteMosaicName);

    if (!config->lockView)
    {
       panel->delegateView(VIEW_MOSAIC);
    }
}

void page_loaders::loadTiling(eTILM_Event event)
{
    if (selectedTilingName.isEmpty())
    {
        return;
    }

    tilingMaker->loadTiling(selectedTilingName,event);

    if (event != TILM_RELOAD && !viewControl->isEnabled(VIEW_TILING_MAKER))
    {
        // delegate the view
        panel->delegateView(VIEW_TILING);
    }
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
       panel->delegateView(VIEW_DESIGN);
    }
    emit sig_refreshView();
}

void page_loaders::slot_loadTiling()
{
    loadTiling(TILM_LOAD_SINGLE);
}

void page_loaders::slot_loadTilingModify()
{
    loadTiling(TILM_RELOAD);
}

void page_loaders::slot_loadTilingMulti()
{
    loadTiling(TILM_LOAD_MULTI);
}

void page_loaders::slot_mosaicLoaded(QString name)
{
    qDebug() << "page_loaders:: loaded XML:" << name;

    mosaicListWidget->blockSignals(true);
    if (mosaicListWidget->selectItemByName(name))
    {
        selecteMosaicName = name;
    }
    else
    {
        selecteMosaicName.clear();
    }
    mosaicListWidget->blockSignals(false);

    // update the tiling too
    QString tileName = FileServices::getTileNameFromDesignName(name);
    slot_tilingLoaded(tileName);
}

void page_loaders::slot_tilingLoaded(QString name)
{
    qDebug() << "page_loaders:: loaded Tiling:" << name;
    tileListWidget->blockSignals(true);
    if (tileListWidget->selectItemByName(name))
    {
        selectedTilingName = name;
    }
    tileListWidget->blockSignals(false);
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
    // DAC designs

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
        eDesign  des = (eDesign)designs.key(config->lastLoadedLegacyDes);
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
    emit sig_sortMosaics(mosaicWorklistCheck->isChecked(),config->showWithBkgds,mosaicSortChk->isChecked());
}

void page_loaders::refillMosaicsCombo()
{
    if (mosaicSortChk->isChecked())
    {
        mosaicListWidget->blockSignals(true);
        mosaicListWidget->rawAdd(Sys::mosaicsList);
        mosaicListWidget->blockSignals(false);
    }
    else
    {
        mosaicListWidget->blockSignals(true);
        mosaicListWidget->addItemList(Sys::mosaicsList);
        mosaicListWidget->blockSignals(false);
    }

    if (mosaicListWidget->selectItemByName(config->lastLoadedMosaic))
    {
        selecteMosaicName = config->lastLoadedMosaic;
    }
    else
    {
        config->lastLoadedMosaic.clear();
    }
}

void page_loaders::loadTilingsCombo()
{
    emit sig_sortTilings(tilingWorklistCheck->isChecked(),config->showWithBkgds,mosaicSortChk->isChecked());
}

void page_loaders::refillTilingsCombo()
{
    if (mosaicSortChk->isChecked())
    {
        tileListWidget->blockSignals(true);
        tileListWidget->rawAdd(Sys::tilingsList);
        tileListWidget->blockSignals(false);
    }
    else
    {
        tileListWidget->blockSignals(true);
        tileListWidget->addItemList(Sys::tilingsList);
        tileListWidget->blockSignals(false);
    }

    for(int i = 0; i < tileListWidget->count(); i++)
    {
        QListWidgetItem * item = tileListWidget->item(i);
        QString tileName = item->text();
        int usecount = Sys::uses.count(tileName);
        if (usecount == 0)
        {
            if (Sys::isDarkTheme)
                tileListWidget->item(i)->setForeground(Qt::lightGray);
            else
                tileListWidget->item(i)->setForeground(Qt::gray);
        }
    }

    if (tileListWidget->selectItemByName(config->lastLoadedTiling))
    {
        selectedTilingName = config->lastLoadedTiling;
    }
    else
    {
        selectedTilingName.clear();
        config->lastLoadedTiling.clear();
    }
}

void page_loaders::designSelected(QListWidgetItem * item, QListWidgetItem* oldItem)
{
    Q_UNUSED(oldItem)
    qDebug() << "page_loaders::designSelected";
    selectedDesign = static_cast<eDesign>(item->data(Qt::UserRole).toInt());
    emit sig_loadDesign(selectedDesign);
}

void page_loaders::tilingSelected(QListWidgetItem *item, QListWidgetItem *oldItem)
{
    Q_UNUSED(oldItem)
    selectedTilingName = item->text();
}

void page_loaders::mosaicSelected(QListWidgetItem *item, QListWidgetItem *oldItem)
{
    Q_UNUSED(oldItem)
    selecteMosaicName = item->text();
}

void page_loaders::designClicked(QListWidgetItem * item)
{
    qDebug() << "page_loaders::designSelected";
    selectedDesign = static_cast<eDesign>(item->data(Qt::UserRole).toInt());
    emit sig_loadDesign(selectedDesign);
    panel->delegateView(VIEW_DESIGN);
}

void page_loaders::tilingClicked(QListWidgetItem *item)
{
    selectedTilingName = item->text();
}

void page_loaders::mosaicClicked(QListWidgetItem *item)
{
    selecteMosaicName = item->text();
}

void page_loaders::slot_mosaicItemEnteredToolTip(QListWidgetItem * item)
{
    QString s = item->text();
    //qDebug() << "slot_itemEntered" << s;

    QString file = FileServices::getMosaicXMLFile(s);
    if (file.isEmpty())
    {
        return;
    }

    QFile afile(file);
    if (!afile.exists())
    {
        item->setToolTip(s);
        return;
    }

    xml_document doc;
    xml_parse_result result = doc.load_file(file.toStdString().c_str());
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
    QMenu myMenu;
    myMenu.addSection(selecteMosaicName);
    myMenu.addSection(FileServices::getTileNameFromDesignName(selecteMosaicName));
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

    myMenu.exec(mosaicListWidget->mapToGlobal(pos));
}

void page_loaders::tileRightClick(QPoint pos)
{
    QMenu myMenu;
    myMenu.addSection(selectedTilingName);
    myMenu.addAction("Load",        this, SLOT(loadTiling2()));
    myMenu.addAction("View XML",    this, SLOT(openTiling()));
    myMenu.addAction("Rename",      this, SLOT(renameTiling()));
    myMenu.addAction("Rebase",      this, SLOT(rebaseTiling()));
    myMenu.addAction("Delete",      this, SLOT(deleteTiling()));
    myMenu.addAction("Reformat",    this, SLOT(reformatTiling()));
    myMenu.addAction("WhereUsed",   this, SLOT(slot_whereTilingUsed()));

    myMenu.exec(tileListWidget->mapToGlobal(pos));
}

void page_loaders::openXML()
{
    QString name = selecteMosaicName;
    QString path = FileServices::getMosaicXMLFile(name);
    if (path.isEmpty())
    {
        return;
    }
    if (!QFile::exists(path))
    {
        return;
    }

    QStringList qsl;
    qsl << path;
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
    QString name = selecteMosaicName;
    QString path = FileServices::getMosaicXMLFile(name);
    if (path.isEmpty())
    {
        return;
    }
    if (!QFile::exists(path))
    {
        return;
    }

    QString extendedName = name + ".xml";
    path = path.remove(extendedName);

    QString path2 = QDir::toNativeSeparators(path);
    qDebug() <<  "Path2:" << path2;

    QStringList args;
    args << path2;

    QProcess::startDetached("explorer",args);
}
#endif

void page_loaders::rebaseMosaic()
{
    QString name    = selecteMosaicName;

    QString oldXMLPath = FileServices::getMosaicXMLFile(name);
    QString oldDatPath = FileServices::getMosaicTemplateFile(name);
    if (oldXMLPath.isEmpty())
    {
        QMessageBox box(this);
        box.setText(QString("File <%1>not found").arg(name));
        box.exec();
        return;
    }

    if (!QFile::exists(oldXMLPath))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Could not find file to rebase");
        box.exec();
        return;
    }

    QStringList parts = name.split('.');
    if (parts.size() == 1)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Information);
        box.setText("Cannot rebase");
        box.exec();
        return;
    }

    QString old_vstring = parts.last();
    if (!old_vstring.contains('v'))
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Information);
        box.setText("Cannot rebase");
        box.exec();
        return;
    }

    QStringList allParts = parts;
    allParts.removeLast();
    QString nameRoot = allParts.join('.');

    old_vstring.remove('v');
    int old_ver = old_vstring.toInt();

    DlgRebase dlg(this);
    dlg.oldVersion->setValue(old_ver);
    dlg.newVersion->setValue(old_ver);

    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    int new_ver = dlg.newVersion->value();

    if (old_ver == new_ver)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Information);
        box.setText("Cannot rebase");
        box.exec();
        return;
    }

    if (new_ver == 0)
    {
        parts.removeLast();
    }
    else
    {
        QString newV = "v" + QString::number(new_ver);
        QString & lastV = parts.last();
        lastV = newV;
    }

    QString newName = parts.join('.');

    // this is the new file
    QString newXMLPath = Sys::newMosaicDir + newName + ".xml";
    QString newDatPath = Sys::templateDir  + newName + ".dat";

    // purge other versions
    Q_ASSERT(new_ver < old_ver);

    QString aname;

    for (int i= new_ver; i < old_ver; i++)
    {
        if (i==0)
            aname = nameRoot;
        else
            aname = nameRoot + ".v" + QString::number(i);
        QString path = FileServices::getMosaicXMLFile(aname);
        if (!path.isEmpty())
        {
            QFile::remove(path);
        }
    }

    for (int i= new_ver; i < old_ver; i++)
    {
        if (i==0)
            aname = nameRoot;
        else
                    aname = nameRoot + ".v" + QString::number(i);
        QString path = FileServices::getMosaicTemplateFile(aname);
        if (!path.isEmpty())
        {
            QFile::remove(path);
        }
    }

    // rename XML to new file
    QFile afile(oldXMLPath);
    bool rv = afile.copy(newXMLPath);
    if (!rv)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText("Rebase - FAILED during copy");
        box.exec();
        return;
    }
    // delete old
    afile.remove();

    // rename template .dat to new file
    QFile bfile(oldDatPath);
    bfile.copy(newDatPath);     // it is ok for this to fail
    // delete old
    bfile.remove();             // it is ok for this to fail

    // setup
    selecteMosaicName = newName;
    if (name == config->lastLoadedMosaic)
    {
        LoadUnit & loadUnit = view->getLoadUnit();
        loadUnit.setLoadState(LOADING_MOSAIC,newName);
        loadUnit.resetLoadState();
    }
    slot_newXML();
    slot_mosaicLoaded(selecteMosaicName);
}

void page_loaders::renameMosaic()
{
    QString name = selecteMosaicName;

    DlgRename dlg(this);

    dlg.oldEdit->setText(name);
    dlg.newEdit->setText(name);

    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);
    QString newName = dlg.newEdit->text();
    if (name == newName)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText("Name not changed in dlg.  Must have a different name");
        box.exec();
        return;
    }

    Q_ASSERT(!newName.contains(".xml"));

    bool rv = false;
    QString oldXMLPath = FileServices::getMosaicXMLFile(name);
    QString oldDatPath = FileServices::getMosaicTemplateFile(name);
    if (oldXMLPath.isEmpty())
    {
        return;
    }

    if (!QFile::exists(oldXMLPath))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText("Could not find existing file to rename");
        box.exec();
        return;
    }

    QString newXMLPath = oldXMLPath;
    newXMLPath.replace(name,newName);

    QString newDatPath = oldDatPath;
    newDatPath.replace(name,newName);

    rv = QFile::rename(oldXMLPath,newXMLPath);
    QFile::rename(oldDatPath,newDatPath);   // it is ok for this to fail

    slot_newXML();

    // report satus
    QMessageBox box(this);
    if (rv)
    {
        selecteMosaicName = newName;
        if (name == config->lastLoadedMosaic)
        {
            LoadUnit & loadUnit = view->getLoadUnit();
            loadUnit.setLoadState(LOADING_MOSAIC,newName);
            loadUnit.resetLoadState();
        }

        slot_mosaicLoaded(selecteMosaicName);
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
    QString name = selecteMosaicName;
    Q_ASSERT(!name.contains(".xml"));

    if (name.isEmpty())
    {

        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select a mosaic - then try again");
        box.exec();
        return;
    }

    qDebug() << "Adding to worklist" << name;
    config->worklist.add(name);
}


void page_loaders::deleteMosaic()
{
    QString name = selecteMosaicName;
    Q_ASSERT(!name.contains(".xml"));

    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setText(QString("Delete file: %1.  Are you sure?").arg(name));
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }

    QString XMLPath = FileServices::getMosaicXMLFile(name);
    QString DatPath = FileServices::getMosaicTemplateFile(name);

    if (XMLPath.isEmpty())
    {
        QMessageBox box(this);
        box.setText(QString("File <%1>not found").arg(name));
        box.exec();
        return;
    }

    if (!QFile::exists(XMLPath))
    {
       QMessageBox box(this);
       box.setIcon(QMessageBox::Critical);
       box.setText(QString("File <%1>not found").arg(XMLPath));
       box.exec();
       return;
    }

    bool rv = QFile::remove(XMLPath);
    QFile::remove(DatPath);     // it is ok for this to fail

    slot_newXML();

    // report satus
    QMessageBox box2(this);
    if (rv)
    {
        slot_mosaicLoaded(selecteMosaicName);
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
    QString name = selecteMosaicName;
    Q_ASSERT(!name.contains(".xml"));


    QString XMLPath = FileServices::getMosaicXMLFile(name);

    if (XMLPath.isEmpty())
    {
        QMessageBox box(this);
        box.setText(QString("File <%1>not found").arg(name));
        box.exec();
        return;
    }

    if (!QFile::exists(XMLPath))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("File <%1>not found").arg(XMLPath));
        box.exec();
        return;
    }

    bool rv = FileServices::reformatXML(XMLPath);
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

void page_loaders::showTilings()
{
    QString tilingName = FileServices::getTileNameFromDesignName(selecteMosaicName);

    QString results = "<pre>";
    results += tilingName;
    results += "<br>";
    results += "</pre>";

    QMessageBox box(this);
    box.setWindowTitle("Tiling used");
    box.setText(results);
    box.exec();

}

void page_loaders::openTiling()
{
    QString name = selectedTilingName;
    
    QString path = FileServices::getTilingXMLFile(name);
    if (!QFile::exists(path))
    {
        return;
    }
    QStringList qsl;
    qsl << path;
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
    QString name = selectedTilingName;

    // find where used
    QStringList used;
    int found = whereTilingUsed(name,used);

    // validate file to rebase exists
    QString oldPath = FileServices::getTilingXMLFile(name);
    if (oldPath.isEmpty())
    {
        QMessageBox box;
        box.setText(QString("File <%1>not found").arg(name));
        box.exec();
        return;
    }

    if (!QFile::exists(oldPath))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText("Could not find file to rebase");
        box.exec();
        return;
    }


    QStringList parts = name.split('.');
    if (parts.size() == 1)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Information);
        box.setText("Cannot rebase");
        box.exec();
        return;
    }

    QString old_vstring = parts.last();
    if (!old_vstring.contains('v'))
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Information);
        box.setText("Cannot rebase");
        box.exec();
        return;
    }

    QStringList allParts = parts;
    allParts.removeLast();
    QString nameRoot = allParts.join('.');

    old_vstring.remove('v');
    int old_ver = old_vstring.toInt();

    DlgRebase dlg(this);
    dlg.oldVersion->setValue(old_ver);
    dlg.newVersion->setValue(old_ver);

    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    int new_ver = dlg.newVersion->value();

    if (old_ver == new_ver)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Information);
        box.setText("Cannot rebase");
        box.exec();
        return;
    }

    if (new_ver == 0)
    {
        parts.removeLast();
    }
    else
    {
        QString newV = "v" + QString::number(new_ver);
        QString & lastV = parts.last();
        lastV = newV;
    }

    QString newName = parts.join('.');

    // this is the new file
    QString newPath = Sys::newTileDir + newName + ".xml";

    // purge other versions
    Q_ASSERT(new_ver < old_ver);

    QString aname;
    for (int i= new_ver; i < old_ver; i++)
    {
        if (i==0)
            aname = nameRoot;
        else
            aname = nameRoot + ".v" + QString::number(i);
        QString path = FileServices::getTilingXMLFile(aname);
        if (!path.isEmpty())
        {
            QFile::remove(path);
        }
    }

    // rename to new file
    QFile afile(oldPath);
    bool rv = afile.rename(newPath);
    if (!rv)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText("Rebase - FAILED during copy");
        box.exec();
        return;
    }

    // fix the name in the tiling file
    putNewTilingNameIntoTiling(newPath,newName);

    // fixup tiling name in designs
    if (found)
    {
        putNewTilingNameIntoDesign(used, newName);
    }

    slot_newTile(newName);
}

void page_loaders::renameTiling()
{
    QString name = selectedTilingName;

    // find where used
    QStringList used;
    int found = whereTilingUsed(name,used);

    // select new name
    bool fixupName = false;
    if (config->lastLoadedTiling == name)
        fixupName = true;

    DlgRename dlg(this);

    dlg.oldEdit->setText(name);
    dlg.newEdit->setText(name);

    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    // validate new name
    QString newName = dlg.newEdit->text();
    if (name == newName)
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText("Name not changed in dlg.  Must have a different name");
        box.exec();
        return;
    }

    Q_ASSERT(!newName.contains(".xml"));

    // validate old file exists
    QString oldPath = FileServices::getTilingXMLFile(name);
    if (oldPath.isEmpty())
    {
        return;
    }

    QFile theFile(oldPath);
    if (!theFile.exists())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText("Could not find existing file to rename");
        box.exec();
        return;
    }

    QString newPath = oldPath;
    newPath.replace(name,newName);

    // rename the the file
    bool rv = theFile.rename(newPath);
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
    rv = putNewTilingNameIntoTiling(newPath,newName);

    // fixup tiling name in  XML design files
    if (found)
    {
        putNewTilingNameIntoDesign(used, newName);
    }

    // report status
    QMessageBox box(this);
    if (rv)
    {
        QFile afile(oldPath);
        afile.remove();     // delete old file

        if (fixupName)
        {
            LoadUnit & loadUnit = view->getLoadUnit();
            loadUnit.setLoadState(LOADING_TILING,newName);
            loadUnit.resetLoadState();
        }

        slot_newTile(newName);

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
    QString name = selectedTilingName;

    QString msg;
    QStringList used;
    int count = whereTilingUsed(name,used);
    if (count)
    {
        msg = "<pre>";
        msg += "This tiling is used in:<br>";
        for (int i=0; i < used.size(); i++)
        {
            msg += used[i];
            msg += "<br>";
        }
        msg += "</pre>";
    }
    msg += QString("Delete file: %1.  Are you sure?").arg(name);

    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setText(msg);
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }

    QString path = FileServices::getTilingXMLFile(name);
    if (!QFile::exists(path))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("File <%1>not found").arg(path));
        box.exec();
        return;
    }

    bool rv = QFile::remove(path);

    // report status
    QMessageBox box2(this);
    if (rv)
    {
        loadTilingsCombo();
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
    QString name = selectedTilingName;

    QString path = FileServices::getTilingXMLFile(name);
    if (!QFile::exists(path))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("File <%1>not found").arg(path));
        box.exec();
        return;
    }

    bool rv = FileServices::reformatXML(path);
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

void  page_loaders::slot_whereTilingUsed()
{
    QString name = selectedTilingName;
    QStringList used;
    int found = whereTilingUsed(name,used);

    if (found == 0)
    {
        QMessageBox box(this);
        box.setWindowTitle(QString("Where tiling %1 is used ").arg(name));
        box.setText("Tiling NOT used");
        box.exec();
        return;
    }

    DlgListNameSelect dlg(used);
    int rv = dlg.exec();

    if (rv == QDialog::Accepted)
    {
        QString str = dlg.newEdit->text();
        QString name = FileServices::stripPath(str);
        name = name.remove(".xml");

        mosaicMaker->loadMosaic(name);

        if (!config->lockView)
        {
            panel->delegateView(VIEW_MOSAIC);
        }
        emit sig_refreshView();
    }
}

int page_loaders::whereTilingUsed(QString name, QStringList & results)
{
    QStringList designs = FileServices::getMosaicFiles();

    int found = 0;
    for (int i=0; i < designs.size(); i++)
    {
        QString designFile = designs[i];
        bool rv = FileServices::usesTilingInDesign(designFile,name);
        if (rv)
        {
            qDebug() <<  name << " found in "  << designFile;
            results  << designFile;
            found++;
        }
    }
    return found;
}

void page_loaders::putNewTilingNameIntoDesign(QStringList & designs, QString newName)
{
    for (auto it = designs.begin(); it != designs.end(); it++)
    {
        QString filename = *it;

        // load file
        xml_document doc;
        xml_parse_result result = doc.load_file(filename.toStdString().c_str());
        if (result == false)
        {
            qWarning().noquote() << result.description();
            continue;
        }

        // edit
        xml_node node1 = doc.child("vector");
        if (!node1) continue;
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
        xml_node node6 = node5.child("string");
        if (!node6) continue;

        std::string  namestr = newName.toStdString();
        node5.remove_child(node6);
        qDebug() << "old name =" << node6.child_value() << " new name = " << namestr.c_str();
        xml_node node7 = node5.prepend_child("string");
        node7.append_child(pugi::node_pcdata).set_value(namestr.c_str());

        // save file
        bool rv = doc.save_file(filename.toStdString().c_str());
        if (rv)
        {
            FileServices::reformatXML(filename);   // also timestamps
        }
        else
        {
            qWarning().noquote() << "Failed to reformat:" << filename;
        }
    }
}

bool  page_loaders::putNewTilingNameIntoTiling(QString filename, QString newName)
{
    // fix name in file and save to new name
    bool rv = false;
    xml_document doc;
    xml_parse_result result = doc.load_file(filename.toStdString().c_str());  // load file
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
            std::string  namestr = newName.toStdString();

            node1.remove_child(node2);

            qDebug() << "old name =" << node2.child_value() << " new name = " << namestr.c_str();
            xml_node node3 = node1.prepend_child("Name");
            node3.append_child(pugi::node_pcdata).set_value(namestr.c_str());

            rv = doc.save_file(filename.toStdString().c_str());  // save file
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
        rv = FileServices::reformatXML(filename);   // also timestamps
    }
    return rv;
}

void  page_loaders::autoLoadStylesClicked(bool enb)
{
    config->autoLoadStyles = enb;
}

void  page_loaders::autoLoadTilingClicked(bool enb)
{
    config->autoLoadTiling = enb;
}

void  page_loaders::autoLoadDesignsClicked(bool enb)
{
    config->autoLoadDesigns = enb;
}
