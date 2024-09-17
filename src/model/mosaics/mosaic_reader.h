#pragma once
#ifndef XMLLOADER_H
#define XMLLOADER_H

#include <string>
#include "model/mosaics/mosaic_reader_base.h"
#include "sys/enums/efilltype.h"
#include "sys/sys/pugixml.hpp"
#include "model/styles/colorset.h"
#include "model/settings/filldata.h"
#include "sys/geometry/circle.h"
#include "sys/geometry/polygon.h"
#include "sys/geometry/xform.h"
#include "sys/enums/emotiftype.h"
#include "sys/enums/estyletype.h"
#include "model/styles/filled.h"
#include "model/motifs/extender.h"
#include "model/motifs/rosette2.h"

using std::shared_ptr;
using std::weak_ptr;

typedef shared_ptr<QPolygonF>              PolyPtr;
typedef shared_ptr<class Border>           BorderPtr;
typedef shared_ptr<class BackgroundImage>  BkgdImagePtr;
typedef shared_ptr<class Crop>             CropPtr;
typedef shared_ptr<class Edge>             EdgePtr;
typedef shared_ptr<class Filled>           FillPtr;
typedef shared_ptr<class Tile>             TilePtr;
typedef shared_ptr<class Motif>            MotifPtr;
typedef shared_ptr<class IrregularMotif>   IrregularPtr;
typedef shared_ptr<class Map>              MapPtr;
typedef shared_ptr<class Mosaic>           MosaicPtr;
typedef shared_ptr<class Prototype>        ProtoPtr;
typedef shared_ptr<class Rosette>          RosettePtr;
typedef shared_ptr<class Rosette2>         Rosette2Ptr;
typedef shared_ptr<class Star>             StarPtr;
typedef shared_ptr<class Star2>            Star2Ptr;
typedef shared_ptr<class Tiling>           TilingPtr;
typedef shared_ptr<class Vertex>           VertexPtr;
typedef shared_ptr<MosaicReaderBase>       MRBasePtr;

class TileReader;
class IrregularMotif;

using std::string;
using namespace pugi;

class MosaicReader
{
public:
    MosaicReader();
    ~MosaicReader();

    MosaicPtr readXML(VersionedFile xfile);

    QString   getFailMessage() { return _failMessage; }

protected:
    void parseXML(xml_document & doc);
    void processVector(xml_node & node);
    void processDesignNotes(xml_node & node);

    void correctMotifScaleandRotation();

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
    QSize   procViewSize(xml_node & node);
    QSize   procCanvasSize(xml_node & node, QSize &defaultSize);
    QColor  procBackgroundColor(xml_node & node);
    void    procBorder(xml_node & node);
    void    procFill(xml_node & node, bool isSingle);
    qreal   procWidth(xml_node & node);
    qreal   procLength(xml_node & node);

    // styles
    void    procesToolkitGeoLayer(xml_node & node, Xform & xf, int &zlevel);
    void    processColorSet(xml_node & node, ColorSet & colorSet);
    void    processColorGroup(xml_node & node, ColorGroup & colorGroup);
    void    processsStyleThick(xml_node & node, eDrawOutline & draw_outline, qreal & width, qreal &outlineWidth, QColor &outlineColor, Qt::PenJoinStyle & pjs, Qt::PenCapStyle & pcs);
    void    processsStyleInterlace(xml_node & node, qreal & gap, qreal & shadow, bool & includeSVerts, bool &start_under);
    void    processsStyleFilled(xml_node & node, bool &draw_inside, bool &draw_outside, eFillType &algorithm);
    void    processsStyleFilledFaces(xml_node & node, QVector<int> &paletteIndices);
    void    processsStyleEmboss(xml_node & node, qreal & angle);
    void    processStylePrototype(xml_node & node, ProtoPtr &proto);

    // tiles
    TilePtr  getTile(TileReader & fr, xml_node & node);

    // motifs
    void                getMotifCommon(xml_node & node, MotifPtr motif,int tile_sides);
    IrregularPtr        getIrregularMotif(xml_node & node, int tile_sides, eMotifType type);
    StarPtr             getStar(xml_node & node, int tile_sides);
    Star2Ptr            getStar2(xml_node & node, int tile_sides);
    RosettePtr          getRosette(xml_node & node, int tile_sides);
    Rosette2Ptr         getRosette2(xml_node & node, int tile_sides);

    void                getStarCommon(    xml_node & node, int & n, qreal & d, int & s);
    void                getStar2Common(   xml_node & node, int & n, qreal & theta, int & s);
    void                getRosetteCommon( xml_node & node, int & n, qreal & q, int & s);
    void                getRosette2Common(xml_node & node, int & n, qreal & x, qreal & y, qreal & k, int & s, eTipType & tt, bool & constrain);

