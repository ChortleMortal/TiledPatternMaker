#pragma once
#ifndef LEGACY_LOADER
#define LEGACY_LOADER

#include <string>
#include "sys/enums/estyletype.h"
#include "sys/sys/pugixml.hpp"
#include "sys/geometry/xform.h"
#include <QtCore>

using std::shared_ptr;
using std::weak_ptr;
using std::make_shared;

typedef shared_ptr<class Mosaic>        MosaicPtr;
typedef shared_ptr<class Prototype>     ProtoPtr;
typedef shared_ptr<class Tile>          FeaturePtr;
typedef shared_ptr<class Tile>          FeaturePtr;
typedef shared_ptr<class Motif>         FigurePtr;
typedef shared_ptr<class PlacedTile>    PlacedTliePtr;
typedef shared_ptr<class Tiling>        TilingPtr;

typedef shared_ptr<class IrregularMotif>ExplicitPtr;
typedef shared_ptr<class Star>          StarPtr;
typedef shared_ptr<class Rosette>       RosettePtr;
typedef shared_ptr<class RosetteConnect>RoseConnectPtr;

typedef shared_ptr<QPolygonF>           PolyPtr;
typedef shared_ptr<class Vertex>        VertexPtr;
typedef shared_ptr<class Edge>          EdgePtr;
typedef shared_ptr<class Map>           MapPtr;

typedef shared_ptr<class Neighbour>     NeighbourPtr;


using std::string;
using namespace pugi;

class Workspace;

class LegacyLoader
{
public:
    LegacyLoader();
    ~LegacyLoader();

    void    processTapratsVector(xml_node & node, MosaicPtr mosaic);

    void    clearStuff();

protected:

    void processThick(xml_node & node);
    void processFilled(xml_node & node);
    void processInterlace(xml_node & node);
    void processOutline(xml_node & node);
    void processPlain(xml_node & node);
    void processSketch(xml_node & node);
    void processEmboss(xml_node & node);

    // styles
    void    procesToolkitGeoLayer(xml_node & node, Xform & xf, int &zlevel);
    QColor  processStyleColored(xml_node & node);
    void    processsStyleThick(xml_node & node, eDrawOutline &draw_outline, qreal & width);
    void    processsStyleInterlace(xml_node & node, qreal & gap, qreal & shadow);
    void    processsStyleFilled(xml_node & node, bool &draw_inside, bool &draw_outside);
    void    processsStyleEmboss(xml_node & node, qreal & angle);
    void    processStyleStyle(xml_node & node, ProtoPtr &proto);

    // features
    FeaturePtr  getFeature(xml_node & node);

    // figures
    ExplicitPtr getExplicitFigure(xml_node & node, FeaturePtr tile);
    StarPtr     getStarFigure(xml_node & node);
    RosettePtr  getRosetteFigure(xml_node & node);
    RosettePtr  getConnectFigure(xml_node & node);

    PolyPtr         getPolygon(xml_node & node);
    VertexPtr       getVertex(xml_node & node);
    NeighbourPtr    getNeighbour(xml_node & node, VertexPtr v);
    EdgePtr         getEdge(xml_node & node);
    MapPtr          getMap(xml_node & node);
    ProtoPtr        getPrototype(xml_node & node);
    PolyPtr         getBoundary(xml_node & node);

    // references
    bool   hasReference(xml_node & node);

    void   setProtoReference(xml_node & node, ProtoPtr ptr);
    void   setVertexReference(xml_node & node, VertexPtr ptr);
    void   setEdgeReference(xml_node & node, EdgePtr ptr);
    void   setNeighbourReference(xml_node & node, NeighbourPtr ptr);
    void   setPolyReference(xml_node & node, PolyPtr ptr);
    void   setFeatureReference(xml_node & node, FeaturePtr ptr);
    void   setFigureReference(xml_node & node, FigurePtr ptr);
    void   setExplicitReference(xml_node & node, ExplicitPtr ptr);
    void   setStarReference(xml_node & node, StarPtr ptr);
    void   setRosetteReference(xml_node & node, RosettePtr ptr);
    void   setMapReference(xml_node & node, MapPtr ptr);

    ProtoPtr        getProtoReferencedPtr(xml_node & node);
    VertexPtr       getVertexReferencedPtr(xml_node & node);
    EdgePtr         getEdgeReferencedPtr(xml_node & node);
    NeighbourPtr    getNeighbourReferencedPtr(xml_node & node);
    PolyPtr         getPolyReferencedPtr(xml_node & node);
    FeaturePtr      getFeatureReferencedPtr(xml_node & node);
    FigurePtr       getFigureReferencedPtr(xml_node & node);
    ExplicitPtr     getExplicitReferencedPtr(xml_node & node);
    StarPtr         getStarReferencedPtr(xml_node & node);
    RosettePtr      getRosetteReferencedPtr(xml_node & node);
    MapPtr          getMapReferencedPtr(xml_node & node);

    TilingPtr      findTiling(QString name);

private:
    MosaicPtr               _mosaic;
    QVector<TilingPtr>      _tilings;

    QMap<int,ProtoPtr>      proto_ids;
    QMap<int,VertexPtr>     vertex_ids;
    QMap<int,EdgePtr>       edge_ids;
    QMap<int,NeighbourPtr>  neighbour_ids;
    QMap<int,PolyPtr>       poly_ids;
    QMap<int,FeaturePtr>    feature_ids;
    QMap<int,FigurePtr>     figure_ids;
    QMap<int,ExplicitPtr>   explicit_ids;
    QMap<int,StarPtr>       star_ids;
    QMap<int,RosettePtr>    rosette_ids;
    QMap<int,MapPtr>        map_ids;

    Workspace             * workspace;

    QString                 _fileName;
    bool                    _loaded;
    bool                    _debug;

    int vOrigCnt;
    int vRefrCnt;
    int eOrigCnt;
    int eRefrCnt;
    int nOrigCnt;
    int nRefrCnt;
};

// dummy class
class Neighbour
{
public:
    Neighbour() {}
};

#endif
