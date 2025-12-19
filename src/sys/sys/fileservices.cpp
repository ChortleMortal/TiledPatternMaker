
#include <QMessageBox>
#include <QDirIterator>
#include <QDebug>
#include <QDate>
#include "sys/sys/fileservices.h"
#include "sys/sys/pugixml.hpp"
#include "sys/sys.h"
#include "model/settings/configuration.h"

using namespace pugi;

///////////////////////////////////////////////////////////
//
// File Services
//
///////////////////////////////////////////////////////////

VersionFileList FileServices::getFiles(eFileType type)
{
    switch (type)
    {
    case FILE_MOSAIC:
        return _getFiles(Sys::rootMosaicDir,"*.xml");

    case FILE_TILING:
        return _getFiles(Sys::rootTileDir,"*.xml");

    case FILE_MAP:
        return _getFiles(Sys::mapsDir,"*.xml");

    case FILE_GIRIH:
        return _getFiles(Sys::config->rootMediaDir + "girih_shapes","*.xml");

    case FILE_TEMPLATE:
    {
        VersionFileList xfiles = _getFiles(Sys::templateDir,"*.dat");
        xfiles += _getFiles(QDir::currentPath(),"*.dat");
        return xfiles;
    }
    case FILE_TEMPLATE2:
    {
        VersionFileList xfiles = _getFiles(Sys::templateDir,"*.xml");
        xfiles += _getFiles(QDir::currentPath(),"*.xml");
        return xfiles;
    }
    }

    VersionFileList compilerBug;
    return compilerBug;
}

VersionedFile FileServices::getFile(const VersionedName & vname, eFileType type)
{
    switch (type)
    {
    case FILE_MOSAIC:
        return _getFile(Sys::rootMosaicDir,"*.xml",vname);

    case FILE_TILING:
        return _getFile(Sys::rootTileDir,"*.xml",vname);

    case FILE_MAP:
        return _getFile(Sys::mapsDir,"*.xml",vname);

    case FILE_GIRIH:
        return _getFile(Sys::config->rootMediaDir + "girih_shapes","*.xml",vname);

    case FILE_TEMPLATE:
    {
        VersionedFile xfile = _getFile(Sys::templateDir,"*.dat",vname);
        if (xfile.isEmpty())
            xfile = _getFile(QDir::currentPath(),"*.dat",vname);
        return xfile;
    }
    case FILE_TEMPLATE2:
    {
        VersionedFile xfile = _getFile(Sys::templateDir,"*.xml",vname);
        if (xfile.isEmpty())
            xfile = _getFile(QDir::currentPath(),"*.xml",vname);
        return xfile;
    }
    }

    VersionedFile compilerBug;
    return compilerBug;
}

VersionFileList FileServices::_getFiles(QString path, QString ext)
{
    VersionFileList files;

    QDirIterator it(path, QStringList() << ext, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        VersionedFile xfile;
        xfile.setFromFullPathname(it.next());
        files.add(xfile);
    }

    files.sort();

    return files;
}

VersionedFile FileServices::_getFile(QString path, QString ext, const VersionedName & vname)
{
    QString ext2 = ext;
    ext2.remove('*');
    QString filename = vname.get() + ext2;

    VersionedFile xfile;
    QDirIterator it(path, QStringList() << ext, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString afile = it.next();
        if (it.fileName() == filename)
        {
            xfile.setFromFullPathname(afile);
            break;
        }
    }

    return xfile;
}

