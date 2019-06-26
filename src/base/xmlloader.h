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

#ifndef XMLLOADER_H
#define XMLLOADER_H

#include "style/Style.h"
#include "tapp/Star.h"
#include "tapp/RosetteConnectFigure.h"
#include "tapp/ExtendedRosette.h"
#include <QtCore>
#include "pugixml.hpp"
#include <string>
#include "base/shared.h"
#include "base/styleddesign.h"

using std::string;
using namespace pugi;

class Workspace;

class XmlLoader
{
public:
    XmlLoader(StyledDesign & styledDesign);
    ~XmlLoader();

    bool    load(QString fileName);
    QString getLoadedFilename();
    QString getFailMessage() { return _failMessage; }

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
    void procScale(xml_node & node);
    void procSize(xml_node & node, int &width, int &height);
    QColor procBackground(xml_node & node);
    void procBorder(xml_node & node);

    // styles
    void    procesToolkitGeoLayer(xml_node & node, Bounds & b);
    void    processColorSet(xml_node & node, QColor & color);
    void    processColorSet(xml_node & node, ColorSet & colorSet);
    void    processColorGroup(xml_node & node, ColorGroup & colorGroup);
    void    processsStyleThick(xml_node & node, bool & draw_outline, qreal & width);
    void    processsStyleInterlace(xml_node & node, qreal & gap, qreal & shadow, bool & includeSVerts);
    void    processsStyleFilled(xml_node & node, bool &draw_inside, bool &draw_outside, int &algorithm);
    void    processsStyleEmboss(xml_node & node, qreal & angle);
    void    processStyleStyle(xml_node & node, PrototypePtr &proto, PolyPtr &poly);

    // features
    FeaturePtr  getFeature(xml_node & node);

    // figures
    void                getFigureCommon(xml_node & node, FigurePtr fig);
    ExplicitPtr         getExplicitFigure(xml_node & node, eFigType figType);
    StarPtr             getStarFigure(xml_node & node);
    ExtStarPtr          getExtendedStarFigure(xml_node & node);
    ExtRosettePtr       getExtendedRosetteFigure(xml_node & node);
    RosettePtr          getRosetteFigure(xml_node & node);
    RosetteConnectPtr   getRosetteConnectFigure(xml_node & node);

    PolyPtr         getPolygon(xml_node & node);
    VertexPtr       getVertex(xml_node & node);
    EdgePtr         getEdge(xml_node & node);
    MapPtr          getMap(xml_node & node);
    PrototypePtr    getPrototype(xml_node & node);
    PolyPtr         getBoundary(xml_node & node);

    // references
    bool   hasReference(xml_node & node);

    void   setProtoReference(xml_node & node, PrototypePtr ptr);
    void   setVertexReference(xml_node & node, VertexPtr ptr);
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
    void   setMapReference(xml_node & node, MapPtr ptr);

    PrototypePtr    getProtoReferencedPtr(xml_node & node);
    VertexPtr       getVertexReferencedPtr(xml_node & node);
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
    MapPtr          getMapReferencedPtr(xml_node & node);


private:
    QColor  processColor(xml_node & n);
    void    procBorder0(xml_node & n);
    void    procBorder1(xml_node & n);
    void    procBorder2(xml_node & n);
    [[noreturn]] void fail(QString a, QString b);

    QMap<int,PrototypePtr>  proto_ids;
    QMap<int,VertexPtr>     vertex_ids;
    QMap<int,EdgePtr>       edge_ids;
    QMap<int,NeighbourPtr>  neighbour_ids;
    QMap<int,PolyPtr>       poly_ids;
    QMap<int,FeaturePtr>    feature_ids;
    QMap<int,FigurePtr>     figure_ids;
    QMap<int,ExplicitPtr>   explicit_ids;
    QMap<int,StarPtr>       star_ids;
    QMap<int,ExtStarPtr>    ext_star_ids;
    QMap<int,ExtRosettePtr> ext_rosette_ids;
    QMap<int,RosettePtr>    rosette_ids;
    QMap<int,RosetteConnectPtr>    rosette_connect_ids;
    QMap<int,MapPtr>        map_ids;

    StyledDesign &          _styledDesign;

    TilingPtr               _tiling;
    QString                 _fileName;
    QString                 _failMessage;

    QColor                  _background;
    int                     _width;
    int                     _height;
    BorderPtr               _border;
    qreal                   _scale;
    unsigned int            _version;

    int vOrigCnt;
    int vRefrCnt;
    int eOrigCnt;
    int eRefrCnt;
    int nRefrCnt;

    Canvas * canvas;
};

#endif
