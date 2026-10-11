#pragma once

#include <QPointF>
#include <QTransform>
#include <QSettings>

#include "model/motifs/motif.h"
#include "sys/enums/eactivecanvas.h"
#include "sys/enums/emousemode.h"
#include "sys/enums/erender.h"
#include "sys/sys/versioning.h"

#undef LEGACY_CONVERT_XML

typedef QPair<VersionedFile,VersionedFile>  TilingUse;      // tiling, mosaic
typedef QVector<TilingUse>                  TilingUses;

typedef std::shared_ptr<class DebugView>            DebugViewPtr;
typedef std::shared_ptr<class GridView>             GridViewPtr;
typedef std::shared_ptr<class ConstructionLinesView> CLinesViewPtr;
typedef std::shared_ptr<class CropMakerView>        CropViewPtr;
typedef std::shared_ptr<class MapEditorView>        MapedViewPtr;
typedef std::shared_ptr<class MotifMakerView>       MMViewiewPtr;
typedef std::shared_ptr<class TilingMakerView>      TMViewPtr;
typedef std::shared_ptr<class PrototypeView>        ProtoViewPtr;
typedef std::shared_ptr<class ImageEngine>          IEViewPtrPtr;
typedef std::shared_ptr<class ImageViewer>          ImageViewPtr;
typedef std::shared_ptr<class BackgroundImage>      BkgdImagePtr;

class Sys
{
public:
    Sys();
    ~Sys();

    static QSettings    settings();

    static void         appDebugBreak();
    static void         render(eRenderType rtype);

    static void         setSysMouseMode(eMouseMode newMode, bool set);
    static bool         getSysMouseMode(eMouseMode mode);

    static Motif::Type  XMLgetMotifType(QString name);
    static QString      XMLgetMotifName(Motif::Type type);
    static QStringList  XMLgetMotifNames(Motif::Type type);

    static QMultiMap<Motif::Type,QString> initMotifAssociations();

    static uint         nextSigid() { return ++rx_sigid; }
    static uint         rxSigid()   { return rx_sigid; }

    static bool         isGuiThread();

    static void         dumpRefs();

    static QString      getRootMediaDir()                   { return _rootMediaDir; }
    static QString      getRootImageDir()                   { return _rootImageDir; }
    static QString      getLogDir()                         { return _logDir; }

    static void         setRootMediaDir(QString dir)        { _rootMediaDir = dir; }
    static void         setRootImageDir(QString dir)        { _rootImageDir = dir; }
    static void         setLogDir(QString dir)              { _logDir       = dir; }


    // system
    static class SplashScreen    * splash;
    static class qtAppLog        * log;
    static class Configuration   * config;
    static class ControlPanel    * controlPanel;
    static class SplitScreen     * splitter;

    // makers
    static class DesignMaker    * designMaker;
    static class MapEditor      * mapEditor;
    static class MosaicMaker    * mosaicMaker;
    static class MotifMaker     * motifMaker;
    static class TilingMaker    * tilingMaker;
    static class ImageEngine    * imageEngine;

    // selections
    static class Layer          * selectedLayer;

    // system view controller
    static class ViewController * systemViewController;

    static DebugViewPtr     debugView;
    static CropViewPtr      cropMakerView;
    static GridViewPtr      gridViewer;
    static CLinesViewPtr    constructionLinesView;
    static MapedViewPtr     mapEditorView;
    static MMViewiewPtr     motifMakerView;
    static TMViewPtr        tilingMakerView;
    static ProtoViewPtr     prototypeView;
    static ImageViewPtr     imageViewer;
    static BkgdImagePtr     backgroundImage;    // the one background image - loaded/unloaded, never replaced

    static class DebugMap   * debugMapCreate;
    static class DebugMap   * debugMapPaint;
    static class DebugMap   * debugMapMotif;
    static class DebugFlags * flags;

    static bool   isDarkTheme;
    static bool   imgGeneratorInUse;
    static bool   localCycle;
    static bool   primaryDisplay;
    static bool   startupAutoloading;
    static bool   tm_hideTiles;    // Tiling Maker: suppress drawing placed tiles; never persisted, always starts false
    static bool   hideCircles;
    static bool   bm_editBorder;   // Border Maker: show current border's crop; never persisted, always starts false
    static bool   enableDetachedPages;
    static bool   showCenterDebug;
    static bool   showCenterMouse;
    static bool   updatePanel;
    static bool   dontReplicate;
    static bool   highlightUnit;
    static bool   dontTrapLog;
    static bool   tm_fill;

    static int    appInstance;
    static qreal  aspectRatio;
    static bool   lockViewAspectRatio;

    static QString gitBranch;       // if there is one
    static QString gitSha;          // if there is one
    static QString gitRoot;         // if there is one

    static QString logName;

    static QString bkgdPhotosDir;
    static QString examplesDir;
    static QString mapsDir;
    static QString newMosaicDir;
    static QString newTileDir;
    static QString originalMosaicDir;
    static QString originalTileDir;
    static QString rootMosaicDir;
    static QString rootTileDir;
    static QString templateDir;
    static QString testMosiacDir;
    static QString testTileDir;
    static QString worklistsDir;
    static QString snapsDir;
    static QString renderedDir;        // svg files
    static QString girihDir;

    static QString lastPanelTitle;
    static QString lastViewTitle;

    static QString xmlTool;
    static QString diffTool;

    static QMultiMap<Motif::Type,QString> motifRepresentation;

    static constexpr QChar MathSymbolSquareRoot = QChar(0x221A);
    static constexpr QChar MathSymbolPi         = QChar(0x03A0);
    static constexpr QChar MathSymbolDelta      = QChar(0x0394);
    static constexpr QChar MathSymbolSigma      = QChar(0x03A3);

    static constexpr qreal GOLDEN_RATIO     = 1.61803398874989484820;

    static constexpr int   DEFAULT_WIDTH    = 1500;
    static constexpr int   DEFAULT_HEIGHT   = 927;
    static constexpr int   MAX_WIDTH        = 4096;
    static constexpr int   MAX_HEIGHT       = 2160;

    static const QString defaultMosaicName;
    static const QString defaultTilingName;

    static VersionFileList tilingsList;
    static VersionFileList mosaicsList;
    static TilingUses      tilingUses;

    // which real owner (Tiling or Mosaic) is the active canvas right now - never
    // persisted, session-only. setActiveCanvas() is the one place that knows how to
    // derive the real Canvas* from the enum, and it pushes that into
    // systemViewController itself, so getActiveCanvas() is just a plain read-back -
    // there's no second, parallel derivation that could disagree with what was set
    static eActiveCanvas  getActiveCanvasOwner()  { return _activeCanvas; }
    static void           setActiveCanvas(eActiveCanvas owner);
    static class Canvas * getActiveCanvas();
    static QTransform     getActiveDistortion();

private:
    static eActiveCanvas _activeCanvas;

    static QString _rootImageDir;
    static QString _rootMediaDir;
    static QString _logDir;

    static int     iMouseMode;
    static uint    rx_sigid;
};
