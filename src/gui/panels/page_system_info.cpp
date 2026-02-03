#include <QCheckBox>
#include <QTreeWidget>
#include <QApplication>
#include <QMessageBox>

#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "gui/top/splash_screen.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/crop_maker_view.h"
#include "gui/viewers/grid_view.h"
#include "gui/viewers/image_view.h"
#include "gui/viewers/map_editor_view.h"
#include "gui/viewers/motif_maker_view.h"
#include "gui/viewers/prototype_view.h"
#include "model/makers/crop_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/prototype_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/border.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/motif.h"
#include "model/prototypes/design_element.h"
#include "model/prototypes/prototype.h"
#include "model/settings/configuration.h"
#include "model/styles/filled.h"
#include "model/tilings/backgroundimage.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "page_system_info.h"
#include "sys/enums/edgetype.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/map.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/transform.h"
#include "sys/geometry/vertex.h"
#include "sys/qt/qtapplog.h"
#include "sys/qt/tpm_io.h"
#include "sys/qt/utilities.h"
#include "sys/sys.h"
#include "sys/tiledpatternmaker.h"

typedef std::shared_ptr<class Filled>       FilledPtr;

page_system_info::page_system_info(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_SYSTEM_INFO,"System Info")
{
    QPushButton * pbDump = new QPushButton("Dump");
    pbDump->setFixedWidth(101);

    chkExpand  = new QCheckBox("Expand All");
    chkExpand->setChecked(false);

    chkLock = new QCheckBox("lock");
    chkLock->setChecked(false);

    chkShowEpoly = new QCheckBox("show edges");
    chkShowEpoly->setChecked(false);

    QPushButton * pbRefresh = new QPushButton("Refresh");
    pbRefresh->setFixedWidth(101);

    QRadioButton * rMakers = new QRadioButton("Makers");
    QRadioButton * rAll    = new QRadioButton("All");
    rMakers->setChecked(!Sys::config->sysinfo_all);
    rAll->setChecked(Sys::config->sysinfo_all);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(pbDump);
    hbox->addSpacing(15);
    hbox->addWidget(chkExpand);
    hbox->addStretch();
    hbox->addWidget(rMakers);
    hbox->addWidget(rAll);
    hbox->addStretch();
    hbox->addWidget(chkShowEpoly);
    hbox->addWidget(chkLock);
    hbox->addWidget(pbRefresh);

    tree = new QTreeWidget();
    tree->setColumnCount(3);
    tree->setHeaderHidden(true);
    tree->setFixedWidth(PANEL_RHS_WIDTH -10);
    tree->setMinimumHeight(690);
    tree->setRootIsDecorated(true);
    tree->setItemsExpandable(true);

    vbox->addLayout(hbox);
    vbox->addWidget(tree);

    connect(pbDump,         &QPushButton::clicked,     this, &page_system_info::dumpTree);
    connect(pbRefresh,      &QPushButton::clicked,     this, &page_system_info::populateTree);
    connect(chkExpand,      &QPushButton::clicked,     this, &page_system_info::populateTree);
    connect(chkShowEpoly,   &QPushButton::clicked,     this, &page_system_info::populateTree);
    connect(rAll,           &QRadioButton::clicked,    this, [this] { Sys::config->sysinfo_all = true; populateTree(); });
    connect(rMakers,        &QRadioButton::clicked,    this, [this] { Sys::config->sysinfo_all = false; populateTree(); });
    connect(tree,           &QTreeWidget::itemClicked, this, &page_system_info::slot_itemClicked);
}

void  page_system_info::onRefresh()
{}

void page_system_info::onEnter()
{
    if (!chkLock->isChecked())
    {
        populateTree();
    }
}

void page_system_info::populateTree()
{
    Sys::splash->display("Collecting system Info.....");
    QApplication::setOverrideCursor(Qt::WaitCursor);
    tree->clear();

    if (Sys::config->sysinfo_all)
        populateAll();
    else
        populateMakers();

    if (chkExpand->isChecked())
        tree->expandAll();

    // adjust
    tree->setColumnWidth(0,201);
    tree->resizeColumnToContents(2);
    QApplication::restoreOverrideCursor();
    Sys::splash->remove();
}

