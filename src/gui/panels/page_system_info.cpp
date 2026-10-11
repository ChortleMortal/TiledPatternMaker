#include <QCheckBox>
#include <QTreeWidget>
#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QDateTime>

#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "gui/top/controlpanel.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/crop_maker_view.h"
#include "gui/viewers/grid_view.h"
#include "gui/viewers/image_view.h"
#include "gui/viewers/map_editor_view.h"
#include "gui/viewers/motif_maker_view.h"
#include "gui/viewers/prototype_view.h"
#include "gui/top/system_view.h"
#include "model/makers/crop_maker.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/motif_maker.h"
#include "model/makers/tiling_maker.h"
#include "model/borders/border.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/motif.h"
#include "model/prototypes/tile_motif.h"
#include "model/prototypes/prototype.h"
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

page_system_info::page_system_info(ControlPanel * cpanel)
    : panel_page(cpanel,PAGE_SYSTEM_INFO,"System Info")
{
    qRegisterMetaType<NodeInfo>("NodeInfo");

    setMinimumWidth(PANEL_RHS_WIDTH);

    firstTime        = true;
    m_restoringState = false;

    QPushButton * pbDump = new QPushButton("Log Visible");
    pbDump->setFixedWidth(101);

    QPushButton * pbExpand  = new QPushButton("Simple Expand");

    chkLock = new QCheckBox("Lock Refresh");
    chkLock->setChecked(false);

    QPushButton * pbRefresh = new QPushButton("Refresh");
    pbRefresh->setFixedWidth(101);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(pbDump);
    hbox->addSpacing(15);
    hbox->addWidget(pbExpand);
    hbox->addStretch();
    hbox->addWidget(chkLock);
    hbox->addWidget(pbRefresh);

    tree = new QTreeWidget();
    tree->setColumnCount(3);
    tree->setHeaderHidden(true);
    tree->setMinimumHeight(690);
    tree->setRootIsDecorated(true);
    tree->setItemsExpandable(true);
    tree->setSelectionMode(QAbstractItemView::NoSelection);

    vbox->addLayout(hbox);
    vbox->addWidget(tree);

    connect(pbDump,         &QPushButton::clicked,      this, &page_system_info::dumpTree);
    connect(pbRefresh,      &QPushButton::clicked,      this, &page_system_info::populateTree);
    connect(pbExpand,       &QPushButton::clicked,      this, &page_system_info::simpleExpand);
    connect(tree,           &QTreeWidget::itemClicked,  this, &page_system_info::slot_itemClicked);
    connect(tree,           &QTreeWidget::itemExpanded, this, &page_system_info::onItemExpanded);
    connect(tree,           &QTreeWidget::itemCollapsed,this, &page_system_info::onItemCollapsed);
}

