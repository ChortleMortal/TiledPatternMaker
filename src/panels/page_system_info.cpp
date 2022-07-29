#include <QCheckBox>
#include <QTreeWidget>
#include <QApplication>
#include <QMessageBox>

#include "page_system_info.h"
#include "figures/figure.h"
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
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/backgroundimage.h"
#include "misc/border.h"
#include "misc/qtapplog.h"
#include "misc/tpm_io.h"
#include "misc/utilities.h"
#include "mosaic/design_element.h"
#include "mosaic/mosaic.h"
#include "mosaic/prototype.h"
#include "style/filled.h"
#include "tile/feature.h"
#include "tile/placed_feature.h"
#include "tile/tiling.h"
#include "viewers/motif_view.h"
#include "viewers/grid.h"
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
    tree->setMinimumHeight(750);
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
{
}

void page_system_info::onEnter()
{
    if (!lockCheck->isChecked())
    {
        populateTree();
    }
}

void page_system_info::populateTree()
{
    qDebug() << "page_system_info::populateTree()";

    QApplication::setOverrideCursor(Qt::WaitCursor);

    tree->clear();

    QTreeWidgetItem * item;

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
    tree->expandItem(item);

    populateStyles(item,mosaic);

    populateBorder(item,mosaic);

    CropPtr crop;
    if (mosaic)
    {
        crop = mosaic->getCrop();
    }
    populateCrop(item,crop);

    qDebug() << "page_system_info::populateTree() - mosaics done";

    // Motif Maker
    const QVector<PrototypePtr> & prototypes = motifMaker->getPrototypes();
    // selected prototype
    item = new QTreeWidgetItem;
    item->setText(0,"Motif Maker");
    item->setText(1,QString("Protos: %1").arg(prototypes.size()));
    int num = 0;
    for (auto prototype : prototypes)
    {
        num += prototype->numDesignElements();
    }
    item->setText(2,QString("Design Elements: %1").arg(num));
    tree->addTopLevelItem(item);

    PrototypePtr pp = motifMaker->getSelectedPrototype();
    populatePrototype(item,pp,"Selected Prototype");

    for (auto & proto : prototypes)
    {
        populatePrototype(item,proto,"Prototype");
    }

    // selected design element
    QTreeWidgetItem * item2 = new QTreeWidgetItem;
    item2->setText(0,"Selected Design Elements");
    auto dels = motifMaker->getSelectedDesignElements();
    item2->setText(1,QString("Count : %1").arg(dels.size()));
    item->addChild(item2);

    for (auto del : dels)
    {
        populateDEL(item2,del);
    }

    item2 = new QTreeWidgetItem;
    item2->setText(0,"Active Feature");
    FeaturePtr fp = motifMaker->getActiveFeature();
    item2->setText(1,addr(fp.get()));
    if (fp)
    {
        item->setText(2, QString("Points: %1 Rot: %2").arg(fp->numPoints()).arg(fp->getRotation()));
    }
    item->addChild(item2);

    qDebug() << "page_system_info::populateTree() - motifs done";

    // Tiling Maker
    const QVector<TilingPtr> & tilings = tilingMaker->getTilings();

    item = new QTreeWidgetItem;
    item->setText(0,"Tiling Maker");
    item->setText(1,QString("Tilings: %1").arg(tilings.size()));
    int features = 0;
    for (auto& tiling : tilings)
    {
        features += tiling->countPlacedFeatures();
    }
    item->setText(2,QString("Placed Features: %1").arg(features));
    tree->addTopLevelItem(item);

    TilingPtr tp = tilingMaker->getSelected();
    populateTiling(item,tp,"Selected Tiling");

    for (auto& tiling : tilings)
    {
        populateTiling(item,tiling,"Tiling");
    }

    qDebug() << "page_system_info::populateTree() - tilings done";

    // background image
    item = new QTreeWidgetItem;
    item->setText(0,"Background Image");
    BkgdImgPtr bip = BackgroundImage::getSharedInstance();
    if (bip->isLoaded())
        item->setText(2, bip->getName());
    else
        item->setText(2, "none");
    tree->addTopLevelItem(item);
    item2 = new QTreeWidgetItem;
    item2->setText(0,"xf_image");
    item2->setText(2,Transform::toInfoString(bip->getCanvasXform().toQTransform()));
    item->addChild(item2);

    // selected feature
    item = new QTreeWidgetItem;
    item->setText(0,"Selected Feature");
    fp = view->getSelectedFeature();
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

    // views
    item = new QTreeWidgetItem;
    item->setText(0,"Views");
    populateViews(item);
    tree->addTopLevelItem(item);

    // map editor
    item = new QTreeWidgetItem;
    item->setText(0,"Map Editor");
    auto maped   =  MapEditor::getInstance();
    auto mapedDb = maped->getDb();
    auto maps    = mapedDb->getDrawMaps();
    QString astring = QString("Draw Maps: %1").arg(maps.size());
    item->setText(1,astring);
    tree->addTopLevelItem(item);

    for (auto map : maps)
    {
        populateMap(item,map,"Draw Map");
    }

    // Crop editor
    item = new QTreeWidgetItem;
    item->setText(0,"Crop Maker");
    auto maker = CropMaker::getInstance();
    item->setText(2,QString("State = %1").arg(sCropMakerState[maker->getState()]));
    crop = maker->getCrop();
    populateCrop(item,crop);
    tree->addTopLevelItem(item);

    // adjust
    tree->setColumnWidth(0,201);
    tree->resizeColumnToContents(2);

    QApplication::restoreOverrideCursor();
    qDebug() << "page_system_info::populateTree() - done";
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
        populatePrototype(item,style->getPrototype(),"Prototype");
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
        QString emb = (crop->isEmbedded()) ? "Embedded" : QString();
        QString app = (crop->isApplied())  ? "Applied" : QString();
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
        item->setText(2,"EMPTY Map");
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

void page_system_info::populatePrototype(QTreeWidgetItem *parent, PrototypePtr pp, QString name)
{
    if  (!pp) return;

    TilingPtr tiling                 = pp->getTiling();
    QVector<DesignElementPtr> & dels = pp->getDesignElements();

    // summary
    QTreeWidgetItem * pitem = new QTreeWidgetItem;
    pitem->setText(0,name);
    pitem->setText(1,addr(pp.get()));
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
        populateDEL(item,del);
    }

    auto map = pp->getExistingProtoMap();
    populateMap(pitem,map,"ProtoMap");

    auto crop = pp->getCrop();
    populateCrop(pitem,crop);

    tree->expandItem(pitem);
}