void page_system_info::populateAll()
{
    qDebug().noquote() << "page_system_info::populateAll" << "- start";

    doMosaicMaker();

    doProtoypeMaker(MVD_PROTO,"Prototype Maker",false);

    doProtoypeMaker(MVD_DELEM,"Design Element Info",false);

    doTilingMaker(false);

    doBackgroundImage();

    doViews();

    doMapEditor();

    doCropMaker();

    doImageViewer();

    qDebug().noquote() << "page_system_info::populateAll" << "- end";
}

void page_system_info::populateMakers()
{
    qDebug().noquote() << "page_system_info::populateMakers" << "- start";

    doMosaicMakerSummary();

    doProtoypeMaker(MVD_PROTO,"Prototype Maker",true);

    doTilingMaker(true);

    qDebug().noquote() << "page_system_info::populateMakers" << "- end";
}

void page_system_info::doMosaicMaker()
{
    qDebug().noquote() << "page_system_info::doMosaicMaker" << "- start";

    // Mosaic maker
    int numStyles = 0;
    QString name = "No Mosaic";
    auto mositem = new QTreeWidgetItem();
    mositem->setText(0,"Mosaic Maker");
    MosaicPtr  mosaic = mosaicMaker->getMosaic();
    Q_ASSERT(mosaic);

    numStyles = mosaic->getStyleSet().size();
    name      = mosaic->getName().get();

    mositem->setText(1,QString("Styles: %1").arg(numStyles));
    mositem->setText(2,QString("Mosaic: %1").arg(name));
    tree->addTopLevelItem(mositem);
    tree->expandItem(mositem);

    populateStyles(mositem,mosaic);

    populateBorder(mositem,mosaic);

    crop = mosaic->getCrop();
    populateCrop(mositem,"Mosaic Crop",crop);

    crop = mosaic->getPainterCrop();
    populateCrop(mositem,"Painter Crop",crop);

    auto bip = mosaic->getBkgdImage();
    populateBackgroundImage(mositem, bip);

    qDebug().noquote() << "page_system_info::doMosaicMaker" << "- end";
}

void page_system_info::doMosaicMakerSummary()
{
    qDebug().noquote() << "page_system_info::doMosaicMakerSummary" << "- start";

    // Mosaic maker
    int numStyles = 0;
    QString name = "No Mosaic";
    auto sumitem = new QTreeWidgetItem();
    sumitem->setText(0,"Mosaic Maker");
    MosaicPtr  mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        numStyles = mosaic->getStyleSet().size();
        name      = mosaic->getName().get();
    }
    sumitem->setText(1,QString("Styles: %1").arg(numStyles));
    sumitem->setText(2,QString("Mosaic: %1").arg(name));
    tree->addTopLevelItem(sumitem);
    tree->expandItem(sumitem);

    populateStylesSummary(sumitem,mosaic);

    qDebug().noquote() << "page_system_info::doMosaicMakerSummary" << "- end";
}

void page_system_info::doProtoypeMaker(eMVDType type, QString name, bool summary)
{
    // Prototype Maker
    qDebug().noquote() << "page_system_info::doProtoypeMaker" << name << "- start";

    auto protos     = Sys::prototypeMaker->getPrototypes();
    auto selectedP  = Sys::prototypeMaker->getSelectedPrototype();

    auto dels       = Sys::prototypeMaker->getAllDELs();
    auto selectedD  = Sys::prototypeMaker->getSelectedDEL();

    // summary
    auto pitem = new QTreeWidgetItem;
    pitem->setText(0,name);
    pitem->setText(1,QString("Protos: %1").arg(protos.size()));
    int numD = 0;
    int numV = Sys::prototypeMaker->getSelectedDELs(type).count();
    for (auto & prototype : std::as_const(protos))
    {
        numD += prototype->numDesignElements();
    }
    pitem->setText(2,QString("Design Elements: %1 visible: %2").arg(numD).arg(numV));
    tree->addTopLevelItem(pitem);
    //tree->expandItem(item);

    // selected prototype
    QTreeWidgetItem * p2item = new QTreeWidgetItem;
    p2item->setText(0,"Selected Prototype");
    p2item->setText(1,addr(selectedP.get()));
    pitem->addChild(p2item);

    // selected design element
    p2item = new QTreeWidgetItem;
    p2item->setText(0,"Selected Design Element");
    p2item->setText(1,addr(selectedD.get()));
    pitem->addChild(p2item);

    // all protos
    for (auto & proto : std::as_const(protos))
    {
        QString state = Sys::prototypeMaker->isHidden(type,proto) ? "hidden" : "visible";
        populatePrototype(pitem,proto,"Prototype",state,summary);
    }

    if (!summary)
    {
        // all dels
        for (auto & del : std::as_const(dels))
        {
            QString state = Sys::prototypeMaker->isHidden(type,del) ? "hidden" : "visible";
            populateDEL(pitem, del, "Design Element",state,summary);
        }
    }
    qDebug().noquote() << "page_system_info::doProtoypeMaker" << "- end";
}

