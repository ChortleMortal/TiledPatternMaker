#ifndef SYS_H
#define SYS_H

#include <QObject>
#include <QMultiMap>
#include <QMutex>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif
#include "enums/emotiftype.h"

typedef QMultiMap<QString, QString> tilingUses;     // tiling name, design name

class Sys
{
public:
    static void init();
    static void close();

    static QMultiMap<eMotifType,QString> initMotifAssociations();

    static void         appDontPaint(bool stop);

    static eMotifType   getMotifType(QString name);
    static QString      getMotifName(eMotifType type);
    static QStringList  getMotifNames(eMotifType type);

    static void         dumpRefs();

    // system
    static class qtAppLog        * log;
    static class Configuration   * config;
    static class ControlPanel    * controlPanel;

    // makers
    static class DesignMaker    * designMaker;
    static class MapEditor      * mapEditor;
    static class MosaicMaker    * mosaicMaker;
    static class PrototypeMaker * prototypeMaker;
    static class TilingMaker    * tilingMaker;

    // viewers
    static class View           * view;
    static class ViewController * viewController;
    static class DebugView      * debugView;
    static class CropViewer     * cropViewer;
    static class GridView       * gridViewer;
    static class MapEditorView  * mapEditorView;
    static class MeasureView    * measureView;
    static class MotifView      * motifView;
    static class TilingMakerView* tilingMakerView;
    static class PrototypeView  * prototypeView;
    static class ImageEngine    * imageEngine;
    static class BackgroundImageView * backgroundImageView;

    // selections
    static class Layer          * selectedLayer;

    static bool   isDarkTheme;
    static bool   usingImgGenerator;
    static bool   localCycle;
    static bool   primaryDisplay;
    static bool   circleX;
    static bool   hideCircles;
    static bool   enableDetachedPages;
    static bool   showCenterMouse;
    static bool   updatePanel;
    static bool   dontReplicate;
    static bool   highlightUnit;
    static bool   motifPropagate;
    static bool   dontTrapLog;
    static bool   flagA;
    static bool   flagB;
    static bool   measure;

    static int    appInstance;

    static QString gitBranch;       // if there is one
    static QString rootTileDir;
    static QString originalTileDir;
    static QString newTileDir;
    static QString testTileDir;
    static QString rootMosaicDir;
    static QString originalMosaicDir;
    static QString newMosaicDir;
    static QString testMosiacDir;
    static QString templateDir;
    static QString examplesDir;
    static QString mapsDir;
    static QString worklistsDir;

    static QMultiMap<eMotifType,QString> motifRepresentation;

    static const QChar MathSymbolSquareRoot;
    static const QChar MathSymbolPi;
    static const QChar MathSymbolDelta;
    static const QChar MathSymbolSigma;

    static const QPointF ORIGIN;

    static const qreal TOL;
    static const qreal TOL2;
    static const qreal NEAR_TOL;
    static const qreal TOLSQ;

    static const qreal GOLDEN_RATIO;

    static const int   DEFAULT_WIDTH;
    static const int   DEFAULT_HEIGHT;

    static const int   MAX_WIDTH;
    static const int   MAX_HEIGHT;

    static QStringList   tilingsList;
    static QStringList   mosaicsList;
    static tilingUses    uses;

private:

};

#endif // SYS_H
