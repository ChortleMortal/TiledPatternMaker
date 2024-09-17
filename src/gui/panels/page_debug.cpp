#include <QGroupBox>
#include <QCheckBox>
#include <QMessageBox>

#include "gui/map_editor/map_editor.h"
#include "gui/panels/page_debug.h"
#include "gui/top/controlpanel.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/debug_view.h"
#include "gui/widgets/dlg_textedit.h"
#include "gui/widgets/transparent_widget.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_manager.h"
#include "model/mosaics/mosaic_reader.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/styles/filled.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling_manager.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/map.h"
#include "sys/geometry/transform.h"
#include "sys/qt/qtapplog.h"
#include "sys/sys/fileservices.h"

using std::make_shared;

typedef std::shared_ptr<class Filled>       FilledPtr;

page_debug:: page_debug(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_DEBUG_TOOLS,"Debug Tools")
{
    log = qtAppLog::getInstance();

    QGroupBox   * debug = createDebugSection();
    QGroupBox   * maps  = createVerifyMaps();
    QGroupBox   * dbVma = creatDebugMapView();

    vbox->addWidget(debug);
    vbox->addWidget(maps);
    vbox->addWidget(dbVma);
    vbox->addStretch();
}

QGroupBox * page_debug::createDebugSection()
{

    QPushButton * pbVerifyTileNames     = new QPushButton("Verify Tile Names");
    QPushButton * pbReformatDesXMLBtn   = new QPushButton("Reformat All Design XML");
    QPushButton * pbReformatTileXMLBtn  = new QPushButton("Reformat All Tiling XML");
    QPushButton * pbReprocessDesXMLBtn  = new QPushButton("Reprocess All Design XML");
    QPushButton * pbReprocessTileXMLBtn = new QPushButton("Reprocess All Tiling XML");
    QPushButton * pbRender              = new QPushButton("Render");
    QPushButton * pbClearMakers         = new QPushButton("Clear Makers");
    QPushButton * pbClearView           = new QPushButton("Clear View");
    QPushButton * pbVerifyTiling        = new QPushButton("Verify Current Tiling");
    QPushButton * pbVerifyAllTilings    = new QPushButton("Verify All Tilings");
    QPushButton * pbExamineMosaic       = new QPushButton("Examine Current Mosaic");
    QPushButton * pbExamineAllMosaics   = new QPushButton("Examine All Mosaics");
    QPushButton * pTestA                = new QPushButton("Test A");
    QPushButton * pTestB                = new QPushButton("Test B");
    QCheckBox   * chkSuspendPaint       = new QCheckBox("Suspend View painting");
    QCheckBox   * chkDontTrap           = new QCheckBox("Dont't Trap Log");
    QCheckBox   * chkDontRefresh        = new QCheckBox("Dont't Refresh Menu");
    QCheckBox   * chkFlagA              = new QCheckBox("Flag A");
    QCheckBox   * chkFlagB              = new QCheckBox("Flag B");

    chkDontRefresh->setChecked(!Sys::updatePanel);

    QGridLayout * grid = new QGridLayout();
    grid->setHorizontalSpacing(11);

    int row = 0;
    grid->addWidget(pbReformatDesXMLBtn,   row,0);
    grid->addWidget(pbReprocessDesXMLBtn,  row,1);
    grid->addWidget(pbExamineAllMosaics,   row,3);
    grid->addWidget(pbExamineMosaic,       row,4);

    row++;
    grid->addWidget(pbReformatTileXMLBtn,  row,0);
    grid->addWidget(pbReprocessTileXMLBtn, row,1);
    grid->addWidget(pbVerifyTileNames,     row,2);
    grid->addWidget(pbVerifyAllTilings,    row,3);
    grid->addWidget(pbVerifyTiling,        row,4);

    row++;
    grid->addWidget(pbClearMakers,         row,0);
    grid->addWidget(pbClearView,           row,1);
    grid->addWidget(pbRender,              row,2);
    grid->addWidget(pTestA,                row,3);
    grid->addWidget(pTestB,                row,4);

    row++;
    grid->addWidget(chkSuspendPaint,         row,0);
    grid->addWidget(chkDontRefresh,          row,1);
    grid->addWidget(chkDontTrap,             row,2);
    grid->addWidget(chkFlagA,                row,3);
    grid->addWidget(chkFlagB,                row,4);

    QGroupBox * debugGroup = new QGroupBox("Debug");
    debugGroup->setLayout(grid);

    connect(pbReformatDesXMLBtn,      &QPushButton::clicked,     this,   &page_debug::slot_reformatDesignXML);
    connect(pbReprocessDesXMLBtn,     &QPushButton::clicked,     this,   &page_debug::slot_reprocessDesignXML);
    connect(pbExamineAllMosaics,      &QPushButton::clicked,     this,   &page_debug::slot_examineAllMosaics);
    connect(pbExamineMosaic,          &QPushButton::clicked,     this,   &page_debug::slot_examineMosaic);

    connect(pbReformatTileXMLBtn,     &QPushButton::clicked,     this,   &page_debug::slot_reformatTilingXML);
    connect(pbReprocessTileXMLBtn,    &QPushButton::clicked,     this,   &page_debug::slot_reprocessTilingXML);

    connect(pbVerifyTileNames,        &QPushButton::clicked,     this,   &page_debug::slot_verifyTilingNames);
    connect(pbVerifyTiling,           &QPushButton::clicked,     this,   &page_debug::slot_verifyTiling);
    connect(pbVerifyAllTilings,       &QPushButton::clicked,     this,   &page_debug::slot_verifyAllTilings);

    connect(pTestA,                   &QPushButton::clicked,     this,   &page_debug::slot_testA);
    connect(pTestB,                   &QPushButton::clicked,     this,   &page_debug::slot_testB);
    connect(chkFlagA,                 &QCheckBox::clicked,       this,   [](bool enb) { Sys::flagA = enb; });
    connect(chkFlagB,                 &QCheckBox::clicked,       this,   [](bool enb) { Sys::flagB = enb; });

    connect(pbRender,                 &QPushButton::clicked,     this,   &panel_page::sig_render);
    connect(pbClearView,              &QPushButton::clicked,     viewControl,   &ViewController::slot_unloadView);
    connect(pbClearMakers,            &QPushButton::clicked,     viewControl,   &ViewController::slot_unloadAll);
    
    connect(chkSuspendPaint,          &QCheckBox::clicked,       this,   [](bool enb) { Sys::view->debugSuspendPaint(enb); } );
    connect(chkDontTrap,              &QCheckBox::clicked,       this,   &page_debug::slot_dontTrapLog);
    connect(chkDontRefresh,           &QCheckBox::clicked,       this,   &page_debug::slot_dontRefresh);

    return debugGroup;
}

