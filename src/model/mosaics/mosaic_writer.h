#pragma once
#ifndef XMLWRITER_H
#define XMLWRITER_H

#include <string>
#include "sys/enums/efilltype.h"
#include "sys/enums/emotiftype.h"
#include "sys/enums/estyletype.h"
#include "sys/geometry/edgepoly.h"
#include "sys/geometry/polygon.h"
#include "sys/geometry/xform.h"
#include "model/styles/colorset.h"
#include "sys/sys/pugixml.hpp"
#include "sys/sys/versioning.h"
#include "model/mosaics/mosaic_writer_base.h"
#include "model/motifs/extender.h"

using std::string;
using std::shared_ptr;
using std::weak_ptr;
using namespace pugi;

class ViewController;
class Configuration;

typedef shared_ptr<class Border>           BorderPtr;
typedef shared_ptr<class Crop>             CropPtr;
typedef shared_ptr<class IrregularMotif>   ExplicitPtr;
typedef shared_ptr<class Tile>             TilePtr;
typedef shared_ptr<class Motif>            MotifPtr;
typedef shared_ptr<class Map>              MapPtr;
typedef shared_ptr<class Mosaic>           MosaicPtr;
typedef shared_ptr<class Prototype>        ProtoPtr;
typedef shared_ptr<class Rosette>          RosettePtr;
typedef shared_ptr<class Rosette2>         Rosette2Ptr;
typedef shared_ptr<class Star>             StarPtr;
typedef shared_ptr<class Star2>            Star2Ptr;
typedef shared_ptr<class Style>            StylePtr;

class MosaicWriter : public MosaicWriterBase
{
    friend class TilingWriter;

public:
    MosaicWriter();
    ~MosaicWriter();

    bool    writeXML(VersionedFile xfile, MosaicPtr design);
    QString getFailMsg() { return _failMsg; }

protected:
    bool    generateVector(QTextStream & ts);
    bool    processVector(QTextStream & ts);
    bool    generateDesignNotes(QTextStream & ts);

    void    processMosaic(QTextStream & ts);
    void    procSize(QTextStream &ts, QSize viewSize, QSize canvasSize);

    void    procBackground(QTextStream &ts, QColor color);
    void    procBorder(QTextStream &ts, BorderPtr border);
    void    procCrop(QTextStream &ts, CropPtr crop, QString name);

    void    procColor(QTextStream & ts, QColor color);
    void    procColor(QTextStream & ts, TPColor tpcolor);
    void    procColorSet(QTextStream &ts, ColorSet *colorSet);
    void    procColorGroup(QTextStream &ts, ColorGroup *colorGroup);

    bool    processInterlace(QTextStream &ts, StylePtr s);
    bool    processThick(QTextStream & ts, StylePtr s);
    bool    processFilled(QTextStream &ts, StylePtr s);
    bool    processOutline(QTextStream &ts, StylePtr s);
    bool    processPlain(QTextStream &ts, StylePtr s);
    bool    processSketch(QTextStream &ts, StylePtr s);
    bool    processEmboss(QTextStream &ts, StylePtr s);
    bool    processTileColors(QTextStream &ts, StylePtr style);

    // styles
    static void procesToolkitGeoLayer(QTextStream &ts, const Xform &xf, int zlevel);
    void    processsStyleThick(QTextStream &ts, eDrawOutline draw_outline, qreal width, qreal outlineWidth, QColor outlineColor, Qt::PenJoinStyle join_style, Qt::PenCapStyle cap_style);
    void    processsStyleInterlace(QTextStream &ts, qreal gap, qreal shadow, bool includeSVerts, bool startUnder);
    void    processsStyleFilled(QTextStream &ts, bool draw_inside, bool draw_outside, eFillType algorithm);
    void    processsStyleFilledFaces(QTextStream &ts, class Filled * filled);
    void    processsStyleEmboss(QTextStream &ts, qreal angle);

    // tiles
    void    setTile(QTextStream & ts,TilePtr tile);

    // motifs
    void    setMotifCommon(   QTextStream & ts,               MotifPtr motif);
    void    setExplicitMotif( QTextStream & ts, QString name, MotifPtr motif);
    void    setStar(          QTextStream & ts, QString name, MotifPtr motif);
    void    setStar2(         QTextStream & ts, QString name, MotifPtr motif);
    void    setRosette(       QTextStream & ts, QString name, MotifPtr motif);
    void    setRosette2(      QTextStream & ts, QString name, MotifPtr motif);

    void    setStarCommon(    QTextStream & ts, StarPtr star);
    void    setStar2Common(   QTextStream & ts, Star2Ptr star);
    void    setRosetteCommon(QTextStream & ts, RosettePtr rose);
    void    setRosette2Common(QTextStream & ts, Rosette2Ptr rose);

    void    setPrototype(QTextStream & ts, ProtoPtr pp);
    bool    setMap(QTextStream & ts, MapPtr map);
    void    setVertices(QTextStream & ts, const QVector<VertexPtr> & vertices);
    void    setEdges(QTextStream & ts, const QVector<EdgePtr> & edges );

    void    setEdgePoly(QTextStream & ts, const EdgePoly & epoly);
    void    setVertexEP(QTextStream & ts,VertexPtr v, QString name);
    void    setVertex(QTextStream & ts, VertexPtr v, QString name = "Vertex");
    void    setEdges(QTextStream & ts, QVector<EdgePtr> &qvec);
    void    setEdge(QTextStream & ts, EdgePtr e);

    void    procRect(QTextStream &ts, QRectF & rect);
    void    procCircle(QTextStream &ts, Circle & c);
    void    procWidth(QTextStream &ts,qreal width);
    void    procAPolygon(QTextStream &ts,APolygon & apoly);
    void    procPolygon(QTextStream &ts,QPolygonF & poly);
    void    procLength(QTextStream &ts, qreal length);
    void    setPos(QTextStream & ts, QPointF qpf);
    void    setPoint(QTextStream & ts, QPointF pt, QString name);

    // references
    bool   hasReference(PolyPtr pp);
    bool   hasReference(ProtoPtr pp);
    bool   hasReference(TilePtr fp);
    bool   hasReference(MotifPtr fp);
    bool   hasReference(ExplicitPtr ep);
    bool   hasReference(RosettePtr ep);
    bool   hasReference(Rosette2Ptr n);
    bool   hasReference(StarPtr ep);
    bool   hasReference(Star2Ptr ep);
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
    void   setStar2Reference(int id, Star2Ptr ptr);

    void   setRosetteReference(int id, RosettePtr ptr);
    void   setRosette2Reference(int id, Rosette2Ptr ptr);

    QString getPolyReference(PolyPtr ptr);
    QString getProtoReference(ProtoPtr ptr);
    QString getTileReference(TilePtr ptr);
    QString getFMotifReference(MotifPtr ptr);
    QString getExplicitReference(ExplicitPtr ptr);
    QString getMapReference(MapPtr ptr);
    QString getEdgeReference(EdgePtr ptr);

    QString getStarReference(StarPtr ptr);
    QString getStar2Reference(Star2Ptr ptr);

    QString getRosetteReference(RosettePtr ptr);
    QString getRosette2Reference(Rosette2Ptr ptr);

    int     currentXMLVersion;
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
    QMap<Star2Ptr,int>      star2_ids;
    QMap<RosettePtr,int>    rosette_ids;
    QMap<Rosette2Ptr,int>   rosette2_ids;

    MosaicPtr               _mosaic;
};

#endif
