﻿#include <QCheckBox>
#include <QTreeWidget>
#include <QApplication>
#include <QMessageBox>

#include "page_system_info.h"
#include "motifs/motif.h"
#include "geometry/crop.h"
#include "geometry/edge.h"
#include "geometry/map.h"
#include "geometry/neighbours.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "makers/crop_maker/crop_maker.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/map_editor/map_editor.h"
#include "makers/map_editor/map_editor_db.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/border.h"
#include "misc/qtapplog.h"
#include "misc/tpm_io.h"
#include "misc/utilities.h"
#include "mosaic/design_element.h"
#include "mosaic/mosaic.h"
#include "makers/prototype_maker/prototype.h"
#include "style/filled.h"
#include "tile/tile.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"
#include "viewers/backgroundimageview.h"
#include "viewers/crop_view.h"
#include "viewers/motif_view.h"
#include "viewers/grid_view.h"
#include "viewers/map_editor_view.h"
#include "viewers/prototype_view.h"
#include "viewers/viewcontrol.h"
#include "viewers/tiling_view.h"

typedef std::shared_ptr<class Filled>       FilledPtr;

page_system_info::page_system_info(ControlPanel * cpanel)  : panel_page(cpanel,"System Info")
{
    QPushButton * pbDump = new QPushButton("Dump");
    pbDump->setFixedWidth(101);

    QPushButton * pbExpand = new QPushButton("Expand All");
    pbExpand->setFixedWidth(101);

    lockCheck = new QCheckBox("lock");
    lockCheck->setChecked(false);

    QPushButton * pbRefresh = new QPushButton("Refresh");
    pbRefresh->setFixedWidth(101);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(pbDump);
    hbox->addSpacing(15);
    hbox->addWidget(pbExpand);
    hbox->addStretch();
    hbox->addWidget(lockCheck);
    hbox->addWidget(pbRefresh);

    tree = new QTreeWidget();
    tree->setColumnCount(3);
    tree->setHeaderHidden(true);
    tree->setMinimumWidth(701);
    tree->setMinimumHeight(690);
    tree->setRootIsDecorated(true);
    tree->setItemsExpandable(true);

    AQVBoxLayout * box = new AQVBoxLayout();
    box->setSizeConstraint(QLayout::SetFixedSize);
    box->addLayout(hbox);
    box->addSpacing(7);
    box->addWidget(tree);

    AQWidget * widget = new AQWidget();
    widget->setLayout(box);

    vbox->addWidget(widget);

    connect(pbRefresh, &QPushButton::clicked,     this, &page_system_info::populateTree);
    connect(pbDump,    &QPushButton::clicked,     this, &page_system_info::dumpTree);
    connect(pbExpand,  &QPushButton::clicked,     this, &page_system_info::expandTree);
    connect(tree,      &QTreeWidget::itemClicked, this, &page_system_info::slot_itemClicked);
}

void  page_system_info::onRefresh()
{ }

void page_system_info::onEnter()
{
    if (!lockCheck->isChecked())
    {
        populateTree();
    }
}

void page_system_info::populateTree()
{
    qDebug() << "page_system_info::populateTree() - start";

    QApplication::setOverrideCursor(Qt::WaitCursor);

    tree->clear();

    doMosaicMaker();

    doProtoypeMaker(MVD_PROTO,"Prototype Info");
    doProtoypeMaker(MVD_DELEM,"Design Element Info");

    doTilingMaker();

    doBackgroundImage();

    doViews();

    doMapEditor();

    doCropMaker();

    // adjust
    tree->setColumnWidth(0,201);
    tree->resizeColumnToContents(2);

    QApplication::restoreOverrideCursor();

    qDebug() << "page_system_info::populateTree() - done";
}