QGroupBox * page_debug::createVerifyMaps()
{
    QCheckBox * cbPopupErrors   = new QCheckBox("Pop-up Map Errors");
    QCheckBox * cbVerifyMaps    = new QCheckBox("Verify Maps");
    QCheckBox * cbBuildEmptyNM  = new QCheckBox("Build Empty Neighbour Maps");
    QCheckBox * cbVerifyDump    = new QCheckBox("Dump Maps");
    QCheckBox * cbVerifyVerbose = new QCheckBox("Verbose");
    QCheckBox * cbVerifyProtos  = new QCheckBox("Verify Protos");
    QCheckBox * cbUndupMerges   = new QCheckBox("Unduplicate Merges (slow)");

    QGridLayout * gbox = new QGridLayout();
    gbox->setColumnStretch(3,1);

    gbox->addWidget(cbVerifyProtos,0,0);
    gbox->addWidget(cbVerifyMaps,0,1);
    gbox->addWidget(cbBuildEmptyNM,0,3);

    gbox->addWidget(cbPopupErrors,1,0);
    gbox->addWidget(cbVerifyDump,1,1);
    gbox->addWidget(cbVerifyVerbose,1,2);
    gbox->addWidget(cbUndupMerges,1,3);

    QGroupBox * gbVerifyMaps = new QGroupBox("Map Verification");
    gbVerifyMaps->setLayout(gbox);

    cbPopupErrors->setChecked(config->verifyPopup);
    cbVerifyMaps->setChecked(config->verifyMaps);
    cbVerifyDump->setChecked(config->verifyDump);
    cbVerifyVerbose->setChecked(config->verifyVerbose);
    cbBuildEmptyNM->setChecked(config->buildEmptyNmaps);
    cbVerifyProtos->setChecked(config->verifyProtos);
    cbUndupMerges->setChecked(config->unDuplicateMerge);

    connect(cbVerifyProtos, &QCheckBox::clicked,    this,   &page_debug::slot_verifyProtosClicked);
    connect(cbVerifyMaps,   &QCheckBox::clicked,    this,   &page_debug::slot_verifyMapsClicked);
    connect(cbPopupErrors,  &QCheckBox::clicked,    this,   &page_debug::slot_verifypopupClicked);
    connect(cbVerifyDump,   &QCheckBox::clicked,    this,   &page_debug::slot_verifyDumpClicked);
    connect(cbVerifyVerbose,&QCheckBox::clicked,    this,   &page_debug::slot_verifyVerboseClicked);
    connect(cbBuildEmptyNM, &QCheckBox::clicked,    this,   &page_debug::slot_buildEmptyNMaps);
    connect(cbUndupMerges,  &QCheckBox::clicked,    this,   &page_debug::slot_unDupMerges);

    return gbVerifyMaps;
}

