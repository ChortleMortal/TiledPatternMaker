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

#include "panels/page_loaders.h"
#include "panels/dlg_rename.h"
#include "panels/dlg_rebase.h"
#include "panels/panel.h"
#include "base/tiledpatternmaker.h"
#include "base/pugixml.hpp"
#include "base/fileservices.h"

using namespace pugi;

page_loaders::page_loaders(ControlPanel * panel) : panel_page(panel,"Loaders")
{
    selectedDesign = NO_DESIGN;

    setupUI();

    designFilter->setText(config->designFilter);
    tilingFilter->setText(config->tileFilter);
    designFilterCheck->setChecked(config->designFilterCheck);
    tilingFilterCheck->setChecked(config->tileFilterCheck);

    // make connections before making selections
    makeConnections();

    // DAC designs
    loadDesignCombo();

    // Tiling
    loadTilingsCombo();

    // XML
    loadXMLCombo();

    refreshPage();
}

void page_loaders::setupUI()
{
    designList = new LoaderListWidget();
    designList->setFixedSize(205,801);
    designList->setSelectionMode(QAbstractItemView::SingleSelection);

    xmlList = new LoaderListWidget();
    xmlList->setFixedSize(231,801);
    xmlList->setSortingEnabled(true);
    xmlList->setMouseTracking(true);
    xmlList->setSelectionMode(QAbstractItemView::SingleSelection);

    tileList = new LoaderListWidget();
    tileList->setFixedSize(201,801);
    tileList->setSortingEnabled(false);
    tileList->setSelectionMode(QAbstractItemView::SingleSelection);

    pbLoadShapes = new QPushButton("Load Shape Factories");
    pbLoadTiling = new QPushButton("Load Tiling");
    pbLoadXML    = new QPushButton("Load XML Design");

    tilingFilter = new QLineEdit();
    designFilter = new QLineEdit();
    designFilterCheck = new QCheckBox();
    tilingFilterCheck = new QCheckBox();

    QGridLayout * grid = new QGridLayout;

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(designFilterCheck);
    hbox->addWidget(designFilter);
    grid->addLayout(hbox,0,1);

    hbox = new QHBoxLayout();
    hbox->addWidget(tilingFilterCheck);
    hbox->addWidget(tilingFilter);
    grid->addLayout(hbox,0,2);

    grid->addWidget(pbLoadShapes,1,0);
    grid->addWidget(pbLoadXML,1,1);
    grid->addWidget(pbLoadTiling,1,2);

    grid->addWidget(designList,2,0);
    grid->addWidget(xmlList,2,1);
    grid->addWidget(tileList,2,2);

    vbox->addLayout(grid);
    vbox->addStretch();
}

void page_loaders::makeConnections()
{
    connect(maker,          &TiledPatternMaker::sig_loadedXML,    this, &page_loaders::slot_loadedXML);
    connect(maker,          &TiledPatternMaker::sig_loadedTiling, this, &page_loaders::slot_loadedTiling);
    connect(maker,          &TiledPatternMaker::sig_loadedDesign, this, &page_loaders::slot_loadedDesign);

    connect(maker,          &TiledPatternMaker::sig_newXML, this,    &page_loaders::slot_newXML);
    connect(workspace,      &Workspace::sig_newTiling,      this,    &page_loaders::slot_newTile);

    connect(pbLoadXML,      &QPushButton::clicked,          this,    &page_loaders::loadXML);
    connect(pbLoadTiling,   &QPushButton::clicked,          this ,   &page_loaders::loadTiling);
    connect(pbLoadShapes,   &QPushButton::clicked,          this ,   &page_loaders::loadShapes);
    connect(designFilterCheck, &QCheckBox::clicked,         this ,   &page_loaders::slot_designCheck);
    connect(tilingFilterCheck, &QCheckBox::clicked,         this ,   &page_loaders::slot_tilingCheck);
    connect(this,           &page_loaders::sig_loadXML,     maker,   &TiledPatternMaker::slot_loadXML);
    connect(this,           &page_loaders::sig_loadTiling,  maker,   &TiledPatternMaker::slot_loadTiling);
    connect(this,           &page_loaders::sig_loadDesign,  maker,   &TiledPatternMaker::slot_loadDesign);
    connect(this,           &page_loaders::sig_buildDesign, maker,   &TiledPatternMaker::slot_buildDesign);

    connect(designList,     &QListWidget::currentItemChanged, this,  &page_loaders::designSelected);
    connect(designList,     SIGNAL(rightClick(QPoint)),       this,  SLOT(desRightClick(QPoint)));

    connect(xmlList,        &QListWidget::currentItemChanged, this,  &page_loaders::xmlSelected);
    connect(xmlList,        &LoaderListWidget::itemEntered,   this,  &page_loaders::slot_itemEnteredToolTip);
    connect(xmlList,        SIGNAL(rightClick(QPoint)),       this,  SLOT(xmlRightClick(QPoint)));
    connect(xmlList,        SIGNAL(leftDoubleClick(QPoint)),  this,  SLOT(loadXML()));
    connect(xmlList,        SIGNAL(listEnter()),              this,  SLOT(loadXML()));

    connect(tileList,       &QListWidget::currentItemChanged, this,  &page_loaders::tilingSelected);
    connect(tileList,       SIGNAL(rightClick(QPoint)),       this, SLOT(tileRightClick(QPoint)));
}

