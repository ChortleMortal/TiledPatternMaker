#include <QGroupBox>
#include <QCheckBox>
#include <QMessageBox>

#include "geometry/map.h"
#include "makers/map_editor/map_editor.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/fileservices.h"
#include "misc/qtapplog.h"
#include "mosaic/mosaic.h"
#include "mosaic/mosaic_reader.h"
#include "mosaic/mosaic_manager.h"
#include "panels/page_debug.h"
#include "panels/page_loaders.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "style/filled.h"
#include "tile/tile.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"
#include "tile/tiling_manager.h"
#include "viewers/viewcontrol.h"
#include "widgets/dlg_textedit.h"
#include "widgets/transparentwidget.h"

using std::make_shared;

typedef std::shared_ptr<class Filled>       FilledPtr;

page_debug:: page_debug(ControlPanel * cpanel)  : panel_page(cpanel,"Debug Tools")
{
    log = qtAppLog::getInstance();

    QGroupBox   * debug = createDebugSection();
    vbox->addWidget(debug);

    //setMaximumWidth(700);
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
    QPushButton * pbVerifyTiling        = new QPushButton("Verify Current Tiling");
    QPushButton * pbVerifyAllTilings    = new QPushButton("Verify All Tilings");
    QPushButton * pbExamineMosaic       = new QPushButton("Examine Current Mosaic");
    QPushButton * pbExamineAllMosaics   = new QPushButton("Examine All Mosaics");
    QPushButton * pTestA                = new QPushButton("Test A");
    QPushButton * pTestB                = new QPushButton("Test B");
    QCheckBox   * pDontPaint            = new QCheckBox("Dont't Paint");
    QCheckBox   * pDontTrap             = new QCheckBox("Dont't Trap Log");

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
    grid->addWidget(pDontPaint,            row,0);
    grid->addWidget(pDontTrap,             row,1);

    QVBoxLayout * vbox = new QVBoxLayout;;
    vbox->addLayout(grid);
    vbox->addWidget(maps);

    QGroupBox * debugGroup = new QGroupBox("Debug");
    debugGroup->setLayout(vbox);

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

    connect(pbRender,                 &QPushButton::clicked,     this,   &panel_page::sig_render);
    connect(pbClearView,              &QPushButton::clicked,     view,   &ViewControl::slot_unloadView);
    connect(pbClearMakers,            &QPushButton::clicked,     view,   &ViewControl::slot_unloadAll);

    connect(pDontPaint,               &QCheckBox::clicked,       view,   &ViewControl::slot_dontPaint);
    connect(pDontTrap,                &QCheckBox::clicked,       this,   &page_debug::slot_dontTrapLog);

    return debugGroup;
}

