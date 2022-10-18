#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QFile>

#include "panels/page_loaders.h"
#include "misc/fileservices.h"
#include "misc/pugixml.hpp"
#include "tiledpatternmaker.h"
#include "legacy/design.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "settings/configuration.h"
#include "widgets/dlg_rebase.h"
#include "widgets/dlg_rename.h"
#include "widgets/layout_sliderset.h"
#include "panels/panel.h"

using namespace pugi;

Q_DECLARE_METATYPE(eSM_Event)

page_loaders::page_loaders(ControlPanel * apanel) : panel_page(apanel,"Load")
{
    selectedDesign = NO_DESIGN;

    setupUI();

    mosaicFilter->setText(config->mosaicFilter);
    mosaicFilterCheck->setChecked(config->mosaicFilterCheck);
    mosaicWorklistCheck->setChecked(config->mosaicWorklistCheck);
    mosaicOrigChk->setChecked(config->mosaicOrigCheck);
    mosaicNewChk->setChecked(config->mosaicNewCheck);
    mosaicTestChk->setChecked(config->mosaicTestCheck);

    tilingFilter->setText(config->tileFilter);
    tilingFilterCheck->setChecked(config->tileFilterCheck);
    tilingOrigChk->setChecked(config->tilingOrigCheck);
    tilingNewChk->setChecked(config->tilingNewCheck);
    tilingTestChk->setChecked(config->tilingTestCheck);

    // make connections before making selections
    makeConnections();

    // DAC designs
    loadDesignCombo();

    // Tiling
    loadTilingsCombo();

    // Mosaic
    loadMosaicCombo();
}

