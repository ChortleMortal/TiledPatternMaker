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

#ifndef PAGE_LOADERS_H
#define PAGE_LOADERS_H

#include "base/configuration.h"
#include "panels/panel_page.h"

class page_loaders : public panel_page
{
    Q_OBJECT

public:
    explicit page_loaders(ControlPanel * apanel);

    void refreshPage() override;
    void onEnter() override;

signals:
    void    sig_loadTiling(QString);
    void    sig_loadXML(QString);
    void    sig_loadDesign(eDesign id);
    void    sig_buildDesign(eDesign id);
    void    sig_viewStyles();
    void    sig_viewWS();

public slots:
   void     slot_newTile();
   void     slot_newXML();

   void     slot_loadedXML(QString name);
   void     slot_loadedTiling (QString name);
   void     slot_loadedDesign(eDesign design);

   void     desRightClick(QPoint pos);
   void     xmlRightClick(QPoint pos);
   void     tileRightClick(QPoint pos);

private slots:
    void    designSelected(QListWidgetItem * item, QListWidgetItem* oldItem);
    void    tilingSelected(QListWidgetItem * item, QListWidgetItem* oldItem);
    void    xmlSelected(QListWidgetItem *item, QListWidgetItem *oldItem);
    void    slot_itemEnteredToolTip(QListWidgetItem * item);

    void    loadShapes();

    void    loadXML();
    void    openXML();
    void    rebaseXML();
    void    renameXML();
    void    deleteXML();

    void    loadTiling();
    void    openTiling();
    void    rebaseTiling();
    void    renameTiling();
    void    deleteTiling();
    void    slot_whereTilingUsed();

    void    loadTilingsCombo();
    void    loadXMLCombo();
    void    loadDesignCombo();

    void    slot_designCheck(bool check);
    void    slot_tilingCheck(bool check);

protected:
    void    setupUI();
    void    refreshPanel();
    void    makeConnections();
    int     whereTilingUsed(QString name, QStringList & results);
    void    putNewTilingNameIntoDesign(QStringList & designs, QString newName);
    bool    putNewTilingNameIntoTiling(QString filename, QString newName);

private:
    LoaderListWidget * tileList;
    LoaderListWidget * xmlList;
    LoaderListWidget * designList;

    QPushButton * pbLoadShapes;
    QPushButton * pbLoadTiling;
    QPushButton * pbLoadXML;

    QCheckBox   * designFilterCheck;
    QLineEdit   * designFilter;
    QCheckBox   * tilingFilterCheck;
    QLineEdit   * tilingFilter;

    eDesign     selectedDesign;
    QString     selectedXMLName;
    QString     selectedTilingName;
};

#endif
