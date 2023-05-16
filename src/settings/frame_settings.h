#pragma once
#ifndef VIEW_SETTINGS_H
#define VIEW_SETTINGS_H

#include "geometry/bounds.h"
#include "enums/eviewtype.h"
#include <QTransform>
#include <QMap>

enum eModelAlignment
{
    M_ALIGN_NONE,
    M_ALIGN_MOSAIC,
    M_ALIGN_TILING,
};

class FrameData
{
public:
    FrameData();
    FrameData(Bounds bounds, QSize size);
    FrameData(const FrameData & other);

    FrameData & operator=(const FrameData & other);

    void            reInit();

    void            setDeltaSize(QSize size);
    void            setCropSize( QSize size) { cropSize = size; }
    void            setZoomSize( QSize size) { zoomSize = size; calculateTransform(); }

    QTransform      getTransform()      const { return transform; }
    QTransform      getDeltaTransform() const { return baseInv; }
    QSize           getCropSize()       const { return cropSize; }
    QSize           getZoomSize()       const { return zoomSize; }
    Bounds &        getBounds()               { return bounds; }

    void            calculateTransform();
    void            calculateBaseInv() { baseInv = transform.inverted(); }

protected:
    QSize           cropSize;
    QSize           zoomSize;
    QTransform      transform;
    Bounds          bounds;

    QTransform      baseInv;

private:
    class Configuration * config;

    QSize           initSize;
    Bounds          initBounds;
};


class FrameSettings
{
public:
    FrameSettings();

    void    reInit();

    void    initialise(eViewType e, QSize cropSize, QSize zoomSize);
    void    initialiseCommon(QSize cropSize, QSize  zoomSize);

    void    setDeltaSize(eViewType e, QSize sz);
    void    setCommonDeltaSizes(QSize sz);

    QTransform  getTransform(eViewType e);
    QSize       getCropSize(eViewType e);
    QSize       getZoomSize(eViewType e);

    const QMap<eViewType,FrameData*> & getFrameSettings() { return  settings; }
    FrameData * getFrameData(eViewType evt)               { return settings[evt]; }

    void    setModelAlignment(eModelAlignment mode) { modelAlignment = mode; }
    eModelAlignment getModelAlignment()              { return modelAlignment; }

protected:
    void    add(eViewType evt, Bounds bounds, QSize size);
    void    reInit(eViewType evt);

private:
    QMap<eViewType,FrameData*> settings;
    eModelAlignment modelAlignment;
};

#endif