void page_system_info::doTilingMaker(bool summary)
{
    // Tiling Maker
    qDebug().noquote() << "page_system_info::doTilingMaker" << "- start";

    const QVector<TilingPtr> & tilings = tilingMaker->getTilings();

    auto tmitem = new QTreeWidgetItem;
    tmitem->setText(0,"Tiling Maker");
    tmitem->setText(1,QString("Tilings: %1").arg(tilings.size()));
    int tiles = 0;
    for (const auto & tiling : std::as_const(tilings))
    {
        tiles += tiling->unit().numIncluded();
    }
    tmitem->setText(2,QString("Placed Tiles: %1").arg(tiles));
    tree->addTopLevelItem(tmitem);
    tree->expandItem(tmitem);

    TilingPtr tp = tilingMaker->getSelected();
    populateTiling(tmitem,tp,"Selected Tiling",summary,true);

    for (auto & tiling : std::as_const(tilings))
    {
        populateTiling(tmitem,tiling,"Tiling",summary,false);
    }

    qDebug().noquote() << "page_system_info::doTilingMaker" << "- end";
}

void page_system_info::doBackgroundImage()
{
    // background image
    auto bk_item = new QTreeWidgetItem;
    bk_item->setText(0,"Background Image");
    bk_item->setText(1,sBkgdImageSource[Sys::currentBkgImage]);
    tree->addTopLevelItem(bk_item);
    auto bip = Sys::getBackgroundImageFromSource();
    if (bip && bip->isLoaded())
    {
        bk_item->setText(2, bip->getTitle());
        auto bk_item2 = new QTreeWidgetItem;
        bk_item2->setText(0,"Model Xform");
        bk_item2->setText(1, addr(bip.get()));
        bk_item2->setText(2,Transform::info(bip->getModelTransform()));
        bk_item->addChild(bk_item2);
        tree->expandItem(bk_item);
    }
    else
    {
        bk_item->setText(2, "none");
    }
}

void page_system_info::populateBackgroundImage(QTreeWidgetItem * parent, BkgdImagePtr bip)
{
    // background image
    auto bkitem = new QTreeWidgetItem;
    bkitem->setText(0,"Background Image");
    parent->addChild(bkitem);
    if (bip && bip->isLoaded())
    {
        bkitem->setText(1, addr(bip.get()));
        bkitem->setText(2, bip->getTitle());
        auto bkitem2 = new QTreeWidgetItem;
        bkitem2->setText(0,"Model Xform");
        bkitem2->setText(2,bip->getModelXform().info(8));
        bkitem->addChild(bkitem2);
        tree->expandItem(bkitem);
    }
    else
    {
        bkitem->setText(2, "none");
    }
}

void page_system_info::doViews()
{
    // views
    auto vitem = new QTreeWidgetItem;
    vitem->setText(0,"Views");
    tree->addTopLevelItem(vitem);
    populateViews(vitem);
}

void page_system_info::doMapEditor()
{
    // map editor
    auto meitem = new QTreeWidgetItem;
    meitem->setText(0,"Map Editor");
    auto maped   = Sys::mapEditor;
    auto mapedDb = maped->getDb();
    auto maps    = mapedDb->getDrawMaps();
    QString astring = QString("Draw Maps: %1").arg(maps.size());
    meitem->setText(1,astring);
    tree->addTopLevelItem(meitem);

    for (auto & map : std::as_const(maps))
    {
        populateMap(meitem,map,"Draw Map");
    }
}

