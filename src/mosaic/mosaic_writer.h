#pragma once
#ifndef XMLWRITER_H
#define XMLWRITER_H

#include <string>
#include "enums/emotiftype.h"
#include "mosaic/mosaic_writer_base.h"
#include "misc/pugixml.hpp"
#include "geometry/edgepoly.h"
#include "geometry/xform.h"
#include "misc/colorset.h"
#include "enums/estyletype.h"

using std::string;
using std::shared_ptr;
using std::weak_ptr;
using namespace pugi;

class ViewControl;
class Configuration;

typedef shared_ptr<class Border>           BorderPtr;
typedef shared_ptr<class Crop>             CropPtr;
typedef shared_ptr<class IrregularMotif>   ExplicitPtr;
typedef shared_ptr<class ExtendedRosette>  ExtRosettePtr;
typedef shared_ptr<class ExtendedStar>     ExtStarPtr;
typedef shared_ptr<class Tile>             TilePtr;
typedef shared_ptr<class Motif>            MotifPtr;
typedef shared_ptr<class Map>              MapPtr;
typedef shared_ptr<class Mosaic>           MosaicPtr;
typedef shared_ptr<class Prototype>        ProtoPtr;
typedef shared_ptr<class Rosette>          RosettePtr;
typedef shared_ptr<class RosetteConnect>   RosetteConnectPtr;
typedef shared_ptr<class Star>             StarPtr;
typedef shared_ptr<class StarConnect>      StarConnectPtr;
typedef shared_ptr<class Style>            StylePtr;

class MosaicWriter : public MosaicWriterBase
{
    friend class TilingWriter;

public:
    MosaicWriter();
    ~MosaicWriter();

    bool writeXML(QString fileName, MosaicPtr design);
    QString getFailMsg() { return _failMsg; }

protected:

    bool generateVector(QTextStream & ts);
    bool processVector(QTextStream & ts);
    bool generateDesignNotes(QTextStream & ts);

    void processDesign(QTextStream & ts);
    void procSize(QTextStream &ts, QSizeF size, QSize zsize);
    void procRect(QTextStream &ts, QRectF rect);
    void procCircle(QTextStream &ts, Circle c);
    void procWidth(QTextStream &ts,qreal width);
    void procBackground(QTextStream &ts, QColor color);
    void procBorder(QTextStream &ts, BorderPtr border);
    void procCrop(QTextStream &ts, CropPtr crop);

    void procColor(QTextStream & ts, QColor color);
    void procColor(QTextStream & ts, TPColor tpcolor);
    void procColorSet(QTextStream &ts, ColorSet *colorSet);
    void procColorGroup(QTextStream &ts, ColorGroup *colorGroup);

    bool processInterlace(QTextStream &ts, StylePtr s);
    bool processThick(QTextStream & ts, StylePtr s);
    bool processFilled(QTextStream &ts, StylePtr s);
    bool processOutline(QTextStream &ts, StylePtr s);
    bool processPlain(QTextStream &ts, StylePtr s);
    bool processSketch(QTextStream &ts, StylePtr s);
    bool processEmboss(QTextStream &ts, StylePtr s);
    bool processTileColors(QTextStream &ts, StylePtr s);

    // styles
    static void procesToolkitGeoLayer(QTextStream &ts, const Xform &xf, int zlevel);
    void    processsStyleThick(QTextStream &ts, eDrawOutline draw_outline, qreal width, qreal outlineWidth, QColor outlineColor, Qt::PenJoinStyle join_style, Qt::PenCapStyle cap_style);
    void    processsStyleInterlace(QTextStream &ts, qreal gap, qreal shadow, bool includeSVerts, bool startUnder);
    void    processsStyleFilled(QTextStream &ts, bool draw_inside, bool draw_outside, int algorithm);
    void    processsStyleEmboss(QTextStream &ts, qreal angle);

    // features
    void    setTile(QTextStream & ts,TilePtr tile);

    // figures
    void    setMotifCommon(QTextStream & ts, MotifPtr motif);
    void    setExplicitMotif(QTextStream & ts,QString name, MotifPtr motif);
    void    setStar(QTextStream & ts,QString name, MotifPtr motif, bool childEnd=false);
    void    setRosette(QTextStream & ts, QString name, MotifPtr motif, bool childEnd=false);
    void    setExtendedStar(QTextStream & ts,QString name, MotifPtr motif);
    void    setExtendedRosette(QTextStream & ts,QString name, MotifPtr motif);
    void    setRosetteConnect(QTextStream & ts,QString name, MotifPtr motif);
    void    setStarConnect(QTextStream & ts,QString name, MotifPtr motif);

