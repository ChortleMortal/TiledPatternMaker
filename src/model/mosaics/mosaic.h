#pragma once

#include <QPainter>
#include <QVector>
#include "gui/top/view_controller.h"
#include "model/settings/canvas.h"
#include "model/tilings/backgroundimage.h"
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

    void        resetStyleMaps();
    void        resetProtoMaps();
    void        build();
    bool        isBuilt();

    bool        hasContent() { return (styleSet.size() > 0); }
    bool        hasDeprecatedFill();
    int         numStyles()  { return styleSet.size(); }

    void        enginePaint(QPainter * painter);      // called by MosaicBMPEngine

    // loaded styles
    void        addStyle(StylePtr style);   // adds at front
    void        addStyleOnTop(StylePtr style);  // adds at back (painted last)
    void        replaceStyle(StylePtr oldStyle, StylePtr newStyle);
    int         moveUp(StylePtr style);
    int         moveDown(StylePtr style);
    void        deleteStyle(StylePtr style);

    QVector<ProtoPtr>   getPrototypes();
    ProtoPtr            getFirstExistingPrototype();
    MapPtr              getFirstExistingPrototypeMap();

    Canvas *            getMosaicCanvas()                 { return &_canvas; };

    const StyleSet &    getStyleSet() { return styleSet; }
    StylePtr            getFirstStyle();
    StylePtr            getFirstRegularStyle();

    void        replaceBorder(BorderPtr oldBorder, BorderPtr newBorder);  // adds if old not found
    BorderPtr   getBorder();                 // first border

    CropPtr     getCrop()       { return _crop; }
    void        setCrop(CropPtr crop);
    void        resetCrop();

    CropPtr     getPainterCrop()       { return _painterCrop; }
    void        setPainterCrop(CropPtr crop);
    void        resetPainterCrop();

    void               setBkgdColor(QColor color)          { _bkgdColor = color; }
    QColor             getBkgdColor()                      { return _bkgdColor; }

    const BkgdRecord & getBkgdRecord()                     { return _bkgdRecord; }
    void               setBkgdRecord(const BkgdRecord & r) { _bkgdRecord = r; }
    void               removeBkgdRecord()                  { _bkgdRecord.clear(); }

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

    void        setViewController(ViewController * vc);

    void        setLoadedXMLVersion(int ver) { _loadedXML_version = ver;}
    int         getLoadedXMLVersion()        { return _loadedXML_version;}

    static int  refs;

protected:

private:
    StyleSet         styleSet;
    VersionedName    name;
    QString          designNotes;
    Canvas           _canvas;
    CropPtr          _crop;
    CropPtr          _painterCrop;
    BkgdRecord       _bkgdRecord;
    QColor           _bkgdColor;
    int              _loadedXML_version;
    bool             _legacyCenterConverted;
};

typedef std::shared_ptr<Mosaic> MosaicPtr;