QGroupBox * page_debug::creatDebugMapView()
{
    chkDebugView      = new QCheckBox("Enable View");
    chkDBVvertices    = new QCheckBox("Show Vertices");
    chkDBCdirn        = new QCheckBox("Show Direction");
    chkArcCen         = new QCheckBox("Show Arc Centres");

    connect(chkDebugView,   &QCheckBox::clicked, this, &page_debug::slot_dbgViewClicked);
    connect(chkDBVvertices, &QCheckBox::clicked, this, &page_debug::slot_viewVerticesClicked);
    connect(chkDBCdirn,     &QCheckBox::clicked, this, &page_debug::slot_viewDirnClicked);
    connect(chkArcCen,      &QCheckBox::clicked, this, &page_debug::slot_viewArcCen);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(chkDebugView);
    hbox->addWidget(chkDBVvertices);
    hbox->addWidget(chkDBCdirn);
    hbox->addWidget(chkArcCen);
    hbox->addStretch();

    QPushButton * clearBtn = new QPushButton("Clear");
    debugMapStatus = new QLabel("");

    connect(clearBtn,      &QPushButton::clicked, this, &page_debug::slot_clearDebugMap);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(clearBtn);
    hbox2->addSpacing(5);
    hbox2->addWidget(debugMapStatus);
    hbox2->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout();
    vbox->addLayout(hbox);
    vbox->addLayout(hbox2);

    QGroupBox * gbDebugMapView = new QGroupBox("Debug Map");
    gbDebugMapView->setLayout(vbox);

    return gbDebugMapView;
}

void  page_debug::onEnter()
{
    auto dview = Sys::debugView;

    chkDebugView->blockSignals(true);
    chkDebugView->setChecked(dview->getShow());
    chkDebugView->blockSignals(false);

    chkDBVvertices->blockSignals(true);
    chkDBVvertices->setChecked(dview->getShowVertices());
    chkDBVvertices->blockSignals(false);

    chkDBCdirn->blockSignals(true);
    chkDBCdirn->setChecked(dview->getShowDirection());
    chkDBCdirn->blockSignals(false);

    chkArcCen->blockSignals(true);
    chkArcCen->setChecked(dview->getShowArcCentres());
    chkArcCen->blockSignals(false);
}