void page_loaders::slot_designCheck(bool check)
{
    config->designFilter      = designFilter->text();
    config->designFilterCheck = check;
    loadXMLCombo();
}

void page_loaders::slot_tilingCheck(bool check)
{
    config->tileFilter      = tilingFilter->text();
    config->tileFilterCheck = check;
    loadTilingsCombo();
}

void page_loaders::slot_newTile()
{
    loadTilingsCombo();
}

void page_loaders::slot_newXML()
{
    loadXMLCombo();
}

void page_loaders::loadXML()
{
    if (selectedXMLName.isEmpty())
    {
        return;
    }
    emit sig_loadXML(selectedXMLName);
    if (config->viewerType == VIEW_DESIGN || config->viewerType == VIEW_TILING)
    {
        emit panel->sig_selectViewer(VIEW_DESIGN,DV_LOADED_STYLE);
    }
}

void page_loaders::slot_loadedXML(QString name)
{
    qDebug() << "page_loaders:: loaded XML:" << name;

    xmlList->blockSignals(true);
    xmlList->selectItemByName(name);
    selectedXMLName = name;
    xmlList->blockSignals(false);

    // update the tiling too
    QString tileName = FileServices::getTileNameFromDesignName(name);
    slot_loadedTiling(tileName);
}

void page_loaders::slot_loadedTiling(QString name)
{
    qDebug() << "page_loaders:: loaded Tiling:" << name;
    tileList->blockSignals(true);
    tileList->selectItemByName(name);
    selectedTilingName = name;
    config->lastLoadedTileName = name;
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
{}

void page_loaders::refreshPage()
{}

void page_loaders::loadDesignCombo()
{
    // DAC designs

    designList->blockSignals(true);

    QListWidgetItem * currentItem = nullptr;
    for (auto it : config->availableDesigns.toStdMap())
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

void page_loaders::loadXMLCombo()
{
    QStringList list;
    if (config->designFilterCheck &&  !config->designFilter.isEmpty())
    {
        list = FileServices::getFilteredDesignNames(config->designFilter);
    }
    else
    {
        list = FileServices::getDesignNames();
    }
    xmlList->blockSignals(true);
    xmlList->addItemList(list);
    xmlList->blockSignals(false);
    xmlList->selectItemByName(config->lastLoadedXML);
}

void page_loaders::loadTilingsCombo()
{
    tilingUses uses = FileServices::getTilingUses();

    QString filter   = tilingFilter->text();
    QStringList qsl;

    if (config->tileFilterCheck && !config->tileFilter.isEmpty())
    {
        qsl = FileServices::getFilteredTilingNames(config->tileFilter);
    }
    else
    {
        qsl = FileServices::getTilingNames();
    }

    tileList->blockSignals(true);
    tileList->clear(); // erase

    //tileList->addItemList(qsl);
    int row = 0;
    for (auto it = qsl.begin(); it != qsl.end(); it++, row++)
    {
        QString tileName = *it;
        int count = uses.count(tileName);
        tileList->addItem(tileName);
        if (count > 0)
        {
            tileList->item(row)->setForeground(Qt::darkBlue);
        }
        else
        {
            tileList->item(row)->setForeground(Qt::gray);
        }
    }
    tileList->selectItemByName(config->lastLoadedTileName);
    selectedTilingName = config->lastLoadedTileName;
    tileList->blockSignals(false);
}

void page_loaders::loadTiling()
{
    QString name = selectedTilingName;
    emit sig_loadTiling(name);
    if (config->viewerType != VIEW_TILIING_MAKER)
    {
        emit panel->sig_selectViewer(VIEW_TILING, TV_WORKSPACE);
    }
    emit sig_viewWS();
}

void page_loaders::loadShapes()
{
    if (config->viewerType == VIEW_DESIGN)
    {
        emit panel->sig_selectViewer(VIEW_DESIGN,DV_SHAPES);
    }
    emit sig_buildDesign(selectedDesign);
}

void page_loaders::designSelected(QListWidgetItem * item, QListWidgetItem* oldItem)
{
    Q_UNUSED(oldItem);
    qDebug() << "page_loaders::designSelected";
    selectedDesign = static_cast<eDesign>(item->data(Qt::UserRole).toInt());
    emit sig_loadDesign(selectedDesign);
}

void page_loaders::tilingSelected(QListWidgetItem *item, QListWidgetItem *oldItem)
{
    Q_UNUSED(oldItem);

    selectedTilingName = item->text();
    loadTiling();
}

void page_loaders::xmlSelected(QListWidgetItem *item, QListWidgetItem *oldItem)
{
    Q_UNUSED(oldItem);
    selectedXMLName = item->text();
}

void page_loaders::slot_itemEnteredToolTip(QListWidgetItem * item)
{
    QString s = item->text();
    //qDebug() << "slot_itemEntered" << s;

    QString file = FileServices::getDesignXMLFile(s);
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
    Q_UNUSED(pos);
}

void page_loaders::xmlRightClick(QPoint pos)
{
    QMenu myMenu;
    myMenu.addSection(selectedXMLName);
    myMenu.addSection(FileServices::getTileNameFromDesignName(selectedXMLName));
    myMenu.addAction("Load",    this, SLOT(loadXML()));
    myMenu.addAction("View XML",this, SLOT(openXML()));
    myMenu.addAction("Rename",  this, SLOT(renameXML()));
    myMenu.addAction("Rebase",  this, SLOT(rebaseXML()));
    myMenu.addAction("Delete",  this, SLOT(deleteXML()));

    myMenu.exec(xmlList->mapToGlobal(pos));
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
    myMenu.addAction("WhereUsed", this, SLOT(whereTilingUsed()));

    myMenu.exec(tileList->mapToGlobal(pos));
}

void page_loaders::openXML()
{
    QString name = selectedXMLName;
    QString path = FileServices::getDesignXMLFile(name);
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
#ifdef WIN32
    QString cmd = "C:/Program Files (x86)/Notepad++/notepad++.exe";
#else
    QString cmd = "gedit";
#endif
    qDebug() << "starting"  << cmd  << qsl;
    QProcess::startDetached(cmd,qsl);
}


void page_loaders::rebaseXML()
{
    QString name    = selectedXMLName;

    QString oldXMLPath = FileServices::getDesignXMLFile(name);
    QString oldDatPath = FileServices::getDesignTemplateFile(name);
    if (oldXMLPath.isEmpty())
    {
        QMessageBox box;
        box.setText(QString("File <%1>not found").arg(name));
        box.exec();
        return;
    }

    if (!QFile::exists(oldXMLPath))
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("Could not find file to rebase");
        box.exec();
        return;
    }

    DlgRebase dlg(this);

    QStringList parts = name.split('.');
    QString nameRoot = parts[0];
    QString old_ver;
    if (parts.size() > 1)
    {
        old_ver = parts[1];
    }

    dlg.oldVersion->setText(old_ver);
    dlg.newVersion->setText(old_ver);

    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    // specify new name
    QString new_ver = dlg.newVersion->text();
    QString newName;
    if (new_ver.isEmpty())
        newName = nameRoot;
    else
        newName = nameRoot + "." + new_ver;

    // this is the new file
    QString newXMLPath = config->newDesignDir + newName + ".xml";
    QString newDatPath = config->templateDir  + newName + ".dat";

    // purge other versions
    old_ver = old_ver.remove('v');
    new_ver = new_ver.remove('v');
    int i_old_ver = old_ver.toInt();
    int i_new_ver;
    if (new_ver.isEmpty())
        i_new_ver = 0;
    else
        i_new_ver = new_ver.toInt();
    Q_ASSERT(i_new_ver < i_old_ver);

    QString aname;

    for (int i= i_new_ver; i < i_old_ver; i++)
    {
        if (i==0)
            aname = nameRoot;
        else
            aname = nameRoot + ".v" + QString::number(i);
        QString path = FileServices::getDesignXMLFile(aname);
        if (!path.isEmpty())
        {
            QFile::remove(path);
        }
    }

    for (int i= i_new_ver; i < i_old_ver; i++)
    {
        if (i==0)
            aname = nameRoot;
        else
            aname = nameRoot + ".v" + QString::number(i);
        QString path = FileServices::getDesignTemplateFile(aname);
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
        QMessageBox box;
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
    slot_loadedXML(selectedXMLName);
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
        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setText("Name not changed in dlg.  Must have a different name");
        box.exec();
        return;
    }

    Q_ASSERT(!newName.contains(".xml"));

    bool rv = false;
    QString oldXMLPath = FileServices::getDesignXMLFile(name);
    QString oldDatPath = FileServices::getDesignTemplateFile(name);
    if (oldXMLPath.isEmpty())
    {
        return;
    }

    if (!QFile::exists(oldXMLPath))
    {
        QMessageBox box;
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
    QMessageBox box;
    if (rv)
    {
        selectedXMLName = newName;
        if (name == config->lastLoadedXML)
            config->lastLoadedXML = newName;
        if (name == config->currentlyLoadedXML)
            config->currentlyLoadedXML = newName;
        slot_loadedXML(selectedXMLName);
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

    QMessageBox box;
    box.setIcon(QMessageBox::Question);
    box.setText(QString("Delete file: %1.  Are you sure?").arg(name));
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }

    QString XMLPath = FileServices::getDesignXMLFile(name);
    QString DatPath = FileServices::getDesignTemplateFile(name);

    if (XMLPath.isEmpty())
    {
        QMessageBox box;
        box.setText(QString("File <%1>not found").arg(name));
        box.exec();
        return;
    }

    if (!QFile::exists(XMLPath))
    {
       QMessageBox box;
       box.setIcon(QMessageBox::Critical);
       box.setText(QString("File <%1>not found").arg(XMLPath));
       box.exec();
       return;
    }

    bool rv = QFile::remove(XMLPath);
    QFile::remove(DatPath);     // it is ok for this to fail

    slot_newXML();

    // report satus
    QMessageBox box2;
    if (rv)
    {
        slot_loadedXML(selectedXMLName);
        box2.setText("Deleted OK");
    }
    else
    {
        box2.setIcon(QMessageBox::Critical);
        box2.setText("Deletion- FAILED");
    }
    box2.exec();
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
#ifdef WIN32
    QString cmd = "C:/Program Files (x86)/Notepad++/notepad++.exe";
#else
    QString cmd = "gedit";
#endif
    qDebug() << "starting"  << cmd  << qsl;
    QProcess::startDetached(cmd,qsl);
}

void page_loaders::rebaseTiling()
{
    QString name = selectedTilingName;

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
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText("Could not find file to rebase");
        box.exec();
        return;
    }

    DlgRebase dlg(this);

    QStringList parts = name.split('.');
    QString nameRoot = parts[0];
    QString old_ver;
    if (parts.size() > 1)
    {
        old_ver = parts[1];
    }

    dlg.oldVersion->setText(old_ver);
    dlg.newVersion->setText(old_ver);

    int retval = dlg.exec();
    if (retval == QDialog::Rejected)
    {
        qDebug() << "Canceled";
        return;
    }
    Q_ASSERT(retval == QDialog::Accepted);

    // specify new name
    QString new_ver = dlg.newVersion->text();
    QString newName;
    if (new_ver.isEmpty())
        newName = nameRoot;
    else
        newName = nameRoot + "." + new_ver;

    // this is the new file
    QString newPath = config->newDesignDir + "/" + newName + ".xml";

    // purge other versions
    old_ver = old_ver.remove('v');
    new_ver = new_ver.remove('v');
    int i_old_ver = old_ver.toInt();
    int i_new_ver;
    if (new_ver.isEmpty())
        i_new_ver = 0;
    else
        i_new_ver = new_ver.toInt();
    Q_ASSERT(i_new_ver < i_old_ver);

    QString aname;
    for (int i= i_new_ver; i < i_old_ver; i++)
    {
        if (i==0)
            aname = nameRoot;
        else
            newName = nameRoot + ".v" + QString::number(i);
        QString path = FileServices::getTilingFile(newName);
        if (!path.isEmpty())
        {
            QFile::remove(path);
        }
    }

    // rename to new file
    QFile afile(oldPath);
    bool rv = afile.copy(newPath);
    if (!rv)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setText("Rebase - FAILED during copy");
        box.exec();
        return;
    }

    // delete old
    afile.remove();

    slot_newTile();
}

