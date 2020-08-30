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

#include "base/workspace.h"
#include "base/misc.h"
#include "base/configuration.h"
#include "base/mosaic_writer.h"
#include "base/mosaic_loader.h"
#include "base/fileservices.h"
#include "base/utilities.h"
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
    ws.resetAll();
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
    ws.resetAll();

    view->dump(true);
}

void Workspace::addDesign(DesignPtr d)
{
    // just added to workspace - the WorkspaceViewer adds it to the canvas
    activeDesigns.push_back(d);
    designName = QString("Design: %1").arg(d->getTitle());
    qDebug() << designName << "addded to workspace";
}

void Workspace::setMosaic(MosaicPtr mosaic)
{
    ws.mosaic = mosaic;
    UniqueQVector<TilingPtr> tilings = mosaic->getTilings();
    ws.tilings = tilings;
}

MosaicPtr Workspace::getMosaic()
{
    if (!ws.mosaic)
    {
        ws.mosaic = make_shared<Mosaic>();
    }
    return ws.mosaic;
}


TilingPtr Workspace::getCurrentTiling()
{
    if (ws.currentTiling)
    {
        return ws.currentTiling;
    }
    else if (ws.tilings.count())
    {
        ws.currentTiling = ws.tilings.first();
        return ws.currentTiling;
    }
    else
    {
        TilingPtr tp = make_shared<Tiling>();
        ws.tilings.push_back(tp);
        ws.currentTiling = tp;
        return tp;
    }
}

TilingPtr Workspace::findTiling(QString name)
{
    for (auto tiling : ws.tilings)
    {
        if (tiling->getName() == name)
        {
            return tiling;
        }
    }
    TilingPtr tp;
    return tp;
}

void Workspace::replaceTiling(TilingPtr oldtp, TilingPtr newtp)
{
    ws.tilings.removeOne(oldtp);
    ws.tilings.push_back(newtp);
    ws.currentTiling = newtp;
}

void Workspace::removeTiling(TilingPtr tp)
{
    ws.tilings.removeOne(tp);
    if (ws.currentTiling == tp)
    {
        ws.currentTiling.reset();
    }
}

void Workspace::setCurrentTiling(TilingPtr tp)
{
    ws.currentTiling = tp;
    ws.tilings.push_back(tp);   // does nothing if already there
}

// prototypes
void  Workspace::addPrototype(PrototypePtr pp)
{
    ws.prototypes.push_back(pp);
}

UniqueQVector<PrototypePtr> Workspace::getPrototypes()
{
    return ws.prototypes;
}

void Workspace::setSelectedPrototype(WeakPrototypePtr proto)
{
    if (ws.selectedPrototype.lock() != proto.lock())
    {
        ws.selectedPrototype     = proto;
        emit sig_selected_proto_changed();
    }
}

PrototypePtr Workspace::getSelectedPrototype()
{
    return ws.selectedPrototype.lock();
}

void Workspace::selectFeature(WeakFeaturePtr wfp)
{
    ws.selectedFeature = wfp;
}

FeaturePtr Workspace::getSelectedFeature()
{
    return ws.selectedFeature.lock();
}

// figures
void  Workspace::setSelectedDesignElement(WeakDesignElementPtr dep)
{
    if (ws.selectedDesignElement.lock() != dep.lock())
    {
        ws.selectedDesignElement = dep;
        emit sig_selected_dele_changed();
    }
}

DesignElementPtr Workspace::getSelectedDesignElement()
{
    return ws.selectedDesignElement.lock();
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

void  Workspace::setProtoMode(eProtoMode mode, bool enb)
{
    if (enb)
        ws.protoViewMode |= mode;
    else
        ws.protoViewMode &= ~mode;
}

void WorkspaceData::resetAll()
{
    mosaic.reset();
    resetTilings();
}

void WorkspaceData::resetTilings()
{
    currentTiling.reset();
    tilings.clear();
    prototypes.clear();

    selectedPrototype.reset();
    selectedDesignElement.reset();
}