VersionList FileServices::getMosaicNames(eLoadType loadType)
{
    // names and version only, no extension
    Configuration * config = Sys::config;

    VersionList names;
    switch (loadType)
    {
    case ALL_MOSAICS:
        names += _getPathVersions(Sys::originalMosaicDir);
        names += _getPathVersions(Sys::newMosaicDir);
        names += _getPathVersions(Sys::testMosiacDir);
        break;

    case WORKLIST:
        names = config->worklist.get();
        break;

    case ALL_MOS_EXCEPT_WL:
    {
        names += _getPathVersions(Sys::originalMosaicDir);
        names += _getPathVersions(Sys::newMosaicDir);
        names += _getPathVersions(Sys::testMosiacDir);
        auto names2 = config->worklist.get();
        for (auto & name : names2)
        {
            names.remove(name);
        }
    }   break;

    case SELECTED_MOSAICS:
        if (config->mosaicOrigCheck)
        {
            names += _getPathVersions(Sys::originalMosaicDir);
        }
        if (config->mosaicNewCheck)
        {
            names += _getPathVersions(Sys::newMosaicDir);
        }
        if (config->mosaicTestCheck)
        {
            names += _getPathVersions(Sys::testMosiacDir);
        }
        if (config->mosaicFilterCheck &&  !config->mosaicFilter.isEmpty())
        {
            QString afilter = config->mosaicFilter;
            names =  names.filter(afilter,true);
        }
        break;

    case SINGLE_MOSAIC:
        qCritical("SINGLE_MOSAIC should not be called");
        break;

    case ALL_TILINGS:
    case ALL_TIL_EXCEPT_WL:
    case SELECTED_TILINGS:
        qCritical("Tilings - should be mosaic");
        break;
    }

    names.sort();

    return names;
}

VersionFileList FileServices::getMosaicFiles(eLoadType loadType)
{
    // names and version only, no extension
    Configuration * config = Sys::config;

    VersionFileList files;
    switch (loadType)
    {
    case ALL_MOSAICS:
        files += _getFiles(Sys::originalMosaicDir,"*.xml");
        files += _getFiles(Sys::newMosaicDir,"*.xml");
        files += _getFiles(Sys::testMosiacDir,"*.xml");
        break;

    case WORKLIST:
    {
        // get a copy because deletions happen to real worklist
        auto wlist = config->worklist.get();
        for (auto & vname : wlist)
        {
            VersionedFile vf =  getFile(vname, FILE_MOSAIC);
            if (!vf.isEmpty())
            {
                files.add(vf);
            }
        }
    }   break;

    case SELECTED_MOSAICS:
        if (config->mosaicOrigCheck)
        {
            files += _getFiles(Sys::originalMosaicDir,"*.xml");
        }
        if (config->mosaicNewCheck)
        {
            files += _getFiles(Sys::newMosaicDir,"*.xml");
        }
        if (config->mosaicTestCheck)
        {
            files += _getFiles(Sys::testMosiacDir,"*.xml");
        }
        if (config->mosaicFilterCheck &&  !config->mosaicFilter.isEmpty())
        {
            QString afilter = config->mosaicFilter;
            files =  files.filter(afilter,true);
        }

        break;

    case ALL_MOS_EXCEPT_WL:
    {
        files += _getFiles(Sys::originalMosaicDir,"*.xml");
        files += _getFiles(Sys::newMosaicDir,"*.xml");
        files += _getFiles(Sys::testMosiacDir,"*.xml");
        auto wlist = config->worklist.get();
        VersionFileList files2;
        for (auto & vname : wlist)
        {
            VersionedFile vf =  getFile(vname, FILE_MOSAIC);
            if (!vf.isEmpty())
            {
                files2.add(vf);
            }
        }
        for (auto & file : files2)
        {
            files.remove(file);
        }
    }   break;

    case SINGLE_MOSAIC:
        qCritical("SINGLE_MOSAIC should not be called");
        break;

    case ALL_TILINGS:
    case ALL_TIL_EXCEPT_WL:
    case SELECTED_TILINGS:
        qCritical("Tilings - should be mosaic");
        break;
    }
    return files;
}

QStringList FileServices::getMosaicRootNames(eLoadType loadType)
{
    // names only, no version, no extension
    VersionList slist = getMosaicNames(loadType);
    QStringList mosaicNames;
    for (const VersionedName & version : std::as_const(slist))
    {
        mosaicNames.push_back(version.getUnversioned());
    }
    mosaicNames.removeDuplicates();
    return mosaicNames;
}

VersionList FileServices::_getPathVersions(QString path)
{
    VersionList vl;
    QDirIterator it(path, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        VersionedName vn;
        vn.set(it.fileName());
        vl.add(vn);
    }
    return vl;
}

