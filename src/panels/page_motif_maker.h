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

#ifndef PAGE_MOTIF_MAKER_H
#define PAGE_MOTIF_MAKER_H

#include "panels/panel_page.h"
#include "panels/motif_display_widget.h"
#include "makers/motif_maker/figure_editors.h"
#include "tapp/figure.h"

class page_motif_maker : public panel_page
{
    Q_OBJECT

public:
    page_motif_maker(ControlPanel * cpanel);

    void	refreshPage(void) override;
    void    onEnter() override;
    void    onExit() override;
    void    setupFigure(bool isRadial);
    void    featureChanged();
    void    tilingChanged();
    void    tilingChoicesChanged();

    void     select(PrototypePtr prototype);
    FeaturePtr getActiveFeature();

public slots:
    void slot_figureChanged(FigurePtr fig);

private slots:
    void    whiteClicked(bool state);
    void    replicateRadialClicked(bool state);
    void    hiliteClicked(bool state);
    void    slot_duplicateCurrent();
    void    slot_deleteCurrent();
    void    slot_prototypeSelected(int);
    void    slot_figureTypeChanged(eFigType type);

protected:

private:
    QCheckBox   * whiteBackground;
    QCheckBox   * replicateRadial;
    QCheckBox   * hiliteUnit;
    QComboBox   * tilingListBox;

    MotifDisplayWidget  * motifWidget;
};

#endif