page_system_info::~page_system_info()
{
    if (!firstTime)
    {
        saveState();
    }
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
    // Top-level sections only; children are populated lazily

    qDebug().noquote() << "page_system_info::populateTree (lazy)" << "- start";

    //Sys::systemViewController->splash_display("Collecting system Info.....");
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (!firstTime)
    {
        saveExpansionState();
    }

    tree->blockSignals(true);
    tree->clear();
    tree->blockSignals(false);

    // Mosaic Maker
    QTreeWidgetItem *mosSection = makeSection("Mosaic Maker", NodeInfo::Type::MosaicMakerSection);

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        int numStyles = mosaic->getStyleSet().size();
        QString name  = mosaic->getName().get();

        mosSection->setText(0, "Mosaic Maker");
        mosSection->setText(1, QString("Styles: %1").arg(numStyles));
        mosSection->setText(2, QString("Mosaic: %1").arg(name));
    }
    else
    {
        mosSection->setText(0, "Mosaic Maker");
        mosSection->setText(1, "Styles: 0");
        mosSection->setText(2, "Mosaic: <none>");
    }

    // Prototype Maker
    QTreeWidgetItem *protoSection  = makeSection("Prototype Info",     NodeInfo::Type::PrototypeMakerSection);
    {
        auto protos = motifMaker->protoData.getPrototypes();
        protoSection->setText(0,"Prototype Info");
        protoSection->setText(1,QString("Protos: %1").arg(protos.size()));
        int numD = 0;
        int numV = motifMaker->protoData.numTileMotifs();
        for (auto & prototype : std::as_const(protos))
        {
            numD += prototype->numTileMotifs();
        }
        protoSection->setText(2,QString("TileMotifs: %1 visible: %2").arg(numD).arg(numV));
    }

    // Motif Maker
    QTreeWidgetItem *motifSection     = makeSection("Motif Maker",         NodeInfo::Type::MotifMakerSection);
    {
        motifSection->setText(0,"Motif Maker");

        int numD = motifMaker->motifData.numTileMotifs();
        int numV = motifMaker->motifData.getVisibleTileMotifs().count();
        motifSection->setText(2,QString("TileMotifs: %1 visible: %2").arg(numD).arg(numV));
    }

    // Tiling Maker
    QTreeWidgetItem *tilingSection = makeSection("Tiling Maker",        NodeInfo::Type::TilingMakerSection);

    const QVector<TilingPtr> & tilings = tilingMaker->getTilings();

    tilingSection->setText(0,"Tiling Maker");
    tilingSection->setText(1,QString("Tilings: %1").arg(tilings.size()));
    int tiles = 0;
    for (const auto & tiling : std::as_const(tilings))
    {
        tiles += tiling->unit().numIncluded();
    }
    tilingSection->setText(2,QString("Placed Tiles: %1").arg(tiles));

    // Background Image
    QTreeWidgetItem *bkgSection    = makeSection("Background Image",    NodeInfo::Type::BackgroundImageSection);

    auto bip = Sys::backgroundImage;
    bkgSection->setText(0,"Background Image");
    bkgSection->setText(1,addr(bip.get()));

    if (bip && bip->isLoaded())
        bkgSection->setText(2, bip->getTitle());
    else
        bkgSection->setText(2, "none");

    // CropMaker
    QTreeWidgetItem *cropSection   = makeSection("Crop Maker",          NodeInfo::Type::CropMakerSection);
    {
        QString s = sCropMaker[Sys::cropMakerView->getMakerType()];
        cropSection->setText(0,"Crop Maker");
        cropSection->setText(2, s);
    }

    // Active Views
    QTreeWidgetItem *viewsSection  = makeSection("Active Views",        NodeInfo::Type::ViewsSection);
    {
        int count = Sys::systemViewController->getActiveLayers().size();
        viewsSection->setText(0,"Active Views");
        viewsSection->setText(1,QString("%1 viewers").arg(count));
    }

    // Map Editor
    QTreeWidgetItem *mapEdSection  = makeSection("Map Editor",          NodeInfo::Type::MapEditorSection);
    mapEdSection->setText(0,"Map Editor");
    auto maped   = Sys::mapEditor;
    auto maps    = maped->getDrawMaps();
    QString astring = QString("Maps: %1").arg(maps.size());
    mapEdSection->setText(1,astring);

    // image viewers
    QTreeWidgetItem *imgSection    = makeSection("Image Viewer",        NodeInfo::Type::ImageViewerSection);
    imgSection->setText(0,"Image Viewer");
    if (Sys::imageViewer->isLoaded())
    {
        const QPixmap & pp = Sys::imageViewer->getPixmap();
        QString str = QString("size %1 x %2").arg(pp.width()).arg(pp.height());
        imgSection->setText(2, str);
    }
    else
    {
        imgSection->setText(1,"Not loaded");
    }

    // system view
    QTreeWidgetItem *sysviewSection    = makeSection("System View",        NodeInfo::Type::SystemViewSection);
    sysviewSection->setText(0,"System View");

    if (firstTime)
    {
        firstTime = false;
        loadState();
    }
    restoreExpansionState();

    // adjust
    slot_itemClicked(nullptr,0);

    QApplication::restoreOverrideCursor();

    //Sys::systemViewController->splash_remove();
    qDebug().noquote() << "page_system_info::populateTree (lazy)" << "- end";
}

void page_system_info::saveExpansionState()
{
    m_expandedPerSection.clear();

    const int topCount = tree->topLevelItemCount();
    for (int i = 0; i < topCount; ++i)
    {
        QTreeWidgetItem *section = tree->topLevelItem(i);

        NodeInfo info = getNodeInfo(section);
        if (!info.isValid() || info.key.isEmpty())
            continue;

        QSet<QString> expanded;
        collectExpanded(section, expanded);

        m_expandedPerSection.insert(info.key, expanded);
    }

    saveState();
}

void page_system_info::collectExpanded(QTreeWidgetItem *item, QSet<QString> &out)
{
    NodeInfo info = getNodeInfo(item);

    if (item->isExpanded() && !info.key.isEmpty())
    {
        out.insert(info.key);
    }

    const int n = item->childCount();
    for (int i = 0; i < n; ++i)
    {
        collectExpanded(item->child(i), out);
    }
}

void page_system_info::restoreExpansionState()
{
    m_restoringState = true;

    const int topCount = tree->topLevelItemCount();
    for (int i = 0; i < topCount; ++i)
    {
        QTreeWidgetItem *section = tree->topLevelItem(i);

        NodeInfo info = getNodeInfo(section);
        if (!info.isValid() || info.key.isEmpty())
            continue;

        const QSet<QString> expanded = m_expandedPerSection.value(info.key);

        restoreExpanded(section, expanded);
    }

    m_restoringState = false;
}