void page_loaders::renameTiling()
{
    QString name = selectedTilingName;

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
    QString newName = dlg.newEdit->text();
    if (name == newName)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setText("Name not changed in dlg.  Must have a different name");
        box.exec();
        return;
    }

    Q_ASSERT(!newName.contains(".xml"));

    bool rv = false;

    QString oldPath = FileServices::getTilingFile(name);
    if (oldPath.isEmpty())
    {
        return;
    }

    if (!QFile::exists(oldPath))
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setText("Could not find existing file to rename");
        box.exec();
        return;
    }

    QString newPath = oldPath;
    newPath.replace(name,newName);

    // fix name in file and save to new name
    xml_document doc;
    xml_parse_result result = doc.load_file(oldPath.toStdString().c_str());
    if (result == false)
    {
        qWarning() << result.description();
        rv = false;
    }
    else
    {
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

                rv = doc.save_file(newPath.toStdString().c_str());
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
    }

    // report status
    QMessageBox box;
    if (rv)
    {
        QFile afile(oldPath);
        afile.remove();     // delete old file

        if (fixupName)
            config->lastLoadedTileName = newName;

        slot_newTile();

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

    QMessageBox box;
    box.setIcon(QMessageBox::Question);
    box.setText(QString("Delete file: %1.  Are you sure?").arg(name));
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }

    QString path = FileServices::getTilingFile(name);
    if (!QFile::exists(path))
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("File <%1>not found").arg(path));
        box.exec();
        return;
    }

    bool rv = QFile::remove(path);

    slot_newTile();

    // report status
    QMessageBox box2;
    if (rv)
    {
        slot_newTile();
        box2.setText("Deleted OK");
    }
    else
    {
        box2.setIcon(QMessageBox::Critical);
        box2.setText("Deletion- FAILED");
    }
    box2.exec();
}

void  page_loaders::whereTilingUsed()
{
    QString name = selectedTilingName;

    QStringList designs = FileServices::getDesignFiles();

    int found = 0;

    QString results = "<pre>";

    for (int i=0; i < designs.size(); i++)
    {
        QString designFile = designs[i];
        bool rv = FileServices::usesTilingInDesign(designFile,name);
        if (rv)
        {
            qDebug() <<  name << " found in "  << designFile;
            results += designFile;
            results += "<br>";
            found++;
        }
    }
    if (found == 0)
    {
        results += "<br><br>Tiling NOT used<br><br>";
    }
    results += "</pre>";

    QMessageBox box;
    box.setWindowTitle(QString("Where tiling %1 is used ").arg(name));
    box.setText(results);
    box.exec();
}
