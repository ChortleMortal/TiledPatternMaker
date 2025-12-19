#pragma once
#ifndef PAGE_SYSTEM_INFO_H
#define PAGE_SYSTEM_INFO_H

#include "gui/panels/panel_page.h"
#include "model/makers/prototype_maker_data.h"
#include "sys/geometry/edge_poly.h"

typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class DCEL>             DCELPtr;
typedef std::shared_ptr<class Crop>             CropPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Layer>            LayerPtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;
typedef std::shared_ptr<class BackgroundImage>  BkgdImagePtr;

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
    void slot_itemClicked(QTreeWidgetItem * item, int col);

protected:
    void populateAll();
    void populateMakers();

    void doTilingMaker(bool summary);
    void doProtoypeMaker(eMVDType type, QString name, bool summary);

    void doMosaicMaker();
    void doMosaicMakerSummary();

    void doBackgroundImage();
    void doViews();
    void doMapEditor();
    void doCropMaker();
    void doImageViewer();

    void populateTiling(QTreeWidgetItem * parent, TilingPtr tp, QString name, bool summary, bool terse);
    void populatePrototype(QTreeWidgetItem * parent, ProtoPtr pp, QString name, QString state, bool summary);
    void populateDEL(QTreeWidgetItem * parent, DesignElementPtr del, QString name, QString state, bool summary);

    void populateStyles(QTreeWidgetItem * parent, MosaicPtr mosaic);
    void populateStylesSummary(QTreeWidgetItem * parent, MosaicPtr mosaic);

    void populateBackgroundImage(QTreeWidgetItem * parent, BkgdImagePtr bip);
    void populateBorder(QTreeWidgetItem * parent, MosaicPtr mosaic);
    void populateCrop(QTreeWidgetItem * parent, CropPtr crop);
    void populateLayer(QTreeWidgetItem * parent, Layer * layer);
    void populateMap(QTreeWidgetItem *parent, MapPtr mp, QString name);
    void populateViews(QTreeWidgetItem * parent);
    void populateEdgePoly(QTreeWidgetItem * parent, const EdgePoly &ep, QTreeWidgetItem * item);
    void dumpWalkTree(QTextStream &ts, QTreeWidgetItem *item );

private:
    QTreeWidget     * tree;

    QCheckBox       * chkExpand;
    QCheckBox       * chkLock;
    QCheckBox       * chkShowEpoly;

    CropPtr         crop;
};

#endif
