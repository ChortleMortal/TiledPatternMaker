#pragma once
#ifndef PAGE_SYSTEM_INFO_H
#define PAGE_SYSTEM_INFO_H

#include "gui/panels/panel_page.h"
#include "model/prototypes/prototype_data.h"
#include "sys/geometry/edgepoly.h"

typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class DCEL>             DCELPtr;
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
    void populateAll();
    void populateMakers();

    void doMosaicMaker();
    void doMosaicMakerSummary();
    void doProtoypeMaker(eMVDType type, QString name);
    void doProtoypeMakerSummary(eMVDType type, QString name);
    void doTilingMaker();
    void doTilingMakerSummary();


    void doBackgroundImage();
    void doViews();
    void doMapEditor();
    void doCropMaker();

    void populateStyles(QTreeWidgetItem * parent, MosaicPtr mosaic);
    void populateStylesSummary(QTreeWidgetItem * parent, MosaicPtr mosaic);
    void populatePrototype(QTreeWidgetItem * parent, ProtoPtr pp, QString name, QString state);
    void populatePrototypeSummary(QTreeWidgetItem * parent, ProtoPtr pp, QString name, QString state);
    void populateTiling(QTreeWidgetItem * parent, TilingPtr tp, QString name);
    void populateTilingSummary(QTreeWidgetItem * parent, TilingPtr tp, QString name);
    void populateDEL(QTreeWidgetItem * parent, DesignElementPtr de, QString name, QString state);
    void populateDELSummary(QTreeWidgetItem * parent, DesignElementPtr de, QString name, QString state);

    void populateBorder(QTreeWidgetItem * parent, MosaicPtr mosaic);
    void populateCrop(QTreeWidgetItem * parent, CropPtr crop);
    void populateLayer(QTreeWidgetItem * parent, Layer * layer);
    void populateMap(QTreeWidgetItem *parent, MapPtr mp, QString name);
    void populateViews(QTreeWidgetItem * parent);
    void populateEdgePoly(QTreeWidgetItem * parent, const EdgePoly &ep);
    void dumpWalkTree(QTextStream &ts, QTreeWidgetItem *item );

private:
    QTreeWidget     * tree;
    QTreeWidgetItem * item;
    QTreeWidgetItem * item2;
    QPushButton     * pbPopulate;
    QCheckBox       * lockCheck;

    QRadioButton    * rMakers;
    QRadioButton    * rAll;

    CropPtr         crop;

};

#endif