void page_system_info::restoreExpanded(QTreeWidgetItem *item, const QSet<QString> &expanded)
{
    NodeInfo info = getNodeInfo(item);

    if (!info.key.isEmpty() && expanded.contains(info.key))
    {
        // Populate children synchronously FIRST, before expanding
        populateChildren(item);

        // Now expand (this will fire the signal, but populated flag
        // means populateChildren will return immediately - no double work)
        if (!item->isExpanded())
            tree->expandItem(item);
    }

    // NOW children exist and can be recursed into
    const int n = item->childCount();
    for (int i = 0; i < n; ++i)
        restoreExpanded(item->child(i), expanded);
}

void page_system_info::saveState()
{
    QSettings settings = Sys::settings();

    settings.beginGroup("ExpandedPerSection");
    settings.remove("");

    for (auto it = m_expandedPerSection.cbegin(); it != m_expandedPerSection.cend(); ++it)
        settings.setValue(it.key(), QStringList(it.value().begin(), it.value().end()));

    settings.endGroup();
}

void page_system_info::loadState()
{
    m_expandedPerSection.clear();

    QSettings settings = Sys::settings();

    settings.beginGroup("ExpandedPerSection");

    for (const QString &key : settings.childKeys())
    {
        QStringList list = settings.value(key).toStringList();
        m_expandedPerSection[key] = QSet<QString>(list.begin(), list.end());
    }

    settings.endGroup();
}

void page_system_info::simpleExpand()
{
    for (int i = 0; i < tree->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = tree->topLevelItem(i);
        expandOneLevel(item);
        tree->expandItem(item);
    }
}

bool page_system_info::isExpandable(QTreeWidgetItem * item)
{
    return item->data(0, Qt::UserRole).isValid();
}

void page_system_info::populateChildren(QTreeWidgetItem *item)
{
    NodeInfo info = getNodeInfo(item);
    if (info.populated)
        return;

    switch (info.type)
    {
    case NodeInfo::Type::MosaicMakerSection:
        doMosaicMaker(item);
        break;

    case NodeInfo::Type::PrototypeMakerSection:
        doProtoypeInfo(item);
        break;

    case NodeInfo::Type::MotifMakerSection:
        doMotifMaker(item);
        break;

    case NodeInfo::Type::TilingMakerSection:
        doTilingMaker(item);
        break;

    case NodeInfo::Type::BackgroundImageSection:
        doBackgroundImage(item);
        break;

    case NodeInfo::Type::ViewsSection:
        doViews(item);
        break;

    case NodeInfo::Type::MapEditorSection:
        doMapEditor(item);
        break;

    case NodeInfo::Type::CropMakerSection:
        doCropMaker(item);
        break;

    case NodeInfo::Type::ImageViewerSection:
        doImageViewer(item);
        break;

    case NodeInfo::Type::SystemViewSection:
        doSystemView(item);
        break;

    case NodeInfo::Type::TilingNode:
    {
        Tiling *tp = static_cast<Tiling*>(info.payload);
        populateTilingUnit(item, tp);
    }   break;

    case NodeInfo::Type::TilingUnitNode:
    {
        Tiling *tp   = static_cast<Tiling*>(info.payload);
        Tile   *tile = static_cast<Tile*>(info.payload2);
        populatePlacements(item, tp, tile);
    }   break;

    case NodeInfo::Type::EdgePolyNode:
    {
        EdgePoly * ep = static_cast<EdgePoly*>(info.payload);
        populateEdgePoly(item, ep);
    }   break;

    case NodeInfo::Type::StylesNode:
    {
        Style *style = static_cast<Style*>(info.payload);
        populateStyle(item, style);
        break;
    }

    case NodeInfo::Type::CropNode:
    {
        Crop *c = static_cast<Crop*>(info.payload);
        populateCrop(item, item->text(0), c);
        break;
    }

    case NodeInfo::Type::BackgroundImageNode:
    {
        Mosaic *m = static_cast<Mosaic*>(info.payload);
        populateBackgroundImage(item, m->getBkgdRecord());
        break;
    }

    case NodeInfo::Type::MapNode:
    {
        Map * map = static_cast<Map*>(info.payload);
        populateMapNode(item, map);
        break;
    }

    case NodeInfo::Type::EdgesNode:
    {
        Map * map = static_cast<Map*>(info.payload);
        populateMapEdges(item, map);
        break;
    }

    case NodeInfo::Type::VerticesNode:
    {
        Map * map = static_cast<Map*>(info.payload);
        populateMapVertices(item, map);
        break;
    }

    case NodeInfo::Type::PrototypeNode:
    {
        Prototype * pp = static_cast<Prototype*>(info.payload);
        populatePrototypeNode(item, pp);
        break;
    }

    case NodeInfo::Type::TileMotifNode:
    {
        TileMotif * timo = static_cast<TileMotif*>(info.payload);
        populateTileMotifNode(item, timo);
        break;
    }

    case NodeInfo::Type::MotifNode:
    {
        Motif * motif = static_cast<Motif*>(info.payload);
        populateMotifNode(item, motif);
        break;
    }

    case NodeInfo::Type::ExtendersNode:
    {
        Motif * motif = static_cast<Motif*>(info.payload);
        populateMotifExtenders(item,motif);
        break;
    }

    default:
        break;
    }

    info.populated = true;
    // Only nodes (not sections) get payload-based keys
    if (info.payload)
        info.key = makeKeyForNode(item,info);
    else
        info.key = makeKeyForSection(info);
    setNodeInfo(item, info);
}

