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

#include "panels/panel_page.h"

class page_config : public panel_page
{
    Q_OBJECT

public:
    page_config(ControlPanel * cpanel);

    void refreshPage() override;
    void onEnter() override;
    void onExit() override {}

private slots:
    void    selectRootDesignDir();
    void    selectRootImageDir();
    void    selectXMLTool();

    void    rootDesignChanged(QString txt);
    void    rootImageChanged(QString txt);
    void    designDefaultChanged(bool checked);
    void    imageDefaultChanged(bool checked);

    void    slot_reconfigurePaths();

    void    slot_mode(int id);

    void    slot_showCenterChanged(int state);

protected:
    void    updatePaths();

    QGroupBox   * createViewControl();

private:
    QPushButton * rootDesignsBtn;
    QPushButton * rootImagesBtn;
    QPushButton * xmlToolBtn;

    QLineEdit   * le_rootDesigns;
    QLineEdit   * le_rootImages;
    QLineEdit   * le_xmlTool;

    QCheckBox   * defaultDesigns;
    QCheckBox   * defaultImages;
};

#endif
