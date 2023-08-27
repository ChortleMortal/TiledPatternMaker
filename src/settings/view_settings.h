#pragma once
#ifndef VIEW_SETTINGS_H
#define VIEW_SETTINGS_H

#include "geometry/bounds.h"
#include "enums/eviewtype.h"
#include <QTransform>
#include <QMap>
#include <QColor>

enum eModelAlignment
{
    M_ALIGN_NONE,
    M_ALIGN_MOSAIC,
    M_ALIGN_TILING,
};

class ViewData
{
    friend class ViewSettings;

public:
    ViewData();
    ViewData(Bounds bounds, QSize size, QColor color);
    ViewData(const ViewData & other);

    ViewData & operator=(const ViewData & other);

    QSize           getCropSize()       const { return cropSize; }
    QSize           getZoomSize()       const { return zoomSize; }
    QColor          getBkgdColor()      const { return bkgdColor; }

protected:
    void            reInit();

    void            setDeltaSize(QSize size);
    void            setCropSize( QSize size)  { cropSize  = size; }
    void            setZoomSize( QSize size)  { zoomSize  = size; calculateTransform(); }
    void            setBkgdColor(QColor c)    { bkgdColor = c; }
    void            setBounds(Bounds & b)     { bounds    = b; calculateTransform(); }

    QTransform      getTransform()      const { return transform; }
    QTransform      getDeltaTransform() const { return baseInv; }
    Bounds          getBounds()               { return bounds; }

    void            calculateTransform();
    void            calculateBaseInv()        { baseInv = transform.inverted(); }

    QSize           cropSize;
    QSize           zoomSize;
    QTransform      transform;
    Bounds          bounds;
    QColor          bkgdColor;

    QTransform      baseInv;

private:
    QSize           initSize;
    Bounds          initBounds;
};


class ViewSettings
{
public:
    ViewSettings();

    void        reInit();
    void        reInitBkgdColors(QColor bcolor);

    void        initialise(eViewType e, QSize cropSize, QSize zoomSize);
    void        initialiseCommon(QSize cropSize, QSize  zoomSize);

    QTransform  getTransform(eViewType e)       { return settings[e]->getTransform(); }
    QTransform  getDeltaTransform(eViewType e)  { return settings[e]->getDeltaTransform(); }
    QSize       getCropSize(eViewType e)        { return settings[e]->getCropSize(); }
    QSize       getZoomSize(eViewType e)        { return settings[e]->getZoomSize(); }
    QColor      getBkgdColor(eViewType e)       { return settings[e]->getBkgdColor();}
    Bounds      getBounds(eViewType e)          { return settings[e]->getBounds(); }

    void        setModelAlignment(eModelAlignment mode) { _modelAlignment = mode; }
    void        setCommonDeltaSizes(QSize sz);

    void        setDeltaSize(eViewType e, QSize sz)     { settings[e]->setDeltaSize(sz); }
    void        setCropSize(eViewType e,  QSize size)   { settings[e]->setCropSize(size); }
    void        setBounds(eViewType e, Bounds & bounds) { settings[e]->setBounds(bounds);}
    void        setBkgdColor(eViewType e, QColor color);

    const QMap<eViewType,ViewData*> & getSettingsMap()  { return settings; }
    eModelAlignment getModelAlignment()                 { return _modelAlignment; }

protected:
    void    add(eViewType evt, Bounds bounds, QSize size, QColor color);
    void    reInit(eViewType evt);

private:
    eModelAlignment            _modelAlignment;
    QMap<eViewType,ViewData*>  settings;
};

#endif