void page_system_info::doCropMaker()
{
    // Crop maker

    QString s = sCropMaker[Sys::cropMakerView->getMakerType()];
    auto citem = new QTreeWidgetItem;
    citem->setText(0,"Crop Viewer");
    citem->setText(2, s);
    auto maker = Sys::cropMakerView->getMaker();
    if (maker)
    {
        crop = maker->getCrop();
        populateCrop(citem,s,crop);
    }
    tree->addTopLevelItem(citem);
}

void page_system_info::doImageViewer()
{
    // Crop maker
    auto ivitem = new QTreeWidgetItem;
    ivitem->setText(0,"Image Viewer");
    if (Sys::imageViewer->isLoaded())
    {
        const QPixmap & pp = Sys::imageViewer->getPixmap();
        QString str = QString("size %1 x %2").arg(pp.width()).arg(pp.height());
        ivitem->setText(2, str);
    }
    else
    {
        ivitem->setText(2,"Not loaded");
    }
    tree->addTopLevelItem(ivitem);
}

void page_system_info::slot_itemClicked(QTreeWidgetItem * item, int col)
{
    Q_UNUSED(col)

    item->setExpanded(!item->isExpanded());
    tree->resizeColumnToContents(2);
}

void page_system_info::populateStyles(QTreeWidgetItem * parent, MosaicPtr mosaic)
{
    bool summary = false;
    if (!mosaic)
    {
        return;
    }

    QTreeWidgetItem * item;

    QVector<TilingPtr> tilings = mosaic->getTilings();
    for (auto& tp : std::as_const(tilings))
    {
        item = new QTreeWidgetItem();
        item->setText(0,"Tiling");
        item->setText(1,addr(tp.get()));
        item->setText(2,tp->getVName().get());
        parent->addChild(item);
        populateTiling(item,tp,"Tiling",false,false);
    }

    const StyleSet & sset = mosaic->getStyleSet();
    for (auto & style : std::as_const(sset))
    {
        item = new QTreeWidgetItem;
        item->setText(0,"Style");
        item->setText(1,addr(style.get()));
        item->setText(2,style->getStyleDesc());
        parent->addChild(item);
        //tree->expandItem(item);

        if (style->getStyleType() == STYLE_FILLED)
        {
            FilledPtr fp = std::dynamic_pointer_cast<Filled>(style);
            Q_ASSERT(fp);
            ColorMaker * cm = fp->getColorMaker();
            QString astring = QString("Filled: algo=%1 blacks=%2 whites=%3")
                .arg(fp->getAlgorithm())
                .arg(cm->getBlackFaces().size())
                .arg(cm->getWhiteFaces().size());
            item->setText(2,astring);  // overwrites
        }

        Layer * layer = dynamic_cast<Layer*>(style.get());
        Q_ASSERT(layer);
        populateLayer(item,layer);
        auto proto = style->getPrototype();
        if (proto)
        {
            populateMap(item,proto->getExistingProtoMap(),"Style Map");
            populatePrototype(item,proto,"Prototype","",summary);
        }
    }
}

void page_system_info::populateStylesSummary(QTreeWidgetItem * parent, MosaicPtr mosaic)
{
    if (!mosaic)
    {
        return;
    }

    QTreeWidgetItem * item;

    QVector<TilingPtr> tilings = mosaic->getTilings();
    for (auto& tp : std::as_const(tilings))
    {
        item = new QTreeWidgetItem();
        item->setText(0,"Tiling");
        item->setText(1,addr(tp.get()));
        item->setText(2,tp->getVName().get());
        parent->addChild(item);
    }

    const StyleSet & sset = mosaic->getStyleSet();
    for (auto & style : std::as_const(sset))
    {
        item = new QTreeWidgetItem;
        item->setText(0,"Style");
        item->setText(1,addr(style.get()));
        item->setText(2,style->getStyleDesc());
        parent->addChild(item);

        auto proto = style->getPrototype();
        if (proto)
        {
            auto item2 = new QTreeWidgetItem();
            item2->setText(0,"Prototype");
            item2->setText(1,addr(proto.get()));
            item->addChild(item2);

            auto tp = proto->getTiling();
            item2 = new QTreeWidgetItem();
            item2->setText(0,"Tiling");
            item2->setText(1,addr(tp.get()));
            item2->setText(2,tp->getVName().get());
            item->addChild(item2);
        }
    }
}