void page_loaders::setupUI()
{
#define LOADER_MAX_HEIGHT   601
    designList = new LoaderListWidget();
    designList->setFixedSize(201,LOADER_MAX_HEIGHT);
    designList->setSelectionMode(QAbstractItemView::SingleSelection);

    mosaicList = new VersionedListWidget();
    mosaicList->setFixedSize(231,LOADER_MAX_HEIGHT);
    mosaicList->setSelectionMode(QAbstractItemView::SingleSelection);

    tileList = new VersionedListWidget();
    tileList->setFixedSize(201,LOADER_MAX_HEIGHT);
    tileList->setSelectionMode(QAbstractItemView::SingleSelection);

    pbLoadShapes = new QPushButton("Load Fixed Design");
    pbLoadShapes->setFixedWidth(129);
    pbLoadShapes->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    pbLoadTiling = new QPushButton("Load Tiling");
    pbLoadTiling->setFixedWidth(129);
    pbLoadTiling->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    cbTilingLoadMulti   = new QCheckBox("Load Multiple");
    cbTilingLoadModify  = new QCheckBox("Load Modify");

    cbTilingLoadMulti->setChecked(config->loadTilingMulti);
    cbTilingLoadModify->setChecked(config->loadTilingModify);

    pbLoadXML = new QPushButton("Load Mosaic");
    pbLoadXML->setFixedWidth(129);
    pbLoadXML->setStyleSheet("QPushButton { background-color: yellow; color: red;}");

    cbAutoLoadMosaics = new QCheckBox("Auto-load");
    cbAutoLoadTiling  = new QCheckBox("Auto-load");
    cbAutoLoadDesigns = new QCheckBox("Auto-load");

    mosaicOrigChk = new QCheckBox("Original");
    mosaicNewChk  = new QCheckBox("New");
    mosaicTestChk = new QCheckBox("Test");

    tilingOrigChk = new QCheckBox("Original");
    tilingNewChk  = new QCheckBox("New");
    tilingTestChk = new QCheckBox("Test");

    tilingFilter = new QLineEdit();
    mosaicFilter = new QLineEdit();
    mosaicFilterCheck = new QCheckBox("Filter");
    mosaicWorklistCheck = new QCheckBox("Worklist");
    tilingFilterCheck = new QCheckBox("Filter");

    QLabel * dummy1 = new QLabel(" ");
    QLabel * dummy2 = new QLabel(" ");

    QVBoxLayout * designLayout = new QVBoxLayout;
    QVBoxLayout * mosaicLayout = new QVBoxLayout;
    QVBoxLayout * tilingLayout = new QVBoxLayout;

    // designs
    QHBoxLayout * hbox;
    QHBoxLayout * hbox2;
    QHBoxLayout * hbox3;
    QHBoxLayout * hbox4;

    // design patterns (shapes)
    hbox = new QHBoxLayout();
    hbox->addWidget(cbAutoLoadDesigns);
    hbox->addStretch();
    hbox->addWidget(pbLoadShapes);

    designLayout->addLayout(hbox);
    designLayout->addWidget(dummy1);
    designLayout->addWidget(dummy2);
    designLayout->addWidget(designList);

    // Mosaics
    hbox = new QHBoxLayout();
    hbox->addWidget(mosaicFilterCheck);
    hbox->addWidget(mosaicFilter);

    hbox2 = new QHBoxLayout();
    hbox2->addWidget(cbAutoLoadMosaics);
    hbox2->addStretch();
    hbox2->addWidget(pbLoadXML);

    hbox3 = new QHBoxLayout();
    hbox3->addWidget(mosaicOrigChk);
    hbox3->addWidget(mosaicNewChk);
    hbox3->addWidget(mosaicTestChk);
    hbox3->addWidget(mosaicWorklistCheck);

    mosaicLayout->addLayout(hbox2);
    mosaicLayout->addLayout(hbox3);
    mosaicLayout->addLayout(hbox);
    mosaicLayout->addWidget(mosaicList);

    // tiling
    hbox = new QHBoxLayout();
    hbox->addWidget(tilingFilterCheck);
    hbox->addWidget(tilingFilter);

    hbox2 = new QHBoxLayout();
    hbox2->addWidget(cbTilingLoadModify);
    hbox2->addWidget(cbTilingLoadMulti);

    hbox3 = new QHBoxLayout();
    hbox3->addWidget(cbAutoLoadTiling);
    hbox3->addStretch();
    hbox3->addWidget(pbLoadTiling);

    hbox4 = new QHBoxLayout();
    hbox4->addWidget(tilingOrigChk);
    hbox4->addWidget(tilingNewChk);
    hbox4->addWidget(tilingTestChk);
    hbox4->addStretch();

    tilingLayout->addLayout(hbox3);
    tilingLayout->addLayout(hbox4);
    tilingLayout->addLayout(hbox);
    tilingLayout->addLayout(hbox2);
    tilingLayout->addWidget(tileList);

    QGroupBox * designGroup = new QGroupBox();
    QGroupBox * mosaicGroup = new QGroupBox();
    QGroupBox * tilingGroup = new QGroupBox();
    designGroup->setLayout(designLayout);
    mosaicGroup->setLayout(mosaicLayout);
    tilingGroup->setLayout(tilingLayout);

    QHBoxLayout * loaderLayout = new QHBoxLayout;
    loaderLayout->addWidget(designGroup);
    loaderLayout->addWidget(mosaicGroup);
    loaderLayout->addWidget(tilingGroup);

    vbox->addLayout(loaderLayout);
    vbox->addStretch();

    cbAutoLoadMosaics->setChecked(config->autoLoadStyles);
    cbAutoLoadTiling->setChecked(config->autoLoadTiling);
    cbAutoLoadDesigns->setChecked(config->autoLoadDesigns);
}

