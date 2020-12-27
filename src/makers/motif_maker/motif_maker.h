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

#ifndef MOTIF_MAKER_H
#define MOTIF_MAKER_H

#include <QtWidgets>
#include "panels/page_motif_maker.h"
#include "base/misc.h"
#include "base/configuration.h"

enum eMMState
{
    MM_EMPTY,
    MM_SINGLE,
    MM_MULTI
};

static QString mm_states[]
{
    E2STR(MM_EMPTY),
    E2STR(MM_SINGLE),
    E2STR(MM_MULTI)
};

class MotifMaker
{
public:
    static MotifMaker * getInstance();

    void         init();
    void         setCallback(page_motif_maker * menu) { this->menu = menu; }

    void         takeDown(PrototypePtr prototype);
    void         sm_take(TilingPtr tiling, eSM_Event event);

    void         erasePrototypes();
    void         removePrototype(TilingPtr tiling);

    const QVector<PrototypePtr> & getPrototypes();

    PrototypePtr findPrototypeByName(TilingPtr tiling);
    PrototypePtr getSelectedPrototype();
    void         setSelectedPrototype(PrototypePtr pp);
    void         resetSelectedPrototype() { _selectedPrototype.reset(); _selectedDesignElement.reset(); }

    DesignElementPtr getSelectedDesignElement() { return  _selectedDesignElement; }
    void         setSelectedDesignElement(DesignElementPtr del) { _selectedDesignElement = del; }
    void         resetSelectedDesignElement() { _selectedDesignElement.reset(); }

    void         duplicateActiveFeature();
    void         deleteActiveFeature();

    MapPtr createExplicitGirihMap(int starSides, qreal starSkip);
    MapPtr createExplicitHourglassMap(qreal d, int s);
    MapPtr createExplicitInferredMap();
    MapPtr createExplicitIntersectMap(int starSides, qreal starSkip, int s, bool progressive);
    MapPtr createExplicitRosetteMap(qreal q, int s, qreal r);
    MapPtr createExplicitStarMap(qreal d, int s);
    MapPtr createExplicitFeatureMap();

    void   setActiveFeature(FeaturePtr feature) { _activeFeature = feature; }
    FeaturePtr getActiveFeature() { return _activeFeature; }

protected:
    eMMState sm_getState();
    void     sm_eraseAllCreateAdd(TilingPtr tiling);
    void     sm_eraseCurrentCreateAdd(TilingPtr tiling);
    void     sm_createAdd(TilingPtr tiling);
    void     sm_replaceTiling(PrototypePtr prototype, TilingPtr tiling);
    void     sm_resetMaps();

    PrototypePtr createPrototype(TilingPtr tiling);
    void         recreatePrototype(TilingPtr tiling);
    void         recreateFigures(TilingPtr tiling);
    bool         askNewProto();

private:
    MotifMaker();

    static MotifMaker * mpThis;

    Configuration          * config;
    ViewControl            * vcontrol;
    TiledPatternMaker      * maker;
    page_motif_maker       * menu;
    class TilingMaker      * tilingMaker;
    class DecorationMaker  * decorationMaker;

    UniqueQVector<PrototypePtr> prototypes;
    PrototypePtr        _selectedPrototype;
    DesignElementPtr    _selectedDesignElement;
    FeaturePtr          _activeFeature;

    MapPtr              nullMap;
};

#endif