void page_system_info::populateBorder(QTreeWidgetItem * parent, MosaicPtr mosaic)
{
    if (!mosaic)
    {
        return;
    }

    QTreeWidgetItem * item;
    BorderPtr bp = mosaic->getBorder();
    item = new QTreeWidgetItem();;
    item->setText(0,"Border");
    item->setText(1,addr(bp.get()));
    if (bp)
    {
        QString astring = QString("%1 %2 %3").arg(bp->getBorderTypeString()).arg(bp->getCropTypeString()).arg(bp->getContentString());
        item->setText(2,astring);
    }
    parent->addChild(item);
}

void page_system_info::populateCrop(QTreeWidgetItem * parent, QString name, CropPtr crop)
{
    QTreeWidgetItem * item = new QTreeWidgetItem();;
    item->setText(0,name);
    item->setText(1,addr(crop.get()));
    if (crop)
    {
        QString emb = (crop->getEmbed()) ? "Embed" : QString();
        QString app = (crop->getApply())  ? "Apply" : QString();
        item->setText(2,QString("%1 %2 %3").arg(crop->getContentString()).arg(emb).arg(app));
    }
    parent->addChild(item);
}

void page_system_info::populateLayer(QTreeWidgetItem * parent, Layer * layer)
{
    QTreeWidgetItem * item = new QTreeWidgetItem;
    item->setText(0,"Canvas Transform");
    auto ct = layer->getCanvasTransform();
    item->setText(2,Transform::info(ct));
    parent->addChild(item);

    item = new QTreeWidgetItem;
    item->setText(0,"Model Xform");
    const Xform & xf = layer->getModelXform();
    item->setText(2,xf.info(8));
    parent->addChild(item);

    QTransform tr;
    item = new QTreeWidgetItem;
    item->setText(0,"Model Transform");
    tr = layer->getModelTransform();
    item->setText(2,Transform::info(tr));
    parent->addChild(item);

    item = new QTreeWidgetItem;
    item->setText(0,"Layer Transform");
    tr = layer->getLayerTransform();
    item->setText(2,Transform::info(tr));
    parent->addChild(item);
}

void page_system_info::populateMap(QTreeWidgetItem *parent, MapPtr mp, QString name)
{
    //mp->dump();

    QTreeWidgetItem * item;

    item = new QTreeWidgetItem;
    item->setText(0,name);
    item->setText(1,addr((mp.get())));
    parent->addChild(item);
    tree->expandItem(item);

    if (mp)
    {
        item->setText(2,mp->info());
    }
    else
    {
        item->setText(2,"NO Map");
        return;
    }

    const QVector<VertexPtr> & vertices = mp->getVertices();
    const QVector<EdgePtr>   & edges    = mp->getEdges();

    QTreeWidgetItem * item2;
    item2 = new QTreeWidgetItem;
    item2->setText(0,"Vertices");
    item2->setText(1,QString("Count: %1").arg(mp->numVertices()));
    item->addChild(item2);
    //tree->expandItem(item2);

    NeighbourMap nmap(mp);

    for (int i=0; i< mp->numVertices(); i++)
    {
        VertexPtr v = vertices.at(i);
        QPointF pos = v->pt;
        QTreeWidgetItem * item3 = new QTreeWidgetItem;
        item3->setText(0,QString("(%1) %2").arg(i).arg(addr(v.get())));
        item3->setText(1,QString::number(pos.x()));
        item3->setText(2,QString::number(pos.y()));
        item2->addChild(item3);

        const NeighboursPtr neighbours = nmap.getNeighbours(v);
        for (auto & wedge : std::as_const(*neighbours))
        {
            EdgePtr edge = wedge.lock();
            QTreeWidgetItem * item4 = new QTreeWidgetItem;
            item4->setText(0,"Edge");
            item4->setText(1,QString::number(edges.indexOf(edge)));
            item3->addChild(item4);
        }
    }

    item2 = new QTreeWidgetItem;
    item2->setText(0,"Edges");
    item2->setText(1,QString("Count: %1").arg(mp->numEdges()));
    item->addChild(item2);
    //tree->expandItem(item2);

    for (int i=0; i< mp->numEdges(); i++)
    {
        EdgePtr e = edges.at(i);
        QTreeWidgetItem * item3 = new QTreeWidgetItem;
        item3->setText(0,QString("(%1) %2").arg(i).arg(addr(e.get())));
        if (e->getType() == EDGETYPE_CURVE)
            item3->setText(2, QString("from %1 to %2 %3").arg(vertices.indexOf(e->v1)).arg(vertices.indexOf(e->v2)).arg(sCurveType[e->getCurveType()]));
        else
            item3->setText(2, QString("from %1 to %2").arg(vertices.indexOf(e->v1)).arg(vertices.indexOf(e->v2)));
        item2->addChild(item3);
    }
}

