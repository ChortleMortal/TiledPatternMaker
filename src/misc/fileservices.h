#pragma once
#ifndef FILESERVICES_H
#define FILESERVICES_H

#include <QMultiMap>
#include "enums/efilesystem.h"

typedef QMultiMap<QString, QString> tilingUses;     // tiling name, design name
typedef QMap<QString,QString>       dirInfo;        // name,  filename

class FileServices
{
public:
    FileServices();

    static QStringList getMosaicFiles();                        // full path
    static QStringList getMosaicNames(eLoadType loadType);      // names and version only, no extension
    static QStringList getMosaicRootNames(eLoadType loadType);  // names only, no version, no extension
    static QString     getMosaicXMLFile(QString name);          // full path
    static QString     getMosaicTemplateFile(QString name);     // full path
    static dirInfo     getMosaicDirInfo();
    static QString     getMosaicNameOnly(QString name);

    static QStringList getTilingFiles();                        // full path
    static QStringList getTilingNames(eLoadType loadType);      // names only, not extension
    static QString     getTilingFile(QString name);             // full path
    static QString     getTileNameFromDesignName(QString designName);

    static QString     getNextVersion(eFileType type, QString name);
    static QStringList getFileVersions(QString name, QString rootPath);
    static bool        reformatXML(QString filename);

    static bool        verifyTilingName(QString name);
    static bool        usesTilingInDesign(QString file, QString tilename);
    static tilingUses  getTilingUses();

    static QMap<QString,QString> getDirBMPFiles(QString path);

    static QStringList getTemplates();
    static QStringList getMaps();
    static QStringList getPolys();

private:
    static QString     getTileNameFromDesignFile(QString designFile);
    static QString     getRoot(QString name);
    static QString     getVersion(QString name);
    static void        addNames(QStringList & names,QString path);  // names only, no extension

};

#endif // FILESERVICES_H