QGroupBox * page_debug::createVerifyMaps()
{
    QCheckBox * cbPopupErrors   = new QCheckBox("Pop-up Map Errors");
    QCheckBox * cbVerifyMaps    = new QCheckBox("Verify Maps");
    QCheckBox * cbVerifyDump    = new QCheckBox("Dump Maps");
    QCheckBox * cbVerifyVerbose = new QCheckBox("Verbose");
    QCheckBox * cbVerifyProtos  = new QCheckBox("Verify Protos");

    QGridLayout * gbox = new QGridLayout();
    gbox->setColumnStretch(2,1);
    gbox->addWidget(cbVerifyProtos,0,0);
    gbox->addWidget(cbVerifyMaps,0,1);
    gbox->addWidget(cbPopupErrors,1,0);
    gbox->addWidget(cbVerifyDump,1,1);
    gbox->addWidget(cbVerifyVerbose,1,2);

    QGroupBox * gbVerifyMaps = new QGroupBox("Map Verification");
    gbVerifyMaps->setLayout(gbox);

    cbPopupErrors->setChecked(config->verifyPopup);
    cbVerifyMaps->setChecked(config->verifyMaps);
    cbVerifyDump->setChecked(config->verifyDump);
    cbVerifyVerbose->setChecked(config->verifyVerbose);

    cbVerifyProtos->setChecked(config->verifyProtos);

    connect(cbVerifyProtos, &QCheckBox::clicked,    this,   &page_debug::slot_verifyProtosClicked);
    connect(cbVerifyMaps,   &QCheckBox::clicked,    this,   &page_debug::slot_verifyMapsClicked);
    connect(cbPopupErrors,  &QCheckBox::clicked,    this,   &page_debug::slot_verifypopupClicked);
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

void  page_debug::onRefresh()
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
    QStringList files = FileServices::getMosaicFiles();
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
    QStringList files = FileServices::getMosaicNames(ALL_MOSAICS);
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

    QStringList files = FileServices::getTilingNames(ALL_TILINGS);
    for (int i=0; i < files.size(); i++)
    {
        bool rv = false;

        QString name = files[i];

        TilingManager tm;
        TilingPtr tp = tm.loadTiling(name,TILM_LOAD_SINGLE);
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

void page_debug::slot_verifyTiling()
{
    // the strategy is to build a map and then verify that
    TilingPtr tiling = tilingMaker->getTilings().first();
    verifyTiling(tiling);
}

bool page_debug::verifyTiling(TilingPtr tiling)
{
    log->trap(true);
    qInfo().noquote() << "Verifying tiling :" << tiling->getName();
    bool rv = true;
    for (const auto & placedTile : tiling->getInTiling())
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

    QString name = tiling->getName();
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
        const QStringList & qsl = qtAppLog::getTrap();
        dlg.set(qsl);
        dlg.exec();
    }
    log->trap(false);
    log->trapClear();
    return rv;
}

void page_debug::identifyDuplicateTiles(TilingPtr tiling)
{
    qDebug() << "Looking for duplicates -" << tiling->getName();
    for (auto tile1 : tiling->getUniqueTiles())
    {
        for (const auto & tile2 : tiling->getUniqueTiles())
        {
            if (tile1 == tile2)
                continue;
            if (tile1->equals(tile2))
            {
                qInfo() << "DUP:" << tiling->getName() << tile1.get() << "equals" << tile2.get();
            }
        }
    }
    qDebug() << "Looking for duplicates - end";
}

void page_debug::slot_verifyAllTilings()
{
    qDebug() << "Verifying all tilings...";

    QStringList files = FileServices::getTilingNames(ALL_TILINGS);
    for (int i=0; i < files.size(); i++)
    {
        QString name = files[i];
        qInfo() << "==========" << name << "==========";
        QStringList used;
        int uses = page_loaders::whereTilingUsed(name,used);
        if (uses == 0)
        {
            qInfo() << "     not used";
            continue;
        }


        TilingManager tm;
        TilingPtr tp = tm.loadTiling(name,TILM_LOAD_SINGLE);
        if (!tp)
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Warning);
            box.setText(QString("Error loading tiling: %1").arg(name));
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

    QStringList files = FileServices::getMosaicFiles();
    for (int i=0; i < files.size(); i++)
    {
        QString name = files[i];
        qDebug() << "==========" << name << "==========";


        MosaicReader loader;
        MosaicPtr mosaic = loader.readXML(name);


        if (!mosaic)
        {
            QString str = QString("Load ERROR - %1").arg(loader.getFailMessage());
            QMessageBox box(ControlPanel::getInstance());
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
    QVector<TilingPtr> tilings = mosaic->getTilings();
    TilingPtr tiling = tilings.first();     // yeah only one for this exercise
    qInfo().noquote() << "tiling:" << tiling->getName();

    verifyTiling(tiling);

    const StyleSet & styles = mosaic->getStyleSet();
    for (auto style : styles)
    {
        FilledPtr fp = std::dynamic_pointer_cast<Filled>(style);
        if (fp)
        {
            qInfo() << fp->getStyleDesc() << "algorithm:" << fp->getAlgorithm();
        }

        log->suspend(true);
        MapPtr map = style->getMap();
        log->suspend(false);

        map->verify(true);

        if (map->hasIntersectingEdges())
        {
            qInfo() << style->getStyleDesc() << "has intersecting edges";
        }
        qDebug().noquote() << map->displayVertexEdgeCounts();
    }
}

void page_debug::slot_dontTrapLog(bool dont)
{
    config->dontTrapLog = dont;
}

void  page_debug::slot_testA()
{
    MapEditor * mep = MapEditor::getInstance();

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

    QPolygonF poly2 = Point::reflectPolygon(poly1,rline);
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
#if 1
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
}
