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
#include <QKeyEvent>
#include <QPalette>

class SplitScreen;
class ImageWidget;
class TransparentWidget;
class Configuration;
class Cycler;
class MapEditor;
class CropMaker;
class TPMSplash;

typedef std::shared_ptr<class Motif>  MotifPtr;

class TiledPatternMaker : public QObject
{
    Q_OBJECT

public:
    TiledPatternMaker(int instance);
    ~TiledPatternMaker();

    void imageKeyPressed(QKeyEvent * k);
    void appCompareImages(QImage & img_left, QImage & img_right, QString title_left, QString title_right, bool autoMode);

    void testMemoryManagement();

    SplitScreen * getSplitter() { return splitter; }

    void    enableSplash(bool enable);
    void    splash(QString & txt);
    void    splash(QString && txt);
    void    removeSplash();

    static QPixmap createTransparentPixmap(QImage img);
    static void    appDebugBreak();

    static int const EXIT_CODE_REBOOT;

signals:
    void sig_start();
    void sig_ready();
    void sig_refreshView();
    void sig_compareResult(QString);
    void sig_image0(QString name);
    void sig_image1(QString name);
    void sig_deleteCurrentInWorklist(bool confirm);
    void sig_lockStatus();
    void sig_workListChanged();
    void sig_colorPick(QColor color);

    void sig_takeNext();
    void sig_cyclerQuit();
    void sig_closeAllImageViewers();
    void sig_primaryDisplay();

public slots:
    void startEverything();

    void slot_render();

    void slot_raiseMenu();
    void slot_bringToPrimaryScreen();
    
    void slot_compareBMPs(QString leftName, QString rightName, bool autoMode);
    void slot_compareBMPsPath(QString file1, QString file2);
    void slot_compareBMMPandLoaded(QString leftName, bool autoMode);
    void slot_cyclerFinished();
    void slot_view_image(QString left, QString right, bool transparent, bool popup);
    void slot_show_png(QString file, int row, int col);

protected:
    void init();
    void setDarkTheme(bool enb);

    void compareImages(bool automode);
    void compareImages(QImage & img_left, QImage & img_right, QString title_left, QString title_right, bool autoMode);
    void splitScreen();

    void ping_pong_images(bool transparent, bool popup);

    ImageWidget       * popupPixmap(QPixmap & pixmap,QString title);
    TransparentWidget * popupTransparentPixmap(QPixmap & pixmap,QString title);
    QPixmap             makeTextPixmap(QString txt,QString txt2=QString(),QString txt3=QString());

private:
    Configuration          * config;
    class ViewControl      * view;
    class ControlPanel     * controlPanel;
    class MosaicMaker      * mosaicMaker;
    class PrototypeMaker   * prototypeMaker;
    class TilingMaker      * tilingMaker;
    SplitScreen            * splitter;

    MapEditor              * mapEditor;
    Cycler                 * cycler;
    TPMSplash              * _splash;

    int                     instance;
    bool                    showFirst;
    QString                 pathLeft;
    QString                 pathRight;
    QString                 nameLeft;
    QString                 nameRight;

    QImage                  _imageA;
    QImage                  _imageB;
    bool                    _showA;
    QString                 _titleA;
    QString                 _titleB;
};

extern TiledPatternMaker * theApp;

#endif // TILEDPATTERNMAKER_H