VersionList FileServices::getTilingNames(eLoadType loadType)
{
    Configuration * config = Sys::config;

    VersionList names;
    switch (loadType)
    {
    case ALL_TILINGS:
        names = _getPathVersions(Sys::rootTileDir);
        break;

    case WORKLIST:
        if (config->tilingWorklistCheck)
        {
            names = config->worklist.get();
        }
        break;

    case SELECTED_TILINGS:
        if (config->tilingOrigCheck)
        {
            names += _getPathVersions(Sys::originalTileDir);
        }
        if (config->tilingNewCheck)
        {
            names += _getPathVersions(Sys::newTileDir);
        }
        if (config->tilingTestCheck)
        {
            names += _getPathVersions(Sys::testTileDir);
        }
        break;


    case ALL_TIL_EXCEPT_WL:
    {
        names = _getPathVersions(Sys::rootTileDir);
        auto names2 = config->worklist.get();
        for (auto & vname : names2)
        {
            names.remove(vname.get());
        }
    }   break;

    case SINGLE_MOSAIC:
        qWarning() << "SINGLE_MOSAIC not supported";
        break;


    case ALL_MOSAICS:
    case ALL_MOS_EXCEPT_WL:
    case SELECTED_MOSAICS:
        qCritical("Selecting mosaics - should be tiling");
    }

    if (loadType != ALL_TILINGS &&  config->tileFilterCheck && !config->tileFilter.isEmpty())
    {
        QString afilter = config->tileFilter;
        names = names.filter(afilter, true);
    }

    names.sort();

    return names;
}

VersionFileList FileServices::getTilingFiles(eLoadType loadType)
{
    Configuration * config = Sys::config;

    VersionFileList files;
    switch (loadType)
    {
    case ALL_TILINGS:
        files = _getFiles(Sys::rootTileDir,"*.xml");
        break;

    case WORKLIST:
        if (config->tilingWorklistCheck)
        {
            for (auto & vname : config->worklist.get())
            {
                files.add(getFile(vname, FILE_TILING));
            }
        }
        break;

    case SELECTED_TILINGS:
        if (config->tilingOrigCheck)
        {
            files += _getFiles(Sys::originalTileDir,"*.xml");
        }
        if (config->tilingNewCheck)
        {
            files += _getFiles(Sys::newTileDir, "*.xml");
        }
        if (config->tilingTestCheck)
        {
            files += _getFiles(Sys::testTileDir,"*.xml");
        }
        break;

    case ALL_MOS_EXCEPT_WL:
        qWarning() << "ALL_MOS_EXCEPT_WL not supported";
        break;

    case SINGLE_MOSAIC:
        qWarning() << "SINGLE_MOSAIC not supported";
        break;

    case ALL_TIL_EXCEPT_WL:
        qWarning() << "ALL_MOS_EXCEPT_WL not supported";
        break;

    case ALL_MOSAICS:
    case SELECTED_MOSAICS:
        qCritical("Selecting mosaics - should be tiling");
    }

    if (loadType != ALL_TILINGS &&  config->tileFilterCheck && !config->tileFilter.isEmpty())
    {
        QString afilter = config->tileFilter;
        files = files.filter(afilter, true);
    }

    return files;
}

QStringList FileServices::getTilingRootNames(eLoadType loadType)
{
    // names only, no version, no extension
    auto slist = getTilingNames(loadType);
    QStringList tilingNames;
    for (auto & version : std::as_const(slist))
    {
        tilingNames.push_back(version.getUnversioned());
    }
    tilingNames.removeDuplicates();
    return tilingNames;
}

VersionedName FileServices::getNextVersion(eFileType type, VersionedName name)
{
    QString root = name.getUnversioned();

    QString path;
    switch (type)
    {
    case FILE_TILING:
        path = Sys::rootTileDir;
        break;
    case FILE_MOSAIC:
        path = Sys::rootMosaicDir;
        break;
    case FILE_MAP:
        path = Sys::mapsDir;
        break;
    case FILE_GIRIH:
    case FILE_TEMPLATE:
    case FILE_TEMPLATE2:
        qCritical("Not Supported");
    }

    VersionList vl = getFileVersions(root,path,false);

    uint new_version = 0;
    for (const VersionedName & vn : std::as_const(vl))
    {
        uint iver = vn.getVersion();
        if (iver > new_version)
        {
            new_version = iver;
        }
    }

    // bump
    new_version++;
    name.set(root + ".v" + QString::number(new_version));
    return name;
}

