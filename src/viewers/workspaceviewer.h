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
#include "base/design.h"
#include "base/layer.h"
#include "base/configuration.h"
#include "geometry/bounds.h"

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

struct SizeAndBounds
{
    eViewType   viewType;
    Bounds      viewBounds;
    QSizeF      viewSize;
};

struct ViewTransform
{
    eViewType   viewType;
    qreal       scale;
    QPointF     translate;
};

class WorkspaceViewer : public QObject
{
    Q_OBJECT

public:
    static WorkspaceViewer *getInstance();
    virtual ~WorkspaceViewer() {}

    void   init();
    void   clear();

    QVector<Layer*> getActiveLayers();

    QTransform  getViewTransform(eViewType e);
    static QTransform  calculateViewTransform(SizeAndBounds &sab);

    static SizeAndBounds viewDimensions[];

public slots:
    void slot_viewWorkspace();
    void slot_update();

signals:
    void sig_title(QString);

protected:
   WorkspaceViewer();

   void                updateDesignInfo();

   void                viewWorkspace();
   void                viewStyledDesign(StyledDesign &  sd);
   ProtoView         * viewProto(StylePtr style);
   ProtoView         * viewProto(PrototypePtr proto);
   ProtoFeatureView  * viewProtoFeature(StylePtr style);
   ProtoFeatureView  * viewProtoFeature(PrototypePtr proto);
   TilingView        * viewTiling(StylePtr style);
   TilingView        * viewTiling(TilingPtr tiling);
   TilingMaker       * viewTilingMaker();
   MapEditor         * viewFigMapEditor(QPointF center);
   FigureView        * viewFigure(DesignElementPtr dep, QPointF center);
   PlacedDesignElementView * viewPlacedDesignElement(PlacedDesignElementPtr pde);
   FaceSetView       * viewFaceSet(FaceSet *set);

   void                setTitle(TilingPtr tp);

private:
    static WorkspaceViewer * mpThis;

    class Configuration * config;
    class Workspace     * workspace;
    class Canvas        * canvas;

    QVector<Layer*>     mViewers;
    QVector<Layer*>     mStyles;    // don't delete - just clear
    QVector<DesignPtr>  mDesigns;

    QVector<QTransform> viewTransforms;
};

#endif // DESIGNVIEWER_H