void page_system_info::slot_itemClicked(QTreeWidgetItem *, int)
{
    static const int minWidths[3] = { 150, 120, 150 };  // your chosen minimums

    for (int col = 0; col < 3; ++col) {
        tree->resizeColumnToContents(col);

        int w = tree->columnWidth(col);
        if (w < minWidths[col])
            tree->setColumnWidth(col, minWidths[col]);
    }
}

void page_system_info::onItemExpanded(QTreeWidgetItem *item)
{
    NodeInfo info = getNodeInfo(item);
    if (!info.isValid())
        return; // leaf node

    if (!info.populated)
        populateChildren(item);
\
    if (!m_restoringState)
        saveExpansionState();
}

void page_system_info::onItemCollapsed(QTreeWidgetItem *item)
{
    NodeInfo info = getNodeInfo(item);
    if (!info.isValid())
        return; // leaf node

    resetSubtree(item);
\
    if (!m_restoringState)
        saveExpansionState();
}

// ===================== Section implementations (now lazy) =====================

void page_system_info::doMosaicMaker(QTreeWidgetItem *sectionItem)
{
    qDebug().noquote() << "page_system_info::doMosaicMaker" << "- start";

    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic)
        return;

    NodeInfo info;

    // --- One child per tiling ---
    for (auto & tp : mosaic->getTilings())
    {
        populateTiling(sectionItem,tp.get(),"Tiling");
    }

    // --- One child per style ---
    for (auto & style : mosaic->getStyleSet())
    {
        QTreeWidgetItem *child = new QTreeWidgetItem(sectionItem);
        child->setText(0, "Style");
        child->setText(1, style->getStyleDesc());
        child->setText(2, addr(style.get()));

        if (style->getStyleType() == STYLE_FILLED)
        {
            FilledPtr fp = std::dynamic_pointer_cast<Filled>(style);
            Q_ASSERT(fp);
            ColorMaker * cm = fp->currentColoring;
            QString astring = QString("Filled: algo=%1 %2").arg(fp->getAlgorithm()).arg(cm->sizeInfo());
            child->setText(2,astring);  // overwrites
        }

        info.type    = NodeInfo::Type::StylesNode;
        info.payload = style.get();
        info.key     = makeKeyForNode(child,info);
        info.populated = false;
        setNodeInfo(child, info);
    }

    // --- Border ---
    {
        QTreeWidgetItem *child = new QTreeWidgetItem(sectionItem);
        BorderPtr bp = mosaic->getBorder();
        child->setText(0,"Border");
        child->setText(1,addr(bp.get()));
        if (bp)
        {
            QString astring = QString("%1 %2 %3").arg(bp->getBorderTypeString()).arg(bp->getCropTypeString()).arg(bp->getContentString());
            child->setText(2,astring);
        }
    }

    // --- Mosaic Crop ---
    auto crop = mosaic->getCrop();
    populateCrop(sectionItem,"Mosaic Crop",crop.get());

    // --- Painter Crop ---
    crop = mosaic->getPainterCrop();
    populateCrop(sectionItem,"Painter Crop",crop.get());

    // --- Background Image ---
    populateBackgroundImage(sectionItem, mosaic->getBkgdRecord());
}

void page_system_info::doProtoypeInfo(QTreeWidgetItem *sectionItem)
{
    // all protos
    auto protos = motifMaker->protoData.getPrototypes();
    for (auto & proto : std::as_const(protos))
    {
        QString visibility = motifMaker->protoData.isVisible(proto) ? "visible" : "hidden";
        populatePrototype(sectionItem,proto.get(),"Prototype",visibility);
    }

}

void page_system_info::doMotifMaker(QTreeWidgetItem *sectionItem)
{
    {
        // delegated tile motif
        auto delegated  = motifMaker->motifData.getDelegatedTileMotif();
        QTreeWidgetItem * child = new QTreeWidgetItem(sectionItem);;
        child->setText(0,"Delegated TileMotif");
        child->setText(1,addr(delegated.get()));
    }

    {
        // all TileMotifs
        auto timos = motifMaker->motifData.getTileMotifs();
        for (auto & timo : std::as_const(timos))
        {
            QString state = motifMaker->motifData.isHidden(timo) ? "hidden" : "visible";
            populateTileMotif(sectionItem, timo, "TileMotif",state);
        }
    }
}

