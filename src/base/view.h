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

#ifndef VIEW_H
#define VIEW_H

#include <QtCore>
#include <QtWidgets>

#include "base/configuration.h"
#include "base/misc.h"

class Canvas;
class Cycler;
class ControlPanel;
class MapEditor;
class TilingMaker;
class WorkspaceViewer;
class Layer;

class View : public QWidget
{
    Q_OBJECT

public:
    static View * getInstance();
    static void  releaseInstance();

    void init();
    void clearView();

    void addLayer(LayerPtr layer);
    void clearLayers()           { layers.clear(); }
    int  numLayers()             { return layers.size(); }
    QVector<LayerPtr> getActiveLayers();

    void   setBackgroundColor(QColor color);
    QColor getBackgroundColor();

    void clearLayout(); // only used by cycler for pngs

    virtual QSize sizeHint() const override;

    void dump(bool force = false);

signals:
    void sig_mousePressed(QPointF pos,Qt::MouseButton);
    void sig_mouseDoublePressed(QPointF pos,Qt::MouseButton);
    void sig_mouseDragged(QPointF pos);
    void sig_mouseReleased(QPointF pos);
    void sig_mouseMoved(QPointF pos);
    void sig_reconstructBorder();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent( QKeyEvent *k ) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent * event) Q_DECL_OVERRIDE;

    void drawForeground(QPainter *painter, const QRectF &rect);

    void drawGridModelUnits(QPainter *painter, const QRectF & r);
    void drawGridSceneUnits(QPainter *painter, const QRectF & r);
    void drawGridModelUnitsCentered(QPainter *painter, QRectF & r);
    void drawGridSceneUnitsCentered(QPainter *painter, QRectF & r);

    void clearLayout(QLayout* layout, bool deleteWidgets = true);

private:
    View();
    ~View() override;

    static View     * mpThis;
    Canvas          * canvas;
    Configuration   * config;
    WorkspaceViewer * wsViewer;
    TilingMakerPtr    tmaker;
    MapEditorPtr      maped;

    UniqueQVector<LayerPtr> layers;

    bool              dragging;
    QColor            backgroundColor;
    QPen              gridPen;
};

#endif // VIEW_H
