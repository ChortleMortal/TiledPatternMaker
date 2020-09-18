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

#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "base/shared.h"
#include "base/mosaic.h"
#include "base/misc.h"
#include "designs/design.h"
#include "tapp/design_element.h"
#include "viewers/workspace_viewer.h"

enum eProtoMode
{
    PROTO_DRAW_MAP       =  0x01,
    PROTO_DRAW_FEATURES  =  0x02,
    PROTO_DRAW_FIGURES   =  0x04
};

class Workspace : public WorkspaceViewer
{
    Q_OBJECT

public:
    static Workspace * getInstance();
    static void        releaseInstance();

    void        init();

    // designs
    void        addDesign(DesignPtr d);
    QVector<DesignPtr> & getDesigns();
    QString     getDesignName() { return designName; }
    void        clearDesigns();

    // Mosaic
    void        resetMosaic() { resetAll(); }
    void        setMosaic(MosaicPtr mosaic);
    MosaicPtr   getMosaic();

    // loaded tilings
    void        resetTilings();
    void        removeTiling(TilingPtr tp);
    void        replaceTiling(TilingPtr oldtp, TilingPtr newtp);
    void        setCurrentTiling(TilingPtr tp);

    UniqueQVector<TilingPtr> getTilings() { return tilings; }
    TilingPtr   getCurrentTiling();
    TilingPtr   findTiling(QString name);

    WorkspaceSettings & getMosaicSettings();
    WorkspaceSettings & getSettings();

    // prototypes
    void         addPrototype(PrototypePtr pp);
    UniqueQVector<PrototypePtr> getPrototypes();

    // selections
    void            selectFeature(WeakFeaturePtr fp);
    void            setSelectedPrototype(WeakPrototypePtr proto);
    void            setSelectedDesignElement(WeakDesignElementPtr del);

    FeaturePtr      getSelectedFeature();
    PrototypePtr    getSelectedPrototype();
    DesignElementPtr getSelectedDesignElement();

    void    setProtoMode(eProtoMode mode, bool enb);
    int     getProtoMode() {  return  protoViewMode; }

    void       setFillData(FillData & fd) { fillData = fd; }
    FillData & getFillData() { return fillData; }

protected:
    void resetAll();

signals:
    void    sig_newTiling();
    void    sig_selected_dele_changed();
    void    sig_selected_proto_changed();

public slots:
    void slot_clearCanvas();
    void slot_clearWorkspace();

private:
    Workspace();
    ~Workspace();

    static Workspace      * mpThis;
    Configuration         * config;

    QVector<DesignPtr>      activeDesigns;
    QString                 designName;

    MosaicPtr                   mosaic;
    UniqueQVector<TilingPtr>    tilings;
    UniqueQVector<PrototypePtr> prototypes;

    TilingPtr               currentTiling;
    WeakPrototypePtr        selectedPrototype;
    WeakDesignElementPtr    selectedDesignElement;
    WeakFeaturePtr          selectedFeature;

    FillData                fillData;

    int                     protoViewMode;
};

#endif // WORKSPACE_H
