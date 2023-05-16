#include "makers/prototype_maker/prototype_data.h"
#include "makers/prototype_maker/prototype.h"
#include "makers/motif_maker/motif_maker_widget.h"
#include "mosaic/design_element.h"
#include "motifs/motif.h"
#include "settings/configuration.h"
#include "viewers/viewcontrol.h"
#include "tile/tiling.h"

/*
 *  PrototypeData
 *
 *  Prototype data is associates with the PrototypeMaker.
 *
 *  The PrototypeMaker is operated on by controls associated with the MotifView which both shows
 *  DesignElements in its view and largely operates on a specific selected DesignElement.
 *
 *  The PrototypeView shows the whole selected prototype, and various parts of which can
 *  be shown/hidden.  It uses the same selections as the MotifView and vice versa
 *
 */

PrototypeData::PrototypeData()
{
    motifMakerWidget = nullptr;
}

void PrototypeData::setWidget(MotifMakerWidget * widget)
{
    motifMakerWidget = widget;
}

void PrototypeData::erase()
{
    selectedPrototype.reset();
    selectedDesignElement.reset();
    prototypes.clear();
    designElements.clear();
}

void PrototypeData::setPrototypes(QVector<ProtoPtr> & protos)
{
    prototypes.clear();
    bool first = true;

    QVector<DesignElementPtr> dels;

    for (auto & proto : protos)
    {
        ProtoInfo info(proto,first,first);
        prototypes.push_back(info);
        if (first)
        {
            selectedPrototype = proto;
            first = false;
        }
        dels += proto->getDesignElements();
    }

    designElements.clear();
    first = true;
    for (auto & del : dels)
    {
        DelInfo info(del,first,first);
        designElements.push_back(info);
        if (first)
        {
            selectedDesignElement = del;
            first = false;
        }
    }

    if (motifMakerWidget)
    {
        motifMakerWidget->selectPrototype(getSelectedPrototype());
    }
}

void PrototypeData::select(eMVDType type, ProtoPtr proto, bool multi, bool hidden)
{
    selectedPrototype = proto;

    if (!multi)
    {
        for (auto & info : prototypes)
        {
            info.show[type] = false;
        }
    }

    // only one instance of the prototyope should be in the vector
    bool found = false;
    for (auto & info : prototypes)
    {
        auto prototype = info.wproto.lock();
        if (prototype == proto)
        {
            info.show[type] = !hidden;
            found = true;
            break;
        }
    }

    if (!found && proto)
    {
        ProtoInfo info(proto,true,true);
        prototypes.push_back(info);
    }

    if (!proto)
    {
        return;
    }

    if (motifMakerWidget)
    {
        motifMakerWidget->selectPrototype(proto);
    }

    auto del = getSelectedDEL();
    if (proto->contains(del))
    {
        if (motifMakerWidget)
        {
            motifMakerWidget->selectDEL(del);
        }
        ViewControl * view = ViewControl::getInstance();
        view->update();
    }
    else
    {
        auto del = proto->getDesignElement(0);
        if (del)
        {
            bool multi = (type == MVD_PROTO) ? true : Configuration::getInstance()->motifMultiView;
            select(type, del,multi);
        }
    }
}

void PrototypeData::select(eMVDType type, DesignElementPtr designElement, bool multi, bool hidden)
{
    selectedDesignElement = designElement;

    if (!multi)
    {
        for (auto & info : designElements)
        {
            info.show[type] = false;
        }
    }

    // only one instance of the design element should be in the vector
    bool found = false;
    for (auto & info : designElements)
    {
        auto del = info.wdel.lock();
        if (del == designElement)
        {
            info.show[type] = !hidden;
            found = true;
            break;
        }
    }

    if (!found && designElement)
    {
        DelInfo info(designElement,true,true);
        designElements.push_back(info);
    }

    if (motifMakerWidget)
    {
        motifMakerWidget->selectDEL(designElement);
    }

    ViewControl * view = ViewControl::getInstance();
    view->update();
}

QVector<DesignElementPtr> PrototypeData::getDELs()
{
    QVector<DesignElementPtr> vec;
    for (auto & info : designElements)
    {
        auto del =  info.wdel.lock();
        if (del)
        {
            vec.push_back(del);
        }
    }
    return vec;
}

