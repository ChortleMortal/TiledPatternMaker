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
#include <QPalette>


class SplitScreen;

class Configuration;
class ImageWidget;
class TransparentImageWidget;
class Cycler;
class MapEditor;
class CropMaker;
class TPMSplash;

typedef std::shared_ptr<class Motif>  MotifPtr;

class TiledPatternMaker : public QObject
{
    Q_OBJECT

public:
    TiledPatternMaker();
    ~TiledPatternMaker();

    SplitScreen * getSplitter() { return splitter; }



    void    enableSplash(bool enable);
    void    splash(QString & txt);
    void    splash(QString && txt);
    void    removeSplash();

    void    testMemoryManagement();

    static void      appDebugBreak();
    static int const EXIT_CODE_REBOOT;

signals:
    void sig_start();
    void sig_refreshView();

    void sig_lockStatus();
    void sig_workListChanged();

    void sig_primaryDisplay();

public slots:
    void startEverything();
    void slot_render();
    void slot_raiseMenu();
    void slot_bringToPrimaryScreen();
    
protected:
    void init();
    void splitScreen();
    void setPaletteColors();

private:
    Configuration          * config;
    class View             * view;
    class ViewController   * viewController;
    class ControlPanel     * controlPanel;
    class MosaicMaker      * mosaicMaker;
    class PrototypeMaker   * prototypeMaker;
    class TilingMaker      * tilingMaker;
    SplitScreen            * splitter;

    MapEditor              * mapEditor;
    TPMSplash              * _splash;

    int                     instance;
};

extern TiledPatternMaker * theApp;

#endif // TILEDPATTERNMAKER_H
