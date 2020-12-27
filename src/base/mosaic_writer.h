/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef XMLWRITER_H
#define XMLWRITER_H

#include <QtCore>
#include <string>
#include "style/style.h"
#include "tapp/star.h"
#include "tapp/extended_rosette.h"
#include "base/pugixml.hpp"
#include "base/shared.h"
#include "base/mosaic.h"
#include "geometry/xform.h"

using std::string;
using namespace pugi;

class ViewControl;
class Configuration;

class MosaicWriter
{
    friend class TilingWriter;

public:
    MosaicWriter();
    ~MosaicWriter();

    bool writeXML(QString fileName, MosaicPtr design);
    bool writeXML(QString fileName, MapPtr map);
    QString getFailMsg() { return _failMsg; }

protected:

    bool generateVector(QTextStream & ts);
    bool processVector(QTextStream & ts);
    bool generateDesignNotes(QTextStream & ts);

    void processDesign(QTextStream & ts);
    void procSize(QTextStream &ts,QSizeF size);
    void procWidth(QTextStream &ts,qreal width);
    void procBackground(QTextStream &ts, QColor color);
    void procBorder(QTextStream &ts, BorderPtr border);

    void procColor(QTextStream & ts, QColor color);
    void procColor(QTextStream & ts, TPColor tpcolor);
    void procColorSet(QTextStream &ts, ColorSet & colorSet);
    void procColorGroup(QTextStream &ts, ColorGroup & colorGroup);

    bool processInterlace(QTextStream &ts, StylePtr s);
    bool processThick(QTextStream & ts, StylePtr s);
    bool processFilled(QTextStream &ts, StylePtr s);
    bool processOutline(QTextStream &ts, StylePtr s);
    bool processPlain(QTextStream &ts, StylePtr s);
    bool processSketch(QTextStream &ts, StylePtr s);
    bool processEmboss(QTextStream &ts, StylePtr s);
    bool processTileColors(QTextStream &ts, StylePtr s);

    // styles
    static void procesToolkitGeoLayer(QTextStream &ts, Xform &xf);
    void    processsStyleThick(QTextStream &ts, bool draw_outline, qreal width);
    void    processsStyleInterlace(QTextStream &ts, qreal gap, qreal shadow, bool includeSVerts);
    void    processsStyleFilled(QTextStream &ts, bool draw_inside, bool draw_outside, int algorithm);
    void    processsStyleEmboss(QTextStream &ts, qreal angle);

    // features
    void    setFeature(QTextStream & ts,FeaturePtr fp);

    // figures
    void    setFigureCommon(QTextStream & ts, FigurePtr fp);
    void    setExplicitFigure(QTextStream & ts,QString name, FigurePtr fp);
    void    setStarFigure(QTextStream & ts,QString name, FigurePtr fp, bool childEnd=false);
    void    setRosetteFigure(QTextStream & ts, QString name, FigurePtr fp, bool childEnd=false);
    void    setExtendedStarFigure(QTextStream & ts,QString name, FigurePtr fp);
    void    setExtendedRosetteFigure(QTextStream & ts,QString name, FigurePtr fp);
    void    setRosetteConnectFigure(QTextStream & ts,QString name, FigurePtr fp);
    void    setStarConnectFigure(QTextStream & ts,QString name, FigurePtr fp);

    bool    setMap(QTextStream & ts, MapPtr map);
    void    setVertices(QTextStream & ts, const QVector<VertexPtr> & vertices);
    void    setEdges(QTextStream & ts, const QVector<EdgePtr> & edges );
    void    setNeighbours(QTextStream & ts, NeighbourMap &nmap );

    void    setBoundary(QTextStream & ts, PolyPtr p);
    void    setPrototype(QTextStream & ts, PrototypePtr pp);
    void    setPolygon(QTextStream & ts,PolyPtr pp);    // deprecated

    void    setEdgePoly(QTextStream & ts, const EdgePoly & epoly);
    void    setVertexEP(QTextStream & ts,VertexPtr v, QString name);
    void    setVertex(QTextStream & ts, VertexPtr v, QString name = "Vertex");
    void    setEdges(QTextStream & ts, QVector<EdgePtr> &qvec);
    void    setEdge(QTextStream & ts, EdgePtr e);
    void    setNeighbour(QTextStream & ts, VertexPtr v, NeighboursPtr np);

    void    setPos(QTextStream & ts, QPointF qpf);
    void    setPoint(QTextStream & ts, QPointF pt, QString name);

    // references
    bool   hasReference(PolyPtr pp);
    bool   hasReference(PrototypePtr pp);
    bool   hasReference(FeaturePtr fp);
    bool   hasReference(FigurePtr fp);
    bool   hasReference(ExplicitPtr ep);
    bool   hasReference(RosettePtr ep);
    bool   hasReference(StarPtr ep);
    bool   hasReference(ExtStarPtr ep);
    bool   hasReference(ExtRosettePtr ep);
    bool   hasReference(RosetteConnectPtr ep);
    bool   hasReference(StarConnectPtr ep);
    bool   hasReference(MapPtr map);
    bool   hasReference(VertexPtr map);
    bool   hasReference(EdgePtr map);

    void   setProtoReference(int id, PrototypePtr ptr);
    void   setPolyReference(int id, PolyPtr ptr);
    void   setFeatureReference(int id, FeaturePtr ptr);
    void   setFigureReference(int id, FigurePtr ptr);
    void   setExplicitReference(int id, ExplicitPtr ptr);
    void   setMapReference(int id, MapPtr ptr);
    void   setVertexReference(int id, VertexPtr ptr);
    void   setEdgeReference(int id, EdgePtr ptr);
    void   setStarReference(int id, StarPtr ptr);
    void   setExtendedStarReference(int id, ExtStarPtr ptr);
    void   setExtendedRosetteReference(int id, ExtRosettePtr ptr);
    void   setRosetteReference(int id, RosettePtr ptr);
    void   setRosetteConnectReference(int id, RosetteConnectPtr ptr);
    void   setStarConnectReference(int id, StarConnectPtr ptr);

    QString getPolyReference(PolyPtr ptr);
    QString getProtoReference(PrototypePtr ptr);
    QString getFeatureReference(FeaturePtr ptr);
    QString getFigureReference(FigurePtr ptr);
    QString getExplicitReference(ExplicitPtr ptr);
    QString getMapReference(MapPtr ptr);
    QString getVertexReference(VertexPtr ptr);
    QString getEdgeReference(EdgePtr ptr);
    QString getStarReference(StarPtr ptr);
    QString getExtendedStarReference(ExtStarPtr ptr);
    QString getExtendedRosetteReference(ExtRosettePtr ptr);
    QString getRosetteReference(RosettePtr ptr);
    QString getRosetteConnectReference(RosetteConnectPtr ptr);
    QString getStarConnectReference(StarConnectPtr ptr);

    // writer methods
    QString  id(int id);
    QString  nextId();
    int     getRef()  { return refId; }
    int     nextRef() { return ++refId; }

private:
    [[noreturn]] void fail(QString a, QString b);

    QMap<PrototypePtr,int>  proto_ids;
    QMap<PolyPtr,int>       poly_ids;
    QMap<FeaturePtr,int>    feature_ids;
    QMap<FigurePtr,int>     figure_ids;
    QMap<MapPtr,int>        map_ids;
    QMap<VertexPtr,int>     vertex_ids;
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
    QString                 _fileName;
    QString                 _failMsg;

    int refId;
};

#endif
