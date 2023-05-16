#pragma once
#ifndef ROSETTE_CONNECT_H
#define ROSETTE_CONNECT_H

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

#include "motifs/rosette.h"
#include "motifs/motif_connector.h"

class RosetteConnect : public Rosette
{
public:
    RosetteConnect(int n, qreal q, int s, qreal k);
    RosetteConnect(const Motif & fig, int n, qreal q, int s, qreal k);
    RosetteConnect(const RosetteConnect & other);

    void buildUnitMap() override;

    qreal computeConnectScale();

    virtual QString getMotifDesc() override { return "RosetteConnect";}
    virtual void    report()       override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "q:" << q << "s" << s << "k" << k; }


protected:
    MotifConnector     connector;

};
#endif

