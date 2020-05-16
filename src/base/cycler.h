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

#ifndef CYCLER_H
#define CYCLER_H

#include <QtCore>
#include <QtWidgets>

class Configuration;
class Canvas;

#define E2STR(x) #x

enum eCycleMode
{
    CYCLE_NONE,
    CYCLE_STYLES,
    CYCLE_TILINGS,
    CYCLE_ORIGINAL_PNGS,
    CYCLE_SAVE_STYLE_BMPS,
    CYCLE_SAVE_TILING_BMPS,
    CYCLE_COMPARE_IMAGES
};

static QString sCycleMode[] = {
    E2STR(CYCLE_NONE),
    E2STR(CYCLE_STYLES),
    E2STR(CYCLE_TILINGS),
    E2STR(CYCLE_ORIGINAL_PNGS),
    E2STR(CYCLE_SAVE_STYLE_BMPS),
    E2STR(CYCLE_SAVE_TILING_BMPS),
    E2STR(CYCLE_COMPARE_IMAGES)
};

Q_DECLARE_METATYPE(eCycleMode)

class TiledPatternMaker;

class Cycler : public QObject
{
    Q_OBJECT

public:
    static Cycler * getInstance();

    void    init(QThread * thread);

    eCycleMode getMode() { return cycleMode; }

signals:
    void sig_clearCanvas();
    void sig_loadXML(QString name);
    void sig_loadTiling(QString name);
    void sig_finished();
    void sig_show_png(QString file, int row, int col);
    void sig_saveAsBMP(QString);
    void sig_saveTilingAsBMP(QString);
    void sig_compare(QString,QString);
    void sig_viewImage(QString filename);

public slots:
   void slot_startCycle();
   void slot_stopCycle();
   void slot_psuedoKey(int key);
   void slot_ready();
   void slot_view_images();
   void slot_timeout();


protected:
    void startCycleStyles();
    void startCycleTilings();
    void startCycleOriginalDesignPngs();
    void startCycleCompareImages();
    void nextCyclePng();

private:
    Cycler();
    ~Cycler();

    static Cycler * mpThis;

    Configuration * config;
    Canvas        * canvas;

    bool            cyclePause;

    bool            busy;

    eCycleMode      cycleMode;
    QStringList     files;
    QStringList     fileFilter;
    int             cIndex;
    int             cCount;     // 4 ticks

    int             pngRow;
    int             pngCol;
    int             pngIndex;

    QMap<QString,QString> mapa;
    QMap<QString,QString> mapb;
    QMap<QString,QString>::iterator map_it;
};

#endif // CYCLER_H