    bool    setMap(QTextStream & ts, MapPtr map);
    void    setVertices(QTextStream & ts, const QVector<VertexPtr> & vertices);
    void    setEdges(QTextStream & ts, const QVector<EdgePtr> & edges );

    void    setBoundary(QTextStream & ts, PolyPtr p);
    void    setPrototype(QTextStream & ts, ProtoPtr pp);
    void    setPolygon(QTextStream & ts,PolyPtr pp);    // deprecated

    void    setEdgePoly(QTextStream & ts, const EdgePoly & epoly);
    void    setVertexEP(QTextStream & ts,VertexPtr v, QString name);
    void    setVertex(QTextStream & ts, VertexPtr v, QString name = "Vertex");
    void    setEdges(QTextStream & ts, QVector<EdgePtr> &qvec);
    void    setEdge(QTextStream & ts, EdgePtr e);

    void    setPos(QTextStream & ts, QPointF qpf);
    void    setPoint(QTextStream & ts, QPointF pt, QString name);

    // references
    bool   hasReference(PolyPtr pp);
    bool   hasReference(ProtoPtr pp);
    bool   hasReference(TilePtr fp);
    bool   hasReference(MotifPtr fp);
    bool   hasReference(ExplicitPtr ep);
    bool   hasReference(RosettePtr ep);
    bool   hasReference(StarPtr ep);
    bool   hasReference(ExtStarPtr ep);
    bool   hasReference(ExtRosettePtr ep);
    bool   hasReference(RosetteConnectPtr ep);
    bool   hasReference(StarConnectPtr ep);
    bool   hasReference(MapPtr map);
    bool   hasReference(EdgePtr map);

    void   setProtoReference(int id, ProtoPtr ptr);
    void   setPolyReference(int id, PolyPtr ptr);
    void   setTileReference(int id, TilePtr ptr);
    void   setMotifReference(int id, MotifPtr ptr);
    void   setExplicitReference(int id, ExplicitPtr ptr);
    void   setMapReference(int id, MapPtr ptr);
    void   setEdgeReference(int id, EdgePtr ptr);
    void   setStarReference(int id, StarPtr ptr);
    void   setExtendedStarReference(int id, ExtStarPtr ptr);
    void   setExtendedRosetteReference(int id, ExtRosettePtr ptr);
    void   setRosetteReference(int id, RosettePtr ptr);
    void   setRosetteConnectReference(int id, RosetteConnectPtr ptr);
    void   setStarConnectReference(int id, StarConnectPtr ptr);

    QString getPolyReference(PolyPtr ptr);
    QString getProtoReference(ProtoPtr ptr);
    QString getTileReference(TilePtr ptr);
    QString getFMotifReference(MotifPtr ptr);
    QString getExplicitReference(ExplicitPtr ptr);
    QString getMapReference(MapPtr ptr);
    QString getEdgeReference(EdgePtr ptr);
    QString getStarReference(StarPtr ptr);
    QString getExtendedStarReference(ExtStarPtr ptr);
    QString getExtendedRosetteReference(ExtRosettePtr ptr);
    QString getRosetteReference(RosettePtr ptr);
    QString getRosetteConnectReference(RosetteConnectPtr ptr);
    QString getStarConnectReference(StarConnectPtr ptr);

    int     currentXMLVersion;
    QString _fileName;
    QString _failMsg;

private:
    [[noreturn]] void fail(QString a, QString b);

    QMap<ProtoPtr,int>      proto_ids;
    QMap<PolyPtr,int>       poly_ids;
    QMap<TilePtr,int>       tile_ids;
    QMap<MotifPtr,int>      motif_ids;
    QMap<MapPtr,int>        map_ids;
    QMap<EdgePtr,int>       edge_ids;
    QMap<ExplicitPtr,int>   explicit_ids;
    QMap<StarPtr,int>       star_ids;
    QMap<ExtStarPtr,int>    extended_star_ids;
    QMap<ExtRosettePtr,int> extended_rosette_ids;
    QMap<RosettePtr,int>    rosette_ids;
    QMap<RosetteConnectPtr,int>  rosette_connect_ids;
    QMap<StarConnectPtr,int>  star_connect_ids;

    Configuration         * config;
    MosaicPtr               _mosaic;
};

#endif
