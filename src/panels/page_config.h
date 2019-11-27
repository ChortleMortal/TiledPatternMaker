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

#ifndef PAGE_CONFIG_H
#define PAGE_CONFIG_H

#include "panel_page.h"

class page_config : public panel_page
{
    Q_OBJECT

public:
    page_config(ControlPanel * cpanel);

    void refreshPage() override;
    void onEnter() override;

signals:

private slots:
    void    selectRootTileDir();
    void    selectNewTileDir();
    void    selectRootDesignDir();
    void    selectRootImageDir();
    void    selectNewDesignDir();
    void    selectExamplesDir();

    void    rootDesignChanged(QString txt);
    void    rootImageChanged(QString txt);
    void    newDesignChanged(QString txt);
    void    rootTileChanged(QString txt);
    void    newTtileChanged(QString txt);
    void    examplesChanged(QString txt);

    void    slot_reconfigurePaths();
    void    slot_verifyMapsClicked(bool enb);
    void    slot_verifyDumpClicked(bool enb);
    void    slot_verifyVerboseClicked(bool enb);

protected:
    void  updatePaths();

private:
    QPushButton *rootDesignBtn;
    QPushButton *newDesignBtn;
    QPushButton *rootTileBtn;
    QPushButton *newTileBtn;
    QPushButton *rootImageBtn;
    QPushButton *examplesBtn;

    QLineEdit   *le_rootDesign;
    QLineEdit   *le_newDesign;
    QLineEdit   *le_rootTile;
    QLineEdit   *le_newTile;
    QLineEdit   *le_rootImage;
    QLineEdit   *le_examples;

};

#endif
