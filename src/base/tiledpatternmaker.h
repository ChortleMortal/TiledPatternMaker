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

#ifndef TILEDPATTERNMAKER_H
#define TILEDPATTERNMAKER_H

#include <QObject>
#include <QKeyEvent>
#include "enums/estatemachineevent.h"
#include "enums/edesign.h"

class SplitScreen;
class ImageWidget;
class TransparentWidget;
class Configuration;
class Cycler;

typedef std::shared_ptr<class TilingMaker>      TilingMakerPtr;
typedef std::shared_ptr<class MapEditor>        MapEditorPtr;

class TiledPatternMaker : public QObject
{
    Q_OBJECT

public:
    TiledPatternMaker();
    ~TiledPatternMaker();

    void imageKeyPressed(QKeyEvent * k);

signals:
    void sig_start();
    void sig_mosaicWritten();
    void sig_tilingWritten();
    void sig_mosaicLoaded(QString name);
    void sig_tilingLoaded(QString name);
    void sig_loadedDesign(eDesign design);
    void sig_ready();
    void sig_refreshView();
    void sig_compareResult(QString);
    void sig_image0(QString name);
    void sig_image1(QString name);
    void sig_lockStatus();

    void sig_takeNext();
    void sig_cyclerQuit();
    void sig_closeAllImageViewers();

public slots:
    void startEverything();

    void slot_loadDesign(eDesign design);
    void slot_buildDesign(eDesign design);

    void slot_loadMosaic(QString name);
    void slot_cycleLoadMosaic(QString name);
    void slot_saveMosaic(QString name);

    void slot_loadTiling(QString name,eSM_Event mode);
    void slot_cyclerLoadTiling(QString name);
    void slot_saveTiling(QString name);

    //  resets protos and syles
    void slot_render();

    void slot_raiseMenu();
    void slot_bringToPrimaryScreen();
    void slot_splitScreen(bool checked);

    void slot_compareImages(QString nameLeft, QString nameRight, bool autoMode);
    void slot_cyclerFinished();
    void slot_view_image(QString left, QString right, bool transparent, bool popup);
    void slot_show_png(QString file, int row, int col);

protected:
    void init();

    ImageWidget       * popupPixmap(QPixmap & pixmap,QString title);
    TransparentWidget * popupTransparentPixmap(QPixmap & pixmap,QString title);
    QPixmap             createTransparentPixmap(QString file);
    QPixmap             makeTextPixmap(QString txt,QString txt2=QString(),QString txt3=QString());

private:
    Configuration          * config;
    class View             * view;
    class ViewControl      * vcontrol;
    class ControlPanel     * controlPanel;
    class DecorationMaker  * decorationMaker;
    class MotifMaker       * motifMaker;
    TilingMakerPtr           tilingMaker;
    MapEditorPtr             mapEditor;
    Cycler                 * cycler;

    bool                    showFirst;
    QString                 pathLeft;
    QString                 pathRight;


};

#endif // TILEDPATTERNMAKER_H
