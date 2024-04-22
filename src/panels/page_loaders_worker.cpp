#include <QDebug>

#include <QFile>

#include "panels/page_loaders_worker.h"
#include "misc/fileservices.h"
#include "misc/sys.h"

PageLoadersWorker::PageLoadersWorker()
{
}

void PageLoadersWorker::sortTilings(bool worklist, bool backgrounds, bool sort)
{
    eLoadType ltype = SELECTED_TILINGS;
    if (worklist)
    {
        ltype = WORKLIST;
    }

    QStringList qsl = FileServices::getTilingNames(ltype);
    if (backgrounds)
    {
        qsl = findTilingsWithBkgds(qsl);
    }

    if (sort)
    {
        qsl = sortTilingsByDate(qsl);
    }

    Sys::tilingsList = qsl;
    Sys::uses        = FileServices::getTilingUses();

    emit tilingsSorted();
}

void PageLoadersWorker::sortMosaics(bool worklist, bool backgrounds, bool sort)
{
    eLoadType ltype = SELECTED_MOSAICS;
    if (worklist)
    {
        ltype = WORKLIST;
    }

    QStringList qsl = FileServices::getMosaicNames(ltype);
    if (backgrounds)
    {
        qsl = findMosaicsWithBkgds(qsl);
    }

    if (sort)
    {
        qsl = sortMosaicsByDate(qsl);
    }

    Sys::mosaicsList = qsl;

    emit mosaicsSorted();
}

QStringList PageLoadersWorker::sortMosaicsByDate(const QStringList & names)
{
    QStringList results;

    QMultiMap<uint,QString> datedMosaics;

    for (auto & name : std::as_const(names))
    {
        QString filename = FileServices::getMosaicXMLFile(name);
        if (!filename.isEmpty())
        {
            QDate date =  FileServices::getDateFromXMLFile(filename);
            uint idate = 0;
            if (date.isValid())
            {
                idate = date.year() * 10000 + date.month() * 100 + date.day();
            }
            datedMosaics.insert(idate,name);
        }
    }

    // multimap sorts by key - so it is already sorted
    // just put the values into the result
    QMultiMap<uint, QString>::iterator it;
    for (it = datedMosaics.begin(); it!= datedMosaics.end(); it++)
    {
        results.push_front(it.value());
    }

    return results;
}

QStringList PageLoadersWorker::sortTilingsByDate(const QStringList & names)
{
    QStringList results;

    QMultiMap<uint,QString> datedMosaics;

    for (auto & name : std::as_const(names))
    {
        QString filename = FileServices::getTilingXMLFile(name);
        QDate date =  FileServices::getDateFromXMLFile(filename);
        int idate = 0;
        if (date.isValid())
        {
            idate = date.year() * 10000 + date.month() * 100 + date.day();
        }
        datedMosaics.insert(idate,name);
    }

    // multimap sorts by key - so it is already sorted
    // just put the values into the result
    QMultiMap<uint, QString>::iterator it;
    for (it = datedMosaics.begin(); it!= datedMosaics.end(); it++)
    {
        results.push_front(it.value());
    }

    return results;
}

QStringList PageLoadersWorker::findMosaicsWithBkgds(const QStringList &names)
{
    QStringList qsl;
    for (auto & name : std::as_const(names))
    {
        QString designFile = FileServices::getMosaicXMLFile(name);

        QFile XMLFile(designFile);
        XMLFile.open(QIODevice::ReadOnly);
        QTextStream in (&XMLFile);
        const QString content = in.readAll();
        if (content.contains("<BackgroundImage"))
        {
            qsl << name;
        }
        else
        {
            // could be in tiling
            // find tiling
            QString tilingName = FileServices::getTileNameFromDesignName(name);
            // is tiling in tiling list
            auto qlist = Sys::tilingsList.filter(tilingName);
            if (qlist.size() > 0)
            {
                qsl << name;
            }
        }
    }
    return qsl;
}

QStringList PageLoadersWorker::findTilingsWithBkgds(const QStringList & names)
{
    QStringList qsl;

    for (auto & name : std::as_const(names))
    {
        QString designFile = FileServices::getTilingXMLFile(name);

        QFile XMLFile(designFile);
        XMLFile.open(QIODevice::ReadOnly);
        QTextStream in (&XMLFile);
        const QString content = in.readAll();
        if (content.contains("<BackgroundImage"))
        {
            qsl << name;
        }
    }
    return qsl;
}
