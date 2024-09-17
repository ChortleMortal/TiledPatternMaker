#pragma once
#ifndef MOTIF_CONNECTOR_H
#define MOTIF_CONNECTOR_H

#include <QObject>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif
#include <QTransform>

class RadialMotif;

class MotifConnector : public QObject
{
    Q_OBJECT

public:
    MotifConnector();

    qreal   build(RadialMotif * motif);
    qreal   getScale()   { return cscale; }

signals:
    void    sig_scaleChanged();

private:
    qreal   cscale;
};

typedef std::shared_ptr<MotifConnector> ConnectPtr;
typedef std::weak_ptr<MotifConnector>  wConnectPtr;

#endif