void page_system_info::doTilingMaker(QTreeWidgetItem *sectionItem)
{
    qDebug().noquote() << "page_system_info::doTilingMaker" << "- start";

    const QVector<TilingPtr> & tilings = tilingMaker->getTilings();

    TilingPtr tp = tilingMaker->getSelectedTiling();
    populateTiling(sectionItem,tp.get(),"Selected Tiling");

    for (auto & tiling : std::as_const(tilings))
    {
        populateTiling(sectionItem,tiling.get(),"Tiling");
    }

    qDebug().noquote() << "page_system_info::doTilingMaker" << "- end";
}

void page_system_info::doBackgroundImage(QTreeWidgetItem *sectionItem)
{
    auto bip = Sys::backgroundImage;
    if (!bip->isLoaded()) return;

    QTreeWidgetItem * child = new QTreeWidgetItem(sectionItem);;
    child->setText(0,"Model Xform");
    child->setText(1, addr(bip.get()));
    child->setText(2,Transform::info(bip->getModelTransform()));
}

void page_system_info::doViews(QTreeWidgetItem *sectionItem)
{
    populateViews(sectionItem);
}

void page_system_info::doMapEditor(QTreeWidgetItem *sectionItem)
{
    auto maps = Sys::mapEditor->getDrawMaps();
    for (auto & map : std::as_const(maps))
    {
        populateMap(sectionItem,map.get(),"Map");
    }
}

void page_system_info::doCropMaker(QTreeWidgetItem *sectionItem)
{
    QString s = sCropMaker[Sys::cropMakerView->getMakerType()];
    auto maker = Sys::cropMakerView->getMaker();
    if (maker)
    {
        CropPtr crop = maker->getCrop();
        populateCrop(sectionItem,s,crop.get());
    }
}

void page_system_info::doImageViewer(QTreeWidgetItem *sectionItem)
{
    Q_UNUSED(sectionItem);
}

void page_system_info::doSystemView(QTreeWidgetItem *sectionItem)
{
    populateTaker(sectionItem,Sys::systemViewController->getTaker());
}

// ===================== Existing detailed population (unchanged) =====================

void page_system_info::populateStyle(QTreeWidgetItem * parent, Style * style)
{
    if (Layer *layer = dynamic_cast<Layer*>(style))
    {
        populateLayer(parent, layer);
    }

    if (auto proto = style->getPrototype())
    {
        populateMap(parent,proto->getExistingProtoMap().get(),"Style Map");

        populatePrototype(parent, proto.get(), "Prototype", "");
    }
}

