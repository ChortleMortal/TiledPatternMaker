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
#include <enums/ecyclemode.h>

class Configuration;
class Canvas;

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
    void sig_clearView();
    void sig_cycleLoadMosaic(QString name);
    void sig_cycleLoadTiling(QString name);
    void sig_finished();
    void sig_show_png(QString file, int row, int col);
    void sig_compare(QString,QString,bool);
    void sig_workList();

public slots:
   void slot_startCycle(eCycleMode mode);
   void slot_stopCycle();
   void slot_psuedoKey(int key);
   void slot_ready();
   void slot_timeout();

protected:
    void startCycleStyles();
    void startCycleTilings();
    void startCycleOriginalDesignPngs();
    void startCycleCompareAllImages();
    void startCycleCompareWorklistImages();
    void nextCyclePng();

private:
    Cycler();
    ~Cycler();

    static Cycler * mpThis;
    Configuration * config;
    QTimer        * timer;

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
    QStringList           imgList;
    QStringList::iterator imgList_it;
};

#endif // CYCLER_H
