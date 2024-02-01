#include <QMessageBox>
#include <QDirIterator>
#include <QDebug>
#include <QDate>

#include "misc/fileservices.h"
#include "misc/pugixml.hpp"
#include "misc/sys.h"

#include "settings/configuration.h"

using namespace pugi;

FileServices::FileServices()
{
}

QStringList FileServices::getMosaicFiles()
{
    QStringList files;
    QString path(Sys::rootMosaicDir);
    QDirIterator it(path, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        files << it.next();
    }

    files.sort(Qt::CaseInsensitive);

    return files;
}

QStringList FileServices::getMosaicNames(eLoadType loadType)
{
    // names and version only, no extension

    Configuration * config = Configuration::getInstance();

    QStringList names;
    switch (loadType)
    {
    case ALL_MOSAICS:
        addNames(names,Sys::originalMosaicDir);
        addNames(names,Sys::newMosaicDir);
        addNames(names,Sys::testMosiacDir);
        break;

    case WORKLIST:
        names = config->worklist.get();
        break;

    case SELECTED_MOSAICS:
        if (config->mosaicOrigCheck)
        {
            addNames(names,Sys::originalMosaicDir);
        }
        if (config->mosaicNewCheck)
        {
            addNames(names,Sys::newMosaicDir);
        }
        if (config->mosaicTestCheck)
        {
            addNames(names,Sys::testMosiacDir);
        }
        break;

    case ALL_TILINGS:
    case SELECTED_TILINGS:
        qCritical("Selecting tilings - should be mosaic");
    }

    if (loadType != ALL_MOSAICS && config->mosaicFilterCheck &&  !config->mosaicFilter.isEmpty())
    {
        QString filter_lc = config->mosaicFilter.toLower();

        QStringList filtered;
        for (const auto & file : std::as_const(names))
        {
            QString file_lc = file.toLower();
            if (file_lc.contains(filter_lc))
            {
                filtered << file;
            }
        }
        names = filtered;
    }

    names.replaceInStrings(".xml","");

    names.sort(Qt::CaseInsensitive);

    return names;
}

QStringList FileServices::getMosaicRootNames(eLoadType loadType)
{
    // names only, no version, no extension
    auto slist = getMosaicNames(loadType);
    QStringList mosaicNames;
    for (auto & name : std::as_const(slist))
    {
        auto nameRoot = getMediaNameOnly(name);
        mosaicNames.push_back(nameRoot);
    }
    mosaicNames.removeDuplicates();
    return mosaicNames;
}

void FileServices::addNames(QStringList & names, QString path)
{
    QDirIterator it(path, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        names << it.fileName();
    }
}

QString FileServices::getMosaicXMLFile(QString name)
{
    Q_ASSERT(!name.contains(".xml"));
    name += ".xml";

    QString file;
    QString root(Sys::rootMosaicDir);
    QDirIterator it(root, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString afile = it.next();
        if (it.fileName() == name)
        {
            file = afile;
            break;
        }
    }

    return file;
}

QString FileServices::getMosaicTemplateFile(QString name)
{
    Q_ASSERT(!name.contains(".dat"));
    name += ".dat";

    QString file;
    QString root(Sys::templateDir);
    QDirIterator it(root, QStringList() << "*.dat", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString afile = it.next();
        if (it.fileName() == name)
        {
            file = afile;
            break;
        }
    }

    return file;
}

QStringList FileServices::getTilingFiles()
{
    QStringList files;
    QString path(Sys::rootTileDir);
    QDirIterator it(path, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        files << it.next();
    }

    files.sort(Qt::CaseInsensitive);

    return files;
}

QStringList FileServices::getTilingNames(eLoadType loadType)
{
    Configuration * config = Configuration::getInstance();

    QStringList names;
    switch (loadType)
    {
    case ALL_TILINGS:
        addNames(names,Sys::rootTileDir);
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
            addNames(names,Sys::originalTileDir);
        }
        if (config->tilingNewCheck)
        {
            addNames(names,Sys::newTileDir);
        }
        if (config->tilingTestCheck)
        {
            addNames(names,Sys::testTileDir);
        }
        break;

    case ALL_MOSAICS:
    case SELECTED_MOSAICS:
        qCritical("Selecting mosaics - should be tiling");
    }

    if (loadType != ALL_TILINGS &&  config->tileFilterCheck && !config->tileFilter.isEmpty())
    {
        QString filter_lc = config->tileFilter.toLower();

        QStringList filtered;
        for (const auto & file : std::as_const(names))
        {
            QString file_lc = file.toLower();
            if (file_lc.contains(filter_lc))
            {
                filtered << file;
            }
        }
        names = filtered;
    }

    names.replaceInStrings(".xml","");

    names.sort(Qt::CaseInsensitive);

    return names;
}

