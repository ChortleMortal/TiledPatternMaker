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

#ifndef MASTER_FIGURE_EDITOR_H
#define MASTER_FIGURE_EDITOR_H

#include <QtWidgets>
#include "base/shared.h"
#include "makers/motif_maker/feature_button.h"
#include "makers/motif_maker/figure_editors.h"
#include "makers/motif_maker/explicit_figure_editors.h"
#include "panels/panel_misc.h"

class MasterFigureWidget : public AQWidget
{
public:
    MasterFigureWidget();
    MasterFigureWidget(FigureEditor * fe);

    void setEditor(FigureEditor * fe);
};


class FigTypeChoiceCombo : public QComboBox
{
    Q_OBJECT

public:
    FigTypeChoiceCombo(class MasterFigureEditor * editor);

    void select(eFigType figType);

    void updateChoices(FigurePtr figure);
    void addChoice(eFigType type, QString name);
    int  getChoiceIndex(eFigType type);


signals:
    void sig_figureTypeChanged(eFigType);

private slots:
    void slot_figureTypeSelected(int index);
};

////////////////////////////////////////////////////////////////////////////
//
// MasterFigureEditor.java
//
// The top-level figure editor that understands the complete range of
// figure editors available in the applet and branches out to the right
// kind of editor as the DesignElement being edited is changed.

class MasterFigureEditor : public QWidget
{
    Q_OBJECT

public:
    MasterFigureEditor(page_motif_maker * menu);

    void  masterResetWithFigure(FigurePtr figure);

public slots:
    void slot_figureTypeChanged(eFigType type);

protected:
    void  selectCurrentEditor(FigureEditor* fe);
    FigureEditor * getEditor(eFigType type);
    void  figureTypeChanged(eFigType type, bool doEmit = true);


private:
    FigurePtr               masterFigure;

    Configuration           * config;

    // Explicit figure editors.
    ExplicitEditor          * explicit_edit;
    ExplicitInferEditor     * explcit_infer_edit;
    ExplicitStarEditor      * explict_star_edit;
    ExplicitRosetteEditor   * explicit_rosette_edit;
    ExplicitHourglassEditor * explicit_hourglass_edit;
    ExplicitGirihEditor     * explicit_girih_edit;
    ExplicitIntersectEditor * explicit_intersect_edit;
    ExplicitFeatureEditor   * explicit_feature_edit;

    // Radial figure editors.
    StarEditor	            * radial_star_edit;
    RosetteEditor           * radial_rosette_edit;
    ConnectRosetteEditor    * connect_rosette_edit;
    ConnectStarEditor       * connect_star_edit;
    ExtendedStarEditor      * ex_star_edit;
    ExtendedRosetteEditor   * ex_rosette_edit;

    QHBoxLayout             * comboLayout;
    FigTypeChoiceCombo      * choiceCombo2;

    MasterFigureWidget      * mfw;
};

#endif

