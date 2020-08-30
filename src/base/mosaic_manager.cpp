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

#include "base/mosaic_manager.h"
#include "base/fileservices.h"
#include "base/mosaic_loader.h"
#include "base/mosaic_writer.h"
#include "panels/panel.h"

MosaicManager::MosaicManager()
{
    workspace = Workspace::getInstance();
    config    = Configuration::getInstance();
}

bool MosaicManager::loadMosaic(QString name)
{
    qDebug() << "MosaicManager::loadMosaic()" << name;


    QString file = FileServices::getDesignXMLFile(name);
    if (file.isEmpty())
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setText(QString("File <%1>not found").arg(name));
        box.exec();
        return false;
    }

    QFile afile(file);
    if (!afile.exists())
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setText(QString("File <%1>not found").arg(file));
        box.exec();
        return false;
    }

    qDebug().noquote() << "Loading:"  << file;

    workspace->resetMosaic();

    MosaicLoader loader;
    MosaicPtr mosaic = loader.loadMosaic(file);
    workspace->setMosaic(mosaic);
    if (mosaic)
    {
        mosaic->setName(name);

        PrototypePtr selectedProto;
        const StyleSet & styleset = mosaic->getStyleSet();
        for (auto style : styleset)
        {
            PrototypePtr pp = style->getPrototype();
            if (pp)
            {
                if (!selectedProto)
                {
                    selectedProto = pp;
                }
                workspace->addPrototype(pp);
                DesignElementPtr dp = pp->getDesignElement(0);
                workspace->setSelectedDesignElement(dp);
            }
        }
        workspace->setSelectedPrototype(selectedProto);
        return true;
    }
    else
    {
        QString str = QString("Load ERROR - %1").arg(loader.getFailMessage());
        QMessageBox box(ControlPanel::getInstance());
        box.setText(str);
        box.exec();
        return false;
    }
}

bool MosaicManager::saveMosaic(QString name, QString & savedName, bool forceOverwrite)
{
    QString filename = FileServices::getDesignXMLFile(name);
    if (!forceOverwrite)
    {
        if (!filename.isEmpty())
        {
            QMessageBox msgBox(ControlPanel::getInstance());
            QString str = QString("The XML design file <%1> already exists").arg(filename);
            msgBox.setText(str);
            msgBox.setInformativeText("Do you want to bump version (Bump) or overwrite (Save)?");
            QPushButton * bump   = msgBox.addButton("Bump",QMessageBox::ApplyRole);
            QPushButton * save   = msgBox.addButton(QMessageBox::Save);
            QPushButton * cancel = msgBox.addButton(QMessageBox::Cancel);
            msgBox.setDefaultButton(bump);

            msgBox.exec();

            if (msgBox.clickedButton() == cancel)
            {
                return false;
            }
            else if (msgBox.clickedButton() == bump)
            {
                // appends a version
                name = FileServices::getNextVersion(name,false);
                filename = config->newDesignDir + "/" + name + ".xml";
            }
            // save drops thru
            Q_UNUSED(save)
        }
        else
        {
            filename = config->newDesignDir + "/" + name + ".xml";
        }
    }

    savedName = name;

    qDebug() << "Saving XML to:"  << filename;

    MosaicPtr mosaic = workspace->getMosaic();
    MosaicWriter writer;
    bool rv = writer.writeXML(filename,mosaic);

    if (!forceOverwrite)
    {
        QMessageBox box(ControlPanel::getInstance());
        QString astring;
        if (rv)
        {
            astring ="File (" +  filename + ") - saved OK";
            mosaic->setName(name);
        }
        else
        {
            QString str = writer.getFailMsg();
            astring = QString("Save File (%1) FAILED %2").arg(filename).arg(str);
            box.setIcon(QMessageBox::Warning);
        }
        box.setText(astring);
        box.exec();
    }
    return rv;
}