QStringList FileServices::getTilingRootNames(eLoadType loadType)
{
    // names only, no version, no extension
    auto slist = getTilingNames(loadType);
    QStringList tilingNames;
    for (auto & name : std::as_const(slist))
    {
        auto nameRoot = getMediaNameOnly(name);
        tilingNames.push_back(nameRoot);
    }
    tilingNames.removeDuplicates();
    return tilingNames;
}

QString FileServices::getTilingXMLFile(QString name)
{
    Q_ASSERT(!name.contains(".xml"));
    name += ".xml";

    QString file;
    QString root(Sys::rootTileDir);
    QDirIterator it(root, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString afile = it.next();
        if (it.fileName() == name)
        {
            file = afile;
            break;
        }
    }

    return file;
}

QString FileServices::getNextVersion(eFileType type, QString name)
{
    Q_ASSERT(!name.contains(".xml"));

    QString root    = getRoot(name);

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
    }

    QStringList qsl = getFileVersions(root,path);

    int new_version = 0;
    for (int i=0; i < qsl.size(); i++)
    {
        QString ver = getVersion(qsl[i]);
        if (ver.isEmpty())
            continue;
        ver.remove("v");
        int iver = ver.toInt();
        if (iver > new_version)
        {
            new_version = iver;
        }
    }

    // bump
    new_version++;
    name = root + ".v" + QString::number(new_version);
    return name;
}

QStringList FileServices::getFileVersions(QString name, QString rootPath)
{
    Q_ASSERT(!name.contains(".xml"));

    QStringList versions;
    QString root(rootPath);
    QDirIterator it(root, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        auto fileName = it.fileName();
        fileName.remove(".xml");
        if (fileName.contains(name))
        {
            QString candidate = fileName;
            QString root = getRoot(candidate);
            if (root == name)
            {
                versions << candidate;
            }
        }
    }
    versions.sort();
    return versions;
}

QString FileServices::getRoot(QString name)
{
    QString str = name.remove(".xml");

    if (getVersion(str).isEmpty())
    {
        return name;
    }

    QStringList parts = name.split('.');
    int sz = parts.size();
    QString root = parts[0];
    for (int i=1; i < sz-1; i++)
    {
        root += ".";
        root += parts[i];
    }
    return root;
}

QString FileServices::getVersion(QString name)
{
    QString str       = name.remove(".xml");
    QStringList parts = str.split('.');

    QString ver;
    if (parts.size() > 1 && parts.last().contains('v'))
        ver = parts.last();

    return ver;
}

QString FileServices::getMediaNameOnly(QString name)
{
    QString str       = name.remove(".xml");
    QStringList parts = str.split('.');

    if (parts.size() > 1 && parts.last().contains('v'))
    {
        str.remove(parts.last());
        str.chop(1);
    }

    return str;
}

QString FileServices::stripPath(QString path)
{
    QFileInfo info(path);
    return info.fileName();
}

bool FileServices::reformatXML(QString filename)
{
    qDebug().noquote() << "reformatting"  << filename;
    Q_ASSERT(filename.contains(".xml"));

    xml_document doc;
    xml_parse_result result = doc.load_file(filename.toStdString().c_str());
    if (result == false)
    {
        qWarning().noquote() << result.description();
        return false;
    }

    // Timestamp
    QDate cd = QDate::currentDate();
    QTime ct = QTime::currentTime();
    QString comment;
    QTextStream out(&comment);
    out << " Written: " << cd.toString() << " " << ct.toString() << " ";
    xml_node cnode = doc.prepend_child(node_comment);
    cnode.set_value(comment.toStdString().c_str());

    bool rv = doc.save_file(filename.toStdString().c_str());
    if (!rv)
    {
       qWarning().noquote() << "Failed to reformat:" << filename;
       return false;
    }
    qDebug() << "reformat OK";
    return true;
}

bool FileServices::verifyTilingName(QString name)
{
    Q_ASSERT(!name.contains(".xml"));

    QString filename = getTilingXMLFile(name);
    if (filename.isEmpty())
    {
        return false;
    }

    xml_document doc;
    xml_parse_result result = doc.load_file(filename.toStdString().c_str());
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
    if (xmlname != name)
    {
        return false;
    }
    return true;
}


