#pragma once
#ifndef TILEDPATTERNMAKER_H
#define TILEDPATTERNMAKER_H

/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright (c) 2016-2023 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 */

#include <QObject>

class SplitScreen;
class Sys;

class TiledPatternMaker : public QObject
{
    Q_OBJECT

public:
    TiledPatternMaker();
    ~TiledPatternMaker();

    SplitScreen * getSplitter() { return splitter; }

    void    testMemoryManagement();

    static void      appDebugBreak();
    static int const EXIT_CODE_REBOOT;

signals:
    void sig_start();
    void sig_floatPages();

    void sig_reconstructView();

    void sig_lockStatus();
    void sig_workListChanged();

    void sig_primaryDisplay();

public slots:
    void slot_start();
    void slot_stop();
    void slot_render();
    void slot_bringToPrimaryScreen();
    
protected:
    void init();
    void splitScreen();
    void setPaletteColors();

private:
    Sys         * sys;
    SplitScreen * splitter;

};

extern TiledPatternMaker * theApp;

#endif // TILEDPATTERNMAKER_H
