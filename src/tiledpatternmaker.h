/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright (c) 2016-2022 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 */

#ifndef TILEDPATTERNMAKER_H
#define TILEDPATTERNMAKER_H

#include <QObject>
#include <QKeyEvent>
#include <QPalette>
#include "enums/estatemachineevent.h"
#include "enums/edesign.h"

class SplitScreen;
class ImageWidget;
class TransparentWidget;
class Configuration;
class Cycler;
class MapEditor;
class CropMaker;

typedef std::shared_ptr<class TilingMaker>      TilingMakerPtr;
typedef std::shared_ptr<class Figure>           FigurePtr;

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
    void sig_tilingWritten(QString name);
    void sig_mosaicLoaded(QString name);
    void sig_tilingLoaded(QString name);
    void sig_loadedDesign(eDesign design);
    void sig_ready();
    void sig_refreshView();
    void sig_compareResult(QString);
    void sig_image0(QString name);
    void sig_image1(QString name);
    void sig_deleteCurrentInWorklist();
    void sig_lockStatus();

    void sig_takeNext();
    void sig_cyclerQuit();
    void sig_closeAllImageViewers();
    void sig_primaryDisplay();

public slots:
    void startEverything();
    void slot_loadDesign(eDesign design);
    void slot_buildDesign(eDesign design);

    void slot_loadMosaic(QString name, bool ready);
    void slot_cycleLoadMosaic(QString name);
    void slot_saveMosaic(QString name, bool test);

    void slot_loadTiling(QString name,eSM_Event mode);
    void slot_cyclerLoadTiling(QString name);
    void slot_saveTiling(QString name);

    //  resets protos and syles
    void slot_render();

    void slot_raiseMenu();
    void slot_bringToPrimaryScreen();
    void slot_splitScreen(bool checked);

    void slot_compareImages(QString leftName, QString rightName, bool autoMode);
    void slot_cyclerFinished();
    void slot_view_image(QString left, QString right, bool transparent, bool popup);
    void slot_show_png(QString file, int row, int col);

    static QPixmap createTransparentPixmap(QImage img);

protected:
    void init();
    void setDarkTheme(bool enb);

    ImageWidget       * popupPixmap(QPixmap & pixmap,QString title);
    TransparentWidget * popupTransparentPixmap(QPixmap & pixmap,QString title);
    QPixmap             makeTextPixmap(QString txt,QString txt2=QString(),QString txt3=QString());

private:
    Configuration          * config;
    class ViewControl      * view;
    class ControlPanel     * controlPanel;
    class MosaicMaker      * mosaicMaker;
    class MotifMaker       * motifMaker;
    TilingMakerPtr           tilingMaker;
    MapEditor              * mapEditor;
    CropMaker              * cropMaker;
    Cycler                 * cycler;

    bool                    showFirst;
    QString                 pathLeft;
    QString                 pathRight;
    QString                 nameLeft;
    QString                 nameRight;
    QPalette                originalPalette;

};

#endif // TILEDPATTERNMAKER_H
