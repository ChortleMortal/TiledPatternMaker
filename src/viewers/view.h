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

#include "base/misc.h"
#include "settings/frame_settings.h"
#include "enums/ekeyboardmode.h"
#include "enums/emousemode.h"

typedef std::shared_ptr<class TilingMaker> TilingMakerPtr;
typedef std::shared_ptr<class MapEditor>   MapEditorPtr;
typedef std::shared_ptr<class Layer> LayerPtr;

class LoadUnit
{
public:
    QString         name;
    QElapsedTimer   loadTimer;
};

class View : public QWidget
{
    Q_OBJECT

public:
    static View *  getInstance();
    static void    releaseInstance();

    void    init();
    void    clearView();
    void    paintEnable(bool enable);

    void    resize(QSize sz);

    void    addLayer(LayerPtr layer);
    void    addTopLayer(LayerPtr layer);
    void    clearLayers()           { layers.clear(); }
    int     numLayers()             { return layers.size(); }
    QVector<LayerPtr> getActiveLayers();

    void    setMouseMode(eMouseMode newMode);
    eMouseMode getMouseMode() { return mouseMode; }

    void    setBackgroundColor(QColor color);
    QColor  getBackgroundColor();

    void    clearLayout(); // only used by cycler for pngs
    void    setKbdMode(eKbdMode mode);
    QString getKbdModeStr();

    LoadUnit & getLoadUnit() { return loadUnit; }

    void    dump(bool summary);

    FrameSettings frameSettings;

signals:
    void sig_viewSizeChanged(QSize sz);

    void sig_mousePressed(QPointF pos,Qt::MouseButton);
    void sig_mouseDragged(QPointF pos);
    void sig_mouseTranslate(QPointF pt);
    void sig_mouseMoved(QPointF pos);
    void sig_mouseReleased(QPointF pos);
    void sig_mouseDoublePressed(QPointF pos,Qt::MouseButton);

    void sig_wheel_scale(qreal angle);
    void sig_wheel_rotate(qreal angle);


    void sig_deltaRotate(int amount);
    void sig_deltaMoveY(int amount);
    void sig_deltaMoveX(int amount);
    void sig_deltaScale(int amount);

    void sig_kbdMode(eKbdMode);

    void sig_refreshView();
    void sig_raiseMenu();
    void sig_figure_changed();
    void sig_cyclerQuit();
    void sig_cyclerKey(int key);

    void sig_saveImage();
    void sig_saveSVG();

protected:
    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
#if 0
    void moveEvent(QMoveEvent *event) override;
#endif
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent( QKeyEvent *k ) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent * event) override;
    void wheelEvent(QWheelEvent *event) override;

    void clearLayout(QLayout* layout, bool deleteWidgets = true);

    bool procKeyEvent(QKeyEvent * k);
    bool ProcKey(QKeyEvent *k);
    bool ProcNavKey(int key, int multiplier);

    void ProcKeyLeft( int delta);
    void ProcKeyRight(int delta);
    void ProcKeyDown( int delta);
    void ProcKeyUp(   int delta);

    void duplicateView();

private:
    View();
    ~View() override;

    static View * mpThis;

    Configuration     * config;
    TilingMakerPtr      tilingMaker;
    MapEditorPtr        mapEditor;
    class DesignMaker * designMaker;
    class ControlPanel* panel;

    UniqueQVector<LayerPtr> layers;

    LoadUnit          loadUnit;
    bool              canPaint;
    eMouseMode        mouseMode;
    eMouseMode        lastMouseMode;
    bool              dragging;
    QColor            backgroundColor;

    QPointF           sLast;    // used by pan
    bool              closed;
};

#endif // VIEW_H