void page_loaders::makeConnections()
{
    connect(theApp,         &TiledPatternMaker::sig_mosaicLoaded,   this,   &page_loaders::slot_mosaicLoaded);
    connect(theApp,         &TiledPatternMaker::sig_tilingLoaded,   this,   &page_loaders::slot_tilingLoaded);
    connect(theApp,         &TiledPatternMaker::sig_loadedDesign,   this,   &page_loaders::slot_loadedDesign);
    connect(theApp,         &TiledPatternMaker::sig_workListChanged,this,   &page_loaders::loadMosaicCombo);

    connect(theApp,         &TiledPatternMaker::sig_mosaicWritten,  this,   &page_loaders::slot_newXML);
    connect(theApp,         &TiledPatternMaker::sig_tilingWritten,  this,   &page_loaders::slot_newTile);

    connect(pbLoadXML,      &QPushButton::clicked,                  this,   &page_loaders::loadXML);
    connect(pbLoadTiling,   &QPushButton::clicked,                  this,   &page_loaders::loadTiling);
    connect(pbLoadShapes,   &QPushButton::clicked,                  this,   &page_loaders::loadShapes);
    connect(mosaicFilter,   &QLineEdit::textEdited,                 this,   &page_loaders::slot_mosaicFilter);
    connect(tilingFilter,   &QLineEdit::textEdited,                 this,   &page_loaders::slot_tilingFilter);
    connect(mosaicFilterCheck, &QCheckBox::clicked,                 this,   &page_loaders::slot_mosaicCheck);
    connect(mosaicWorklistCheck, &QCheckBox::clicked,               this,   &page_loaders::slot_mosaicWorklistCheck);
    connect(mosaicOrigChk,  &QCheckBox::clicked,                    this,   &page_loaders::slot_mosOrigCheck);
    connect(mosaicNewChk,   &QCheckBox::clicked,                    this,   &page_loaders::slot_mosNewCheck);
    connect(mosaicTestChk,  &QCheckBox::clicked,                    this,   &page_loaders::slot_mosTestCheck);
    connect(tilingOrigChk,  &QCheckBox::clicked,                    this,   &page_loaders::slot_tilingOrigCheck);
    connect(tilingNewChk,   &QCheckBox::clicked,                    this,   &page_loaders::slot_tilingNewCheck);
    connect(tilingTestChk,  &QCheckBox::clicked,                    this,   &page_loaders::slot_tilingTestCheck);
    connect(tilingFilterCheck, &QCheckBox::clicked,                 this,   &page_loaders::slot_tilingCheck);
    connect(this,           &page_loaders::sig_loadMosaic,          theApp, &TiledPatternMaker::slot_loadMosaic);
    connect(this,           &page_loaders::sig_loadTiling,          theApp, &TiledPatternMaker::slot_loadTiling);
    connect(this,           &page_loaders::sig_loadDesign,          theApp, &TiledPatternMaker::slot_loadDesign);
    connect(this,           &page_loaders::sig_buildDesign,         theApp, &TiledPatternMaker::slot_buildDesign);

  //connect(designList,     &QListWidget::currentItemChanged,       this,   &page_loaders::designSelected);
    connect(designList,     &QListWidget::itemClicked,              this,   &page_loaders::designClicked);
    connect(designList,     SIGNAL(rightClick(QPoint)),             this,   SLOT(desRightClick(QPoint)));

  //connect(mosaicList,     &QListWidget::currentItemChanged,       this,   &page_loaders::mosaicSelected);
    connect(mosaicList,     &QListWidget::itemClicked,              this,   &page_loaders::mosaicClicked);
    connect(mosaicList,     &LoaderListWidget::itemEntered,         this,   &page_loaders::slot_itemEnteredToolTip);
    connect(mosaicList,     SIGNAL(rightClick(QPoint)),             this,   SLOT(xmlRightClick(QPoint)));
    connect(mosaicList,     SIGNAL(leftDoubleClick(QPoint)),        this,   SLOT(loadXML()));
    connect(mosaicList,     SIGNAL(listEnter()),                    this,   SLOT(loadXML()));

    connect(tileList,       &QListWidget::currentItemChanged,       this,   &page_loaders::tilingSelected);
    connect(tileList,       &QListWidget::itemClicked,              this,   &page_loaders::tilingClicked);
    connect(tileList,       SIGNAL(rightClick(QPoint)),             this,   SLOT(tileRightClick(QPoint)));

    connect(cbAutoLoadMosaics,  &QCheckBox::clicked,                this,   &page_loaders::autoLoadStylesClicked);
    connect(cbAutoLoadTiling,   &QCheckBox::clicked,                this,   &page_loaders::autoLoadTilingClicked);
    connect(cbAutoLoadDesigns,  &QCheckBox::clicked,                this,   &page_loaders::autoLoadDesignsClicked);
    connect(cbTilingLoadMulti,        &QCheckBox::clicked,  [=]  { config->loadTilingMulti = cbTilingLoadMulti->isChecked(); });
    connect(cbTilingLoadModify,       &QCheckBox::clicked,  [=]  { config->loadTilingModify = cbTilingLoadModify->isChecked(); });
}

