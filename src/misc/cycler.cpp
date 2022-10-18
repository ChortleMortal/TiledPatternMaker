#include "misc/cycler.h"
#include "settings/configuration.h"
#include "misc/fileservices.h"
#include "widgets/versioned_list_widget.h"
#include "viewers/viewcontrol.h"

#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextEdit>

Q_DECLARE_METATYPE(QTextCharFormat)
Q_DECLARE_METATYPE(QTextCursor)

Cycler * Cycler::mpThis = nullptr;

Cycler * Cycler::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new Cycler();
    }
    return mpThis;
}

Cycler::Cycler() : QObject()
{
    busy        = false;
    cycleMode   = CYCLE_NONE;

    config      = Configuration::getInstance();
}

void Cycler::init(QThread * thread)
{
    moveToThread(thread);

    timer = new QTimer();

    qRegisterMetaType<eCycleMode>();
    qRegisterMetaType<QTextEdit*>();
    qRegisterMetaType<QTextCharFormat>();
    qRegisterMetaType<QTextCursor>();

    ViewControl * vc = ViewControl::getInstance();
    connect(this,   &Cycler::sig_clearView, vc,   &ViewControl::slot_unloadView);
    connect(timer,  &QTimer::timeout,       this, &Cycler::slot_timeout);

    timer->start(1000);
}

Cycler::~Cycler()
{
    qDebug() << "Cycler destructor";
}

void Cycler::slot_startCycle(eCycleMode mode)
{
    if (cycleMode != CYCLE_NONE)
    {
        qDebug() << "Stopping the cycler";
        slot_stopCycle();
        return;
    }

    cycleMode = mode;
    qDebug() << "slot_cycle" << sCycleMode[cycleMode];

    cyclePause = false;
    busy       = false;

    switch(cycleMode)
    {
    case CYCLE_STYLES:
        startCycleStyles();
        break;
    case CYCLE_TILINGS:
        startCycleTilings();
        break;
    case CYCLE_ORIGINAL_PNGS:
        startCycleOriginalDesignPngs();
        break;
    case CYCLE_COMPARE_ALL_IMAGES:
        startCycleCompareAllImages();
        break;
    case CYCLE_COMPARE_WORKLIST_IMAGES:
        startCycleCompareWorklistImages();
        break;
    case CYCLE_NONE:
    case CYCLE_SAVE_TILING_BMPS:
    case CYCLE_SAVE_STYLE_BMPS:
        qWarning() << "slot_startCycle: unexpeced mode";
    }
}

void Cycler::slot_stopCycle()
{
    eCycleMode oldMode = cycleMode;
    qDebug() << "slot_stopCycle";

    if ((oldMode == CYCLE_COMPARE_ALL_IMAGES || oldMode == CYCLE_COMPARE_WORKLIST_IMAGES) && config->generate_workList)
    {
        emit sig_workList();
    }

    cycleMode = CYCLE_NONE;
    if (oldMode != CYCLE_NONE)
    {
        imgList.clear();
        emit sig_finished();
    }
    busy = false;
}

void Cycler::slot_ready()
{
    busy = false;
    slot_timeout(); //force immediate
}

void Cycler::slot_timeout()
{
    if (cycleMode == CYCLE_NONE)
    {
        return;
    }

    if (cyclePause || busy)
    {
        return;
    }

    switch (cycleMode)
    {
    case CYCLE_TILINGS:
        if (cCount++ <= config->cycleInterval)
        {
            qDebug() << "Tick";
            return;
        }
        cCount = 0;
        busy   = true;
        if (++cIndex < files.size())
        {
            QString name = files.at(cIndex);
            emit sig_cycleLoadTiling(name);
        }
        else
        {
            slot_stopCycle();
        }
        break;

    case CYCLE_STYLES:
        if (cCount++ <= config->cycleInterval)
        {
            qDebug() << "Tick";
            return;
            break;
        }
        cCount = 0;
        busy   = true;
        if (++cIndex < files.size())
        {
            QString name = files.at(cIndex);
            emit sig_cycleLoadMosaic(name);
        }
        else
        {
            slot_stopCycle();
        }
        break;


    case CYCLE_COMPARE_ALL_IMAGES:
    case CYCLE_COMPARE_WORKLIST_IMAGES:
        if (imgList_it == imgList.end())
        {
            slot_stopCycle();
        }
        else
        {
            busy = true;
            QString name = *imgList_it;
            imgList_it++;
            emit sig_compare(name,name,true);
        }
        break;

    case CYCLE_NONE:
    case CYCLE_ORIGINAL_PNGS:
    case CYCLE_SAVE_STYLE_BMPS:
    case CYCLE_SAVE_TILING_BMPS:
        break;
    }
}

void Cycler::slot_psuedoKey(int key )
{
    qDebug() << "key=" << key;

    if (key ==  Qt::Key_Space)
    {
        if (cycleMode == CYCLE_ORIGINAL_PNGS)
        {
            nextCyclePng();
        }
        else
        {
            cyclePause = !cyclePause;
            cCount     = config->cycleInterval;
            qDebug() << "pause="  << cyclePause;
        }
    }
}

void Cycler::startCycleStyles()
{
    files = FileServices::getMosaicNames( config->fileFilter);

    cIndex = -1;
    cCount = config->cycleInterval;     // start now
}

void Cycler::startCycleTilings()
{
    files = FileServices::getTilingNames(LOAD_ALL);

    cIndex = -1;
    cCount = config->cycleInterval;     // start now
}

void Cycler::startCycleCompareAllImages()
{
    QMap<QString,QString>  mapa = FileServices::getDirBMPFiles(config->compareDir0);
    QStringList names = mapa.keys();

    VersionList vlist;
    vlist.create(names);

    imgList    = vlist.recompose();
    imgList_it = imgList.begin();

    if  (config->generate_workList)
    {
        config->clearWorkList();
    }
}

void Cycler::startCycleCompareWorklistImages()
{
    VersionList vlist;
    vlist.create(config->getWorkList());

    imgList    = vlist.recompose();
    imgList_it = imgList.begin();

    if  (config->generate_workList)
    {
        config->clearWorkList();
    }
}

void Cycler::startCycleOriginalDesignPngs()
{
    pngRow   = 0;
    pngCol   = 0;
    pngIndex = 0;

    fileFilter << "*.png";

    QString path = config->examplesDir;
    QDir adir(path);
    files = adir.entryList(fileFilter);
    qDebug() << "num pngs =" << files.size();

    nextCyclePng();
}

void Cycler::nextCyclePng()
{
    qDebug() << "page starting: " << pngIndex;

    emit sig_clearView();
    while (pngIndex < files.size())
    {
        emit sig_show_png(files.at(pngIndex),pngRow,pngCol);
        pngIndex++;

        if (++pngCol > 4)
        {
            pngCol = 0;
            if (++pngRow > 4)
            {
                // pause
                pngRow = 0;
                return;
            }
        }
    }
    pngIndex   = 0;
    pngRow = 0;
    pngCol = 0;
}

