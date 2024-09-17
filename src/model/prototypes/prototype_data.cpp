#include "model/prototypes/prototype_data.h"
#include "model/prototypes/prototype.h"
#include "gui/model_editors/motif_edit/motif_maker_widget.h"
#include "sys/sys.h"
#include "model/prototypes/design_element.h"
#include "model/motifs/motif.h"
#include "model/settings/configuration.h"
#include "gui/top/view_controller.h"
#include "model/tilings/tiling.h"

/*
 *  ProtoMakerData
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

ProtoMakerData::ProtoMakerData()
{
    _motifMakerWidget = nullptr;
}

void ProtoMakerData::erase()
{
    selectedPrototype.reset();
    selectedDesignElement.reset();
    prototypes.clear();
    designElements.clear();
}

void ProtoMakerData::setPrototypes(QVector<ProtoPtr> & protos)
{
    prototypes.clear();
    bool first = true;

    QVector<DesignElementPtr> dels;

    for (const auto & proto : std::as_const(protos))
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
    for (const auto & del : std::as_const(dels))
    {
        DelInfo info(del,first,first);
        designElements.push_back(info);
        if (first)
        {
            selectedDesignElement = del;
            first = false;
        }
    }
}

void ProtoMakerData::select(eMVDType type, ProtoPtr proto, bool multi, bool hidden)
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

    auto del = getSelectedDEL();
    if (proto->containsDesignElement(del))
    {
        emit _motifMakerWidget->sig_updateView();
    }
    else
    {
        auto del = proto->getDesignElement(0);
        if (del)
        {
            bool multi = (type == MVD_PROTO) ? true : Sys::config->motifMultiView;
            select(type, del,multi);
        }
    }
}

void ProtoMakerData::select(eMVDType type, DesignElementPtr designElement, bool multi, bool hidden)
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
}

void ProtoMakerData::deselect(eMVDType type, DesignElementPtr designElement, bool multi, bool hidden)
{
    Q_ASSERT(multi);

    selectedDesignElement.reset();

    for (auto & info : designElements)
    {
        auto del = info.wdel.lock();
        if (del == designElement)
        {
            info.show[type] = hidden;
            break;
        }
    }
}


QVector<DesignElementPtr> ProtoMakerData::getDELs()
{
    QVector<DesignElementPtr> vec;
    for (const auto & info : std::as_const(designElements))
    {
        auto del =  info.wdel.lock();
        if (del)
        {
            vec.push_back(del);
        }
    }
    return vec;
}

QVector<DesignElementPtr> ProtoMakerData::getSelectedDELs(eMVDType type)
{
    QVector<DesignElementPtr> vec;
    for (const auto & info : std::as_const(designElements))
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

const QVector<ProtoPtr> ProtoMakerData::getPrototypes()
{
    QVector<ProtoPtr> vec;
    for (const auto & info : std::as_const(prototypes))
    {
        auto proto = info.wproto.lock();
        if (proto)
        {
            vec.push_back(proto);
        }
    }
    return vec;
}

const QVector<ProtoPtr> ProtoMakerData::getSelectedPrototypes(eMVDType type)
{
    QVector<ProtoPtr> vec;
    for (const auto & info : std::as_const(prototypes))
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

void ProtoMakerData::rebuildCurrentMotif()
{
    // this is triggered when toggling debug of motif or unusually when regularity is changed
    auto del = selectedDesignElement.lock();
    if (del)
    {
        auto motif = del->getMotif();
        motif->resetMotifMap();
        motif->buildMotifMap();
        bool multi = Sys::config->motifMultiView;
        select(MVD_DELEM,del,multi);
    }
}

void ProtoMakerData::remove(TilingPtr tiling)
{
    QVector<ProtoPtr> forRemoval;

    for (const auto & prototype : getPrototypes())
    {
        if (prototype->getTiling() == tiling)
        {
            forRemoval.push_back(prototype);
        }
    }

    for (const auto & prototype : forRemoval)
    {
        remove(prototype);
        if (getSelectedPrototype() == prototype)
        {
            selectedPrototype.reset();
        }
    }
}

void ProtoMakerData::add(ProtoPtr proto)
{
    ProtoInfo  info(proto,true,true);
    prototypes.push_front(info);
}

void ProtoMakerData::remove(ProtoPtr proto)
{
    QVector<ProtoInfo> cleaned;
    for (const auto & info : std::as_const(prototypes))
    {
        auto aproto = info.wproto.lock();
        if (aproto != proto)
        {
            cleaned.push_back(info);
        }
    }
    prototypes = cleaned;
}

DesignElementPtr ProtoMakerData::getDesignElement(TilePtr tile)
{
    for (const auto & info : std::as_const(designElements))
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

void ProtoMakerData::hide(eMVDType type, DesignElementPtr del, bool hide)
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

void ProtoMakerData::hide(eMVDType type, ProtoPtr proto, bool hide)
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

bool ProtoMakerData::isHidden(eMVDType type, ProtoPtr proto)
{
    for (const auto & info : prototypes)
    {
        auto aproto = info.wproto.lock();
        if (aproto && aproto == proto)
        {
            return !info.show[type];
        }
    }
    return false;
}

bool ProtoMakerData::isHidden(eMVDType type, MotifPtr motif)
{
    for (const auto & info : designElements)
    {
        auto del  = info.wdel.lock();
        if (del && del->getMotif() == motif)
        {
            return !info.show[type];
        }
    }
    return false;
}

bool ProtoMakerData::isHidden(eMVDType type, DesignElementPtr dep)
{
    for (const auto & info : designElements)
    {
        auto del  = info.wdel.lock();
        if (del && del == dep)
        {
            return !info.show[type];
        }
    }
    return false;
}

ProtoPtr ProtoMakerData::getPrototype(TilingPtr tiling)
{
    const auto & protos = getPrototypes();
    for (auto & proto : std::as_const(protos))
    {
        if (proto->getTiling()->getName().get() == tiling->getName().get())
        {
            return proto;
        }
    }
    ProtoPtr pp;
    return pp;
}

void ProtoMakerData::dumpData(eMVDType type)
{
    qDebug() << "selected prototype" << selectedPrototype.lock().get();
    for (const auto & p : prototypes)
    {
        auto pr = p.wproto.lock();
        if (pr)
            qDebug() << "proto" << pr.get() << p.show[type];
    }

    qDebug() << "selected designElement" << selectedDesignElement.lock().get();
    for (const auto & d : designElements)
    {
        auto del = d.wdel.lock();
        if (del)
        {
            auto motif = del->getMotif();
            if (motif)
                qDebug() << "DEL" << del.get() << d.show[type] << del->getMotif()->getMotifDesc();
            else
                qDebug() << "DEL" << del.get() << d.show[type] << "No MOTIF";
        }
    }
}