void page_loaders::slot_mosaicFilter(const QString & filter)
{
    config->mosaicFilter = filter;
    if (config->mosaicFilterCheck)
    {
        loadMosaicCombo();
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
    loadMosaicCombo();
}

void page_loaders::slot_mosaicWorklistCheck(bool check)
{
    config->mosaicWorklistCheck = check;
    loadMosaicCombo();
}

void page_loaders::slot_mosOrigCheck(bool check)
{
    config->mosaicOrigCheck = check;
    loadMosaicCombo();
}

void page_loaders::slot_mosNewCheck(bool check)
{
    config->mosaicNewCheck = check;
    loadMosaicCombo();
}

void page_loaders::slot_mosTestCheck(bool check)
{
    config->mosaicTestCheck = check;
    loadMosaicCombo();
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
    config->lastLoadedTileName = name;
    loadTilingsCombo();
}

void page_loaders::slot_newXML()
{
    loadMosaicCombo();
}

void page_loaders::loadXML()
{
    if (selectedXMLName.isEmpty())
    {
        return;
    }

    emit sig_loadMosaic(selectedXMLName,false);

    if (!config->lockView)
    {
       panel->selectViewer(VIEW_MOSAIC);
    }
    //view->setMouseMode
    emit sig_refreshView();
}

void page_loaders::loadTiling()
{
    switch (config->getViewerType())
    {
    case VIEW_TILING_MAKER:
        break;

    default:
        panel->selectViewer(VIEW_TILING); // delegate the view
        break;
    }

    eSM_Event mode;
    QString name = selectedTilingName;
    if (tilingMaker->isLoaded(name))
    {
        // same tiling is already loadeded - must be a reload
        mode = (config->loadTilingMulti) ? SM_RELOAD_MULTI : SM_RELOAD_SINGLE;
    }
    else if (config->loadTilingModify)
    {
       // it is a different tiling - but we want to modify - so is a reload too
       mode = (config->loadTilingMulti) ? SM_RELOAD_MULTI : SM_RELOAD_SINGLE;
    }
    else
    {
        // replace any tiling
        mode = (config->loadTilingMulti) ? SM_LOAD_MULTI : SM_LOAD_SINGLE;
    }

    emit sig_loadTiling(name,mode);

    emit sig_refreshView();
}

void page_loaders::loadShapes()
{
    emit sig_buildDesign(selectedDesign);

    if (!config->lockView)
    {
       panel->selectViewer(VIEW_DESIGN);
    }
    emit sig_refreshView();
}

void page_loaders::slot_mosaicLoaded(QString name)
{
    qDebug() << "page_loaders:: loaded XML:" << name;

    mosaicList->blockSignals(true);
    if (mosaicList->selectItemByName(name))
    {
        selectedXMLName = name;
    }
    else
    {
        selectedXMLName.clear();
    }
    mosaicList->blockSignals(false);

    // update the tiling too
    QString tileName = FileServices::getTileNameFromDesignName(name);
    slot_tilingLoaded(tileName);
}

void page_loaders::slot_tilingLoaded(QString name)
{
    qDebug() << "page_loaders:: loaded Tiling:" << name;
    tileList->blockSignals(true);
    if (tileList->selectItemByName(name))
    {
        selectedTilingName = name;
        config->lastLoadedTileName = name;
    }
    tileList->blockSignals(false);
}

void page_loaders::slot_loadedDesign(eDesign design)
{
    qDebug() << "page_loaders:: loaded Design:" << design;
    designList->blockSignals(true);
    if (designList->selectItemByValue(design))
    {
        selectedDesign =  design;
        config->lastLoadedDesignId = design;
    }
    designList->blockSignals(false);
}

void page_loaders::onEnter()
{
    QListWidgetItem * item;

    item = tileList->currentItem();
    tileList->scrollToItem(item);

    item = mosaicList->currentItem();
    mosaicList->scrollToItem(item);

    item = designList->currentItem();
    designList->scrollToItem(item);
}

void page_loaders::onRefresh()
{}

void page_loaders::loadDesignCombo()
{
    // DAC designs

    designList->blockSignals(true);

    QListWidgetItem * currentItem = nullptr;
    for (auto& it : config->availableDesigns.toStdMap())
    {
        eDesign id   = it.first;
        DesignPtr dp = it.second;
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(dp->getTitle());
        item->setData(Qt::UserRole,id);
        designList->addItem(item);
        if (config->lastLoadedDesignId == id)
        {
            currentItem = item;
        }
    }

    if (currentItem)
    {
        designList->setCurrentItem(currentItem);
        selectedDesign = static_cast<eDesign>(currentItem->data(Qt::UserRole).toInt());
    }

    designList->blockSignals(false);
}

void page_loaders::loadMosaicCombo()
{
    auto list = FileServices::getMosaicNames(LOAD_FILTERED);

    mosaicList->blockSignals(true);
    mosaicList->addItemList(list);
    mosaicList->blockSignals(false);
    if (mosaicList->selectItemByName(config->lastLoadedXML))
    {
        selectedXMLName = config->lastLoadedXML;
    }
    else
    {
        config->lastLoadedXML.clear();
    }
}

void page_loaders::loadTilingsCombo()
{
    tilingUses uses = FileServices::getTilingUses();

    QStringList qsl = FileServices::getTilingNames(LOAD_FILTERED);

    tileList->blockSignals(true);
    tileList->clear(); // erase

    //tileList->addItemList(qsl);
    int row = 0;
    bool dark = config->darkTheme;
    for (auto it = qsl.begin(); it != qsl.end(); it++, row++)
    {
        QString tileName = *it;
        int usecount = uses.count(tileName);
        tileList->addItem(tileName);
        if (usecount == 0)
        {
            if (dark)
                tileList->item(row)->setForeground(Qt::lightGray);
            else
                tileList->item(row)->setForeground(Qt::gray);
        }
    }

    if (tileList->selectItemByName(config->lastLoadedTileName))
    {
        selectedTilingName = config->lastLoadedTileName;
    }
    else
    {
        selectedTilingName.clear();
        config->lastLoadedTileName.clear();
    }
    tileList->blockSignals(false);
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
    loadTiling();
}

void page_loaders::mosaicSelected(QListWidgetItem *item, QListWidgetItem *oldItem)
{
    Q_UNUSED(oldItem)
    selectedXMLName = item->text();
}

void page_loaders::designClicked(QListWidgetItem * item)
{
    qDebug() << "page_loaders::designSelected";
    selectedDesign = static_cast<eDesign>(item->data(Qt::UserRole).toInt());
    emit sig_loadDesign(selectedDesign);
    panel->selectViewer(VIEW_DESIGN);
}

void page_loaders::tilingClicked(QListWidgetItem *item)
{
    selectedTilingName = item->text();
    loadTiling();
}

void page_loaders::mosaicClicked(QListWidgetItem *item)
{
    selectedXMLName = item->text();
}

void page_loaders::slot_itemEnteredToolTip(QListWidgetItem * item)
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
        xml_node n = doc.child("designNotes");
        if (n)
        {
            QString s = item->text() + ": " + n.child_value();
            item->setToolTip(s);
            return;
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
    myMenu.addSection(selectedXMLName);
    myMenu.addSection(FileServices::getTileNameFromDesignName(selectedXMLName));
    myMenu.addAction("Load",       this, SLOT(loadXML()));
    myMenu.addAction("View XML",   this, SLOT(openXML()));
    myMenu.addAction("Tiling used",this, SLOT(showTilings()));
    myMenu.addAction("Rename",     this, SLOT(renameXML()));
    myMenu.addAction("Rebase",     this, SLOT(rebaseXML()));
    myMenu.addAction("Delete",     this, SLOT(deleteXML()));

    myMenu.exec(mosaicList->mapToGlobal(pos));
}

void page_loaders::tileRightClick(QPoint pos)
{
    QMenu myMenu;
    myMenu.addSection(selectedTilingName);
    myMenu.addAction("Load",    this, SLOT(loadTiling()));
    myMenu.addAction("View XML",this, SLOT(openTiling()));
    myMenu.addAction("Rename",  this, SLOT(renameTiling()));
    myMenu.addAction("Rebase",  this, SLOT(rebaseTiling()));
    myMenu.addAction("Delete",  this, SLOT(deleteTiling()));
    myMenu.addAction("WhereUsed", this, SLOT(slot_whereTilingUsed()));

    myMenu.exec(tileList->mapToGlobal(pos));
}

void page_loaders::openXML()
{
    QString name = selectedXMLName;
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


void page_loaders::rebaseXML()
{
    QString name    = selectedXMLName;

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
    QString newXMLPath = config->newDesignDir + newName + ".xml";
    QString newDatPath = config->templateDir  + newName + ".dat";

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
    selectedXMLName = newName;
    if (name == config->lastLoadedXML)
        config->lastLoadedXML = newName;
    if (name == config->currentlyLoadedXML)
        config->currentlyLoadedXML = newName;
    slot_newXML();
    slot_mosaicLoaded(selectedXMLName);
}

void page_loaders::renameXML()
{
    QString name = selectedXMLName;

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
        selectedXMLName = newName;
        if (name == config->lastLoadedXML)
            config->lastLoadedXML = newName;
        if (name == config->currentlyLoadedXML)
            config->currentlyLoadedXML = newName;
        slot_mosaicLoaded(selectedXMLName);
        box.setText("Rename OK");
    }
    else
    {
        box.setIcon(QMessageBox::Critical);
        box.setText("Rename - FAILED");
    }
    box.exec();
}

void page_loaders::deleteXML()
{
    QString name = selectedXMLName;
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
        slot_mosaicLoaded(selectedXMLName);
        box2.setText("Deleted OK");
    }
    else
    {
        box2.setIcon(QMessageBox::Critical);
        box2.setText("Deletion- FAILED");
    }
    box2.exec();
}

