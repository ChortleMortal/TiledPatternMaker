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

class WorkspaceData
{
public:
    void    clear();

    MosaicPtr           mosaic;
    TilingPtr           tiling;
    UniqueQVector<PrototypePtr> prototypes;

    PrototypePtr        selectedPrototype;
    DesignElementPtr    selectedDesignElement;
};

class Workspace : public QObject
{
    Q_OBJECT

public:
    static Workspace * getInstance();
    static void        releaseInstance();

    void        init();
    void        clear() { ws.clear(); }

    // loaders
    bool        loadMosaic(QString name);
    bool        saveMosaic(QString name, QString &savedName, bool forceOverwrite);
    bool        loadTiling(QString name);
    bool        saveTiling(QString name, TilingPtr tp);

    // designs
    void        addDesign(DesignPtr d);
    QVector<DesignPtr> & getDesigns();
    QString     getDesignName() { return designName; }
    void        clearDesigns();

    // Mosaic
    void        setMosaic(MosaicPtr mosaic);
    MosaicPtr   getMosaic();

    // loaded tilings
    void        setTiling( TilingPtr t) { ws.tiling = t; }
    TilingPtr   getTiling();

    // prototypes
    void         addPrototype(PrototypePtr pp);
    QVector<PrototypePtr> getPrototypes();

    void            setSelectedPrototype(PrototypePtr proto);
    PrototypePtr    getSelectedPrototype();

    // figures
    void             setSelectedDesignElement(DesignElementPtr del);
    DesignElementPtr getSelectedDesignElement();

    CanvasSettings & getMosaicSettings();

protected:

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

    static Workspace    * mpThis;
    Configuration       * config;
    View                * view;

    QVector<DesignPtr>  activeDesigns;
    QString             designName;

    WorkspaceData       ws;

};

#endif // WORKSPACE_H
