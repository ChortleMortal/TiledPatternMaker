/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PAGE_SYSTEM_INFO_H
#define PAGE_SYSTEM_INFO_H

#include "panels/panel_page.h"

typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Prototype>        PrototypePtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Layer>            LayerPtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;

class page_system_info : public panel_page
{
    Q_OBJECT

public:
    page_system_info(ControlPanel * cpanel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void populateTree();
    void dumpTree();
    void expandTree();
    void slot_itemClicked(QTreeWidgetItem * item, int col);

protected:
    void populateStyles(QTreeWidgetItem * parent, MosaicPtr mosaic);
    void populateLayer(QTreeWidgetItem * parent, Layer * layer);
    void populateMap(QTreeWidgetItem *parent, MapPtr mp);
    void populatePrototype(QTreeWidgetItem * parent, PrototypePtr pp, QString name);
    void populateTiling(QTreeWidgetItem * parent, TilingPtr tp, QString name);
    void populateDEL(QTreeWidgetItem * parent, DesignElementPtr de);

    void dumpWalkTree(QTextStream &ts, QTreeWidgetItem *item );

private:
    QTreeWidget     * tree;
    QPushButton     * pbPopulate();
    QCheckBox       * lockCheck;
};

#endif
