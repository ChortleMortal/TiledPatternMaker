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

#ifndef WORKSPACE_VIEWER_H
#define WORKSPACE_VIEWER_H

#include <QtCore>
#include <QtWidgets>
#include "base/view.h"
#include "base/configuration.h"
#include "base/shared.h"

class WorkspaceViewer : public View
{
    Q_OBJECT

public:
    void    init();

    void    viewEnable(eViewType view, bool enable);
    void    disableAll();

public slots:
    void    slot_viewWorkspace();

signals:
    void    sig_viewUpdated();

protected:
   WorkspaceViewer();
   ~WorkspaceViewer() {}

   void     setupViewers();
   void     viewWorkspace();

   void     viewDesign();
   void     viewMosaic();
   void     viewPrototype();
   void     viewDesignElement();
   void     viewPrototypeMaker();
   void     viewTiling();
   void     viewTilingMaker();
   void     viewMapEditor();
   void     viewFaceSet();

   void     setTitle(TilingPtr tp);
   void     setBackgroundImg(BkgdImgPtr bkgd);
   void     setBorder(BorderPtr bp);

private:
    class   Configuration * config;
    class   Workspace     * workspace;
    class   ControlPanel  * panel;

    bool    enabledViews[VIEW_MAX+1];
};

#endif // DESIGNVIEWER_H
