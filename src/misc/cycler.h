#ifndef CYCLER_H
#define CYCLER_H

#include <QMetaType>
#include <QObject>
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

    QStringList           imgList;
    QStringList::iterator imgList_it;
};

#endif // CYCLER_H