void page_system_info::doMosaicMaker()
{
    qDebug() << "page_system_info::populateTree() - mosaics start";

    // Mosaic maker
    int numStyles = 0;
    QString name = "No Mosaic";
    item = new QTreeWidgetItem();
    item->setText(0,"Mosaic Maker");
    MosaicPtr  mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        numStyles = mosaic->getStyleSet().size();
        name      = mosaic->getName();
    }
    item->setText(1,QString("Styles: %1").arg(numStyles));
    item->setText(2,QString("Mosaic: %1").arg(name));
    tree->addTopLevelItem(item);
    //tree->expandItem(item);

    populateStyles(item,mosaic);

    populateBorder(item,mosaic);

    if (mosaic)
    {
        crop = mosaic->getCrop();
    }
    populateCrop(item,crop);

    qDebug() << "page_system_info::populateTree() - mosaics done";
}

void page_system_info::doProtoypeMaker(eMVDType type, QString name)
{
    // Prototype Maker
    qDebug() << "page_system_info::populateTree() -" << name << "start";

    PrototypeData * data = prototypeMaker->getProtoMakerData();

    auto protos     = data->getPrototypes();
    auto selectedP  = data->getSelectedPrototype();

    auto dels       = data->getDELs();
    auto selectedD  = data->getSelectedDEL();

    // summary
    item = new QTreeWidgetItem;
    item->setText(0,name);
    item->setText(1,QString("Protos: %1").arg(protos.size()));
    int numD = 0;
    int numV = data->getSelectedDELs(type).count();
    for (auto & prototype : protos)
    {
        numD += prototype->numDesignElements();
    }
    item->setText(2,QString("Design Elements: %1 visible: %2").arg(numD).arg(numV));
    tree->addTopLevelItem(item);

    // selected prototype
    populatePrototype(item,selectedP,"Selected Prototype","");

    // all protos
    for (auto & proto : protos)
    {
        QString state = data->isHidden(type,proto) ? "hidden" : "visible";
        populatePrototype(item,proto,"Prototype",state);
    }

    // selected design element
    populateDEL(item,selectedD,"Selected Design Element","");

    // all dels
    for (auto & del : dels)
    {
        QString state = data->isHidden(type,del) ? "hidden" : "visible";
        populateDEL(item, del, "Design Element",state);
    }
    qDebug() << "page_system_info::populateTree() -" << name <<  "done";
}

void page_system_info::doTilingMaker()
{
    // Tiling Maker
    qDebug() << "page_system_info::populateTree() - tilings start";

    const QVector<TilingPtr> & tilings = tilingMaker->getTilings();

    item = new QTreeWidgetItem;
    item->setText(0,"Tiling Maker");
    item->setText(1,QString("Tilings: %1").arg(tilings.size()));
    int tiles = 0;
    for (auto& tiling : tilings)
    {
        tiles += tiling->getData().countPlacedTiles();
    }
    item->setText(2,QString("Placed Tiles: %1").arg(tiles));
    tree->addTopLevelItem(item);

    TilingPtr tp = tilingMaker->getSelected();
    populateTiling(item,tp,"Selected Tiling");

    for (auto& tiling : tilings)
    {
        populateTiling(item,tiling,"Tiling");
    }

    qDebug() << "page_system_info::populateTree() - tilings done";
}
void page_system_info::doBackgroundImage()
{
    // background image
    item = new QTreeWidgetItem;
    item->setText(0,"Background Image");
    auto bip = BackgroundImageView::getInstance();
    if (bip && bip->isLoaded())
        item->setText(2, bip->getName());
    else
        item->setText(2, "none");
    tree->addTopLevelItem(item);
    item2 = new QTreeWidgetItem;
    item2->setText(0,"xf_image");
    item2->setText(2,Transform::toInfoString(bip->getCanvasXform().toQTransform()));
    item->addChild(item2);

#if 0
    // selected tile
    item = new QTreeWidgetItem;
    item->setText(0,"ProtoView Selected Tile");
    fp = PrototypeView::getSharedInstance()->getSelectedTile();
    if (fp)
    {
        item->setText(1,addr(fp.get()));
        item->setText(2, QString("Points: %1 Rot: %2").arg(fp->numPoints()).arg(fp->getRotation()));
    }
    else
    {
        item->setText(2, "none");
    }
    tree->addTopLevelItem(item);
#endif
}

