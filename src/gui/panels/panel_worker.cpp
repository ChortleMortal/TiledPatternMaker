#include <QDebug>
#include <QFile>

#include "gui/panels/panel_worker.h"
#include "model/settings/configuration.h"
#include "sys/sys.h"
#include "sys/sys/fileservices.h"

PanelWorker::PanelWorker()
{
}

void PanelWorker::sortTilings()
{
    //qDebug() << "PanelWorker::sortTilings";

    eLoadType ltype = SELECTED_TILINGS;
    if (Sys::config->tilingWorklistCheck)
    {
        ltype = WORKLIST;
    }
    else if (Sys::config->tilingWorklistXCheck)
    {
        ltype = ALL_TIL_EXCEPT_WL;
    }

    VersionFileList vfl = FileServices::getTilingFiles(ltype);
    if (Sys::config->showWithBkgds)
    {
        vfl = findTilingsWithBkgds(vfl);
    }

    if (Sys::config->mosaicSortCheck)
    {
        vfl = sortTilingsByDate(vfl);
    }
    else
    {
        vfl.sort();
    }

    Sys::tilingsList = vfl;

    //qInfo() << "PanelWorker::sortTilings" << "- end";

    emit tilingsSorted();
}

void PanelWorker::getTilingUses()
{
    //qInfo() << "PanelWorker::getTilingUses";
    Sys::tilingUses  = FileServices::getTilingUses();
    emit tilingsSorted();
    //qInfo() << "PanelWorker::getTilingUses" << "- end";
}

void PanelWorker::sortMosaics()
{
    //qInfo() << "PanelWorker::sortMosaics";

    eLoadType ltype = SELECTED_MOSAICS;
    if (Sys::config->mosaicWorklistCheck)
    {
        ltype = WORKLIST;
    }
    else if (Sys::config->mosaicWorklistXCheck)
    {
        ltype = ALL_MOS_EXCEPT_WL;
    }

    VersionFileList vfl = FileServices::getMosaicFiles(ltype);
    if (Sys::config->showWithBkgds)
    {
        vfl = findMosaicsWithBkgds(vfl);
    }

    if (Sys::config->mosaicSortCheck)
    {
        vfl = sortMosaicsByDate(vfl);
    }
    else
    {
        vfl.sort();
    }

    Sys::mosaicsList = vfl;

    //qDebug() << #PanelWorker::sortMosaics" << "- end";

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
        if (afile.open(QIODevice::ReadOnly))
        {
            QTextStream in (&afile);
            const QString content = in.readAll();
            if (content.contains("<BackgroundImage"))
            {
                flist.add(file);
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
            if (afile.open(QIODevice::ReadOnly))
            {
                QTextStream in (&afile);
                const QString content = in.readAll();
                if (content.contains("<BackgroundImage"))
                {
                    vfl.add(file);
                }
            }
        }
    }
    return vfl;
}