    void            getExtendedData(xml_node & node, ExtenderPtr ep);
    void            getExtendedBoundary(xml_node & node, ExtendedBoundary & eb);
    ProtoPtr        getPrototype(xml_node & node);
    MapPtr          getMap(xml_node & node);
    EdgePtr         getEdge(xml_node & node);
    EdgePtr         getCurve(xml_node & node);
    EdgePtr         getChord(xml_node & node);
    VertexPtr       getVertex(xml_node & node);

    PolyPtr         getPolygonV1(xml_node & node);
    QPolygonF       getPolygonV2(xml_node & node);
    QRectF          getRectangle(xml_node node);
    Circle          getCircle(xml_node node);
    QPointF         getPos(xml_node & node);
    APolygon        getApoly(xml_node & node);

    // references
    void   setProtoReference(xml_node & node, ProtoPtr ptr);
    void   setEdgeReference(xml_node & node, EdgePtr ptr);
    void   setPolyReference(xml_node & node, PolyPtr ptr);
    void   setTileReference(xml_node & node, TilePtr ptr);
    void   setFMotifReference(xml_node & node, MotifPtr ptr);
    void   setExplicitReference(xml_node & node, IrregularPtr ptr);
    void   setStarReference(xml_node & node, StarPtr ptr);
    void   setStar2Reference(xml_node & node, Star2Ptr ptr);
    void   setRosetteReference(xml_node & node, RosettePtr ptr);
    void   setRosette2Reference(xml_node & node, Rosette2Ptr ptr);
    void   setMapReference(xml_node & node, MapPtr ptr);

    ProtoPtr        getProtoReferencedPtr(xml_node & node);
    EdgePtr         getEdgeReferencedPtr(xml_node & node);
    PolyPtr         getPolyReferencedPtr(xml_node & node);
    TilePtr         getTileReferencedPtr(xml_node & node);
    MotifPtr        getMotifReferencedPtr(xml_node & node);
    IrregularPtr    getExplicitReferencedPtr(xml_node & node);
    StarPtr         getStarReferencedPtr(xml_node & node);
    Star2Ptr        getStar2ReferencedPtr(xml_node & node);
    RosettePtr      getRosetteReferencedPtr(xml_node & node);
    Rosette2Ptr     getRosette2ReferencedPtr(xml_node & node);
    MapPtr          getMapReferencedPtr(xml_node & node);

    TilingPtr       getFirstTiling() { return _tilings.first(); }

protected:
    QColor  processColor(xml_node & n);
    void    procBorderPlain(xml_node & n);
    void    procBorderTwoColor(xml_node & n);
    void    procBorderBlocks(xml_node & n);
    void    procCropV1(xml_node & n);
    CropPtr procCropV2(xml_node & node, QString name);
    bool    getAttributeTF(xml_node & node, const char_t *name);
    uint    getAttributeUint(xml_node & node, const char_t *name);

    [[noreturn]] void fail(QString a, QString b);
    QTransform getQTransform(QString txt);

    QMap<int,ProtoPtr>      proto_ids;
    QMap<int,EdgePtr>       edge_ids;
    QMap<int,PolyPtr>       poly_ids;
    QMap<int,TilePtr>       tile_ids;
    QMap<int,MotifPtr>      motif_ids;
    QMap<int,IrregularPtr>  explicit_ids;
    QMap<int,StarPtr>       star_ids;
    QMap<int,Star2Ptr>      star2_ids;
    QMap<int,RosettePtr>    rosette_ids;
    QMap<int,Rosette2Ptr>   rosette2_ids;
    QMap<int,MapPtr>        map_ids;

    MapPtr                  _currentMap;
    MosaicPtr               _mosaic;
    ProtoPtr                _prototype;

    QVector<TilingPtr>      _tilings;
    VersionedFile           _xfile;
    QString                 _failMessage;

    QColor                  _background;
    QSize                   _viewSize;
    QSize                   _canvasSize;
    BorderPtr               _border;
    CropPtr                 _crop;
    CropPtr                 _painterCrop;
    unsigned int            _version;
    FillData                _fillData;
    uint                    _cleanseLevel;
    qreal                   _cleanseSensitivity;
    BkgdImagePtr            _bip;
    ColorGroup              _tilingColorGroup;
    Xform                   _firstStyleXform;   // used when _version < 23
    bool                    _debug;

    MRBasePtr               base;
};

#endif
