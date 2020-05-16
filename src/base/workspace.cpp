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

#include "base/canvas.h"
#include "base/configuration.h"
#include "base/workspace.h"
#include "base/xmlwriter.h"
#include "base/xmlloader.h"
#include "base/fileservices.h"
#include "base/tilingmanager.h"
#include "base/utilities.h"
#include "makers/tiling_maker/tiling_maker.h"

Workspace * Workspace::mpThis = nullptr;


Workspace * Workspace::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new Workspace();
    }
    return mpThis;
}

void Workspace::releaseInstance()
{
    if (mpThis != nullptr)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

Workspace::Workspace()
{
}

void Workspace::init()
{
    config = Configuration::getInstance();
    canvas = Canvas::getInstance();
}

Workspace::~Workspace()
{
    clearDesigns();
    ws[WS_LOADED].clear();
    ws[WS_TILING].clear();
}

void Workspace::clearDesigns()
{
    for (auto it = activeDesigns.begin(); it != activeDesigns.end(); it++)
    {
        DesignPtr d = *it;
        d->destoryPatterns();
    }
    activeDesigns.clear();
}

void Workspace::slot_clearCanvas()
{
    // remove from scene but does not delete
    canvas->clearScene();
}

void Workspace::slot_clearWorkspace()
{
    canvas->dump(true);

    clearDesigns();
    ws[WS_LOADED].clear();
    ws[WS_TILING].clear();

    canvas->dump(true);
}

void Workspace::addDesign(DesignPtr d)
{
    // just added to workspace - the WorkspaceViewer adds it to the canvas
    activeDesigns.push_back(d);
    designName = QString("Design: %1").arg(d->getTitle());
    qDebug() << designName << "addded to workspace";
}

bool Workspace::loadDesignXML(QString name)
{
    qDebug() << "Workspace::loadTapratsXML()" << name;

#if 0
    if (name == config->currentlyLoadedXML)
    {
        qDebug() << "Already loaded";
        return true;     // pattern already loadded
    }
#endif

    canvas->dump(true);
    ws[WS_LOADED].clear();
    canvas->dump(true);

    QString file = FileServices::getDesignXMLFile(name);
    if (file.isEmpty())
    {
        QMessageBox box;
        box.setText(QString("File <%1>not found").arg(name));
        box.exec();
        return false;
    }

    QFile afile(file);
    if (!afile.exists())
    {
        QMessageBox box;
        box.setText(QString("File <%1>not found").arg(file));
        box.exec();
        return false;
    }

    qDebug().noquote() << "Loading:"  << file;

    StyledDesign * sd = &ws[WS_LOADED].styles;
    XmlLoader loader;
    bool rv = loader.loadXML(file,sd);

    if (rv)
    {
        config->lastLoadedXML      = name;
        config->currentlyLoadedXML = name;
        ws[WS_LOADED].styles.setName(name);

        StylePtr sp = ws[WS_LOADED].styles.getFirstStyle();
        if (sp)
        {
            PrototypePtr pp = sp->getPrototype();
            if (pp)
            {
                ws[WS_LOADED].prototype = sp->getPrototype();
                DesignElementPtr dp = pp->getDesignElement(0);
                if (dp)
                {
                    ws[WS_LOADED].desEle = dp;
                }
            }
        }
    }
    else
    {
        QString str = QString("Load ERROR - %1").arg(loader.getFailMessage());
        QMessageBox box;
        box.setText(str);
        box.exec();
    }
    return rv;
}

bool Workspace::saveStyledDesign(eWsData wsdata, QString name, QString & savedName, bool forceOverwrite)
{
    QString filename = FileServices::getDesignXMLFile(name);
    if (!forceOverwrite)
    {
        if (!filename.isEmpty())
        {
            QMessageBox msgBox;
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

    StyledDesign * sd = &getStyledDesign(wsdata);
    XmlWriter writer;
    bool rv = writer.writeXML(filename,sd);

    if (!forceOverwrite)
    {
        QMessageBox box;
        QString astring;
        if (rv)
        {
            astring ="File (" +  filename + ") - saved OK";
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

bool Workspace::loadTiling(QString name)
{
    TilingManager * tm = TilingManager::getInstance();

    ws[WS_TILING].clear();
    ws[WS_TILING].tiling = tm->loadTiling(name);

    if (!ws[WS_TILING].tiling)
    {
        return false;
    }
    return true;
}

bool Workspace::saveTiling(QString name, TilingPtr tp)
{
    qDebug() << "Workspace::saveTiling"  << name;
    if (!tp)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Information);
        box.setText("Nothing to save");
        box.exec();
        return false;
    }

    if (tp->getName() != name)
    {
        tp->setName(name);
    }

    bool rv = tp->writeTilingXML();   // uses the name in the tiling
    if (rv)
    {
        tp->setDirty(false);
        emit sig_newTiling();
    }
    return rv;
}

// prototypes
void  Workspace::setPrototype(eWsData dataset, PrototypePtr pp)
{
    ws[dataset].prototype = pp;
}

PrototypePtr Workspace::getPrototype(eWsData dataset)
{
    return ws[dataset].prototype;
}

// figures
void  Workspace::setSelectedDesignElement(eWsData dataset, DesignElementPtr dep)
{
    qDebug() << "setWSDesignElement" << Utils::addr(dep.get());
    ws[dataset].desEle = dep;
    emit sig_ws_dele_changed();
}

DesignElementPtr Workspace::getSelectedDesignElement(eWsData dataset)
{
    return ws[dataset].desEle;
}

QVector<DesignPtr> & Workspace::getDesigns()
{
    return activeDesigns;
}

void WorkspaceData::clear()
{
    styles.clear();
    tiling.reset();
    desEle.reset();
    prototype.reset();
}
