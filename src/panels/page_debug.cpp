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

#include "panels/page_debug.h"
#include "base/fileservices.h"
#include "base/mosaic_manager.h"
#include "base/qtapplog.h"
#include "base/shared.h"
#include "base/tiledpatternmaker.h"
#include "geometry/map_cleanser.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "panels/dlg_name.h"
#include "panels/layout_sliderset.h"
#include "panels/panel.h"
#include "panels/versioned_list_widget.h"
#include "tapp/explicit_figure.h"
#include "tapp/prototype.h"
#include "tile/tiling.h"
#include "tile/tiling_manager.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"

page_debug:: page_debug(ControlPanel * cpanel)  : panel_page(cpanel,"Debug Tools")
{
     QGroupBox   * debug = createDebugSection();

     vbox->addWidget(debug);
}

QGroupBox * page_debug::createDebugSection()
{
    QGroupBox   * maps  = createVerifyMaps();

    QPushButton * pbVerifyTileNames     = new QPushButton("Verify Tile Names");
    QPushButton * pbReformatDesXMLBtn   = new QPushButton("Reformat All Design XML");
    QPushButton * pbReformatTileXMLBtn  = new QPushButton("Reformat All Tiling XML");
    QPushButton * pbReprocessDesXMLBtn  = new QPushButton("Reprocess All Design XML");
    QPushButton * pbReprocessTileXMLBtn = new QPushButton("Reprocess All Tiling XML");
    QPushButton * pbRender              = new QPushButton("Render");
    QPushButton * pbClearMakers         = new QPushButton("Clear Makers");
    QPushButton * pbClearView           = new QPushButton("Clear View");
    QPushButton * pbVerifyTiling        = new QPushButton("Verify Tiling");

    QGridLayout * grid = new QGridLayout();
    grid->setHorizontalSpacing(51);

    int row = 0;
    grid->addWidget(pbReformatDesXMLBtn,   row,0);
    grid->addWidget(pbReprocessDesXMLBtn,  row,1);
    grid->addWidget(pbReformatTileXMLBtn,  row,2);
    grid->addWidget(pbReprocessTileXMLBtn, row,3);

    row++;
    grid->addWidget(pbClearMakers,         row,0);
    grid->addWidget(pbClearView,           row,1);
    grid->addWidget(pbRender,              row,2);
    grid->addWidget(pbVerifyTileNames,     row,3);

    row++;
    grid->addWidget(pbVerifyTiling,        row,0);

    QVBoxLayout * vbox = new QVBoxLayout;;
    vbox->addLayout(grid);
    vbox->addWidget(maps);

    QGroupBox * debugGroup = new QGroupBox("Debug");
    debugGroup->setLayout(vbox);

    connect(pbVerifyTileNames,        &QPushButton::clicked,     this,   &page_debug::slot_verifyTilingNames);
    connect(pbReformatDesXMLBtn,      &QPushButton::clicked,     this,   &page_debug::slot_reformatDesignXML);
    connect(pbReformatTileXMLBtn,     &QPushButton::clicked,     this,   &page_debug::slot_reformatTilingXML);
    connect(pbReprocessDesXMLBtn,     &QPushButton::clicked,     this,   &page_debug::slot_reprocessDesignXML);
    connect(pbReprocessTileXMLBtn,    &QPushButton::clicked,     this,   &page_debug::slot_reprocessTilingXML);
    connect(pbVerifyTiling,           &QPushButton::clicked,     this,   &page_debug::slot_verifyTiling);
    connect(pbRender,                 &QPushButton::clicked,     this,   &panel_page::sig_render);
    connect(pbClearView,              &QPushButton::clicked,     vcontrol,  &ViewControl::slot_clearView);
    connect(pbClearMakers,            &QPushButton::clicked,     vcontrol,  &ViewControl::slot_clearMakers);

    return debugGroup;
}

QGroupBox * page_debug::createVerifyMaps()
{
    QGroupBox * gbVerifyMaps = new QGroupBox("Verify Maps");
    gbVerifyMaps->setCheckable(true);

    QCheckBox * cbVerifyDump    = new QCheckBox("Dump Map");
    QCheckBox * cbVerifyVerbose = new QCheckBox("Verbose");

    QHBoxLayout * hverBox = new QHBoxLayout();
    hverBox->addSpacing(15);
    hverBox->addWidget(cbVerifyDump);
    hverBox->addWidget(cbVerifyVerbose);
    hverBox->addStretch();

    gbVerifyMaps->setLayout(hverBox);

    gbVerifyMaps->setChecked(config->verifyMaps);
    cbVerifyDump->setChecked(config->verifyDump);
    cbVerifyVerbose->setChecked(config->verifyVerbose);

    connect(gbVerifyMaps,   &QGroupBox::clicked,    this,   &page_debug::slot_verifyMapsClicked);
    connect(cbVerifyDump,   &QCheckBox::clicked,    this,   &page_debug::slot_verifyDumpClicked);
    connect(cbVerifyVerbose,&QCheckBox::clicked,    this,   &page_debug::slot_verifyVerboseClicked);

    return gbVerifyMaps;
}

void  page_debug::onEnter()
{
}

