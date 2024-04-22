#pragma once
#ifndef STAR_CONNECT_H
#define STAR_CONNECT_H

////////////////////////////////////////////////////////////////////////////
//
// ConnectFigure.java
//
// A ConnectFigure is a special kind of ScaleFigure.  It knows how to
// compute just the right scale factor so that scaled out edges will join
// up to create a fancier figure.  This is how we turn Rosettes into
// Extended Rosettes.  To make sure that the resulting figure still lines
// up with the tile that will eventually contain it, we need to do
// some fancy reshuffling of the basic unit to move the apex to (1,0).

#include "motifs/star.h"
#include "motifs/motif_connector.h"

class StarConnect : public Star
{
public:
    StarConnect(int n, qreal d, int s);
    StarConnect(const Motif & motif, int n, qreal d, int s);
    StarConnect(const StarConnect & other);

    void buildUnitMap() override;

    virtual QString getMotifDesc() override { return "StarConnect";}
    virtual void    dump()       override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "d:" << d << "s" << s; }

    qreal computeConnectScale();

protected:
    MotifConnector     connector;
};

#endif

