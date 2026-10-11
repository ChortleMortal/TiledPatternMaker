#pragma once

#include "model/tilings/backgroundimage.h"

#include <memory>
#include <QHash>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QCheckBox>

#include "gui/panels/panel_page.h"
#include "model/makers/proto_view_data.h"
#include "sys/geometry/edge_poly.h"

class Motif;

typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Style>            StylePtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class DCEL>             DCELPtr;
typedef std::shared_ptr<class Crop>             CropPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Tile>             TilePtr;
typedef std::shared_ptr<class Layer>            LayerPtr;
typedef std::shared_ptr<class TileMotif>        TileMotifPtr;
typedef std::shared_ptr<class BackgroundImage>  BkgdImagePtr;

struct NodeInfo
{
    enum class Type
    {
        None = 0,

        // Top-level sections
        MosaicMakerSection,
        PrototypeMakerSection,
        MotifMakerSection,
        TilingMakerSection,
        BackgroundImageSection,
        ViewsSection,
        MapEditorSection,
        CropMakerSection,
        ImageViewerSection,
        SystemViewSection,

        // deeper lazy nodes
        PrototypeNode,
        TileMotifNode,
        TilingNode,
        MapNode,
        StylesNode,
        CropNode,
        BackgroundImageNode,
        EdgePolyNode,
        VerticesNode,
        EdgesNode,
        TilingUnitNode,
        MotifNode,
        ExtendersNode,
    };

    bool isValid()  const { return type != Type::None; }

    Type    type      = Type::None;
    void   *payload   = nullptr;   // raw pointer to underlying object (lifetime managed elsewhere)
    void   *payload2  = nullptr;   // raw pointer to underlying object (lifetime managed elsewhere)
    bool    populated = false;     // have we already populated this node's children?
    QString key;                   // persistent key for expansionDepth
};

Q_DECLARE_METATYPE(NodeInfo)

class page_system_info : public panel_page
{
    Q_OBJECT

public:
    explicit page_system_info(ControlPanel * cpanel);
    ~page_system_info();

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void populateTree();
    void simpleExpand();
    void dumpTree();
    void slot_itemClicked(QTreeWidgetItem *, int);
    void onItemExpanded(QTreeWidgetItem *item);
    void onItemCollapsed(QTreeWidgetItem *item);

protected:
    void populateChildren(QTreeWidgetItem *item);

    // Section population (called lazily from populateChildren)
    void doMosaicMaker(QTreeWidgetItem * sectionItem);
    void doProtoypeInfo(QTreeWidgetItem * sectionItem);
    void doMotifMaker(QTreeWidgetItem * sectionItem);
    void doTilingMaker(QTreeWidgetItem * sectionItem);
    void doBackgroundImage(QTreeWidgetItem * sectionItem);
    void doViews(QTreeWidgetItem * sectionItem);
    void doMapEditor(QTreeWidgetItem * sectionItem);
    void doCropMaker(QTreeWidgetItem * sectionItem);
    void doImageViewer(QTreeWidgetItem * sectionItem);
    void doSystemView(QTreeWidgetItem * sectionItem);

    // Detailed population helpers
    void populateStyle(QTreeWidgetItem * parent, Style * style);
    void populateTiling(QTreeWidgetItem * parent, Tiling *tp, QString name);
    void populateTilingUnit(QTreeWidgetItem * parent, Tiling *tp);
    void populatePlacements(QTreeWidgetItem * parent, Tiling *tiling, Tile *tile);
    void populatePrototype(QTreeWidgetItem * parent, Prototype * pp, QString name, QString visibility);
    void populatePrototypeNode(QTreeWidgetItem * parent, Prototype * pp);
    void populateTileMotif(QTreeWidgetItem * parent, TileMotifPtr timo, QString name, QString state);
    void populateTileMotifNode(QTreeWidgetItem * parent, TileMotif * timo);
    void populateMotifNode(QTreeWidgetItem * parent, Motif * motif);
    void populateMotifExtenders(QTreeWidgetItem * parent, Motif * motif);
    void populateBackgroundImage(QTreeWidgetItem * parent, const BkgdRecord & rec);
    void populateCrop(QTreeWidgetItem * parent, QString name, Crop * crop);
    void populateLayer(QTreeWidgetItem * parent, Layer * layer);
    void populateMap(QTreeWidgetItem *parent, Map *map, QString name);
    void populateMapNode(QTreeWidgetItem *parent, Map * map);
    void populateMapEdges(QTreeWidgetItem *parent, Map * map);
    void populateMapVertices(QTreeWidgetItem *parent, Map * map);
    void populateViews(QTreeWidgetItem * parent);
    void populateEdgePoly(QTreeWidgetItem * parent, const EdgePoly *ep);
    void populateTaker(QTreeWidgetItem * parent, const class Taker * taker);

    void dumpWalkTree(QTextStream &ts, QTreeWidgetItem *item );

    // Lazy-loading helpers
    QTreeWidgetItem * makeSection(const QString &title, NodeInfo::Type type);

    bool isExpandable(QTreeWidgetItem *item);
    void expandOneLevel(QTreeWidgetItem *item);
    void resetSubtree(QTreeWidgetItem *item);

    void     setNodeInfo(QTreeWidgetItem *item, const NodeInfo &info);
    NodeInfo getNodeInfo(QTreeWidgetItem *item) const;
    QString  makeKeyForNode(QTreeWidgetItem *item, const NodeInfo &info) const;
    QString  makeKeyForSection(NodeInfo & node) const;

    void     saveExpansionState();
    void     restoreExpansionState();
    void     collectExpanded(QTreeWidgetItem *item, QSet<QString> &out);
    void     restoreExpanded(QTreeWidgetItem *item, const QSet<QString> &expanded);

private:
    void    saveState();
    void    loadState();

    QTreeWidget     * tree;
    QCheckBox       * chkLock;
    bool              firstTime;
    bool              m_restoringState;

    // Per-section expansion state: sectionKey -> set of expanded item keys
    QHash<QString, QSet<QString>> m_expandedPerSection;
};
