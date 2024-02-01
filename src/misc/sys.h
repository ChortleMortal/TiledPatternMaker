#ifndef SYS_H
#define SYS_H

#include <QObject>
#include <QMultiMap>
#include <QMutex>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif
#include "enums/emotiftype.h"

class Layer;
class View;
class ViewController;
class ImageEngine;

class Sys
{
public:
    static QMultiMap<eMotifType,QString> init();

    static void         appDontPaint(bool stop);

    static eMotifType   getMotifType(QString name);
    static QString      getMotifName(eMotifType type);
    static QStringList  getMotifNames(eMotifType type);

    static void         dumpRefs();

    static View            * view;
    static ViewController  * viewController;
    static Layer           * selectedLayer;
    static ImageEngine     * imageEngine;

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
    static bool   debugMapEnable;
    static bool   dontTrapLog;
    static bool   flagA;
    static bool   flagB;
    static bool   measure;

    static int    appInstance;

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

private:

};

#endif // SYS_H
