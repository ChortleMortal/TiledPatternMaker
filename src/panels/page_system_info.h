#ifndef PAGE_SYSTEM_INFO_H
#define PAGE_SYSTEM_INFO_H

class QTreeWidgetItem;
class QTextStream;
class QTreeWidget;
class QCheckBox;
class QPushButton;

#include "widgets/panel_page.h"

typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Crop>             CropPtr;
typedef std::shared_ptr<class Prototype>        PrototypePtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Layer>            LayerPtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;

class page_system_info : public panel_page
{
    Q_OBJECT

public:
    page_system_info(ControlPanel * cpanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void populateTree();
    void dumpTree();
    void expandTree();
    void slot_itemClicked(QTreeWidgetItem * item, int col);

protected:
    void populateStyles(QTreeWidgetItem * parent, MosaicPtr mosaic);
    void populateBorder(QTreeWidgetItem * parent, MosaicPtr mosaic);
    void populateCrop(QTreeWidgetItem * parent, CropPtr crop);
    void populateLayer(QTreeWidgetItem * parent, Layer * layer);
    void populateMap(QTreeWidgetItem *parent, MapPtr mp, QString name);
    void populatePrototype(QTreeWidgetItem * parent, PrototypePtr pp, QString name);
    void populateTiling(QTreeWidgetItem * parent, TilingPtr tp, QString name);
    void populateDEL(QTreeWidgetItem * parent, DesignElementPtr de);
    void populateViews(QTreeWidgetItem * parent);

    void dumpWalkTree(QTextStream &ts, QTreeWidgetItem *item );

private:
    QTreeWidget     * tree;
    QPushButton     * pbPopulate();
    QCheckBox       * lockCheck;
};

#endif
