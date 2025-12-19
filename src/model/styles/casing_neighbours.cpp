#include "casing_neighbours.h"
#include "model/styles/interlace_casing.h"
#include "model/styles/interlace.h"

CasingSide * NeighbouringCasing::getSide()
{
    return casing->side(side);
}

CasingNeighbours::CasingNeighbours(const VertexPtr& vp) : Neighbours(vp)
{}

CasingNeighbours::CasingNeighbours(const Neighbours & np) : Neighbours(np)
{}

void CasingNeighbours::findNeighbouringCasings(CasingSet * cset)
{
    casings.clear();

    for (WeakEdgePtr & wedge : *this)
    {
        EdgePtr edge = wedge.lock();
        Q_ASSERT(edge);

        NeighbouringCasingPtr ncasing = std::make_shared<NeighbouringCasing>();
        ncasing->wedge   = edge;
        ncasing->casing = cset->find(edge).get();
        ncasing->side    = edge->side(vertex.lock());
        casings.push_back(ncasing);
    }
}

BeforeAndAfterCasings CasingNeighbours::getBeforeAndAfterCasings(Casing *casing)
{
    BeforeAndAfterCasings ret;

    int sz = (int)casings.size();

    for (int i=0; i < sz; i++)
    {
        if (casings.at(i)->casing == casing)
        {
            if (i == 0)
            {
                ret.before = casings.back();
            }
            else
            {
                ret.before = casings.at(i-1);
            }

            if (i == sz-1)
            {
                ret.after = casings.front();
            }
            else
            {
                ret.after  = casings.at(i+1);
            }
            return ret;
        }
    }
    qCritical("getBeforeAndAfter - should not reach here");
    return ret;
}

// first
void CasingNeighbours::doUnders()
{
    //qDebug() << "CasingNeighbours::doUnders";

    Q_ASSERT(casings.size() == 4);

    for (const NeighbouringCasingPtr & nc : std::as_const(casings))
    {
        Casing * casing        = nc->casing;
        qreal width            = casing->getWidth();
        CasingSide * cside     = nc->getSide();
        InterlaceSide * side   = static_cast<InterlaceSide*>(cside);
        Q_ASSERT(side);
        if (side->under())
        {
            // this is first under at casing
            EdgePtr edge = nc->wedge.lock();
            if (Interlace::dbgDump2 & 0x1000) qDebug().noquote()  << "CasingNeighbours::doUnders" << "edge" << casing->edgeIndex << sSide[side->side];
            if (side->side == SIDE_1)
                side->underSide1(edge,width);
            else
                side->underSide2(edge,width);
        }
    }
}

// second
void CasingNeighbours::doOvers()
{
    Q_ASSERT(casings.size() == 4);

    for (const NeighbouringCasingPtr & nc : std::as_const(casings))
    {
        Casing * casing           = nc->casing;
        InterlaceCasing * icasing = static_cast<InterlaceCasing*>(casing);
        CasingSide * cside        = nc->getSide();
        InterlaceSide * side      = static_cast<InterlaceSide*>(cside);
        if (side->over())
        {
            NeighbouringCasingPtr nc2 = getContinuationOver(casing);
            if (nc2)
            {
                Casing * ocasing   = nc2->casing;
                CasingSide * oside = nc2->getSide();

                if (Interlace::dbgDump2 & 0x1000)
                 qDebug().noquote()  << "CasingNeighbours::doOvers" << "edge" << casing->edgeIndex  << sSide[side->side]
                                                              << "other edge" << ocasing->edgeIndex << sSide[oside->side];
                icasing->setIsectsForOvers(cside,oside);
            }
            break;      // only once
        }
    }
}

QString CasingNeighbours::infoWeave()
{
    QString astring;
    QDebug  deb(&astring);

    for (const NeighbouringCasingPtr & nc : std::as_const(casings))
    {
        Casing * casing        = nc->casing;
        CasingSide * cside     = nc->getSide();
        InterlaceSide * side   = static_cast<InterlaceSide*>(cside);
        deb.noquote() << casing->edgeIndex << sSide[cside->side];
        if (side->over())
            deb << "over  :";
        else
            deb << "under :";
    }
    return astring;
}

Casing * CasingNeighbours::getCasing(EdgePtr edge)
{
    for (NeighbouringCasingPtr  & ncasing : casings)
    {
        if (ncasing->wedge.lock() == edge)
            return ncasing->casing;
    }

    return nullptr;
}

InterlaceSide * CasingNeighbours::getContinuationOver(const EdgePtr edge)
{
    qDebug() << "edge" << edge->casingIndex << "num" << size();
    for (NeighbouringCasingPtr & neighbourcasing : casings)
    {
        EdgePtr oedge = neighbourcasing->wedge.lock();
        if (oedge == edge)
            continue;

        CasingSide * side   = neighbourcasing->getSide();
        InterlaceSide * is  = static_cast<InterlaceSide*>(side);
        Q_ASSERT(is);
        if (is->over())
        {
            //qDebug() << "continuation edge" << othercasing->edgeIndex;
            return is;
        }
    }
    qWarning() << "continuation over NOT FOUND";
    return nullptr;
}

NeighbouringCasingPtr CasingNeighbours::getContinuationOver(const Casing * casing)
{
    for (NeighbouringCasingPtr & nc : casings)
    {
        if (nc->casing == casing)
            continue;

        CasingSide * side   = nc->getSide();
        InterlaceSide * is  = static_cast<InterlaceSide*>(side);
        Q_ASSERT(is);
        if (is->over())
        {
            //qDebug() << "continuation edge" << othercasing->edgeIndex;
            return nc;
        }
    }
    qWarning() << "continuation over NOT FOUND";
    return nullptr;
}

NeighbouringCasingPtr CasingNeighbours::getContinuationUnder(const Casing * casing)
{
    for (NeighbouringCasingPtr & nc : casings)
    {
        if (nc->casing == casing)
            continue;

        CasingSide * side   = nc->getSide();
        InterlaceSide * is  = static_cast<InterlaceSide*>(side);
        Q_ASSERT(is);
        if (is->under())
        {
            //qDebug() << "continuation edge" << othercasing->edgeIndex;
            return nc;
        }
    }
    qWarning() << "continuation under NOT FOUND";
    return nullptr;
}

void CasingNeighbours::dump()
{
    QString astring;
    QDebug  deb(&astring);

    for (NeighbouringCasingPtr & nc : casings)
    {
        Casing * casing     = nc->casing;
        CasingSide * side   = nc->getSide();
        InterlaceSide * is  = static_cast<InterlaceSide*>(side);
        if (is->over())
        {
            deb << "edge" << casing->edgeIndex << "over";
        }
        else
        {
            Q_ASSERT(is->under());
            deb << "edge" << casing->edgeIndex << "under";
        }
    }
    qDebug().noquote() << astring;
}
