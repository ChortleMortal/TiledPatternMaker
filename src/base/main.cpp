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

#include <QtCore>
#include <QtWidgets>

#include "qtapplog.h"
#include "base/configuration.h"
#include "base/canvas.h"
#include "viewers/workspaceviewer.h"
#include "base/tiledpatternmaker.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setStyle("Fusion");

    qtAppLog::getInstance();
    qInstallMessageHandler(&qtAppLog::crashMessageOutput);

    QDate cd = QDate::currentDate();
    QTime ct = QTime::currentTime();

#ifdef QT_DEBUG
    qDebug().noquote() << "=========== Debug Build:" << cd.toString() << ct.toString();
#else
    qDebug().noquote() << "============Release Build:" << cd.toString() << ct.toString();
#endif
    qDebug().noquote() << "App path:" << qApp->applicationDirPath();
    qDebug().noquote() << "Log:"  << qtAppLog::currentLogName;

#ifdef __linux__
    qDebug().noquote() << "Font:" << QApplication::font().toString();
    QFont afont = QApplication::font();
    afont.setPointSize(8);
    QApplication::setFont(afont);
#endif
    qDebug().noquote() << "Font:" << QApplication::font().toString();

    QCoreApplication::setOrganizationName("DAC");
    QCoreApplication::setApplicationName("TiledPatternMaker");

    // parse input
    bool noAutoLoad = false;
    bool mapVerify  = false;
    bool forceLoad  = false;
    bool firstBirthday = false;
    QString loadFile;
    for (int i = 1; i < argc; ++i)
    {
        if (qstrcmp(argv[i], "--no-autoload") == 0)
        {
            noAutoLoad = true;
        }
        if (qstrcmp(argv[i], "--map-verify") == 0)
        {
            mapVerify = true;
        }
        if (qstrcmp(argv[i], "--load") == 0)
        {
            forceLoad = true;
            i++;
            loadFile = argv[i];
        }
        if (qstrcmp(argv[i], "--first-birthday") == 0)
        {
            firstBirthday = true;
        }
    }

    // holding down shift key on start stop autoload
    Qt::KeyboardModifiers mods = QApplication::queryKeyboardModifiers();
    if (mods & Qt::ShiftModifier)
    {
        noAutoLoad = true;
    }

    // preconfigure
    if (noAutoLoad || mapVerify || forceLoad || firstBirthday)
    {
        Configuration * config = Configuration::getInstance();
        if (noAutoLoad)
        {
            config->autoLoadStyles  = false;
            config->autoLoadTiling  = false;
            config->autoLoadDesigns = false;
        }
        if (mapVerify)
        {
            config->verifyMaps      = true;
        }
        if (forceLoad)
        {
            config->autoLoadStyles  = true;
            config->lastLoadedXML   = loadFile;
        }
        if (firstBirthday)
        {
            config->reconfigurePaths();
        }
    }

    // instantiate and run
    TiledPatternMaker * maker = new TiledPatternMaker();

    int rv =  app.exec();

    qDebug() << "Exiting...";

    delete maker;

    qInstallMessageHandler(nullptr);				// restores
    qtAppLog::releaseInstance();

    return rv;
}
