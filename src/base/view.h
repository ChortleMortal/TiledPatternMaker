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
#include "base/frame_settings.h"

class Canvas;
class Cycler;
class ControlPanel;
class MapEditor;
class TilingMaker;
class Layer;
class MouseAction;
class DesignControl;

class View : public QWidget
{
    Q_OBJECT

public:
    void    init();
    void    clearView();

    void    addLayer(LayerPtr layer);
    void    addTopLayer(LayerPtr layer);
    void    clearLayers()           { layers.clear(); }
    int     numLayers()             { return layers.size(); }
    QVector<LayerPtr> getActiveLayers();

    void    setMouseMode(eMouseMode mode);
    eMouseMode getMouseMode() { return mouseMode; }

    void    setBackgroundColor(QColor color);
    QColor  getBackgroundColor();

    void    setAllMosaicFrameSizes(QSize sz);
    void    setAllTilingFrameSizes(QSize sz);
    void    setAllFrameSizes();

    void    setAllMosaicActiveSizes(QSize sz);
    void    setAllTilingActiveSizes(QSize sz);
    void    setAllActiveSizes();

    FrameSettings & getFrameSettings(eViewType e);
    void            setFrameSize(eViewType e, QSize sz);
    QSize           getFrameSize(eViewType e);
    QTransform      getFrameTransform(eViewType e);
    void            setActiveSize(eViewType e, QSize sz);
    QSize           getActiveSize(eViewType e);

    void    clearLayout(); // only used by cycler for pngs
    void    setKbdMode(eKbdMode mode);
    QString getKbdModeStr();

    void    dump(bool summary);
    void    drainTheSwamp();

signals:
    void sig_mousePressed(QPointF pos,Qt::MouseButton);
    void sig_mouseDoublePressed(QPointF pos,Qt::MouseButton);
    void sig_mouseDragged(QPointF pos);
    void sig_mouseReleased(QPointF pos);
    void sig_mouseMoved(QPointF pos);

    void sig_wheel_scale(qreal angle);
    void sig_wheel_rotate(qreal angle);
    void sig_mouseTranslate(QPointF pt);

    void sig_deltaRotate(int amount);
    void sig_deltaMoveY(int amount);
    void sig_deltaMoveX(int amount);
    void sig_deltaScale(int amount);

    void sig_setCenter(QPointF sPos);
    void sig_reconstructBorder();

    void sig_kbdMode(eKbdMode);

    void sig_viewWS();
    void sig_unload();
    void sig_raiseMenu();
    void sig_figure_changed();
    void sig_cyclerQuit();
    void sig_cyclerKey(int key);

    void sig_saveImage();
    void sig_saveSVG();

protected:
    View();
    ~View() override;

    void resize(QSize sz);

    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent( QKeyEvent *k ) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent * event) override;
    void wheelEvent(QWheelEvent *event) override;

    void drawForeground(QPainter *painter, const QRectF &rect);

    void drawGridModelUnits(QPainter *painter, const QRectF & r);
    void drawGridSceneUnits(QPainter *painter, const QRectF & r);
    void drawGridModelUnitsCentered(QPainter *painter, QRectF & r);
    void drawGridSceneUnitsCentered(QPainter *painter, QRectF & r);

    void clearLayout(QLayout* layout, bool deleteWidgets = true);

    bool procKeyEvent(QKeyEvent * k);
    bool ProcKey(QKeyEvent *k, bool isALT);
    bool ProcNavKey(int key, int multiplier, bool isALT);

    void ProcKeyLeft( int delta, bool isALT);
    void ProcKeyRight(int delta, bool isALT);
    void ProcKeyDown( int delta, bool isALT);
    void ProcKeyUp(   int delta, bool isALT);

    void duplicateView();

private:

    Configuration   * config;
    TilingMaker     * tilingMaker;
    MapEditor       * mapEditor;
    DesignControl   * designCtrl;

    UniqueQVector<LayerPtr> layers;

    FrameSettings     frameSettings[VIEW_MAX+1];

    eMouseMode        mouseMode;
    bool              dragging;
    QColor            backgroundColor;
    QPen              gridPen;

    QPointF           sLast;    // used by pan
    bool              closed;
};

#endif // VIEW_H
