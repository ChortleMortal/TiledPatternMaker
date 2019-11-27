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

#ifndef FIGURE_MAKER_H
#define FIGURE_MAKER_H

#include "base/shared.h"
#include "tile/Tiling.h"
#include <QtWidgets>

class FeatureButton;
class FeatureLauncher;
class MasterFigureEditor;
class TiledPatternMaker;

class FigureMaker : public QVBoxLayout
{
    Q_OBJECT

public:
    FigureMaker(TiledPatternMaker * maker);

    void unload();

    void setNewTiling(TilingPtr tiling);
    void setNewStyle(StylePtr style);
    void setTilingChanged();

    void        setActiveFeature(FeatureBtnPtr fb);
    FeaturePtr  getActiveFeature();
    bool        duplicateActiveFeature();

    PrototypePtr getPrototype();
    PrototypePtr makePrototype();

    StylePtr  createDefaultStyleFromPrototype();

    MapPtr createExplicitGirihMap(int starSides, qreal starSkip);
    MapPtr createExplicitHourglassMap(qreal d, int s);
    MapPtr createExplicitInferredMap();
    MapPtr createExplicitIntersectMap(int starSides, qreal starSkip, int s, bool progressive);
    MapPtr createExplicitRosetteMap(qreal q, int s, qreal r);
    MapPtr createExplicitStarMap(qreal d, int s);
    MapPtr createExplicitFeatureMap();

    bool   verify();

signals:
    void sig_viewWS();

public slots:
    void slot_figureChanged();
    void slot_launcherButton();

private slots:

protected:
     PrototypePtr createPrototypeFromLauncherButtons();
     void         populateProtoWithDELsFromLauncherButtons(PrototypePtr proto);
     QPolygonF    boundingRect();

private:
    TilingPtr         tiling;
    MapPtr            nullMap;      // DAC
    PrototypePtr      designPrototype;

    FeatureBtnPtr        viewerBtn;
    FeatureLauncher   *  launcher;
    MasterFigureEditor*	 masterEdit;
    TiledPatternMaker *  maker;
};

#endif
