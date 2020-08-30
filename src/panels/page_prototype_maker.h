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

#ifndef PAGE_FIGURE_MAKER_H
#define PAGE_FIGURE_MAKER_H

#include "panels/panel_page.h"
#include "makers/figure_maker/figure_editors.h"

class page_prototype_maker : public panel_page
{
    Q_OBJECT

public:
    page_prototype_maker(ControlPanel * cpanel);

    void	refreshPage(void) override;
    void    onEnter() override;
    void    onExit() override;
    void    setupFigure(bool isRadial);

public slots:
    void    slot_tilingChanged();
    void    slot_loadedXML(QString name);
    void    slot_loadedTiling (QString name);
    void    slot_replaceInStyle();
    void    slot_addToStyle();
    void    slot_render();
    void    slot_reload();

 private slots:
    void    slot_unload();
    void    whiteClicked(bool state);
    void    repRadClicked(bool state);
    void    hiliteClicked(bool state);
    void    slot_duplicateCurrent();
    void    slot_prototypeSelected(int);

protected:
    void    reload();

    PrototypeMaker  * prototypeMaker;

private:
    QCheckBox      * whiteBackground;
    QCheckBox      * replicateRadial;
    QCheckBox      * hiliteUnit;
    QComboBox      * protoListBox;
};

#endif
