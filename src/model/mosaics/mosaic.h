#pragma once
#ifndef MOSAIC_H
#define MOSAIC_H

#include <QPainter>
#include <QVector>
#include "sys/sys/versioning.h"
#include "model/settings/canvas_settings.h"

typedef std::shared_ptr<class Style>        StylePtr;
typedef std::shared_ptr<class Tiling>       TilingPtr;
typedef std::shared_ptr<class Border>       BorderPtr;
typedef std::shared_ptr<class Crop>         CropPtr;
typedef std::shared_ptr<class Prototype>    ProtoPtr;
typedef std::shared_ptr<class Map>          MapPtr;
typedef std::shared_ptr<class BackgroundImage>  BkgdImagePtr;

typedef QVector<StylePtr>  StyleSet;

class Mosaic
{
public:
    Mosaic();
    ~Mosaic();

    bool        hasContent() { return (styleSet.size() > 0); }
    int         numStyles()  { return styleSet.size(); }

    // loaded styles
    void        addStyle(StylePtr style);   // adds at front
    void        replaceStyle(StylePtr oldStyle, StylePtr newStyle);
    int         moveUp(StylePtr style);
    int         moveDown(StylePtr style);
    void        deleteStyle(StylePtr style);

    void        build();
    void        build(class ViewController * vc);
    void        enginePaint(QPainter * painter);      // called by MosaicBMPEngine

    QVector<ProtoPtr> getPrototypes();
    MapPtr            getFirstExistingPrototypeMap();
    void              resetStyleMaps();
    void              resetProtoMaps();
    
    CanvasSettings &  getCanvasSettings()                   { return _canvasSettings; };
    void              setCanvasSettings(CanvasSettings& ms) { _canvasSettings = ms; }

    const StyleSet & getStyleSet() { return styleSet; }
    StylePtr         getFirstStyle();

    // mosaic can have a crop and/or a border
    BorderPtr   getBorder()     { return _border; }
    void        setBorder(BorderPtr bp);
    void        resetBorder()   { _border.reset(); }

    CropPtr     getCrop()       { return _crop; }
    void        setCrop(CropPtr crop);
    void        resetCrop();

    CropPtr     getPainterCrop()       { return _painterCrop; }
    void        setPainterCrop(CropPtr crop);
    void        resetPainterCrop();

    BkgdImagePtr getBkgdImage()                 { return _bip; }
    void         setBkgdImage(BkgdImagePtr bp)  { _bip = bp; }
    void         removeBkgdImage()              { _bip.reset(); }

    void            setName(VersionedName &vname);
    VersionedName   getName();

    void        setNotes(QString notes);
    QString     getNotes();

    QVector<TilingPtr> getTilings();

    void        setCleanseLevel(uint level)     { _cleanseLevel = level; setupCleansing();}
    uint        getCleanseLevel()               { return _cleanseLevel; }
    void        setCleanseSensitivity(qreal val){ _cleanseSensitivity = val; setupCleansing(); }
    qreal       getCleanseSensitivity()         { return _cleanseSensitivity; }

    void        dump();
    void        dumpMotifs();
    void        dumpStyles();

    void        setViewController(ViewController * vc);

    static int  refs;

protected:
    void        setupCleansing();

private:
    StyleSet         styleSet;
    VersionedName    name;
    QString          designNotes;
    CanvasSettings   _canvasSettings;
    BorderPtr        _border;
    CropPtr          _crop;
    CropPtr          _painterCrop;
    BkgdImagePtr     _bip;
    uint             _cleanseLevel;
    qreal            _cleanseSensitivity;

};

typedef std::shared_ptr<Mosaic> MosaicPtr;

#endif
