#pragma once
#include "geometry/edgepoly.h"
#ifndef PAGE_SYSTEM_INFO_H
#define PAGE_SYSTEM_INFO_H

#include "panels/panel_page.h"
#include "makers/prototype_maker/prototype_data.h"

typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Crop>             CropPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
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
    void doMosaicMaker();
    void doProtoypeMaker(eMVDType type, QString name);
    void doTilingMaker();
    void doBackgroundImage();
    void doViews();
    void doMapEditor();
    void doCropMaker();

    void populateStyles(QTreeWidgetItem * parent, MosaicPtr mosaic);
    void populateBorder(QTreeWidgetItem * parent, MosaicPtr mosaic);
    void populateCrop(QTreeWidgetItem * parent, CropPtr crop);
    void populateLayer(QTreeWidgetItem * parent, Layer * layer);
    void populateMap(QTreeWidgetItem *parent, MapPtr mp, QString name);
    void populatePrototype(QTreeWidgetItem * parent, ProtoPtr pp, QString name, QString state);
    void populateTiling(QTreeWidgetItem * parent, TilingPtr tp, QString name);
    void populateDEL(QTreeWidgetItem * parent, DesignElementPtr de, QString name, QString state);
    void populateViews(QTreeWidgetItem * parent);
    void populateEdgePoly(QTreeWidgetItem * parent, const EdgePoly &ep);
    void dumpWalkTree(QTextStream &ts, QTreeWidgetItem *item );

private:
    QTreeWidget     * tree;
    QTreeWidgetItem * item;
    QTreeWidgetItem * item2;
    QPushButton     * pbPopulate;
    QCheckBox       * lockCheck;
    CropPtr         crop;

};

#endif
