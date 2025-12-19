#ifndef SYS_H
#define SYS_H

#include <QPointF>

#include "sys/enums/ebkgdimage.h"
#include "sys/enums/ecyclemode.h"
#include "sys/enums/emotiftype.h"
#include "sys/enums/erender.h"
#include "sys/sys/versioning.h"

#undef LEGACY_CONVERT_XML

typedef QPair<VersionedFile,VersionedFile>  TilingUse;      // tiling, mosaic
typedef QVector<TilingUse>                  TilingUses;

typedef std::shared_ptr<class DebugView>            DebugViewPtr;
typedef std::shared_ptr<class GridView>             GridViewPtr;
typedef std::shared_ptr<class CropViewer>           CropViewPtr;
typedef std::shared_ptr<class MapEditorView>        MapedViewPtr;
typedef std::shared_ptr<class MotifMakerView>       MMViewiewPtr;
typedef std::shared_ptr<class TilingMakerView>      TMViewPtr;
typedef std::shared_ptr<class PrototypeView>        ProtoViewPtr;
typedef std::shared_ptr<class ImageEngine>          IEViewPtrPtr;
typedef std::shared_ptr<class ImageViewer>          ImageViewPtr;
typedef std::shared_ptr<class BackgroundImage>      BkgdImagePtr;
typedef std::weak_ptr<class BackgroundImage>       wBkgdImagePtr;

class Sys
{
public:
    Sys();
    ~Sys();

    static void         appDebugBreak();
    static void         render(eRenderType rtype);

    static eMotifType   XMLgetMotifType(QString name);
    static QString      XMLgetMotifName(eMotifType type);
    static QStringList  XMLgetMotifNames(eMotifType type);

    static QMultiMap<eMotifType,QString> initMotifAssociations();

    static uint         nextSigid() { return ++rx_sigid; }
    static uint         rxSigid()   { return rx_sigid; }

    static bool         isGuiThread();

    static void         dumpRefs();

    static QString      createBMPDirectory();
    static void         setWorkingBMPDirectory(QString dir) { workingBMPDir = dir; }
    static QString      getWorkingBMPDirectory()            { return  workingBMPDir; }
    static QString      getSysBMPDirectory()                { return sysBMPDir; }

    static QString      getBMPPath(eActionType generatorType);

    static BkgdImagePtr  getBackgroundImageFromSource();

    // system
    static class qtAppLog        * log;
    static class Configuration   * config;
    static class ControlPanel    * controlPanel;
    static class SplitScreen     * splitter;
    static class SplashScreen    * splash;
    static class GuiModes        * guiModes;

    // makers
    static class DesignMaker    * designMaker;
    static class MapEditor      * mapEditor;
    static class MosaicMaker    * mosaicMaker;
    static class PrototypeMaker * prototypeMaker;
    static class TilingMaker    * tilingMaker;
    static class ImageEngine    * imageEngine;

    // selections
    static class Layer          * selectedLayer;

    // viewers
    static class SystemView           * sysview;
    static class SystemViewController * viewController;

    static DebugViewPtr     debugView;
    static CropViewPtr      cropViewer;
    static GridViewPtr      gridViewer;
    static MapedViewPtr     mapEditorView;
    static MMViewiewPtr     motifMakerView;
    static TMViewPtr        tilingMakerView;
    static ProtoViewPtr     prototypeView;
    static ImageViewPtr     imageViewer;

    static class DebugMap   * debugMapCreate;
    static class DebugMap   * debugMapPaint;
    static class DebugFlags * flags;

    static bool   isDarkTheme;
    static bool   imgGeneratorInUse;
    static bool   localCycle;
    static bool   primaryDisplay;
    static bool   hideCircles;
    static bool   enableDetachedPages;
    static bool   showCenterMouse;
    static bool   updatePanel;
    static bool   dontReplicate;
    static bool   highlightUnit;
    static bool   dontTrapLog;
    static bool   tm_fill;

    static int    appInstance;

    static QString gitBranch;       // if there is one
    static QString gitSha;          // if there is one
    static QString gitRoot;         // if there is one

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

    static eBkgdImgSource  currentBkgImage;
    static wBkgdImagePtr   definedBkImage;

private:
    static QString sysBMPDir;
    static QString workingBMPDir;
    static uint rx_sigid;
};

#endif // SYS_H
