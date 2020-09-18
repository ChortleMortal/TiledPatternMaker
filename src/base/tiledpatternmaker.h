﻿/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
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

#include "base/configuration.h"
#include "base/workspace.h"

class SplitScreen;
class AQLabel;
class ControlPanel;

class TiledPatternMaker : public QObject
{
    Q_OBJECT

public:
    TiledPatternMaker();
    ~TiledPatternMaker();

signals:
    void sig_start();
    void sig_newXML();
    void sig_loadedXML(QString name);
    void sig_loadedTiling(QString name);
    void sig_loadedDesign(eDesign design);
    void sig_ready();
    void sig_viewWS();
    void sig_compareResult(QString);
    void sig_image0(QString name);
    void sig_image1(QString name);

public slots:
    void startEverything();

    void slot_loadDesign(eDesign design);
    void slot_buildDesign(eDesign design);

    void slot_loadMosaic(QString name);
    void slot_cycleLoadMosaic(QString name);
    void slot_saveMosaic(QString name);
    void slot_loadTiling(QString name);
    void slot_cyclerLoadTiling(QString name);

    //  resets protos and syles
    void slot_render();

    void slot_raiseMenu();
    void slot_bringToPrimaryScreen();
    void slot_splitScreen(bool checked);

    void slot_compareImagesReplace(QString fileLeft, QString fileRight);
    void slot_compareImages(QString fileLeft, QString fileRight);
    void slot_cyclerFinished();
    void slot_view_image(QString filename);
    void slot_show_png(QString file, int row, int col);

protected:
    void resetStyles();
    void resetProtos();
    void init();
    void SplatShowImage(QImage & image, QString title);
    void SplatCompareResult(QPixmap & pixmap, QString title);
    void SplatCompareResultTransparent(QPixmap & pixmap, QString title);
    QPixmap makeTextPixmap(QString txt,QString txt2=QString(),QString txt3=QString());

private:
    Configuration   * config;
    Workspace       * workspace;
    ControlPanel    * controlPanel;
    TilingMaker     * tilingMaker;
    MapEditor       * mapEditor;

    Cycler          * cycler;
    AQLabel         * cyclerWindow;
};

#endif // TILEDPATTERNMAKER_H