void page_debug::onExit()
{
}

void  page_debug::onRefresh()
{
    DebugMap * dmap = Sys::debugView->getMap();
    debugMapStatus->setText(dmap->info());
}

void page_debug::slot_verifyTilingNames()
{
    TilingManager tm;
    bool rv = tm.verifyTilingFiles();

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
    VersionFileList files = FileServices::getFiles(FILE_MOSAIC);
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

    VersionFileList files = FileServices::getFiles(FILE_TILING);
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
    VersionFileList files = FileServices::getMosaicFiles(ALL_MOSAICS);
    for (VersionedFile & file : files)
    {
        MosaicPtr mosaic = mosaicMaker->loadMosaic(file);
        if (mosaic)
        {
            bool rv = mosaicMaker->saveMosaic(mosaic,true);
            if (rv)
                goodDes++;
            else
                badDes++;
        }
        else
        {
            badDes++;
        }
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

    VersionFileList files = FileServices::getTilingFiles(ALL_TILINGS);
    for (VersionedFile & file : files)
    {
        bool rv = false;

        TilingManager tm;
        TilingPtr tp = tm.loadTiling(file,TILM_LOAD_SINGLE);
        if (tp)
        {
            Q_ASSERT(tp->getName().get() == file.getVersionedName().get());
            rv = tm.saveTiling(tp);
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

void page_debug::slot_verifyProtosClicked(bool enb)
{
    config->verifyProtos = enb;
}

void page_debug::slot_verifyMapsClicked(bool enb)
{
    config->verifyMaps = enb;
}

void page_debug::slot_verifyDumpClicked(bool enb)
{
    config->verifyDump = enb;
}

void page_debug::slot_verifypopupClicked(bool enb)
{
    config->verifyPopup = enb;
}

void page_debug::slot_verifyVerboseClicked(bool enb)
{
    config->verifyVerbose = enb;
}

void page_debug::slot_unDupMerges(bool enb)
{
    config->unDuplicateMerge = enb;
}

void page_debug::slot_buildEmptyNMaps(bool enb)
{
    config->buildEmptyNmaps = enb;
}

void page_debug::slot_verifyTiling()
{
    // the strategy is to build a map and then verify that
    TilingPtr tiling = tilingMaker->getTilings().first();
    verifyTiling(tiling);
}

bool page_debug::verifyTiling(TilingPtr tiling)
{
    log->trap(true);
    qInfo().noquote() << "Verifying tiling :" << tiling->getName().get();
    bool rv = true;
    TilingPlacements tilingUnit = tiling->getTilingUnitPlacements();
    for (const auto & placedTile : std::as_const(tilingUnit))
    {
        auto tile = placedTile->getTile();
        if (!tile->isClockwise())
        {
            qWarning() << "Clockwise Tile";
            rv = false;
        }
    }

    log->suspend(true);

    bool intrinsicOverlaps = tiling->hasIntrinsicOverlaps();
    bool tiledOverlaps     = tiling->hasTiledOverlaps();

    MapPtr protomap = tiling->debug_createProtoMap();

    log->suspend(false);

    bool protomap_verify   = protomap->verify(true);
    bool protomap_overlaps = protomap->hasIntersectingEdges();

    log->suspend(true);

    MapPtr fillmap = tiling->debug_createFilledMap();

    log->suspend(false);

    bool fillmap_verify   = fillmap->verify(true);
    bool fillmap_overlaps = fillmap->hasIntersectingEdges();

    QString name = tiling->getName().get();
    if (intrinsicOverlaps)
    {
        qWarning() << name << ": intrinsic overlaps";
        rv = false;
    }
    if (tiledOverlaps)
    {
        qWarning() << name << ": tiled overlaps";
        rv = false;
    }
    if (!protomap_verify)
    {
        qWarning() << name << ": proto map verify errors";
        rv = false;
    }
    if (protomap_overlaps)
    {
        qWarning() << name << ": proto map intersecting edges";
        rv = false;
    }
    if (!fillmap_verify)
    {
        qWarning() << name << ": fill map verify errors";
        rv = false;
    }
    if (fillmap_overlaps)
    {
        qWarning() << name << ": fill map intersecting edges";
        rv = false;
    }

    if (!rv)
    {
        DlgTextEdit dlg(this);
        const auto & msgs = qtAppLog::getTrap();
        dlg.set(msgs);
        dlg.exec();
    }
    log->trap(false);
    return rv;
}

void page_debug::identifyDuplicateTiles(TilingPtr tiling)
{
    qDebug() << "Looking for duplicates -" << tiling->getName().get();
    for (auto tile1 : tiling->getUniqueTiles())
    {
        for (const auto & tile2 : tiling->getUniqueTiles())
        {
            if (tile1 == tile2)
                continue;
            if (tile1->equals(tile2))
            {
                qInfo() << "DUP:" << tiling->getName().get() << tile1.get() << "equals" << tile2.get();
            }
        }
    }
    qDebug() << "Looking for duplicates - end";
}

void page_debug::slot_verifyAllTilings()
{
    qDebug() << "Verifying all tilings...";

    VersionFileList files = FileServices::getTilingFiles(ALL_TILINGS);
    for (VersionedFile & file : files)
    {
        qInfo() << "==========" << file.getVersionedName().get() << "==========";
        QStringList used;
        VersionFileList uses = FileServices::whereTilingUsed(file.getVersionedName());
        if (uses.count() == 0)
        {
            qInfo() << "     not used";
            continue;
        }

        TilingManager tm;
        TilingPtr tp = tm.loadTiling(file,TILM_LOAD_SINGLE);
        if (!tp)
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Warning);
            box.setText(QString("Error loading tiling: %1").arg(file.getVersionedName().get()));
            box.setStandardButtons(QMessageBox::Ok);
            box.exec();
            continue;
        }

        //verifyTiling(tp);
        identifyDuplicateTiles(tp);

    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("All tilings verification complete");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void page_debug::slot_examineAllMosaics()
{
    qDebug() << "Examining all Mosaics...";

    VersionFileList files = FileServices::getFiles(FILE_MOSAIC);
    for (VersionedFile & file : files)
    {
        qDebug() << "==========" << file.getVersionedName().get() << "==========";

        MosaicReader reader;
        MosaicPtr mosaic = reader.readXML(file);
        if (!mosaic)
        {
            QString str = QString("Load ERROR - %1").arg(reader.getFailMessage());
            QMessageBox box(panel);
            box.setIcon(QMessageBox::Warning);
            box.setText(str);
            box.exec();
            continue;
        }

        examineMosaic(mosaic);
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText("All Mosaics examined");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void page_debug::slot_examineMosaic()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    examineMosaic(mosaic);
}

void page_debug::examineMosaic(MosaicPtr mosaic)
{
    qInfo() << "examining mosaic :" << mosaic->getName().get() << "- START";

    QVector<TilingPtr> tilings = mosaic->getTilings();
    TilingPtr tiling = tilings.first();     // yeah only one for this exercise
    qInfo().noquote() << "tiling:" << tiling->getName().get();

    verifyTiling(tiling);

    const StyleSet & styles = mosaic->getStyleSet();
    for (auto & style : styles)
    {
        FilledPtr fp = std::dynamic_pointer_cast<Filled>(style);
        if (fp)
        {
            qInfo() << fp->getStyleDesc() << "algorithm:" << fp->getAlgorithm() << "proto" << style->getPrototype().get();
        }
        else
        {
            qInfo() << style->getStyleDesc() << "proto" << style->getPrototype().get();
        }
    }

    QVector<ProtoPtr> protos = mosaic->getPrototypes();
    for (auto prototype : protos)
    {
        qInfo() << "proto" << prototype.get();

        log->suspend(true);
        MapPtr map = prototype->getProtoMap();
        log->suspend(false);

        map->verify(true);

        if (map->hasIntersectingEdges())
        {
            qInfo() << prototype.get() << "has intersecting edges";
        }
        qDebug().noquote() << map->displayVertexEdgeCounts();
    }
    qInfo() << "examining mosaic :" << mosaic->getName().get() << "- END";
}

void page_debug::slot_dontTrapLog(bool dont)
{
    Sys::dontTrapLog = dont;
}

void page_debug::slot_dontRefresh(bool enb)
{
    Sys::updatePanel = !enb;
}

void  page_debug::slot_dbgViewClicked(bool checked)
{
    Sys::debugView->show(checked);
    emit sig_reconstructView();
}

void  page_debug::slot_viewDirnClicked(bool checked)
{
    Sys::debugView->showDirection(checked);
    emit sig_updateView();
}

void  page_debug::slot_viewArcCen(bool checked)
{
    Sys::debugView->showArcCentres(checked);
    emit sig_updateView();
}
void  page_debug::slot_viewVerticesClicked(bool checked)
{
    Sys::debugView->showVertices(checked);
    emit sig_reconstructView();
}

void page_debug::slot_clearDebugMap()
{
    DebugMap * dmap = Sys::debugView->getMap();
    dmap->wipeout();
}

void  page_debug::slot_testA()
{
    MapEditor * mep = Sys::mapEditor;

#if 1
    QPointF p0(0,0);
    QPointF p1(0,1);
    QPointF p2(1,1);
    QPointF p3(1,0);
#else
    QPointF p0(0,0.25);
    QPointF p1(0,0.75);
    QPointF p2(1,0.75);
    QPointF p3(1,0.25);
#endif

    //QPointF q0(1,0);
    //QPointF q1(1,1.0001);
    //QPointF q2(2,1);
    //QPointF q3(2,0);

#if 0

#if 1
    MapPtr map1 = make_shared<Map>("map1");
    VertexPtr v0 = map1->insertVertex(p0);
    VertexPtr v1 = map1->insertVertex(p1);
    VertexPtr v2 = map1->insertVertex(p2);
    VertexPtr v3 = map1->insertVertex(p3);
    map1->insertEdge(v0,v1);
    map1->insertEdge(v1,v2);
    map1->insertEdge(v2,v3);
    map1->insertEdge(v3,v0);
    map1->dumpMap(true);
    mep->loadFromMap(map1,MAPED_TYPE_CREATED);
#endif
#if 0
    MapPtr map2 = make_shared<Map>("map2");
    v0 = map2->insertVertex(q0);
    v1 = map2->insertVertex(q1);
    v2 = map2->insertVertex(q2);
    v3 = map2->insertVertex(q3);
    map2->insertEdge(v0,v1);
    map2->insertEdge(v1,v2);
    map2->insertEdge(v2,v3);
    map2->insertEdge(v3,v0);
    map2->dumpMap(true);
    mep->loadFromMap(map2,MAPED_TYPE_CREATED);
#endif
#else
    QPolygonF poly1;
    poly1 << p0 << p1 << p2 << p3 << p0;
    MapPtr map3 = make_shared<Map>("map3",poly1);
    mep->loadFromMap(map3,MAPED_TYPE_CREATED);

    //QLineF rline(QPointF(0.5,-3),QPointF(2,0));
    QLineF rline(QPointF(2,0),QPointF(2,1));
    
    QPolygonF poly2 = Geo::reflectPolygon(poly1,rline);
    MapPtr map4 = make_shared<Map>("map4",poly2);
    mep->loadFromMap(map4,MAPED_TYPE_CREATED);
#if 0
    QPolygonF poly2;
    poly2 << q0 << q1 << q2 << q3 << q0;

    if (poly2.intersects(poly1))
    {
        QPolygonF p3 = poly2.intersected(poly1);
        if (!p3.isEmpty())
        {
            qDebug() << "overlapping";
        }
        else
        {
            qDebug() << "touching";
        }
    }
    else
        qDebug() << "no intersect";
#endif
#endif
}

void  page_debug::slot_testB()
{
#if 1
    qDebug() << "tile / ins1 / ins / circum";

    int n = 5;
    TilePtr f;
    QTransform t;

    Tile atile(n);
    qDebug() << atile.getPoints();
    f = make_shared<Tile>(EdgePoly(atile.getPoints()));
    tilingMaker->addNewPlacedTile(make_shared<PlacedTile>(f,t));

    QPolygonF p;
    p = Geo::getInscribedPolygon(n);
    qDebug() << p;
    f = make_shared<Tile>(EdgePoly(p));
    tilingMaker->addNewPlacedTile(make_shared<PlacedTile>(f,t));

    p = Geo::getCircumscribedPolygon(n);
    qDebug() << p;
    f = make_shared<Tile>(EdgePoly(p));
    tilingMaker->addNewPlacedTile(make_shared<PlacedTile>(f,t));

#endif
#if 0
    QPointF p0(0,0);
    QPointF p1(0,2);
    QPointF p2(2.00005,2);
    QPointF p3(2,0);

    QPointF q0(1,1);
    QPointF q1(1,3);
    QPointF q2(3,3.00005);
    QPointF q3(3,1);

    MapEditor * mep = MapEditor::getInstance();

    MapPtr map1 = make_shared<Map>("map1");
    VertexPtr v0 = map1->insertVertex(p0);
    VertexPtr v1 = map1->insertVertex(p1);
    VertexPtr v2 = map1->insertVertex(p2);
    VertexPtr v3 = map1->insertVertex(p3);
    map1->insertEdge(v0,v1);
    map1->insertEdge(v1,v2);
    map1->insertEdge(v2,v3);
    map1->insertEdge(v3,v0);
    qDebug().noquote() << map1->namedSummary();

    MapPtr map2 = make_shared<Map>("map2");
    v0 = map2->insertVertex(q0);
    v1 = map2->insertVertex(q1);
    v2 = map2->insertVertex(q2);
    v3 = map2->insertVertex(q3);
    map2->insertEdge(v0,v1);
    map2->insertEdge(v1,v2);
    map2->insertEdge(v2,v3);
    map2->insertEdge(v3,v0);
    qDebug().noquote() << map2->namedSummary();

    mep->loadFromMap(map1,MAPED_TYPE_CREATED);
    mep->loadFromMap(map2,MAPED_TYPE_CREATED);
#endif
#if 0
    qDebug() << "Looking for duplicates - start";
    auto tiling = tilingMaker->getTilings().first();
    Q_ASSERT(tiling);
    for (auto tile1 : tiling->getUniqueTiles())
    {
        for (const auto & tile2 : tiling->getUniqueTiles())
        {
            if (tile1 == tile2)
                continue;
            if (tile1->equals(tile2))
            {
                qDebug() << tile1.get() << "equals" << tile2.get();
			}
        }
    }
    qDebug() << "Looking for duplicates - end";
#endif
#if 1
    auto mosaic  = mosaicMaker->getMosaic();
    qInfo() << "layer - start";
    for (const auto & style : mosaic->getStyleSet())
    {
        auto t = style->getCanvasTransform();
        qInfo().noquote() << Transform::info(t);
        
        t = style->getModelTransform();
        qInfo().noquote() << Transform::info(t);

        t = style->getLayerTransform();
        qInfo().noquote() << Transform::info(t);
    }
    qInfo() << "layer - end";
#endif
}
