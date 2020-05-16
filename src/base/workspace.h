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
#include "base/styleddesign.h"
#include "designs/design.h"
#include "tapp/DesignElement.h"

enum eWsData
{
    WS_LOADED = 0,
    WS_TILING = 1
};

class WorkspaceData
{
public:
    void    clear();

    StyledDesign        styles;
    TilingPtr           tiling;
    PrototypePtr        prototype;      // set by FigureMaker and page-styles
    DesignElementPtr    desEle;         // set by FigureMaker and tiling maker
};

class Workspace : public QObject
{
    Q_OBJECT

public:
    static Workspace * getInstance();
    static void        releaseInstance();

    void        init();
    void        clear(eWsData dataset) { ws[dataset].clear(); }

    // loaders
    bool        loadDesignXML(QString name);
    bool        saveStyledDesign(eWsData wsdata, QString name, QString &savedName, bool forceOverwrite);
    bool        loadTiling(QString name);
    bool        saveTiling(QString name, TilingPtr tp);

    // designs
    void        addDesign(DesignPtr d);
    QVector<DesignPtr> & getDesigns();
    QString     getDesignName() { return designName; }
    void        clearDesigns();

    // styles
    StyledDesign & getStyledDesign(eWsData dataset) { return ws[dataset].styles; }

    // loaded tilings
    void        setTiling(eWsData dataset, TilingPtr t) { ws[dataset].tiling = t; }
    TilingPtr   getTiling(eWsData dataset)              { return ws[dataset].tiling; }

    // prototypes
    void         setPrototype(eWsData dataset,PrototypePtr pp);
    PrototypePtr getPrototype(eWsData dataset);

    // figures
    void             setSelectedDesignElement(eWsData dataset,DesignElementPtr dp);
    DesignElementPtr getSelectedDesignElement(eWsData dataset);

protected:

signals:
    void    sig_newTiling();
    void    sig_ws_dele_changed();

public slots:
    void slot_clearCanvas();
    void slot_clearWorkspace();

private:
    Workspace();
    ~Workspace();

    static Workspace    * mpThis;
    Configuration       * config;
    Canvas              * canvas;

    QVector<DesignPtr>  activeDesigns;
    QString             designName;

    WorkspaceData       ws[2];

};

#endif // WORKSPACE_H
