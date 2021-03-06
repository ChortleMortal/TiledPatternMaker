﻿/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
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

#ifndef PAGE_LOADERS_H
#define PAGE_LOADERS_H

#include "panels/panel_page.h"
//#include "settings/configuration.h"
#include "panels/versioned_list_widget.h"
#include "enums/estatemachineevent.h"
#include "enums/edesign.h"


class page_loaders : public panel_page
{
    Q_OBJECT

public:
    explicit page_loaders(ControlPanel * apanel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override {}

    static int  whereTilingUsed(QString name, QStringList & results);

signals:
    void    sig_loadTiling(QString,eSM_Event);
    void    sig_loadMosaic(QString);
    void    sig_loadDesign(eDesign id);
    void    sig_buildDesign(eDesign id);

public slots:
   void     slot_newTile();
   void     slot_newXML();

   void     slot_mosaicLoaded(QString name);
   void     slot_tilingLoaded (QString name);
   void     slot_loadedDesign(eDesign design);

   void     desRightClick(QPoint pos);
   void     xmlRightClick(QPoint pos);
   void     tileRightClick(QPoint pos);

private slots:
    void    designSelected(QListWidgetItem * item, QListWidgetItem* oldItem);
    void    tilingSelected(QListWidgetItem * item, QListWidgetItem* oldItem);
    void    mosaicSelected(QListWidgetItem *item, QListWidgetItem *oldItem);
    void    designClicked(QListWidgetItem * item);
    void    tilingClicked(QListWidgetItem * item);
    void    mosaicClicked(QListWidgetItem *item);

    void    slot_itemEnteredToolTip(QListWidgetItem * item);

    void    loadShapes();
    void    loadTiling();

    void    loadXML();
    void    openXML();
    void    rebaseXML();
    void    renameXML();
    void    deleteXML();
    void    showTilings();

    //void    tilingLoadChanged(int id);
    void    openTiling();
    void    rebaseTiling();
    void    renameTiling();
    void    deleteTiling();
    void    slot_whereTilingUsed();

    void    loadTilingsCombo();
    void    loadMosaicCombo();
    void    loadDesignCombo();

    void    slot_mosaicFilter(const QString & filter);
    void    slot_tilingFilter(const QString & filter);
    void    slot_mosaicCheck(bool check);
    void    slot_tilingCheck(bool check);

    void    autoLoadStylesClicked(bool enb);
    void    autoLoadTilingClicked(bool enb);
    void    autoLoadDesignsClicked(bool enb);

protected:
    void    setupUI();
    void    refreshPanel();
    void    makeConnections();
    void    putNewTilingNameIntoDesign(QStringList & designs, QString newName);
    bool    putNewTilingNameIntoTiling(QString filename, QString newName);

private:
    VersionedListWidget * tileList;
    VersionedListWidget * mosaicList;
    LoaderListWidget    * designList;

    QPushButton * pbLoadShapes;
	QPushButton * pbLoadTiling;
    QPushButton * pbLoadXML;

    QCheckBox   * cbLoadMulti;
    QCheckBox   * cbLoadModify;

    QCheckBox   * mosaicFilterCheck;
    QLineEdit   * mosaicFilter;
    QCheckBox   * tilingFilterCheck;
    QLineEdit   * tilingFilter;

    eDesign     selectedDesign;
    QString     selectedXMLName;
    QString     selectedTilingName;

    QCheckBox    * cbAutoLoadMosaics;
    QCheckBox    * cbAutoLoadTiling;
    QCheckBox    * cbAutoLoadDesigns;
};

#endif
