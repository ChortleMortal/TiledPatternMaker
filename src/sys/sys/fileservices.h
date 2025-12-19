#pragma once
#ifndef FILESERVICES_H
#define FILESERVICES_H

#include <QString>
#include <QDate>
#include "sys/enums/efilesystem.h"
#include "sys/sys/versioning.h"
#include "sys/sys.h"

namespace FileServices
{
    VersionFileList  getFiles(eFileType type);
    VersionFileList  getMosaicFiles(eLoadType loadType);
    VersionFileList  getTilingFiles(eLoadType loadType);
    VersionFileList  whereTilingUsed(VersionedName tiling);

    VersionedFile    getFile(const VersionedName &vname, eFileType type);
    VersionedFile    getTileFileFromMosaicFile(VersionedFile mosaicFile);
    QString          getBackgroundFilenameFromMosaicFile(VersionedFile mosaicFile);

    bool             usesTilingInMosaic(VersionedFile mosaic, VersionedName tile);
    bool             reformatXML(VersionedFile file);
    bool             verifyTilingFile(VersionedFile filename);
    TilingUses       getTilingUses();
    QDate            getDateFromXMLFile(VersionedFile file);

    VersionList      getMosaicNames(eLoadType loadType);      // names and version only, no extension
    VersionList      getTilingNames(eLoadType loadType);      // names only, not extension
    VersionList      getFileVersions(QString nameroot, QString path, bool useWorkList);
    VersionList      getDirBMPFiles(QString path);

    VersionedName    getNextVersion(eFileType type, VersionedName name);

    QStringList      getMosaicRootNames(eLoadType loadType);  // names only, no version, no extension
    QStringList      getTilingRootNames(eLoadType loadType);  // names only, no version, no extension

    bool             relocateTemplateFiles();                 // move template files if necessary

    VersionFileList  _getFiles(QString path, QString ext);
    VersionedFile    _getFile(QString path, QString ext, const VersionedName &vname);

    VersionList      _getPathVersions(QString path);
};

#endif // FILESERVICES_H