VersionList FileServices::getFileVersions(QString nameroot, QString path, bool useWorkList)
{
    VersionList versions;
    QString root(path);
    QDirIterator it(root, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        VersionedFile xfile;
        xfile.setFromFullPathname(it.next());
        if (xfile.getVersionedName().getUnversioned() == nameroot)
        {
            if (useWorkList && !Sys::config->worklist.get().contains(xfile.getVersionedName()))
                continue;   // exclude
            versions.add(xfile.getVersionedName());
        }
    }
    versions.sort();
    return versions;
}

bool FileServices::reformatXML(VersionedFile file)
{
    qDebug().noquote() << "reformatting"  << file.getPathedName();

    xml_document doc;
    xml_parse_result result = doc.load_file(file.getPathedName().toStdString().c_str());
    if (result == false)
    {
        qWarning().noquote() << result.description();
        return false;
    }

    QFile afile(file.getPathedName());
    afile. setPermissions(QFile::ReadOther | QFile::WriteOther);

    // Timestamp
    QDate cd = QDate::currentDate();
    QTime ct = QTime::currentTime();
    QString comment;
    QTextStream out(&comment);
    out << " Written: " << cd.toString() << " " << ct.toString() << " ";
    xml_node cnode = doc.prepend_child(node_comment);
    cnode.set_value(comment.toStdString().c_str());

    bool rv = doc.save_file(file.getPathedName().toStdString().c_str());
    if (!rv)
    {
        qWarning().noquote() << "Failed to reformat:" << file.getPathedName();
       return false;
    }
    qDebug() << "reformat OK";
    return true;
}

bool FileServices::verifyTilingFile(VersionedFile filename)
{
    if (filename.isEmpty())
    {
        return false;
    }

    xml_document doc;
    xml_parse_result result = doc.load_file(filename.getPathedName().toStdString().c_str());
    if (result == false)
    {
        qWarning().noquote() << result.description();
        return false;
    }

    xml_node node1 = doc.child("Tiling");
    if (!node1)
    {
        return false;
    }
    xml_node node2 =  node1.child("Name");
    if (!node2)
    {
        return false;
    }

    QString xmlname = node2.child_value();
    if (xmlname != filename.getVersionedName().get())
    {
        return false;
    }
    return true;
}

bool FileServices::usesTilingInMosaic(VersionedFile mosaic, VersionedName tile)
{
    VersionedFile tiling = getTileFileFromMosaicFile(mosaic);
    return (tiling.getVersionedName().get() == tile.get());
}

VersionFileList FileServices::whereTilingUsed(VersionedName tiling)
{
    VersionFileList mosaics = getFiles(FILE_MOSAIC);

    VersionFileList results;
    for (VersionedFile & mosaic : mosaics)
    {
        bool rv = usesTilingInMosaic(mosaic,tiling);
        if (rv)
        {
            qDebug() <<  tiling.get() << " found in "  << mosaic.getVersionedName().get();
            results.add(mosaic);
        }
    }
    return results;
}

VersionedFile FileServices::getTileFileFromMosaicFile(VersionedFile mosaicFile)
{
    VersionedFile fname;

    QFile afile(mosaicFile.getPathedName());
    bool rv = afile.open(QFile::ReadOnly);
    if (!rv) return fname;

    QTextStream str(&afile);
    QString aline;
    do
    {
        aline = str.readLine();
        if (aline.contains("<Tiling>"))
        {
            aline.remove("<Tiling>");
            aline.remove("</Tiling>");
            aline = aline.trimmed();
            VersionedName vn(aline);
            fname = getFile(vn,FILE_TILING);
            return fname;
        }
        else if (aline.contains("<string>"))
        {
            aline.remove("<string>");
            aline.remove("</string>");
            aline = aline.trimmed();
            VersionedName vn(aline);
            fname = getFile(vn,FILE_TILING);
            return fname;
        }
    } while (!aline.isNull());

    return fname;
}