void page_system_info::doViews()
{
    // views
    item = new QTreeWidgetItem;
    item->setText(0,"Views");
    populateViews(item);
    tree->addTopLevelItem(item);
}

void page_system_info::doMapEditor()
{
    // map editor
    item = new QTreeWidgetItem;
    item->setText(0,"Map Editor");
    auto maped   =  MapEditor::getInstance();
    auto mapedDb = maped->getDb();
    auto maps    = mapedDb->getDrawMaps();
    QString astring = QString("Draw Maps: %1").arg(maps.size());
    item->setText(1,astring);
    tree->addTopLevelItem(item);

    for (auto & map : maps)
    {
        populateMap(item,map,"Draw Map");
    }
}

void page_system_info::doCropMaker()
{
    // Crop maker
    auto viewer = CropViewer::getInstance();
    item = new QTreeWidgetItem;
    item->setText(0,"Crop Viewer");
    item->setText(2, (viewer->getShowCrop()) ? "active" : "inactive");
    auto maker  = viewer->getMaker();
    if (maker)
    {
        crop = maker->getCrop();
        populateCrop(item,crop);
    }
    tree->addTopLevelItem(item);
}

void page_system_info::slot_itemClicked(QTreeWidgetItem * item, int col)
{
    Q_UNUSED(col)

    if (item->isExpanded())
    {
        tree->collapseItem(item);
    }
    else
    {
        tree->expandItem(item);
    }

    tree->resizeColumnToContents(2);
}

void page_system_info::populateStyles(QTreeWidgetItem * parent, MosaicPtr mosaic)
{
    if (!mosaic)
    {
        return;
    }

    QTreeWidgetItem * item;

    QVector<TilingPtr> tilings = mosaic->getTilings();
    for (auto& tp : tilings)
    {
        item = new QTreeWidgetItem();
        item->setText(0,"Tiling");
        item->setText(1,addr(tp.get()));
        item->setText(2,tp->getName());
        parent->addChild(item);
        populateTiling(item,tp,"Tiling");
    }

    const StyleSet & sset = mosaic->getStyleSet();
    for (auto style : sset)
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
            QString astring = QString("Filled: algo=%1 blacks=%2 whites=%3")
                .arg(fp->getAlgorithm())
                .arg(fp->blackFaces.size())
                .arg(fp->whiteFaces.size());
            item->setText(2,astring);  // overwrites
        }

        Layer * layer = dynamic_cast<Layer*>(style.get());
        Q_ASSERT(layer);
        populateLayer(item,layer);
        populateMap(item,style->getExistingMap(),"Style Map");
        populatePrototype(item,style->getPrototype(),"Prototype","");
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
        QString astring = QString("%1 %2 %3").arg(bp->getBorderTypeString()).arg(bp->getCropTypeString()).arg(bp->getCropString());
        item->setText(2,astring);
    }
    parent->addChild(item);
}

void page_system_info::populateCrop(QTreeWidgetItem * parent, CropPtr crop)
{
    QTreeWidgetItem * item = new QTreeWidgetItem();;
    item->setText(0,"Crop");
    item->setText(1,addr(crop.get()));
    if (crop)
    {
        QString emb = (crop->getEmbed()) ? "Embed" : QString();
        QString app = (crop->getApply())  ? "Apply" : QString();
        item->setText(2,QString("%1 %2 %3").arg(crop->getCropString()).arg(emb).arg(app));
    }
    parent->addChild(item);
}

