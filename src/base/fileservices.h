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

#ifndef FILESERVICES_H
#define FILESERVICES_H

#include <QtCore>

typedef QMultiMap<QString, QString> tilingUses;     // tiling name, design name
typedef QMap<QString,QString>       dirInfo;        // name,  filename

class FileServices
{
public:
    FileServices();

    static QStringList getDesignFiles();                        // full path
    static QStringList getDesignNames();                        // names only, no extension
    static QStringList getFilteredDesignNames(QString filter);  // names only, filtered
    static QString     getDesignXMLFile(QString name);          // full path
    static QString     getDesignTemplateFile(QString name);     // full path
    static dirInfo     getDesignDirInfo();

    static QStringList getTilingFiles();                        // full path
    static QStringList getTilingNames();                        // names only, not extension
    static QStringList getFilteredTilingNames(QString filter);  // names only, filtered
    static QString     getTilingFile(QString name);             // full path
    static QString     getTileNameFromDesignName(QString designName);

    static QString     getNextVersion(QString name, bool isTiling);
    static bool        reformatXML(QString filename);

    static bool        verifyTilingName(QString name);
    static bool        usesTilingInDesign(QString file, QString tilename);
    static tilingUses  getTilingUses();

    static QMap<QString,QString> getDirFiles(QString path);

    static QStringList getTemplates();
    static QStringList getPolys();

private:
    static QString     getTileNameFromDesignFile(QString designFile);
    static QStringList getFileVersions(QString name, QString rootPath);
    static QString     getRoot(QString name);
    static QString     getVersion(QString name);

};

#endif // FILESERVICES_H
