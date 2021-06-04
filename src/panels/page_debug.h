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

#ifndef PAGE_DEBUG_H
#define PAGE_DEBUG_H

#include "panels/panel_page.h"
#include "base/configuration.h"

class page_debug : public panel_page
{
    Q_OBJECT

public:
    page_debug(ControlPanel * cpanel);

    void    refreshPage() override;
    void    onEnter() override;
    void    onExit() override;

private slots:
    void    slot_reformatDesignXML();
    void    slot_reprocessDesignXML();
    void    slot_examineAllMosaics();

    void    slot_verifyMapsClicked(bool enb);
    void    slot_verifyDumpClicked(bool enb);
    void    slot_verifyVerboseClicked(bool enb);

    void    slot_reprocessTilingXML();
    void    slot_reformatTilingXML();
    void    slot_verifyTilingNames();
    void    slot_verifyTiling();
    void    slot_verifyAllTilings();

protected:
    void    verifyTiling(TilingPtr tiling);

    QGroupBox   * createDebugSection();
    QGroupBox   * createVerifyMaps();

private:
};

#endif
