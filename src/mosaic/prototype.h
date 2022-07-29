#ifndef PROTOTYPE_H
#define PROTOTYPE_H

#include <QString>
#include <QTransform>

typedef std::shared_ptr<class Crop>             CropPtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Feature>          FeaturePtr;
typedef std::shared_ptr<class Figure>           FigurePtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;

////////////////////////////////////////////////////////////////////////////
//
// Prototype.java
//
// The complete information needed to build a pattern: the tiling and
// a mapping from features to figures.  Prototype knows how to turn
// this information into a finished design, returned as a Map.

// DAC: The implementation requires 'figures' (now designElements) to be an ordered list.
// Taprats uses a hash based on an algorithm which presumably provided an order.
// Generally I use QMap to replace the hashes but its
// order would be randomly based on an address in memory, so instead I have
// created an array of the feature and figure pointers

class Prototype
{
public:
    Prototype(TilingPtr t);
    Prototype(MapPtr map);
    virtual ~Prototype();

    bool    operator==(const Prototype & other);

    MapPtr  getProtoMap();
    MapPtr  getExistingProtoMap() { return protoMap; }
    void    createProtoMap();
    void    wipeoutProtoMap();

    void    setTiling(const TilingPtr & newTiling);
    void    addElement(const DesignElementPtr & element);
    void    removeElement(const DesignElementPtr &element);

    bool    hasContent() { return (designElements.size() > 0); }

    int     numDesignElements() { return designElements.size(); }
    QString getInfo() const;

    TilingPtr         getTiling()   { return tiling; }
    QList<FeaturePtr> getFeatures();
    FeaturePtr        getFeature(const FigurePtr & figure);
    FigurePtr         getFigure(const FeaturePtr & feature);
    DesignElementPtr  getDesignElement(const FeaturePtr & feature);
    DesignElementPtr  getDesignElement(int index);

    QVector<DesignElementPtr> &  getDesignElements() { return designElements; }

    void              initCrop(CropPtr crop);
    void              resetCrop(CropPtr crop);
    CropPtr           getCrop() { return _crop; }

    void    walk();
    void    dump();

    static int refs;

protected:
    void    analyze(TilingPtr newTiling);

private:
    TilingPtr                   tiling;
    CropPtr                     _crop;
    QVector<DesignElementPtr>   designElements;
    MapPtr                      protoMap;

    class ControlPanel        * panel;
    class ViewControl         * vcontrol;
};
#endif

