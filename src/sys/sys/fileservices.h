#pragma once
#ifndef FILESERVICES_H
#define FILESERVICES_H

#include <QString>
#include <QDate>
#include "sys/enums/efilesystem.h"
#include "sys/sys/versioning.h"
#include "sys/sys.h"

class FileServices
{
public:
    static VersionFileList  getFiles(eFileType type);
    static VersionFileList  getMosaicFiles(eLoadType loadType);
    static VersionFileList  getTilingFiles(eLoadType loadType);
    static VersionFileList  whereTilingUsed(VersionedName tiling);

    static VersionedFile    getFile(const VersionedName &vname, eFileType);
    static VersionedFile    getTileFileFromMosaicFile(VersionedFile mosaicFile);

    static bool             usesTilingInMosaic(VersionedFile mosaic, VersionedName tile);
    static bool             reformatXML(VersionedFile file);
    static bool             verifyTilingFile(VersionedFile filename);
    static TilingUses       getTilingUses();
    static QDate            getDateFromXMLFile(VersionedFile file);

    static VersionList      getMosaicNames(eLoadType loadType);      // names and version only, no extension
    static VersionList      getTilingNames(eLoadType loadType);      // names only, not extension
    static VersionList      getFileVersions(QString nameroot, QString path, bool useWorkList);
    static VersionList      getDirBMPFiles(QString path);

    static VersionedName    getNextVersion(eFileType type, VersionedName name);

    static QStringList      getMosaicRootNames(eLoadType loadType);  // names only, no version, no extension
    static QStringList      getTilingRootNames(eLoadType loadType);  // names only, no version, no extension

private:
    static VersionFileList  getFiles(QString path, QString ext);
    static VersionedFile    getFile(QString path, QString ext, const VersionedName &vname);

    static VersionList      getPathVersions(QString path);
};

#endif // FILESERVICES_H
