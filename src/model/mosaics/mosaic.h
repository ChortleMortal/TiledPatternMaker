#pragma once
#ifndef MOSAIC_H
#define MOSAIC_H

#include <QPainter>
#include <QVector>
#include "gui/top/system_view_controller.h"
#include "model/settings/canvas_settings.h"
#include "sys/sys/versioning.h"

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

    bool        isBuilt();
    void        build();
    void        enginePaint(QPainter * painter);      // called by MosaicBMPEngine

    QVector<ProtoPtr>   getPrototypes();
    MapPtr              getFirstExistingPrototypeMap();
    void                resetStyleMaps();
    void                resetProtoMaps();

    CanvasSettings &    getCanvasSettings()                   { return _canvasSettings; };
    void                setCanvasSettings(CanvasSettings& ms) { _canvasSettings = ms; }

    const StyleSet &    getStyleSet() { return styleSet; }
    StylePtr            getFirstStyle();
    StylePtr            getFirstRegularStyle();

    void        setBorder(BorderPtr bp);     // currently supports only one border per mosaic
    BorderPtr   getBorder();

    CropPtr     getCrop()       { return _crop; }
    void        setCrop(CropPtr crop);
    void        resetCrop();

    CropPtr     getPainterCrop()       { return _painterCrop; }
    void        setPainterCrop(CropPtr crop);
    void        resetPainterCrop();

    BkgdImagePtr getBkgdImage()                 { return _bip; }
    void         setBkgdImage(BkgdImagePtr bp)  { _bip = bp; }
    void         removeBkgdImage()              { _bip.reset(); }

    void         correctBorderAlignment(BorderPtr border);

    void            setName(VersionedName &vname);
    VersionedName   getName();

    void        setNotes(QString notes);
    QString     getNotes();
    QVector<TilingPtr> getTilings();

    void        dump();
    void        dumpMotifs();
    void        dumpStyles();
    void        dumpTransforms();

    void        setViewController(SystemViewController * vc);

    void        setLoadedXMLVersion(int ver) { _loadedXML_version = ver;}
    int         getLoadedXMLVersion()        { return _loadedXML_version;}

    void        setLegacyModelConverted(bool set) { _legacyCenterConverted = set; }
    bool        legacyModelConverted()            { return _legacyCenterConverted; }

    static int  refs;

protected:

private:
    StyleSet         styleSet;
    VersionedName    name;
    QString          designNotes;
    CanvasSettings   _canvasSettings;
    CropPtr          _crop;
    CropPtr          _painterCrop;
    BkgdImagePtr     _bip;
    int              _loadedXML_version;
    bool             _legacyCenterConverted;
};

typedef std::shared_ptr<Mosaic> MosaicPtr;

#endif