void page_system_info::populateDEL(QTreeWidgetItem * parent, DesignElementPtr de)
{
    if (!de) return;

    QTreeWidgetItem * item2 = new QTreeWidgetItem;
    item2->setText(0,"DEL");
    item2->setText(1,addr(de.get()));
    parent->addChild(item2);
    tree->expandItem(item2);

    QTreeWidgetItem * item = new QTreeWidgetItem;
    FeaturePtr feature = de->getFeature();
    item->setText(0,"Feature");
    item->setText(1, addr(feature.get()));
    item->setText(2, QString("Points: %1 Rot: %2 %3").arg(feature->numPoints())
                                                     .arg(feature->getRotation())
                                                     .arg((feature->isRegular()) ? "Regular" : "Irregular"));
    item2->addChild(item);
    tree->expandItem(item);

    item = new QTreeWidgetItem;
    FigurePtr figure = de->getFigure();
    item->setText(0,"Figure");
    item->setText(1,addr(figure.get()));
    QString astring = figure->getFigureDesc() + " " + figure->getFigTypeString();
    item->setText(2,astring);
    item2->addChild(item);
    tree->expandItem(item);

    switch (figure->getFigType())
    {
    case FIG_TYPE_EXPLICIT_FEATURE:
    case FIG_TYPE_EXPLICIT:
        populateMap(parent,figure->getFigureMap(),"Figure Map");
        break;

    default:
        break;
    }
}

void page_system_info::populateTiling(QTreeWidgetItem * parent, TilingPtr tp, QString name)
{
    if (!tp) return;

    const QVector<PlacedFeaturePtr> & qlpf = tp->getPlacedFeatures();

    // summary
    QTreeWidgetItem * pitem = new QTreeWidgetItem;
    pitem->setText(0,name);
    pitem->setText(1,addr(tp.get()));
    QString astring = tp->getName() + " - Features: " + QString::number(qlpf.size());
    astring += " T1" + Utils::str(tp->getTrans1()) + " T2" + Utils::str(tp->getTrans2());
    pitem->setText(2,astring);
    parent->addChild(pitem);

    for (auto pfp : qlpf)
    {
        QTransform tr  = pfp->getTransform();
        FeaturePtr fp  = pfp->getFeature();

        QTreeWidgetItem * item = new QTreeWidgetItem;
        item->setText(0,"Placed Feature");
        item->setText(1,addr(pfp.get()));
        item->setText(2,Transform::toInfoString(tr));
        pitem->addChild(item);

        QTreeWidgetItem * item2 = new QTreeWidgetItem;
        item2->setText(0,"Feature");
        item2->setText(1, addr(fp.get()));
        item->setText(2, QString("Points: %1 Rot: %2 %3").arg(fp->numPoints())
                                                         .arg(fp->getRotation())
                                                         .arg((fp->isRegular()) ? "Regular" : "Irregular"));
        item->addChild(item2);
    }
}

void page_system_info::populateViews(QTreeWidgetItem * parent)
{
    QTreeWidgetItem * item = new QTreeWidgetItem;
    item->setText(0,"Tiling Maker");
    item->setText(2,Transform::toInfoString(TilingMaker::getSharedInstance()->getLayerTransform()));
    parent->addChild(item);

    item = new QTreeWidgetItem;
    item->setText(0,"Tiling View");
    item->setText(2,Transform::toInfoString(TilingView::getSharedInstance()->getLayerTransform()));
    parent->addChild(item);

    item = new QTreeWidgetItem;
    item->setText(0,"Map Editor");
    item->setText(2,Transform::toInfoString(MapEditor::getInstance()->getMapedView()->getLayerTransform()));
    parent->addChild(item);

    item = new QTreeWidgetItem;
    item->setText(0,"Prototype View");
    item->setText(2,Transform::toInfoString(PrototypeView::getSharedInstance()->getLayerTransform()));
    parent->addChild(item);

    item = new QTreeWidgetItem;
    item->setText(0,"Motif View");
    item->setText(2,Transform::toInfoString(MotifView::getSharedInstance()->getLayerTransform()));
    parent->addChild(item);

    item = new QTreeWidgetItem;
    item->setText(0,"Background Image");
    item->setText(2,Transform::toInfoString(BackgroundImage::getSharedInstance()->getLayerTransform()));
    parent->addChild(item);

    item = new QTreeWidgetItem;
    item->setText(0,"Grid");
    item->setText(2,Transform::toInfoString(Grid::getSharedInstance()->getLayerTransform()));
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
