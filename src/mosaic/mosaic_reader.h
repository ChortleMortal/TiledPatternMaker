#ifndef XMLLOADER_H
#define XMLLOADER_H

#include <string>
#include "mosaic_reader_base.h"
#include "misc/pugixml.hpp"
#include "misc/colorset.h"
#include "settings/filldata.h"
#include "geometry/circle.h"
#include "geometry/xform.h"
#include "enums/efigtype.h"
#include "enums/estyletype.h"

typedef std::shared_ptr<QPolygonF>              PolyPtr;
typedef std::shared_ptr<class BackgroundImage>  BkgdImgPtr;
typedef std::shared_ptr<class Border>           BorderPtr;
typedef std::shared_ptr<class Crop>             CropPtr;
typedef std::shared_ptr<class Edge>             EdgePtr;
typedef std::shared_ptr<class ExplicitFigure>   ExplicitPtr;
typedef std::shared_ptr<class ExtendedRosette>  ExtRosettePtr;
typedef std::shared_ptr<class ExtendedStar>     ExtStarPtr;
typedef std::shared_ptr<class Feature>          FeaturePtr;
typedef std::shared_ptr<class Figure>           FigurePtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Prototype>        PrototypePtr;
typedef std::shared_ptr<class Rosette>          RosettePtr;
typedef std::shared_ptr<class Star>             StarPtr;
typedef std::shared_ptr<class RosetteConnectFigure> RosetteConnectPtr;
typedef std::shared_ptr<class StarConnectFigure>    StarConnectPtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Vertex>           VertexPtr;

class FeatureReader;

using std::string;
using namespace pugi;

class MosaicReader : public MosaicReaderBase
{
public:
    MosaicReader();
    ~MosaicReader();

    MosaicPtr readXML(QString fileName);

    QString   getLoadedFilename();
    QString   getFailMessage() { return _failMessage; }

protected:
    void parseXML(xml_document & doc);
    void processVector(xml_node & node);
    void processDesignNotes(xml_node & node);

    void processDesign(xml_node & node);
    void processThick(xml_node & node);
    void processFilled(xml_node & node);
    void processInterlace(xml_node & node);
    void processOutline(xml_node & node);
    void processPlain(xml_node & node);
    void processSketch(xml_node & node);
    void processEmboss(xml_node & node);
    void processTileColors(xml_node & node);

    // design
    void    procSize(xml_node & node, int &width, int &height, int &zwidth, int &zheight);
    QColor  procBackgroundColor(xml_node & node);
    void    procBorder(xml_node & node);
    void    procFill(xml_node & node, bool isSingle);
    qreal   procWidth(xml_node & node);

    // styles
    void    procesToolkitGeoLayer(xml_node & node, Xform & xf, int &zlevel);
    void    processColorSet(xml_node & node, ColorSet & colorSet);
    void    processColorGroup(xml_node & node, ColorGroup & colorGroup);
    void    processsStyleThick(xml_node & node, eDrawOutline & draw_outline, qreal & width, qreal &outlineWidth, QColor &outlineColor, Qt::PenJoinStyle & pjs, Qt::PenCapStyle & pcs);
    void    processsStyleInterlace(xml_node & node, qreal & gap, qreal & shadow, bool & includeSVerts, bool &start_under);
    void    processsStyleFilled(xml_node & node, bool &draw_inside, bool &draw_outside, int &algorithm);
    void    processsStyleEmboss(xml_node & node, qreal & angle);
    void    processStyleStyle(xml_node & node, PrototypePtr &proto);

    // features
    FeaturePtr  getFeature(FeatureReader & fr, xml_node & node);

    // figures
    void                getFigureCommon(xml_node & node, FigurePtr fig);
    ExplicitPtr         getExplicitFigure(xml_node & node, eFigType figType);
    StarPtr             getStarFigure(xml_node & node);
    ExtStarPtr          getExtendedStarFigure(xml_node & node);
    ExtRosettePtr       getExtendedRosetteFigure(xml_node & node);
    RosettePtr          getRosetteFigure(xml_node & node);
    FigurePtr           getConnectFigure(xml_node & node);
    RosetteConnectPtr   getRosetteConnectFigure(xml_node & node);
    StarConnectPtr      getStarConnectFigure(xml_node & node);