QVector<DesignElementPtr> PrototypeData::getSelectedDELs(eMVDType type)
{
    QVector<DesignElementPtr> vec;
    for (auto & info : designElements)
    {
        if (info.show[type])
        {
            auto del =  info.wdel.lock();
            if (del)
            {
                vec.push_back(del);
            }
        }
    }
    return vec;
}

QVector<ProtoPtr> PrototypeData::getPrototypes()
{
    QVector<ProtoPtr> vec;
    for (auto & info : prototypes)
    {
        auto proto = info.wproto.lock();
        if (proto)
        {
            vec.push_back(proto);
        }
    }
    return vec;
}

QVector<ProtoPtr> PrototypeData::getSelectedPrototypes(eMVDType type)
{
    QVector<ProtoPtr> vec;
    for (auto & info : prototypes)
    {
        if (info.show[type])
        {
            auto proto = info.wproto.lock();
            if (proto)
            {
                vec.push_back(proto);
            }
        }
    }
    return vec;
}

void PrototypeData::rebuildCurrentMotif()
{
    // this is triggered when toggling debug of motif or unusually when regularity is changed
    auto del = selectedDesignElement.lock();
    if (del)
    {
        auto motif = del->getMotif();
        motif->resetMotifMaps();
        bool multi = Configuration::getInstance()->motifMultiView;
        select(MVD_DELEM,del,multi);
    }
}

void PrototypeData::remove(TilingPtr tiling)
{
    QVector<ProtoPtr> forRemoval;

    for (auto & prototype : getPrototypes())
    {
        if (prototype->getTiling() == tiling)
        {
            forRemoval.push_back(prototype);
        }
    }

    for (auto & prototype : forRemoval)
    {
        remove(prototype);
        if (getSelectedPrototype() == prototype)
        {
            selectedPrototype.reset();
        }
    }
}

void PrototypeData::add(ProtoPtr proto)
{
    ProtoInfo  info(proto,true,true);
    prototypes.push_front(info);
}

void PrototypeData::remove(ProtoPtr proto)
{
    QVector<ProtoInfo> cleaned;
    for (auto & info : prototypes)
    {
        auto aproto = info.wproto.lock();
        if (aproto != proto)
        {
            cleaned.push_back(info);
        }
    }
    prototypes = cleaned;
}

DesignElementPtr PrototypeData::getDesignElement(TilePtr tile)
{
    for (auto & info : designElements)
    {
        auto del  = info.wdel.lock();
        if (del)
        {
            if (del->getTile() == tile)
            {
                return del;
            }
        }
    }
    return DesignElementPtr();
}

void PrototypeData::hide(eMVDType type, DesignElementPtr del, bool hide)
{
    for (auto & info : designElements)
    {
        auto adel = info.wdel.lock();
        if (adel && adel == del)
        {
            info.show[type] = !hide;
        }
    }
}

void PrototypeData::hide(eMVDType type, ProtoPtr proto, bool hide)
{
    for (auto & info : prototypes)
    {
        auto aproto = info.wproto.lock();
        if (aproto && aproto == proto)
        {
            info.show[type] = !hide;
            return;
        }
    }
}

bool PrototypeData::isHidden(eMVDType type, ProtoPtr proto)
{
    for (auto & info : prototypes)
    {
        auto aproto = info.wproto.lock();
        if (aproto && aproto == proto)
        {
            return !info.show[type];
        }
    }
    return false;
}

bool PrototypeData::isHidden(eMVDType type, MotifPtr motif)
{
    for (auto & info : designElements)
    {
        auto del  = info.wdel.lock();
        if (del && del->getMotif() == motif)
        {
            return !info.show[type];
        }
    }
    return false;
}

bool PrototypeData::isHidden(eMVDType type, DesignElementPtr dep)
{
    for (auto & info : designElements)
    {
        auto del  = info.wdel.lock();
        if (del && del == dep)
        {
            return !info.show[type];
        }
    }
    return false;
}

ProtoPtr PrototypeData::getPrototype(TilingPtr tiling)
{
    auto protos = getPrototypes();
    for (auto & proto : protos)
    {
        if (proto->getTiling()->getName() == tiling->getName())
        {
            return proto;
        }
    }
    ProtoPtr pp;
    return pp;
}
