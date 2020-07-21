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

#include "base/configuration.h"
#include "base/workspace.h"
#include "base/xml_writer.h"
#include "base/xml_loader.h"
#include "base/fileservices.h"
#include "base/tilingmanager.h"
#include "base/utilities.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "panels/panel.h"

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
{}

void Workspace::init()
{
    config = Configuration::getInstance();
    view   = View::getInstance();
}

Workspace::~Workspace()
{
    clearDesigns();
    ws.clear();
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
    view->clearView();
}

void Workspace::slot_clearWorkspace()
{
    view->dump(true);

    clearDesigns();
    ws.clear();

    view->dump(true);
}

void Workspace::addDesign(DesignPtr d)
{
    // just added to workspace - the WorkspaceViewer adds it to the canvas
    activeDesigns.push_back(d);
    designName = QString("Design: %1").arg(d->getTitle());
    qDebug() << designName << "addded to workspace";
}

bool Workspace::loadMosaic(QString name)
{
    qDebug() << "Workspace::loadTapratsXML()" << name;

#if 0
    if (name == config->currentlyLoadedXML)
    {
        qDebug() << "Already loaded";
        return true;     // pattern already loadded
    }
#endif

    view->dump(true);
    ws.mosaic.reset();
    view->dump(true);

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

    XmlLoader loader;
    MosaicPtr mosaic = loader.loadMosaic(file);

    ws.mosaic = mosaic;

    if (mosaic)
    {
        config->lastLoadedXML      = name;
        config->currentlyLoadedXML = name;
        ws.mosaic->setName(name);

        const StyleSet & styleset = mosaic->getStyleSet();
        for (auto style : styleset)
        {
            PrototypePtr pp = style->getPrototype();
            if (pp)
            {
                addPrototype(pp);
                setSelectedPrototype(pp);
                DesignElementPtr dp = pp->getDesignElement(0);
                setSelectedDesignElement(dp);
            }
        }
        return true;
    }
    else
    {
        QString str = QString("Load ERROR - %1").arg(loader.getFailMessage());
        QMessageBox box;
        box.setText(str);
        box.exec();
        return false;
    }
}

bool Workspace::saveMosaic(QString name, QString & savedName, bool forceOverwrite)
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

    XmlWriter writer;
    bool rv = writer.writeXML(filename,ws.mosaic);

    if (!forceOverwrite)
    {
        QMessageBox box;
        QString astring;
        if (rv)
        {
            astring ="File (" +  filename + ") - saved OK";
            ws.mosaic->setName(name);
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

    TilingPtr tp = tm->loadTiling(name);
    if (tp)
    {
        ws.tiling = tp;
        return  true;
    }
    else
    {
        return false;
    }
}

bool Workspace::saveTiling(QString name, TilingPtr tp)
{
    qDebug() << "Workspace::saveTiling"  << name;
    if (!tp)
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Information);
        box.setText("Nothing to save");
        box.exec();
        return false;
    }

    if (tp->getName() != name)
    {
        tp->setName(name);
    }

    View * view = View::getInstance();
    QSize size  = view->size();
    tp->setCanvasSize(size);

    TilingMakerPtr maker = TilingMaker::getInstance();
    if (maker->currentTiling == tp)
    {
        Xform xf = maker->getCanvasXform();
        tp->setCanvasXform(xf);
    }

    TilingWriter writer(tp);
    bool rv = writer.writeTilingXML();   // uses the name in the tiling
    if (rv)
    {
        tp->setDirty(false);
        emit sig_newTiling();
    }
    return rv;
}

// prototypes
void  Workspace::addPrototype(PrototypePtr pp)
{
    ws.prototypes.push_back(pp);
}

QVector<PrototypePtr> Workspace::getPrototypes()
{
    return ws.prototypes;
}

void Workspace::setSelectedPrototype(PrototypePtr proto)
{
    if (ws.selectedPrototype != proto)
    {
        ws.selectedPrototype     = proto;
        ws.selectedDesignElement  = proto->getDesignElement(0);
        emit sig_selected_proto_changed();
    }
}

PrototypePtr Workspace::getSelectedPrototype()
{
    return ws.selectedPrototype;
}

// figures
void  Workspace::setSelectedDesignElement(DesignElementPtr dep)
{
    if (!ws.selectedPrototype->getDesignElements().contains(dep))
    {
        qWarning("selected designEelement not in selectedPrototype");
    }

    if (ws.selectedDesignElement != dep)
    {
        ws.selectedDesignElement = dep;
        emit sig_selected_dele_changed();
    }
}

DesignElementPtr Workspace::getSelectedDesignElement()
{
    return ws.selectedDesignElement;
}

QVector<DesignPtr> & Workspace::getDesigns()
{
    return activeDesigns;
}

CanvasSettings & Workspace::getMosaicSettings()
{
    static CanvasSettings dummy;    // default
    if (ws.mosaic)
    {
        return ws.mosaic->getCanvasSettings();
    }
    else
    {
        return dummy;
    }
}

void WorkspaceData::clear()
{
    mosaic.reset();
    tiling.reset();
    prototypes.clear();

    selectedPrototype.reset();
    selectedDesignElement.reset();
}