void page_system_info::populatePrototype(QTreeWidgetItem *parent, ProtoPtr pp, QString name, QString state, bool summary)
{
    if  (!pp) return;

    TilingPtr tiling                 = pp->getTiling();
    QVector<DELPtr> & dels = pp->getDesignElements();

    // summary
    QTreeWidgetItem * pitem = new QTreeWidgetItem;
    pitem->setText(0,name);
    pitem->setText(1,addr(pp.get()));
    state += "  DELs: " + QString::number(dels.count());
    pitem->setText(2,state);
    parent->addChild(pitem);

    // tiling
    if (tiling)
    {
        populateTiling(pitem,tiling,"Tiling",summary,summary);
    }

    // design elements
    for (auto & del : std::as_const(dels))
    {
        populateDEL(pitem,del,"Design Element","",summary);
    }

    //tree->expandItem(pitem);

    if (!summary)
    {
        auto map = pp->getExistingProtoMap();
        populateMap(pitem,map,"ProtoMap");

        auto dcel = pp->getExistingDCEL();
        if (dcel && !dcel->isEmpty())
        {
            QTreeWidgetItem * item = new QTreeWidgetItem();
            item->setText(0,"DCEL");
            item->setText(1,addr((dcel.get())));
            item->setText(2,dcel->info());
            pitem->addChild(item);
        }

        auto crop = pp->getCrop();
        populateCrop(pitem,"Mosaic Crop",crop);
    }
}

void page_system_info::populateDEL(QTreeWidgetItem * parent, DELPtr del, QString name, QString state, bool summary)
{
    if (!del) return;

    QTreeWidgetItem * item2 = new QTreeWidgetItem;
    item2->setText(0,name);
    item2->setText(1,addr(del.get()));
    item2->setText(2,state);
    parent->addChild(item2);
    //tree->expandItem(item2);

    QTreeWidgetItem * item = new QTreeWidgetItem;
    TilePtr tile = del->getTile();
    item->setText(0,"Tile");
    item->setText(1, addr(tile.get()));
    item->setText(2, QString("Points: %1 Rot: %2 %3").arg(tile->numPoints())
                                                     .arg(tile->getRotation())
                                                     .arg((tile->isRegular()) ? "Regular" : "Irregular"));
    item2->addChild(item);

    item = new QTreeWidgetItem;
    MotifPtr motif = del->getMotif();
    item->setText(0,"Motif");
    item->setText(1,addr(motif.get()));
    QString astring = motif->getMotifDesc() + " " + motif->getMotifTypeString();
    item->setText(2,astring);
    item2->addChild(item);

    if (!summary)
    {
        for (ExtenderPtr ep : motif->getExtenders())
        {
            item = new QTreeWidgetItem();
            auto eb = ep->getExtendedBoundary();
            astring = QString("scale:%1 rot:%2 escale:%3").arg(motif->getMotifScale()).arg(motif->getMotifRotate()).arg(eb.getScale());
            item->setText(2,astring);
            item2->addChild(item);
            //tree->expandItem(item);
        }

        if (motif->isIrregular())
        {
            populateMap(item2,motif->getMotifMap(),"Tile Map");
        }
    }
}