void page_system_info::populateCrop(QTreeWidgetItem * parent, QString name, Crop *crop)
{
    QTreeWidgetItem * item = new QTreeWidgetItem();;
    item->setText(0,name);
    item->setText(1,addr(crop));
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

void page_system_info::populateMap(QTreeWidgetItem *parent, Map * map, QString name)
{
    QTreeWidgetItem * child = new QTreeWidgetItem(parent);
    child->setText(0,name);
    child->setText(1,addr(map));
    if (map)
    {
        child->setText(2,map->summary());

        NodeInfo info;
        info.type    = NodeInfo::Type::MapNode;
        info.payload = map;
        info.key     = makeKeyForNode(child,info);
        info.populated = false;
        setNodeInfo(child, info);
    }
    else
    {
        child->setText(2,"No Map");
    }
}

void page_system_info::populateMapNode(QTreeWidgetItem *parent, Map * map)
{
    {
        QTreeWidgetItem * child = new QTreeWidgetItem(parent);
        child->setText(0,"Edges");
        child->setText(1,QString("Count: %1").arg(map->numEdges()));

        NodeInfo info;
        info.type    = NodeInfo::Type::EdgesNode;
        info.payload = map;
        info.key     = makeKeyForNode(child,info);
        info.populated = false;
        setNodeInfo(child, info);
    }

    {
        QTreeWidgetItem * child = new QTreeWidgetItem(parent);
        child->setText(0,"Vertices");
        child->setText(1,QString("Count: %1").arg(map->numVertices()));

        NodeInfo info;
        info.type    = NodeInfo::Type::VerticesNode;
        info.payload = map;
        info.key     = makeKeyForNode(child,info);
        info.populated = false;
        setNodeInfo(child, info);
    }
}

void page_system_info::populateMapEdges(QTreeWidgetItem *parent, Map * map)
{
    if (!map) return;

    const QVector<VertexPtr> & vertices = map->getVertices();

    const QVector<EdgePtr> & edges = map->getEdges();
    for (int i=0; i< map->numEdges(); i++)
    {
        EdgePtr e = edges.at(i);
        QTreeWidgetItem * child = new QTreeWidgetItem(parent);
        child->setText(0,QString("(%1) %2").arg(i).arg(addr(e.get())));
        if (e->getType() == EDGETYPE_CURVE)
            child->setText(2, QString("from %1 to %2 %3").arg(vertices.indexOf(e->v1)).arg(vertices.indexOf(e->v2)).arg(sCurveType[e->getCurveType()]));
        else
            child->setText(2, QString("from %1 to %2").arg(vertices.indexOf(e->v1)).arg(vertices.indexOf(e->v2)));
    }
}

void page_system_info::populateMapVertices(QTreeWidgetItem *parent, Map * map)
{
    if (!map) return;

    NeighbourMap nmap(map);
    const QVector<EdgePtr> & edges = map->getEdges();

    const QVector<VertexPtr> & vertices = map->getVertices();
    for (int i=0; i< map->numVertices(); i++)
    {
        VertexPtr v = vertices.at(i);
        QPointF pos = v->pt;
        QTreeWidgetItem * child = new QTreeWidgetItem(parent);
        child->setText(0,QString("(%1) %2").arg(i).arg(addr(v.get())));
        child->setText(1,QString::number(pos.x()));
        child->setText(2,QString::number(pos.y()));

        const NeighboursPtr neighbours = nmap.getNeighbours(v);
        for (auto & wedge : std::as_const(*neighbours))
        {
            EdgePtr edge = wedge.lock();
            QTreeWidgetItem * gchild = new QTreeWidgetItem(child);
            gchild->setText(0,"Edge");
            gchild->setText(1,QString::number(edges.indexOf(edge)));
        }
    }
}

void page_system_info::populatePrototype(QTreeWidgetItem *parent, Prototype *pp, QString name, QString visibility)
{
    if  (!pp) return;

    TileMotifSet & timos = pp->getTileMotifs();

    QTreeWidgetItem * child = new QTreeWidgetItem(parent);
    child->setText(0,name);
    child->setText(1,addr(pp));
    visibility += "  TileMotifs: " + QString::number(timos.count());
    child->setText(2,visibility);

    NodeInfo info;
    info.type      = NodeInfo::Type::PrototypeNode;
    info.payload   = pp;
    info.key       = makeKeyForNode(child,info);
    info.populated = false;
    setNodeInfo(child, info);
}

void page_system_info::populatePrototypeNode(QTreeWidgetItem *parent, Prototype *pp)
{
    // tiling
    TilingPtr tiling     = pp->getTiling();
    if (tiling)
    {
        populateTiling(parent,tiling.get(),"Tiling");
    }

    // tile motifs
    TileMotifSet & timos = pp->getTileMotifs();
    for (auto & timo : std::as_const(timos))
    {
        populateTileMotif(parent,timo,"TileMotif","");
    }

    auto map = pp->getExistingProtoMap();
    populateMap(parent,map.get(),"ProtoMap");

    auto dcel = pp->getExistingDCEL();
    if (dcel && !dcel->isEmpty())
    {
        QTreeWidgetItem * child = new QTreeWidgetItem(parent);
        child->setText(0,"DCEL");
        child->setText(1,addr((dcel.get())));
        child->setText(2,dcel->info());
        parent->addChild(child);
    }

    CropPtr crop = pp->getCrop();
    populateCrop(parent,"Mosaic Crop",crop.get());
}

void page_system_info::populateTileMotif(QTreeWidgetItem * parent, TileMotifPtr timo, QString name, QString state)
{
    if (!timo) return;

    QTreeWidgetItem * child = new QTreeWidgetItem(parent);
    child->setText(0,name);
    child->setText(1,addr(timo.get()));
    child->setText(2,state);

    NodeInfo info;
    info.type      = NodeInfo::Type::TileMotifNode;
    info.payload   = timo.get();
    info.key       = makeKeyForNode(child,info);
    info.populated = false;
    setNodeInfo(child, info);
}

void page_system_info::populateTileMotifNode(QTreeWidgetItem * parent, TileMotif * timo)
{
    {
        // Tile
        TilePtr tile = timo->getTile();
        QTreeWidgetItem * child = new QTreeWidgetItem(parent);
        child->setText(0,"Tile");
        child->setText(1, addr(tile.get()));
        child->setText(2, QString("Points: %1 Rot: %2 %3").arg(tile->numPoints())
                              .arg(tile->getRotation())
                              .arg((tile->isRegular()) ? "Regular" : "Irregular"));
    }

    {
        // Motif
        MotifPtr motif = timo->getMotif();
        QTreeWidgetItem * child = new QTreeWidgetItem(parent);
        child->setText(0,"Motif");
        child->setText(1,addr(motif.get()));
        child->setText(2,motif->getMotifDesc());

        NodeInfo info;
        info.type    = NodeInfo::Type::MotifNode;
        info.payload = motif.get();
        info.key     = makeKeyForNode(child,info);
        info.populated = false;
        setNodeInfo(child, info);
    }
}

void page_system_info::populateMotifNode(QTreeWidgetItem * parent, Motif * motif)
{
    populateMap(parent,motif->getExistingMotifMap().get(),"Motif Map");

    QTreeWidgetItem * child = new QTreeWidgetItem(parent);
    child->setText(0,"Extenders");
    int count = motif->getExtenders().count();
    child->setText(1,QString::number(count));
    if (count)
    {
        NodeInfo info;
        info.type    = NodeInfo::Type::ExtendersNode;
        info.payload = motif;
        info.key     = makeKeyForNode(child,info);
        info.populated = false;
        setNodeInfo(child, info);
    }
}


void page_system_info::populateMotifExtenders(QTreeWidgetItem * parent, Motif * motif)
{
    for (ExtenderPtr ep : motif->getExtenders())
    {
        {
            QString astring;
            if (ep->getStellation())
            {
                astring += "Stellation | ";
                if (ep->getFitToTile())
                {
                    astring += "Fit to tile | ";
                    qreal scale = ep->getFitScale();
                    astring += QString::number(scale);
                    astring += " | ";
                }
            }

            QTreeWidgetItem * child = new QTreeWidgetItem(parent);
            child->setText(0,"Extender");
            child->setText(1,addr(ep.get()));
            child->setText(2,astring);
        }

        {
            QTreeWidgetItem * child = new QTreeWidgetItem(parent);
            auto & eb = ep->getExtendedBoundary();
            QString astring = QString("scale:%1 rot:%2 sides:%3").arg(eb.getScale()).arg(eb.getRotate()).arg(eb.getSides());
            child->setText(0,"Extended Boundary");
            child->setText(2,astring);
        }
    }
}

void page_system_info::populateTiling(QTreeWidgetItem * parent, Tiling * tp, QString name)
{
    if (!tp) return;

    QTreeWidgetItem *child = new QTreeWidgetItem(parent);
    child->setText(0,name);
    child->setText(1,addr(tp));
    int size = tp->unit().getIncluded().size();
    QString astring = tp->getVName().get() + "   Tiles: " + QString::number(size);
    astring += "   T1" + Utils::str(tp->hdr().getTrans1()) + " T2" + Utils::str(tp->hdr().getTrans2());
    child->setText(2,astring);

    NodeInfo info;
    info.type    = NodeInfo::Type::TilingNode;
    info.payload = tp;
    info.key     = makeKeyForNode(child,info);
    info.populated = false;
    setNodeInfo(child, info);
}

void page_system_info::populateTilingUnit(QTreeWidgetItem * parent, Tiling * tp)
{
    const TilingUnit & tilingUnit = tp->unit().get();

    for (const TileUnit & tileUnit : tilingUnit)
    {
        TilePtr tile = tileUnit.first;
        QTreeWidgetItem * child = new QTreeWidgetItem(parent);
        child->setText(0,"Unique Tile");
        child->setText(1,addr(tileUnit.first.get()));
        child->setText(2, QString("Points: %1 Rot: %2  scale: %3  %4").arg(tile->numPoints())
                        .arg(tile->getRotation())
                        .arg(tile->getScale())
                        .arg((tile->isRegular()) ? "Regular" : "Irregular"));

        NodeInfo info;
        info.type      = NodeInfo::Type::TilingUnitNode;
        info.payload   = tp;
        info.payload2  = tileUnit.first.get();
        info.key       = makeKeyForNode(child,info);
        info.populated = false;
        setNodeInfo(child, info);
    }
    populateBackgroundImage(parent, tp->getBkgdRecord());
}

void page_system_info::populatePlacements(QTreeWidgetItem * parent, Tiling * tiling, Tile * tile)
{
    const TilingUnit & tilingUnit = tiling->unit().get();
    for (const TileUnit & tileUnit : tilingUnit)
    {
        if (tileUnit.first.get() != tile)
            continue;

        const PlacedTileSet & tps = tileUnit.second;

        for (auto & placed : tps)
        {
            auto child = new QTreeWidgetItem(parent);
            child->setText(0,"Placement");
            if (placed->isExcluded())
                 child->setText(1,"Excluded");
            child->setText(2,Transform::info(placed->getPlacement()));

            NodeInfo info;
            info.type      = NodeInfo::Type::EdgePolyNode;
            info.payload   = (void*)&tile->getEdgePoly();
            info.key       =  makeKeyForNode(child,info);
            info.populated = false;
            setNodeInfo(child, info);
        }

        return;     // each tile only ocurrs once in tiling unit
    }
}

void page_system_info::populateBackgroundImage(QTreeWidgetItem * parent, const BkgdRecord & rec)
{
    // background image record
    auto child = new QTreeWidgetItem(parent);
    child->setText(0,"Background Image");
    if (!rec.isEmpty())
    {
        child->setText(2, rec.name + (rec.perspective.isIdentity() ? "" : "  (keystone adjusted)"));
        auto bkitem2 = new QTreeWidgetItem;
        bkitem2->setText(0,"Model Xform");
        bkitem2->setText(2,rec.xf.info(8));
        child->addChild(bkitem2);
    }
}

void page_system_info::populateEdgePoly(QTreeWidgetItem * parent, const EdgePoly * ep)
{
    for (const EdgePtr & edge : ep->get())
    {
        QTreeWidgetItem * child = new QTreeWidgetItem(parent);
        child->setText(0,"Edge");
        child->setText(1,addr(edge.get()));
        child->setText(2,edge->info());
    }
}

void page_system_info::populateViews(QTreeWidgetItem * parent)
{
    const QVector<Layer *> layers = Sys::systemViewController->getActiveLayers();

    for (Layer * layer : layers)
    {
        QTreeWidgetItem * item = new QTreeWidgetItem;
        item->setText(0,layer->layerName());
        item->setText(2,Transform::info(layer->getLayerTransform()));
        parent->addChild(item);
    }
}

void page_system_info::populateTaker(QTreeWidgetItem * parent, const Taker * taker)
{
    QSize     sz   =  taker->getSize();
    QColor color   =  taker->getColor();
    Layer::Type vt =  taker->getColorSource();
    QString str    =  QString("size: %1x%2 color: %3 source: %4").arg(sz.width()).arg(sz.height()).arg(color.name()).arg(sLayerType2[(int)vt]);

    QTreeWidgetItem * item = new QTreeWidgetItem;
    item->setText(0,"Taker");
    item->setText(2,str);
    parent->addChild(item);
}

// ===================== Functions =====================

void page_system_info::expandOneLevel(QTreeWidgetItem *item)
{
    NodeInfo info = getNodeInfo(item);
    if (info.type == NodeInfo::Type::None)
        return;

    populateChildren(item);
}

void page_system_info::resetSubtree(QTreeWidgetItem *item)
{
    // delete all children
    while (item->childCount() > 0)
        delete item->takeChild(0);

    // reset NodeInfo
    NodeInfo info = getNodeInfo(item);
    info.populated = false;
    setNodeInfo(item, info);
}

// ===================== Utils =====================

QString page_system_info::makeKeyForSection(NodeInfo & info) const
{
    Q_ASSERT(info.isValid());
    return QStringLiteral("Section_%1").arg(int(info.type));
}

QString page_system_info::makeKeyForNode(QTreeWidgetItem *item, const NodeInfo &info) const
{
    Q_ASSERT(info.isValid());

    QStringList parts;
    QTreeWidgetItem *cur = item;

    while (cur && cur != tree->invisibleRootItem())
    {
        int index = cur->parent()
        ? cur->parent()->indexOfChild(cur)
        : tree->indexOfTopLevelItem(cur);

        parts.prepend(QString::number(index));
        cur = cur->parent();
    }

    // Path like "0/2/1"
    QString path = parts.join('/');

    // Optionally include type if you want to distinguish different node types at same path
    return QString("%1|%2").arg(path).arg(int(info.type));
}

NodeInfo page_system_info::getNodeInfo(QTreeWidgetItem *item) const
{
    QVariant v = item->data(0, Qt::UserRole);
    if (!v.isValid())
        return NodeInfo{};
    return v.value<NodeInfo>();
}

void page_system_info::setNodeInfo(QTreeWidgetItem *item, const NodeInfo &info)
{
    item->setData(0, Qt::UserRole, QVariant::fromValue(info));

    // Mark as expandable
    item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
#if 0
    // Optional: add symbol or color
    QString label = item->text(0);
    if (!label.startsWith("▶ "))
        item->setText(0, "▶ " + label);
#endif
    if (Sys::isDarkTheme)
        item->setForeground(0, QBrush(Qt::yellow));
    else
        item->setForeground(0, QBrush(Qt::darkBlue));
}

QTreeWidgetItem * page_system_info::makeSection(const QString &title, NodeInfo::Type type)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, title);
    tree->addTopLevelItem(item);

    NodeInfo info;
    info.type = type;
    info.key  = makeKeyForSection(info);
    setNodeInfo(item, info);

    return item;
}

// ===================== Dump =====================

void page_system_info::dumpTree()
{
    QDate date = QDate::currentDate();
    QString stamp =  QString::number(date.toJulianDay()) + QTime::currentTime().toString();
    stamp = stamp.replace(':', '.');
    QString fileName  = Sys::getLogDir() + "wsDumpFile" + stamp + ".txt";
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