void page_system_info::populateLayer(QTreeWidgetItem * parent, Layer * layer)
{
    QTransform tr = layer->getFrameTransform();
    QTreeWidgetItem * item = new QTreeWidgetItem;
    item->setText(0,"Frame Transform");
    item->setText(2,Transform::toInfoString(tr));
    parent->addChild(item);

    tr = layer->getCanvasTransform();
    item = new QTreeWidgetItem;
    item->setText(0,"Canvas Transform");
    item->setText(2,Transform::toInfoString(tr));
    parent->addChild(item);

    tr = layer->getLayerTransform();
    item = new QTreeWidgetItem;
    item->setText(0,"Layer Transform");
    item->setText(2,Transform::toInfoString(tr));
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
        item->setText(2,mp->name());
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

    for (int i=0; i< mp->numVertices(); i++)
    {
        VertexPtr v = vertices.at(i);
        QPointF pos = v->pt;
        QTreeWidgetItem * item3 = new QTreeWidgetItem;
        item3->setText(0,QString("(%1) %2").arg(i).arg(addr(v.get())));
        item3->setText(1,QString::number(pos.x()));
        item3->setText(2,QString::number(pos.y()));
        item2->addChild(item3);

        const NeighboursPtr neighbours = mp->getNeighbours(v);
        for (auto & wedge : *neighbours)
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
        if (e->getType() == EDGETYPE_CURVE || e->getType() == EDGETYPE_CHORD)
            item3->setText(2, QString("from %1 to %2 %3").arg(vertices.indexOf(e->v1)).arg(vertices.indexOf(e->v2)).arg(e->isConvex() ? "convex" : "concave"));
        else
            item3->setText(2, QString("from %1 to %2").arg(vertices.indexOf(e->v1)).arg(vertices.indexOf(e->v2)));
        item2->addChild(item3);
    }
}

void page_system_info::populatePrototype(QTreeWidgetItem *parent, ProtoPtr pp, QString name, QString state)
{
    if  (!pp) return;

    TilingPtr tiling                 = pp->getTiling();
    QVector<DesignElementPtr> & dels = pp->getDesignElements();

    // summary
    QTreeWidgetItem * pitem = new QTreeWidgetItem;
    pitem->setText(0,name);
    pitem->setText(1,addr(pp.get()));
    pitem->setText(2,state);
    parent->addChild(pitem);

    // tiling
    if (tiling)
    {
        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setText(0,"Tiling");
        item->setText(1,addr(tiling.get()));
        item->setText(2,tiling->getName());
        pitem->addChild(item);
        populateTiling(item,tiling,"Tiling");
    }

    // design elements
    QTreeWidgetItem * item = new QTreeWidgetItem();
    item->setText(0,"Design Elements");
    item->setText(1,QString("Count: %1").arg(dels.count()));
    pitem->addChild(item);
    tree->expandItem(item);
    for (auto& del : dels)
    {
        populateDEL(item,del,"Design Element","");
    }

    auto map = pp->getExistingProtoMap();
    populateMap(pitem,map,"ProtoMap");

    auto crop = pp->getCrop();
    populateCrop(pitem,crop);

    //tree->expandItem(pitem);
}

void page_system_info::populateDEL(QTreeWidgetItem * parent, DesignElementPtr de, QString name, QString state)
{
    if (!de) return;

    QTreeWidgetItem * item2 = new QTreeWidgetItem;
    item2->setText(0,name);
    item2->setText(1,addr(de.get()));
    item2->setText(2,state);
    parent->addChild(item2);
    //tree->expandItem(item2);

    QTreeWidgetItem * item = new QTreeWidgetItem;
    TilePtr tile = de->getTile();
    item->setText(0,"Tile");
    item->setText(1, addr(tile.get()));
    item->setText(2, QString("Points: %1 Rot: %2 %3").arg(tile->numPoints())
                                                     .arg(tile->getRotation())
                                                     .arg((tile->isRegular()) ? "Regular" : "Irregular"));
    item2->addChild(item);
    //tree->expandItem(item);

    item = new QTreeWidgetItem;
    MotifPtr motif = de->getMotif();
    item->setText(0,"Motif");
    item->setText(1,addr(motif.get()));
    QString astring = motif->getMotifDesc() + " " + motif->getMotifTypeString();
    item->setText(2,astring);
    item2->addChild(item);
    //tree->expandItem(item);

    item = new QTreeWidgetItem();
    auto eb = motif->getExtendedBoundary();
    astring = QString("scale:%1 rot:%2 escale:%3").arg(motif->getMotifScale()).arg(motif->getMotifRotate()).arg(eb.getScale());
    item->setText(2,astring);
    item2->addChild(item);
    //tree->expandItem(item);

    if (motif->isExplicit())
    {
        populateMap(item2,motif->getMotifMap(),"Tile Map");
    }
}