void page_debug::onExit()
{
}

void  page_debug::refreshPage()
{
}

void page_debug::slot_verifyTilingNames()
{
    TilingManager tm;
    bool rv = tm.verifyNameFiles();

    QMessageBox box(this);
    if (rv)
    {
        box.setIcon(QMessageBox::Information);
        box.setText("Tiling Names Verified: OK");
    }
    else
    {
        box.setIcon(QMessageBox::Warning);
        box.setText("ERROR in verifying tiling names. See log");
    }
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}
void page_debug::slot_reformatDesignXML()
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setText("Reformat XML: this is very drastic. Are you sure?");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }

    qDebug() << "Reformatting designs...";

    int goodDes = 0;
    int badDes  = 0;
    QStringList files = FileServices::getDesignFiles();
    for (int i=0; i < files.size(); i++)
    {
        bool rv =  FileServices::reformatXML(files[i]);
        if (rv)
            goodDes++;
        else
            badDes++;
    }
    qDebug() << "Reformatted" << goodDes << "good designs, " << badDes << "bad designs";

    QMessageBox box2(this);
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reformat XML: %1 good designs, %2 bad designs").arg(goodDes).arg(badDes));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::slot_reformatTilingXML()
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setText("Reformat XML: this is very drastic. Are you sure?");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }
    int badTiles  = 0;
    int goodTiles = 0;

    qDebug() << "Reformatting tilings...";

    QStringList files = FileServices::getTilingFiles();
    for (int i=0; i < files.size(); i++)
    {
        bool rv =  FileServices::reformatXML(files[i]);
        if (rv)
            goodTiles++;
        else
            badTiles++;
    }
    qDebug() << "Reformatted" << goodTiles << "good tilings, " << badTiles << "bad tilings";

    QMessageBox box2(this);
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reformat XML: %1 good tilings, %2 bad tilings").arg(goodTiles).arg(badTiles));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::slot_reprocessDesignXML()
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setText("Reprocessing Design XML: this is very drastic. Are you sure?");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }

    qDebug() << "Reprocessing designs...";

    int goodDes = 0;
    int badDes  = 0;
    QStringList files = FileServices::getDesignNames();
    for (int i=0; i < files.size(); i++)
    {
        QString name = files[i];
        MosaicManager mm;
        bool rv = mm.loadMosaic(name);
        if (rv)
        {
            QString outfile;
            rv = mm.saveMosaic(name,outfile,true);
        }
        if (rv)
            goodDes++;
        else
            badDes++;
    }

    qDebug() << "Reprocessed" << goodDes << "good designs, " << badDes << "bad designs";

    QMessageBox box2(this);
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reprocess Design XML: %1 good designs, %2 bad designs").arg(goodDes).arg(badDes));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::slot_reprocessTilingXML()
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setText("Reprocssing Tiling XML: this is very drastic. Are you sure?");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() == QMessageBox::No)
    {
        return;
    }
    int badTiles  = 0;
    int goodTiles = 0;

    qDebug() << "Reprocessing tilings...";

    QStringList files = FileServices::getTilingNames();
    for (int i=0; i < files.size(); i++)
    {
        bool rv = false;

        QString name = files[i];

        TilingManager tm;
        TilingPtr tp = tm.loadTiling(name,SM_LOAD_SINGLE);
        if (tp)
        {
            Q_ASSERT(tp->getName() == name);
            rv = tm.saveTiling(name,tp);
        }
        if (rv)
            goodTiles++;
        else
            badTiles++;
    }
    qDebug() << "Reformatted" << goodTiles << "good tilings, " << badTiles << "bad tilings";

    QMessageBox box2(this);
    box2.setIcon(QMessageBox::Information);
    box2.setText(QString("Reformat XML: %1 good tilings, %2 bad tilings").arg(goodTiles).arg(badTiles));
    box2.setStandardButtons(QMessageBox::Ok);
    box2.exec();
}

void page_debug::slot_verifyMapsClicked(bool enb)
{
    config->verifyMaps = enb;
}

void page_debug::slot_verifyDumpClicked(bool enb)
{
    config->verifyDump = enb;
}

void page_debug::slot_verifyVerboseClicked(bool enb)
{
    config->verifyVerbose = enb;
}

void page_debug::slot_verifyTiling()
{
    TilingMaker * tm = TilingMaker::getInstance();
    TilingPtr tiling = tm->getTilings().first();

    // the strategy is to build a map and then verify that

    Prototype proto(tiling);

    QVector<FeaturePtr> uniqueFeatures = tiling->getUniqueFeatures();

    for (auto feature :  uniqueFeatures)
    {
        EdgePoly & ep = feature->getEdgePoly();
        MapPtr     fm = make_shared<Map>("feature map",ep);
        FigurePtr fig = make_shared<ExplicitFigure>(fm,FIG_TYPE_EXPLICIT_FEATURE,feature->numSides());
        DesignElementPtr  dep = make_shared<DesignElement>(feature, fig);
        proto.addElement(dep);
    }

    MapPtr map = proto.createProtoMap();

    bool rv = map->verifyMap("Tiling map",true);

    qDebug() << "tiling verify:" << rv;
}
