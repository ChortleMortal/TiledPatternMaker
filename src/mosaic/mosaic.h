#ifndef MOSAIC_H
#define MOSAIC_H

#include <QVector>
#include "settings/model_settings.h"

typedef std::shared_ptr<class Style>        StylePtr;
typedef std::shared_ptr<class Tiling>       TilingPtr;
typedef std::shared_ptr<class Border>       BorderPtr;
typedef std::shared_ptr<class Crop>         CropPtr;
typedef std::shared_ptr<class Prototype>    PrototypePtr;
typedef std::shared_ptr<class Map>          MapPtr;

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

    QVector<PrototypePtr> getPrototypes();
    MapPtr                getPrototypeMap();
    void                  resetStyleMaps();
    void                  resetProtoMaps();

    ModelSettings & getSettings() { return settings; };

    const StyleSet & getStyleSet() { return styleSet; }
    StylePtr         getFirstStyle();

    // mosaic has a crop or a border (but not both)
    BorderPtr   getBorder() { return border; }
    void        setBorder(BorderPtr bp);

    CropPtr     getCrop() { return crop; }
    void        initCrop(CropPtr crop);     // call this when creating mosaic
    void        resetCrop(CropPtr crop);    // call this when mosaic is already loaded

    void        setName(QString name);
    QString     getName();

    void        setNotes(QString notes);
    QString     getNotes();

    QVector<TilingPtr> getTilings();

    void        dump();

    static const QString defaultName;

protected:
    void        _eraseCrop();

private:
    StyleSet         styleSet;
    QString          name;
    QString          designNotes;
    ModelSettings    settings;
    BorderPtr        border;
    CropPtr          crop;
};

#endif