    PolyPtr         getPolygon(xml_node & node);
    VertexPtr       getVertex(xml_node & node);
    EdgePtr         getEdge(xml_node & node);
    EdgePtr         getCurve(xml_node & node);
    EdgePtr         getChord(xml_node & node);
    MapPtr          getMap(xml_node & node);
    PrototypePtr    getPrototype(xml_node & node);
    PolyPtr         getBoundary(xml_node & node);
    QPointF         getPos(xml_node & node);
    QRectF          getRectangle(xml_node node);
    CirclePtr       getCircle(xml_node node);

    // references
    void   setProtoReference(xml_node & node, PrototypePtr ptr);
    void   setEdgeReference(xml_node & node, EdgePtr ptr);
    void   setPolyReference(xml_node & node, PolyPtr ptr);
    void   setFeatureReference(xml_node & node, FeaturePtr ptr);
    void   setFigureReference(xml_node & node, FigurePtr ptr);
    void   setExplicitReference(xml_node & node, ExplicitPtr ptr);
    void   setStarReference(xml_node & node, StarPtr ptr);
    void   setExtStarReference(xml_node & node, ExtStarPtr ptr);
    void   setExtRosetteReference(xml_node & node, ExtRosettePtr ptr);
    void   setRosetteReference(xml_node & node, RosettePtr ptr);
    void   setRosetteConnectReference(xml_node & node, RosetteConnectPtr ptr);
    void   setStarConnectReference(xml_node & node, StarConnectPtr ptr);
    void   setMapReference(xml_node & node, MapPtr ptr);

    PrototypePtr    getProtoReferencedPtr(xml_node & node);
    EdgePtr         getEdgeReferencedPtr(xml_node & node);
    PolyPtr         getPolyReferencedPtr(xml_node & node);
    FeaturePtr      getFeatureReferencedPtr(xml_node & node);
    FigurePtr       getFigureReferencedPtr(xml_node & node);
    ExplicitPtr     getExplicitReferencedPtr(xml_node & node);
    StarPtr         getStarReferencedPtr(xml_node & node);
    ExtStarPtr      getExtStarReferencedPtr(xml_node & node);
    ExtRosettePtr   getExtRosetteReferencedPtr(xml_node & node);
    RosettePtr      getRosetteReferencedPtr(xml_node & node);
    RosetteConnectPtr getRosetteConnectReferencedPtr(xml_node & node);
    StarConnectPtr  getStarConnectReferencedPtr(xml_node & node);
    MapPtr          getMapReferencedPtr(xml_node & node);

    TilingPtr       findTiling(QString name);
    TilingPtr       getFirstTiling() { return _tilings.first(); }

protected:
    QColor  processColor(xml_node & n);
    void    procBorderPlain(xml_node & n);
    void    procBorderTwoColor(xml_node & n);
    void    procBorderBlocks(xml_node & n);
    void    procCrop(xml_node & n);

    [[noreturn]] void fail(QString a, QString b);
    QTransform getQTransform(QString txt);

    QMap<int,PrototypePtr>  proto_ids;
    QMap<int,EdgePtr>       edge_ids;
    QMap<int,PolyPtr>       poly_ids;
    QMap<int,FeaturePtr>    feature_ids;
    QMap<int,FigurePtr>     figure_ids;
    QMap<int,ExplicitPtr>   explicit_ids;
    QMap<int,StarPtr>       star_ids;
    QMap<int,ExtStarPtr>    ext_star_ids;
    QMap<int,ExtRosettePtr> ext_rosette_ids;
    QMap<int,RosettePtr>    rosette_ids;
    QMap<int,RosetteConnectPtr> rosette_connect_ids;
    QMap<int,StarConnectPtr>star_connect_ids;
    QMap<int,MapPtr>        map_ids;

    MapPtr                  _currentMap;
    MosaicPtr               _mosaic;

    QVector<TilingPtr>      _tilings;
    QString                 _fileName;
    QString                 _failMessage;

    QColor                  _background;
    int                     _width;
    int                     _height;
    int                     _zwidth;
    int                     _zheight;
    BorderPtr               _border;
    CropPtr                 _crop;
    unsigned int            _version;
    FillData                _fillData;

    bool _debug;

    class ViewControl * view;
};

#endif
