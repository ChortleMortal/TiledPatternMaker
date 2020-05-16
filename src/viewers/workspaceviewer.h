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
#include "base/layer.h"
#include "base/misc.h"
#include "base/configuration.h"
#include "designs/design.h"
#include "geometry/bounds.h"
#include "base/shared.h"

class Canvas;
class DesignElement;
class PlacedDesignElementView;
class FigureView;
class ProtoView;
class ProtoFeatureView;
class Tiling;
class TilingMaker;
class TilingView;
class Workspace;
class Transformer;
class MapEditor;
class StyledDesign;
class FaceSetView;
class FaceSet;
class Scene;

class WorkspaceViewer : public QObject
{
    Q_OBJECT

public:
    static WorkspaceViewer *getInstance();
    virtual ~WorkspaceViewer() {}

    void   init();
    void   clear();

    bool            hasLayers();
    QVector<Layer*> getActiveLayers();

    ViewSettings   &  getViewSettings(eViewType e);
    void              setViewSize(eViewType e, QSize sz);
    QSize             getViewSize(eViewType e);
    QRect             getViewRect(eViewType e);
    QTransform        getViewTransform(eViewType e);

    CanvasSettings &  GetCanvasSettings();
    void              setCanvasSettings(CanvasSettings & cs);

    QSizeF            getSceneSize();
    void              setSceneSize(QSizeF sz);

    BorderPtr         getBorder() { return bp; }
    BkgdImgPtr        getBkgdImage() { return bip; }

public slots:
    void slot_viewWorkspace();
    void slot_update();

signals:
    void sig_title(QString);
    void sig_viewUpdated();

protected:
   WorkspaceViewer();

   void     viewWorkspace();

   void     viewDesign();
   void     viewProto();
   void     viewProtoFeature();
   void     viewDesignElement();
   void     viewFigureMaker();
   void     viewTiling();
   void     viewTilingMaker();
   void     viewMapEditor();
   void     viewFaceSet();

   void     setTitle(TilingPtr tp);

private:
   void     updateScene(Scene * scene);

   static WorkspaceViewer * mpThis;

    class Configuration * config;
    class Workspace     * workspace;
    class Canvas        * canvas;

    UniqueQVector<Layer*> mViewers;
    UniqueQVector<Layer*> mStyles;    // don't delete - just clear

    QVector<DesignPtr>    mDesigns;

    BorderPtr       bp;
    BkgdImgPtr      bip;

    ViewSettings        viewSettings[VIEW_MAX+1];
    CanvasSettings      canvasSettings;
};

#endif // DESIGNVIEWER_H
