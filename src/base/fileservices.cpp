/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/fileservices.h"
#include "base/pugixml.hpp"
#include "base/configuration.h"
#include <QMessageBox>

using namespace pugi;

FileServices::FileServices()
{
}

QStringList FileServices::getDesignFiles()
{
    Configuration * config = Configuration::getInstance();

    QStringList files;
    QString path(config->rootDesignDir);
    QDirIterator it(path, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        files << it.next();
    }

    files.sort(Qt::CaseInsensitive);

    return files;
}

QStringList FileServices::getDesignNames()
{
    Configuration * config = Configuration::getInstance();

    QStringList names;
    QString path(config->rootDesignDir);
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

QStringList FileServices::getFilteredDesignNames(QString filter)
{
    QString filter_lc = filter.toLower();
    Configuration * config = Configuration::getInstance();

    QStringList names;
    QString path(config->rootDesignDir);
    QDirIterator it(path, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString file = it.next();
        QString file_lc = file.toLower();
        if (file_lc.contains(filter_lc))
        {
            names << it.fileName();
        }
    }

    names.replaceInStrings(".xml","");

    names.sort(Qt::CaseInsensitive);

    return names;
}

QString FileServices::getDesignXMLFile(QString name)
{
    Q_ASSERT(!name.contains(".xml"));
    name += ".xml";

    Configuration * config = Configuration::getInstance();

    QString file;
    QString root(config->rootDesignDir);
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

QString FileServices::getDesignTemplateFile(QString name)
{
    Q_ASSERT(!name.contains(".dat"));
    name += ".dat";

    Configuration * config = Configuration::getInstance();

    QString file;
    QString root(config->templateDir);
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
    Configuration * config = Configuration::getInstance();

    QStringList files;
    QString path(config->rootTileDir);
    QDirIterator it(path, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        files << it.next();
    }

    files.sort(Qt::CaseInsensitive);

    return files;
}

QStringList FileServices::getTilingNames()
{
    Configuration * config = Configuration::getInstance();

    QStringList names;
    QString path(config->rootTileDir);
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

QStringList FileServices::getFilteredTilingNames(QString filter)
{
    Configuration * config = Configuration::getInstance();

    QStringList names;
    QString path(config->rootTileDir);
    QDirIterator it(path, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString path = it.next();
        if (path.contains(filter))
        {
            names << it.fileName();
        }
    }

    names.replaceInStrings(".xml","");

    names.sort(Qt::CaseInsensitive);

    return names;
}

QString FileServices::getTilingFile(QString name)
{
    Q_ASSERT(!name.contains(".xml"));
    name += ".xml";

    Configuration * config = Configuration::getInstance();

    QString file;
    QString root(config->rootTileDir);
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

QString FileServices::getNextVersion(QString name, bool isTiling)
{
    Q_ASSERT(!name.contains(".xml"));

#if 0
    QStringList parts = name.split('.');
    int sz = parts.size();
    QString old_ver = parts[sz-1];
    QString root = parts[0];
    for (int i=1; i < sz-1; i++)
    {
        root += ".";
        root += parts[i];
    }
#else
    //QString old_ver = getVersion(name);
    QString root    = getRoot(name);
#endif

    Configuration * config = Configuration::getInstance();
    QString path;
    if (isTiling)
        path = config->rootTileDir;
    else
        path = config->rootDesignDir;

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
    if (isTiling)
        path = config->newTileDir;
    else
        path = config->newDesignDir;
    name = root + ".v" + QString::number(new_version);
    return name;
}

QStringList FileServices::getFileVersions(QString name, QString rootPath)
{
    Q_ASSERT(!name.contains(".xml"));

    QStringList versions;
    //QString file;
    QString root(rootPath);
    QDirIterator it(root, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        if (it.fileName().contains(name))
        {
            QString candidate = it.fileName();
#if 0
            QStringList parts = candidate.split('.');
            QString root = parts[0];
            for (int i=1; i < parts.size()-2; i++)  // name contains .xml
            {
                root += ".";
                root += parts[i];
            }
#else
            QString root = getRoot(candidate);
#endif
            if (root == name)
            {
                versions << candidate;
            }
        }
    }
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

    QString old_ver;
    if (parts.last().contains('v'))
        old_ver = parts.last();

    return old_ver;
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

    QString filename = getTilingFile(name);
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

    QString tilingName = getTileNameFromDesignFile(designFile);
    return (tilingName == tilename);
}

QString FileServices::getTileNameFromDesignName(QString designName)
{
    Q_ASSERT(!designName.contains(".xml"));

    QString designFile = getDesignXMLFile(designName);
    if (designFile.isEmpty())
        return designFile;  // corner case after deletion
    return getTileNameFromDesignFile(designFile);
}

QString FileServices::getTileNameFromDesignFile(QString designFile)
{
    Q_ASSERT(designFile.contains(".xml"));

    QString tiling;
    QFile afile(designFile);
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

dirInfo  FileServices::getDesignDirInfo()
{
    dirInfo info;

    Configuration * config = Configuration::getInstance();
    qInfo().noquote() << "Design Dir is:" << config->rootDesignDir;
    QString path(config->rootDesignDir);
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
    dirInfo info = getDesignDirInfo();
    for (auto it = info.begin(); it != info.end(); it++)
    {
        QString designName  = it.key();
        QString designFile  = it.value();
        QString tileName    = getTileNameFromDesignFile(designFile);
        uses.insert(tileName, designName);
    }
    return uses;
}

QMap<QString,QString> FileServices::getDirBMPFiles(QString path)
{
    QMap<QString,QString> map;

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
    Configuration * config = Configuration::getInstance();
    QString path(config->templateDir);
    QDirIterator it(path, QStringList() << "*.dat", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString name  = it.next();
        QFileInfo afile(name);
        QDateTime dt  = afile.lastModified();
        QString date  = dt.toString("yyyy-MM-dd hh:mm:ss");
        QString label = QString("%1 + %2").arg(name,date);
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
        QString label = QString("%1 + %2").arg(name,date);
        files.push_back(label);
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
