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
#include "viewers/workspace_viewer.h"

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
    protoViewMode = PROTO_DRAW_FIGURES | PROTO_DRAW_FEATURES;
}

void Workspace::init()
{
    config = Configuration::getInstance();
    WorkspaceViewer::init();
}

Workspace::~Workspace()
{
    clearDesigns();
    resetAll();
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
    clearView();
}

void Workspace::slot_clearWorkspace()
{
    dump(true);

    clearDesigns();
    resetAll();

    dump(true);
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
    this->mosaic = mosaic;
    UniqueQVector<TilingPtr> tilings = mosaic->getTilings();
    this->tilings = tilings;
    setFillData(mosaic->getSettings().getFillData());
}

MosaicPtr Workspace::getMosaic()
{
    if (!mosaic)
    {
        mosaic = make_shared<Mosaic>();
    }
    return mosaic;
}

void Workspace::setCurrentTiling(TilingPtr tp)
{
    currentTiling = tp;
    tilings.push_front(tp);   // does nothing if already there
    fillData = tp->getFillData();
}

TilingPtr Workspace::getCurrentTiling()
{
    if (currentTiling)
    {
        return currentTiling;
    }
    else if (tilings.count())
    {
        currentTiling = tilings.first();
        return currentTiling;
    }
    else
    {
        TilingPtr tp = make_shared<Tiling>();
        setCurrentTiling(tp);
        return tp;
    }
}

TilingPtr Workspace::findTiling(QString name)
{
    for (auto tiling : tilings)
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
    tilings.removeOne(oldtp);
    tilings.push_back(newtp);
    currentTiling = newtp;
}

void Workspace::removeTiling(TilingPtr tp)
{
    tilings.removeOne(tp);
    if (currentTiling == tp)
    {
        currentTiling.reset();
    }
}

void Workspace::resetTilings()
{
    currentTiling.reset();
    tilings.clear();
    prototypes.clear();

    selectedPrototype.reset();
    selectedDesignElement.reset();
}
// prototypes
void  Workspace::addPrototype(PrototypePtr pp)
{
    prototypes.push_back(pp);
}

UniqueQVector<PrototypePtr> Workspace::getPrototypes()
{
    return prototypes;
}

void Workspace::setSelectedPrototype(WeakPrototypePtr proto)
{
    if (selectedPrototype.lock() != proto.lock())
    {
        selectedPrototype     = proto;
        emit sig_selected_proto_changed();
    }
}

PrototypePtr Workspace::getSelectedPrototype()
{
    return selectedPrototype.lock();
}

void Workspace::selectFeature(WeakFeaturePtr wfp)
{
    selectedFeature = wfp;
}

FeaturePtr Workspace::getSelectedFeature()
{
    return selectedFeature.lock();
}

// figures
void  Workspace::setSelectedDesignElement(WeakDesignElementPtr dep)
{
    if (selectedDesignElement.lock() != dep.lock())
    {
        selectedDesignElement = dep;
        emit sig_selected_dele_changed();
    }
}

DesignElementPtr Workspace::getSelectedDesignElement()
{
    return selectedDesignElement.lock();
}

QVector<DesignPtr> & Workspace::getDesigns()
{
    return activeDesigns;
}

WorkspaceSettings & Workspace::getMosaicSettings()
{
    static WorkspaceSettings dummy;    // default
    if (mosaic)
    {
        return mosaic->getSettings();
    }
    else
    {
        return dummy;
    }
}

void  Workspace::setProtoMode(eProtoMode mode, bool enb)
{
    if (enb)
       protoViewMode |= mode;
    else
       protoViewMode &= ~mode;
}

void Workspace::resetAll()
{
    mosaic.reset();
    resetTilings();
}