QString FileServices::getBackgroundFilenameFromMosaicFile(VersionedFile mosaicFile)
{
    QFile afile(mosaicFile.getPathedName());
    bool rv = afile.open(QFile::ReadOnly);
    if (!rv) return QString();

    QTextStream str(&afile);
    QString aline;
    do
    {
        aline = str.readLine();
        if (aline.contains("<BackgroundImage"))
        {
            aline.remove("<BackgroundImage name=");
            aline.remove('"');
            aline.remove('>');
            aline = aline.trimmed();
            aline.remove(".jpg",Qt::CaseInsensitive);
            aline.remove(".jpeg",Qt::CaseInsensitive);
            aline.remove(".png",Qt::CaseInsensitive);
            aline.remove(".bmp",Qt::CaseInsensitive);
            aline.remove(".svg",Qt::CaseInsensitive);
            return aline;
        }
    } while (!aline.isNull());

    return QString();
}

QDate FileServices::getDateFromXMLFile(VersionedFile file)
{
    if (file.isEmpty()) return QDate();

    QFile afile(file.getPathedName());
    bool rv = afile.open(QFile::ReadOnly);
    if (!rv) return QDate();

    // <!-- Written: Tue Jul 11 2023 10:36:18 -->
    //QString date("Tue Jul 11 2023");

    // 'Written' appears in line 2 or line 3 or not at all
    QTextStream str(&afile);

    QString aline;
    for (int i=0; i< 3; i++)
    {
        str.readLineInto(&aline);
        if (aline.contains("Written:"))
        {
            aline = aline.trimmed();
            aline.remove("<!-- Written: ");
            aline.resize(aline.length()-13);
            QDate da = QDate::fromString(aline,"ddd MMM dd yyyy");
            if (!da.isValid())
            {
                da = QDate::fromString(aline,"ddd MMM d yyyy");
            }
            return da;
        }
    }
    return QDate();
}

TilingUses FileServices::getTilingUses()
{
    TilingUses uses;
    QString path(Sys::rootMosaicDir);
    QDirIterator it(path, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        VersionedFile mosaicFile;
        mosaicFile.setFromFullPathname(it.next());
        VersionedFile tile   = getTileFileFromMosaicFile(mosaicFile);
        uses.push_back(TilingUse(tile, mosaicFile));
    }
    return uses;
}

VersionList FileServices::getDirBMPFiles(QString path)
{
    VersionList vl;

    if (path.isEmpty())
    {
        qInfo() << "directory not configured for BMP file comparision";
        return vl;
    }

    QDirIterator it(path, QStringList() << "*.bmp", QDir::Files, QDirIterator::NoIteratorFlags);
    while (it.hasNext())
    {
        it.next();
        VersionedName vn(it.fileName());
        vn.set(it.fileName());
        vl.add(vn);
    }
    vl.sort();
    return vl;
}

bool FileServices::relocateTemplateFiles()
{
    // move template files if necessary
    QString oldLoc = Sys::rootMosaicDir + "/templates";
    QString newLoc = Sys::templateDir;

    QDir newDir(newLoc);
    if (!newDir.exists())
    {
        bool rv = QDir().mkdir(newLoc);
        if (!rv)
        {
            qWarning() << "Could not create directory:" << newLoc;
            return false;
        }
    }

    QDir oldDir(oldLoc);
    if (oldDir.exists() && !oldDir.isEmpty())
    {
        QFileInfoList infos = oldDir.entryInfoList(QDir::Files);
        for (QFileInfo & info : infos)
        {
            QString name    = info.fileName();
            QString newname = newLoc + "/" + name;
            QFile afile(info.canonicalFilePath());
            bool rv = afile.rename(newname);
            if (!rv)
            {
                qWarning().noquote() << "could not move" << info.canonicalFilePath() << "to" << newname;
                return false;
            }
        }
        bool rv = oldDir.removeRecursively();
        if (!rv)
        {
            qWarning().noquote() << "could not delete directory:" << oldLoc;
            return false;
        }
        qInfo().noquote() << "moved template files from" << oldLoc << "to:" << newLoc;
        return true;
    }
    return true;
}
