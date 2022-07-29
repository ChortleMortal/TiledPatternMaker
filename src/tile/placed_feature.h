////////////////////////////////////////////////////////////////////////////
//
// PlacedFeature.java
//
// A PlacedFeature is a Feature together with a transform matrix.
// It allows us to share an underlying feature while putting several
// copies together into a tiling.  A tiling is represented as a
// collection of PlacedFeatures (that may share Features) that together
// make up a translational unit.

#ifndef PLACED
#define PLACED

#include <QString>
#include <QTransform>
#include <QTextStream>
#include "misc/pugixml.hpp"
#include "geometry/edgepoly.h"

typedef std::shared_ptr<class PlacedFeature>   PlacedFeaturePtr;
typedef std::shared_ptr<class Feature>         FeaturePtr;

class PlacedFeature
{
public:
    // Creation.
    PlacedFeature();
    PlacedFeature(FeaturePtr feature, QTransform T );
    PlacedFeature(PlacedFeaturePtr other);
    ~PlacedFeature() {}

    // Data.
    void             setTransform(QTransform newT);
    void             setFeature(FeaturePtr feature);
    FeaturePtr       getFeature();
    QTransform       getTransform();
    EdgePoly         getFeatureEdgePoly();
    QPolygonF        getFeaturePolygon();
    EdgePoly         getPlacedEdgePoly();
    QPolygonF        getPlacedPolygon();

    bool saveAsGirihShape(QString name);
    bool loadFromGirihShape(QString name);

    bool isGirihShape() { return !girihShapeName.isEmpty(); }
    QString getGirihShapeName() { return girihShapeName; }

    bool show() { return _show; }
    void setShow(bool show) { _show = show; }

protected:
    void saveGirihShape(QTextStream & out, QString name);
    void loadGirihShape(pugi::xml_node & poly_node);
    void loadGirihShape(int sides, pugi::xml_node & poly_node);
    void loadGirihShapeOld(pugi::xml_node & poly_node);

private:
    QString      girihShapeName;
    FeaturePtr   feature;
    QTransform   T;
    bool         _show;  // used by tiling maker view
};
#endif