void page_loaders::showTilings()
{
    QString tilingName = FileServices::getTileNameFromDesignName(selectedXMLName);

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

    QString path = FileServices::getTilingFile(name);
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
    QString oldPath = FileServices::getTilingFile(name);
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
    QString newPath = config->newTileDir + newName + ".xml";

    // purge other versions
    Q_ASSERT(new_ver < old_ver);

    QString aname;
    for (int i= new_ver; i < old_ver; i++)
    {
        if (i==0)
            aname = nameRoot;
        else
            aname = nameRoot + ".v" + QString::number(i);
        QString path = FileServices::getTilingFile(aname);
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
    if (config->lastLoadedTileName == name)
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
    QString oldPath = FileServices::getTilingFile(name);
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
            config->lastLoadedTileName = newName;

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

    QString path = FileServices::getTilingFile(name);
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

void  page_loaders::slot_whereTilingUsed()
{
    QString name = selectedTilingName;
    QStringList used;
    int found = whereTilingUsed(name,used);

    QString results = "<pre>";
    if (found)
    {
        for (int i=0; i < used.size(); i++)
        {
            QString designFile = used[i];
            results += designFile;
            results += "<br>";
        }
    }
    if (found == 0)
    {
        results += "<br><br>Tiling NOT used<br><br>";
    }
    results += "</pre>";

    QMessageBox box(this);
    box.setWindowTitle(QString("Where tiling %1 is used ").arg(name));
    box.setText(results);
    box.exec();

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
        if (!rv)
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
