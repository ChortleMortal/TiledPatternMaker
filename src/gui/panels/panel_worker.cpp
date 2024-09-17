#include <QDebug>
#include <QFile>

#include "gui/panels/panel_worker.h"
#include "sys/sys/fileservices.h"
#include "sys/sys.h"

PanelWorker::PanelWorker()
{
}

void PanelWorker::sortTilings(bool worklist, bool backgrounds, bool sortByDate)
{
    //qDebug() << __FUNCTION__;

    eLoadType ltype = SELECTED_TILINGS;
    if (worklist)
    {
        ltype = WORKLIST;
    }

    VersionFileList vfl = FileServices::getTilingFiles(ltype);
    if (backgrounds)
    {
        vfl = findTilingsWithBkgds(vfl);
    }

    if (sortByDate)
    {
        vfl = sortTilingsByDate(vfl);
    }
    else
    {
        vfl.sort();
    }

    Sys::tilingsList = vfl;
    Sys::tilingUses  = FileServices::getTilingUses();

    //qDebug() << __FUNCTION__ << "- end";

    emit tilingsSorted();
}

void PanelWorker::sortMosaics(bool worklist, bool backgrounds, bool sortByDate)
{
    //qDebug() << __FUNCTION__;

    eLoadType ltype = SELECTED_MOSAICS;
    if (worklist)
    {
        ltype = WORKLIST;
    }

    VersionFileList vfl = FileServices::getMosaicFiles(ltype);
    if (backgrounds)
    {
        vfl = findMosaicsWithBkgds(vfl);
    }

    if (sortByDate)
    {
        vfl = sortMosaicsByDate(vfl);
    }
    else
    {
        vfl.sort();
    }

    Sys::mosaicsList = vfl;

    //qDebug() << __FUNCTION__ << "- end";

    emit mosaicsSorted();
}

VersionFileList PanelWorker::sortMosaicsByDate(VersionFileList & vlist)
{
    VersionFileList results;

    QMultiMap<uint,VersionedFile> datedMosaics;

    for (VersionedFile & file : vlist)
    {
        QDate date = FileServices::getDateFromXMLFile(file);
        uint idate = date.toJulianDay();
        datedMosaics.insert(idate,file);
    }

    // multimap sorts by key - so it is already sorted
    // just put the values into the result
    QMultiMap<uint, VersionedFile>::iterator it;
    for (it = datedMosaics.begin(); it!= datedMosaics.end(); it++)
    {
        results.prepend(it.value());
    }

    return results;
}

VersionFileList PanelWorker::sortTilingsByDate(VersionFileList &vlist)
{
    VersionFileList results;

    QMultiMap<uint,VersionedFile> datedMosaics;

    for (VersionedFile & file : vlist)
    {
        QDate date = FileServices::getDateFromXMLFile(file);
        uint idate = date.toJulianDay();
        datedMosaics.insert(idate,file);
    }

    // multimap sorts by key - so it is already sorted
    // just put the values into the result
    QMultiMap<uint, VersionedFile>::iterator it;
    for (it = datedMosaics.begin(); it!= datedMosaics.end(); it++)
    {
        results.prepend(it.value());
    }

    return results;
}

VersionFileList PanelWorker::findMosaicsWithBkgds(VersionFileList & files)
{
    VersionFileList flist;
    for (VersionedFile & file : files)
    {
        QFile afile(file.getPathedName());
        afile.open(QIODevice::ReadOnly);
        QTextStream in (&afile);
        const QString content = in.readAll();
        if (content.contains("<BackgroundImage"))
        {
            flist.push_back(file);
        }
        else
        {
            // could be in tiling
            // find tiling
            VersionedFile tilingFile = FileServices::getTileFileFromMosaicFile(file);
            // is tiling in tiling list
            auto qlist = Sys::tilingsList.filter(tilingFile.getVersionedName().get(),true);
            if (qlist.count() > 0)
            {
                flist.push_back(tilingFile);
            }
        }
    }
    return flist;
}

VersionFileList PanelWorker::findTilingsWithBkgds(VersionFileList & files)
{
    VersionFileList vfl;

    for (auto & file : files)
    {
        if (!file.isEmpty())
        {
            QFile afile(file.getPathedName());
            afile.open(QIODevice::ReadOnly);
            QTextStream in (&afile);
            const QString content = in.readAll();
            if (content.contains("<BackgroundImage"))
            {
                vfl.push_back(file);
            }
        }
    }
    return vfl;
}