bool  FileServices::usesTilingInDesign(QString designFile, QString tilename)
{
    Q_ASSERT(designFile.contains(".xml"));
    Q_ASSERT(!tilename.contains(".xml"));

    QString tilingName = getTileNameFromMosaicFile(designFile);
    return (tilingName == tilename);
}

QString FileServices::getTileNameFromDesignName(QString designName)
{
    Q_ASSERT(!designName.contains(".xml"));

    QString designFile = getMosaicXMLFile(designName);
    if (designFile.isEmpty())
        return designFile;  // corner case after deletion
    return getTileNameFromMosaicFile(designFile);
}

QString FileServices::getTileNameFromMosaicFile(QString mosaicFile)
{
    Q_ASSERT(mosaicFile.contains(".xml"));

    QString tiling;
    QFile afile(mosaicFile);
    bool rv = afile.open(QFile::ReadOnly);
    if (!rv) return tiling;

    QTextStream str(&afile);
    QString aline;
    do
    {
        aline = str.readLine();
        if (aline.contains("<string>"))
        {
            aline.remove("<string>");
            aline.remove("</string>");
            aline = aline.trimmed();
            return aline;
        }
    } while (!aline.isNull());
    return tiling;
}

QDate FileServices::getDateFromXMLFile(QString fileName)
{
    Q_ASSERT(fileName.contains(".xml"));

    QFile afile(fileName);
    bool rv = afile.open(QFile::ReadOnly);
    if (!rv) return QDate();

    // <!-- Written: Tue Jul 11 2023 10:36:18 -->
    //QString date("Tue Jul 11 2023");

    QTextStream str(&afile);
    QString aline;
    do
    {
        aline = str.readLine();
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
    } while (!aline.isNull());
    return QDate();
}

dirInfo  FileServices::getMosaicDirInfo()
{
    dirInfo info;

    QString path(Sys::rootMosaicDir);
    QDirIterator it(path, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString file = it.next();
        QString name = it.fileName();
        name.replace(".xml","");
        info.insert(name,file);
    }
    return info;
}

tilingUses  FileServices::getTilingUses()
{
    tilingUses uses;
    dirInfo info = getMosaicDirInfo();
    for (auto it = info.begin(); it != info.end(); it++)
    {
        QString designName  = it.key();
        QString designFile  = it.value();
        QString tileName    = getTileNameFromMosaicFile(designFile);
        uses.insert(tileName, designName);
    }
    return uses;
}

QMap<QString,QString> FileServices::getDirBMPFiles(QString path)
{
    QMap<QString,QString> map;

    if (path.isEmpty())
    {
        qInfo() << "directory not configured for BMP file comparision";
        return map;
    }

    QDirIterator it(path, QStringList() << "*.bmp", QDir::Files, QDirIterator::NoIteratorFlags);
    while (it.hasNext())
    {
        QString file = it.next();
        QString name = it.fileName();
        name.remove(".bmp");
        map[name] = file;
    }
    return map;
}


QStringList FileServices::getTemplates()
{
    QStringList files;

    // saved templates
    QString path(Sys::templateDir);
    QDirIterator it(path, QStringList() << "*.dat", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString name  = it.next();
        QFileInfo afile(name);
        QDateTime dt  = afile.lastModified();
        QString date  = dt.toString("yyyy-MM-dd hh:mm:ss");
        QString label = QString("%1 + %2").arg(name).arg(date);
        files.push_back(label);
    }

    // local templates
    path = QDir::currentPath();
    QDirIterator it3(path, QStringList() << "*.dat", QDir::Files, QDirIterator::Subdirectories);
    while (it3.hasNext())
    {
        QString name = it3.next();
        QFileInfo afile(name);
        QDateTime dt  = afile.lastModified();
        QString date  = dt.toString("yyyy-MM-dd hh:mm:ss");
        QString label = QString("%1 + %2").arg(name).arg(date);
        files.push_back(label);
    }
    return files;
}

QStringList FileServices::getMaps()
{
    QStringList files;

    // saved templates
    QString path(Sys::mapsDir);
    QDirIterator it(path, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString name  = it.next();
        name.remove(path);
        name.remove(".xml");
        files.push_back(name);
    }

    return files;
}

QStringList FileServices::getPolys()
{
    QStringList names;

    // saved templates
    Configuration * config = Configuration::getInstance();
    QString path = config->rootMediaDir + "girih_shapes";
    QDirIterator it(path, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        names << it.fileName();
    }

    names.replaceInStrings(".xml","");

    names.sort(Qt::CaseInsensitive);
    return names;
}