void page_system_info::populateTiling(QTreeWidgetItem * parent, TilingPtr tp, QString name, bool summary, bool terse)
{
    Q_UNUSED(summary);

    if (!tp) return;

    const PlacedTiles tilingUnit = tp->unit().getIncluded();

    // summary
    QTreeWidgetItem * pitem = new QTreeWidgetItem;
    pitem->setText(0,name);
    pitem->setText(1,addr(tp.get()));
    QString astring = tp->getVName().get() + "   Tiles: " + QString::number(tilingUnit.size());
    astring += "   T1" + Utils::str(tp->hdr().getTrans1()) + " T2" + Utils::str(tp->hdr().getTrans2());
    pitem->setText(2,astring);
    parent->addChild(pitem);

    populateBackgroundImage(pitem, tp->getBkgdImage());

    if (terse)
    {
        return;
    }

    for (const auto & placedTile : std::as_const(tilingUnit))
    {
        QTransform tr = placedTile->getPlacement();

        QTreeWidgetItem * item = new QTreeWidgetItem;
        item->setText(0,"Placed Tile");
        item->setText(1,addr(placedTile.get()));
        item->setText(2,Transform::info(tr));

        auto ep = placedTile->getPlacedEdgePoly();
        populateEdgePoly(pitem,ep,item);


        TilePtr tile  = placedTile->getTile();
        QTreeWidgetItem * item2 = new QTreeWidgetItem;
        item2->setText(0,"Unique Tile");
        item2->setText(1, addr(tile.get()));
        item2->setText(2, QString("Points: %1 Rot: %2  scale: %3  %4").arg(tile->numPoints())
                              .arg(tile->getRotation())
                              .arg(tile->getScale())
                              .arg((tile->isRegular()) ? "Regular" : "Irregular"));
        auto & ep3 = tile->getEdgePoly();
        populateEdgePoly(pitem,ep3,item2);
    }
}

void page_system_info::populateEdgePoly(QTreeWidgetItem * parent, const EdgePoly & ep, QTreeWidgetItem * item)
{
    parent->addChild(item);

    if (!chkShowEpoly->isChecked())
        return;

    for (const EdgePtr & edge : ep.get())
    {
        QTreeWidgetItem * item2 = new QTreeWidgetItem;
        item2->setText(0,"edge");
        item2->setText(1,addr(edge.get()));
        QString txt = sEdgeType[edge->getType()];
        item2->setText(2,txt);
        item->addChild(item2);
    }
}

void page_system_info::populateViews(QTreeWidgetItem * parent)
{
    const QVector<Layer *> layers = Sys::viewController->getActiveLayers();

    for (Layer * layer : layers)
    {
        QTreeWidgetItem * item = new QTreeWidgetItem;
        item->setText(0,layer->layerName());
        item->setText(2,Transform::info(layer->getLayerTransform()));
        parent->addChild(item);
    }
}

void page_system_info::dumpTree()
{
    QDate date = QDate::currentDate();
    QString stamp =  QString::number(date.toJulianDay()) + QTime::currentTime().toString();
    stamp = stamp.replace(':', '.');
    QString fileName  = qtAppLog::getInstance()->logDir() + "wsDumpFile" + stamp + ".txt";
    qDebug() << "saving:" << fileName;

    QFile afile(fileName);
    if (!afile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QString astring = QString("Could not write to file: %1").arg(fileName);
        qDebug().noquote() << astring;
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setText(astring);
        box.exec();
        return;
    }

    QTextStream ts(&afile);
    ts.setRealNumberPrecision(16);

    ts << "== START TREE ==  " <<  qApp->applicationDirPath() << endl;
    dumpWalkTree(ts,tree->invisibleRootItem());
    ts << "== END   TREE ==" << endl;

    afile.close();

    QString astring = QString("Saved to file: %1").arg(fileName);
    qDebug().noquote() << astring;
    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setText(astring);
    box.exec();
}

void  page_system_info::dumpWalkTree(QTextStream & ts,  QTreeWidgetItem * item )
{
    ts << item->text(0) << "  " << item->text(1) << "  " << item->text(2) << endl;

    for( int i = 0; i < item->childCount(); ++i )
    {
        dumpWalkTree(ts, item->child(i));
    }
}
