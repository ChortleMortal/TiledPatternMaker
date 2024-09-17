#ifndef SYS_H
#define SYS_H

#include <QObject>
#include <QMultiMap>
#include <QMutex>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif
#include "sys/enums/emotiftype.h"
#include "sys/sys/versioning.h"

typedef QPair<VersionedFile,VersionedFile>  TilingUse;      // tiling, mosaic
typedef QVector<TilingUse>                  TilingUses;

class Sys
{
public:
    Sys();
    ~Sys();

    static QMultiMap<eMotifType,QString> initMotifAssociations();

    static eMotifType   XMLgetMotifType(QString name);
    static QString      XMLgetMotifName(eMotifType type);
    static QStringList  XMLgetMotifNames(eMotifType type);

    static void         dumpRefs();

    static void         setTilingChange()       { _tilingViewChange = true;  }
    static bool         isTilingViewChanged()   { return _tilingViewChange;  }
    static void         resetTilingViewChange() { _tilingViewChange = false; }

    // system
    static class qtAppLog        * log;
    static class Configuration   * config;
    static class ControlPanel    * controlPanel;
    static class SplashScreen    * splash;
    static class GuiModes        * guiModes;
    static class LoadUnit        * loadUnit;

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
    static bool   tm_fill;

    static int    appInstance;

    static QString gitBranch;       // if there is one
    static QString gitSha;          // if there is one
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
    static QString lastPanelTitle;
    static QString lastViewTitle;

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

    static const QString defaultMosaicName;
    static const QString defaultTilingName;

    static VersionFileList tilingsList;
    static VersionFileList mosaicsList;
    static TilingUses      tilingUses;

private:
    static bool _tilingViewChange;
};

#endif // SYS_H