void page_system_info::populateTiling(QTreeWidgetItem * parent, TilingPtr tp, QString name)
{
    if (!tp) return;

    const PlacedTiles & placedTiles = tp->getData().getPlacedTiles();

    // summary
    QTreeWidgetItem * pitem = new QTreeWidgetItem;
    pitem->setText(0,name);
    pitem->setText(1,addr(tp.get()));
    QString astring = tp->getName() + " - Tiles: " + QString::number(placedTiles.size());
    astring += " T1" + Utils::str(tp->getData().getTrans1()) + " T2" + Utils::str(tp->getData().getTrans2());
    pitem->setText(2,astring);
    parent->addChild(pitem);

    for (auto placedTile : placedTiles)
    {
        QTransform tr = placedTile->getTransform();
        TilePtr tile  = placedTile->getTile();

        QTreeWidgetItem * item = new QTreeWidgetItem;
        item->setText(0,"Placed Tile");
        item->setText(1,addr(placedTile.get()));
        item->setText(2,Transform::toInfoString(tr));
        pitem->addChild(item);

        QTreeWidgetItem * item2 = new QTreeWidgetItem;
        item2->setText(0,"Tile");
        item2->setText(1, addr(tile.get()));
        item2->setText(2, QString("Points: %1 Rot: %2  scale: %3  %4").arg(tile->numPoints())
                                                         .arg(tile->getRotation())
                                                         .arg(tile->getScale())
                                                         .arg((tile->isRegular()) ? "Regular" : "Irregular"));
        item->addChild(item2);
    }
}

void page_system_info::populateViews(QTreeWidgetItem * parent)
{
    QTreeWidgetItem * item = new QTreeWidgetItem;
    item->setText(0,"Tiling Maker View");
    item->setText(2,Transform::toInfoString(TilingMakerView::getInstance()->getLayerTransform()));
    parent->addChild(item);

    item = new QTreeWidgetItem;
    item->setText(0,"Tiling View");
    item->setText(2,Transform::toInfoString(TilingView::getInstance()->getLayerTransform()));
    parent->addChild(item);

    item = new QTreeWidgetItem;
    item->setText(0,"Map Editor View");
    item->setText(2,Transform::toInfoString(MapEditorView::getInstance()->getLayerTransform()));
    parent->addChild(item);

    item = new QTreeWidgetItem;
    item->setText(0,"Prototype View");
    item->setText(2,Transform::toInfoString(PrototypeView::getInstance()->getLayerTransform()));
    parent->addChild(item);

    item = new QTreeWidgetItem;
    item->setText(0,"Motif View");
    item->setText(2,Transform::toInfoString(MotifView::getInstance()->getLayerTransform()));
    parent->addChild(item);

    item = new QTreeWidgetItem;
    item->setText(0,"Background Image View");
    auto bip = BackgroundImageView::getInstance();
    item->setText(2,Transform::toInfoString(bip->getLayerTransform()));
    parent->addChild(item);

    item = new QTreeWidgetItem;
    item->setText(0,"Grid View");
    item->setText(2,Transform::toInfoString(GridView::getInstance()->getLayerTransform()));
    parent->addChild(item);
}

void page_system_info::expandTree()
{
    tree->expandAll();
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
