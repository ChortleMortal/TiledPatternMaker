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

#include "base/mosaic_writer.h"
#include "base/border.h"
#include "base/fileservices.h"
#include "base/mosaic.h"
#include "geometry/crop.h"
#include "geometry/edge.h"
#include "geometry/map.h"
#include "geometry/vertex.h"
#include "settings/model_settings.h"
#include "style/colored.h"
#include "style/emboss.h"
#include "style/filled.h"
#include "style/interlace.h"
#include "style/outline.h"
#include "style/plain.h"
#include "style/sketch.h"
#include "style/thick.h"
#include "style/tile_colors.h"
#include "tapp/design_element.h"
#include "tapp/explicit_figure.h"
#include "tapp/extended_rosette.h"
#include "tapp/extended_star.h"
#include "tapp/prototype.h"
#include "tapp/rosette_connect_figure.h"
#include "tapp/star.h"
#include "tapp/star_connect_figure.h"
#include "tile/backgroundimage.h"
#include "tile/feature.h"
#include "tile/tiling.h"
#include "tile/tiling_writer.h"

//const int currentXMLVersion = 3;
//const int currentXMLVersion = 4;  // 05OCT19 use ColorSets in Colored
//const int currentXMLVersion = 5;  // 25OCT19 revised way of defining maps
//const int currentXMLVersion = 6;  // 26JUL20 includes FillData
//const int currentXMLVersion = 7;  // 15DEC20 Feature epolys being saved correctly
//const int currentXMLVersion = 8;  // 31DEC20 Indepedent background image being saved
//const int currentXMLVersion = 9;  // 03MAR21 Neighbour Map no longer needed
  const int currentXMLVersion = 10; // 07APR21 Border enhancements


MosaicWriter::MosaicWriter()
{
    config = Configuration::getInstance();
}

MosaicWriter::~MosaicWriter()
{
}

bool MosaicWriter::writeXML(QString fileName, MosaicPtr mosaic)
{
    qDebug() << "Writing XML:" << fileName;
    _fileName  = fileName;
    _mosaic    = mosaic;

    QFile xml(fileName);
    if (!xml.open(QFile::WriteOnly | QFile::Truncate))
    {
        _failMsg = QString("Could not open file to write: %1").arg(fileName);
        return false;
    }

    QTextStream ts(& xml);
    ts.setRealNumberPrecision(16);

    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;

    // write vector containing styles
    bool rv;

    try
    {
        rv = generateVector(ts);
    }
    catch (...)
    {
        xml.close();
        return false;
    }

    xml.close();

    if (!rv)
    {
        xml.close();
        _failMsg = QString("ERROR writing XML file: %1").arg(fileName);
        return false;
    }

    rv = FileServices::reformatXML(fileName);
    return rv;
}

bool MosaicWriter::writeXML(QString fileName, MapPtr map)
{
    qDebug() << "Writing XML:" << fileName;
    _fileName = fileName;

    QFile xml(fileName);
    if (!xml.open(QFile::WriteOnly | QFile::Truncate))
    {
        _failMsg = QString("Could not open file to write: %1").arg(fileName);
        return false;
    }

    QTextStream ts(& xml);
    ts.setRealNumberPrecision(16);

    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;

    bool rv;

    try
    {
        refId = 0;
        qDebug() << "version=" << currentXMLVersion;
        QString qs = QString(" version=\"%1\"").arg(currentXMLVersion);
        ts << "<vector" << nextId() << qs << ">" << endl;
        rv = setMap(ts,map);
        if (!rv)
            return false;
        ts << "</vector>" << endl;

    }
    catch (...)
    {
        xml.close();
        return false;
    }

    xml.close();

    if (!rv)
    {
        xml.close();
        _failMsg = QString("ERROR writing XML file: %1").arg(fileName);
        return false;
    }

    rv = FileServices::reformatXML(fileName);
    return rv;
}

bool MosaicWriter::generateDesignNotes(QTextStream & ts)
{
    if (!_mosaic->getNotes().isEmpty())
    {
        ts << "<designNotes>" << _mosaic->getNotes() << "</designNotes>" << endl;
    }
    return true;
}

bool MosaicWriter::generateVector(QTextStream & ts)
{
    refId    = 0;
    qDebug() << "XML Writer - start generation";

    qDebug() << "version=" << currentXMLVersion;
    QString qs = QString(" version=\"%1\"").arg(currentXMLVersion);
    ts << "<vector" << nextId() << qs << ">" << endl;

    bool rv = processVector(ts);
    if (!rv) return false;

    ts << "</vector>" << endl;

    qDebug() << "XML Writer - end generation";

    return true;
}

bool MosaicWriter::processVector(QTextStream &ts)
{
    qDebug() << "start vector";

    generateDesignNotes(ts);
#if 0
    // write tiling
    StylePtr      sp = design.getFirstStyle();
    PrototypePtr  pp = sp->getPrototype();
    TilingPtr tiling = pp->getTiling();
    if (!tiling)
    {
        qWarning("Tiling not found in style");
        return false;
    }
    tiling->writeTilingXML(ts);
#endif

    //design
    processDesign(ts);

    // styles
    const StyleSet & sset = _mosaic->getStyleSet();
    for (auto it = sset.rbegin(); it != sset.rend(); it++)
    {
        StylePtr s = *it;
        eStyleType est = s->getStyleType();
        QString str;
        switch(est)
        {
        case STYLE_THICK:
            str = "style.Thick";
            ts << "<" << str << nextId() << " serialization=\"custom\">" << endl;
            processThick(ts,s);
            break;
        case STYLE_FILLED:
            str = "style.Filled";
            ts << "<" << str << nextId() << " serialization=\"custom\">" << endl;
            processFilled(ts,s);
            break;
        case STYLE_INTERLACED:
            str = "style.Interlace";
            ts << "<" << str << nextId() << " serialization=\"custom\">" << endl;
            processInterlace(ts,s);
            break;
        case STYLE_OUTLINED:
            str = "style.Outline";
            ts << "<" << str << nextId() << " serialization=\"custom\">" << endl;
            processOutline(ts,s);
            break;
        case STYLE_EMBOSSED:
            str = "style.Emboss";
            ts << "<" << str << nextId() << " serialization=\"custom\">" << endl;
            processEmboss(ts,s);
            break;
        case STYLE_PLAIN:
            str = "style.Plain";
            ts << "<" << str << nextId() << " serialization=\"custom\">" << endl;
            processPlain(ts,s);
            break;
        case STYLE_SKETCHED:
            str = "style.Sketch";
            ts << "<" << str << nextId() << " serialization=\"custom\">" << endl;
            processSketch(ts,s);
            break;
        case STYLE_TILECOLORS:
            str = "style.TileColors";
            ts << "<" << str << nextId() << " serialization=\"custom\">" << endl;
            processTileColors(ts,s);
            break;
        case STYLE_STYLE:
            fail("Unexpected style","STYLE_STYLE");
        }
        ts << "</" << str << ">" << endl;
    }

    qDebug() << "end vector";
    return true;
}

void MosaicWriter::processDesign(QTextStream &ts)
{
    ModelSettingsPtr info = _mosaic->getSettings();
    QColor bkgdColor      = info->getBackgroundColor();
    QSize  size           = info->getSize();
    QSize  zsize          = info->getZSize();
    BorderPtr border      = _mosaic->getBorder();
    BkgdImgPtr bip        = BackgroundImage::getSharedInstance();

    ts << "<design>" << endl;
    procSize(ts,size,zsize);
    procBackground(ts,bkgdColor);
    procBorder(ts,border);
    int minX,minY,maxX,maxY;
    info->getFillData().get(minX,maxX,minY,maxY);
    ts << "<Fill>" << minX << "," << maxX << "," << minY << "," << maxY << "</Fill>" << endl;
    TilingWriter::writeBackgroundImage(ts,bip);
    ts << "</design>" << endl;
}

void MosaicWriter::procWidth(QTextStream &ts,qreal width)
{
    ts << "<width>" << width << "</width>" << endl;
}

void MosaicWriter::procSize(QTextStream &ts,QSizeF size, QSize zsize)
{
    ts << "<size>"    << endl;
    ts << "<width>"   << size.width()   << "</width>" << endl;
    ts << "<height>"  << size.height()  << "</height>" << endl;
    ts << "<zwidth>"  << zsize.width()  << "</zwidth>" << endl;
    ts << "<zheight>" << zsize.height() << "</zheight>" << endl;
    ts << "</size>"   << endl;
}

void MosaicWriter::procBackground(QTextStream &ts,QColor color)
{
    ts << "<background>" << endl;
    procColor(ts,color);
    ts << "</background>" << endl;
}

void MosaicWriter::procColor(QTextStream & ts, QColor color)
{
    ts << "<color>";
    ts << color.name(QColor::HexArgb);
    ts << "</color>" << endl;
}

void MosaicWriter::procColor(QTextStream & ts, TPColor tpcolor)
{
    QString qs = QString("<color hide=\"%1\">").arg(tpcolor.hidden ? 't' : 'f');
    ts << qs << tpcolor.color.name(QColor::HexArgb) << "</color>" << endl;
}

void MosaicWriter::procBorder(QTextStream &ts,BorderPtr border)
{
    if (!border)
    {
        return;
    }

    eBorderType type = border->getType();
    QString txt = QString("<border type=\"%1\">") .arg(type);
    ts << txt << endl;

    if (type == BORDER_INTEGRATED)
    {
        InnerBorder * ib = dynamic_cast<InnerBorder*>(border.get());
        Q_ASSERT(ib);
        CropPtr crop = ib->getInnerBoundary();
        Q_ASSERT(crop);
        QRectF r = crop->getRect();
        if (r.isValid())
        {
            ts << "<boundary>" << r.x() << "," << r.y() << "," << r.width() << "," << r.height() << "</boundary>" << endl;
        }
    }

    if (type == BORDER_PLAIN)
    {
        BorderPlain * b0 = dynamic_cast<BorderPlain*>(border.get());
        Q_ASSERT(b0);
        QColor color;
        qreal  width;
        b0->get(width, color);
        procColor(ts,color);
        procWidth(ts,width);
    }
    else if (type == BORDER_TWO_COLOR)
    {
        BorderTwoColor * b1 = dynamic_cast<BorderTwoColor*>(border.get());
        Q_ASSERT(b1);
        QColor color1,color2;
        qreal  width;
        b1->get(color1,color2,width);
        procColor(ts,color1);
        procColor(ts,color2);
        procWidth(ts,width);
    }
    else if (type == BORDER_BLOCKS)
    {
        BorderBlocks * b2 = dynamic_cast<BorderBlocks*>(border.get());
        Q_ASSERT(b2);
        QColor color;
        qreal  diameter;
        int    rows;
        int    cols;
        b2->get(color,diameter,rows,cols);
        procColor(ts,color);
        procWidth(ts,diameter);
        ts << "<rows>" << rows << "</rows>" << endl;
        ts << "<cols>" << cols << "</cols>" << endl;
    }

    ts << "</border>" << endl;
}

bool MosaicWriter::processThick(QTextStream &ts, StylePtr s)
{
    Thick * th = dynamic_cast<Thick*>(s.get());
    if (th == nullptr)
    {
        fail("Style error","dynamic cast of Thick");
    }

    ColorSet * cset         = th->getColorSet();
    bool    draw_outline    = th->getDrawOutline();
    qreal   width           = th->getLineWidth();
    PrototypePtr proto      = th->getPrototype();
    Xform   xf              = th->getCanvasXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    setPrototype(ts,proto);
    ts << "</" << str << ">" << endl;

    str = "style.Colored";
    ts << "<" << str << ">" << endl;
    procColorSet(ts,cset);
    ts << "</" << str << ">" << endl;

    str = "style.Thick";
    ts << "<" << str << ">" << endl;
    processsStyleThick(ts,draw_outline,width);
    ts << "</" << str << ">" << endl;

    qDebug() << "end thick";
    return true;
}

bool MosaicWriter::processInterlace(QTextStream & ts, StylePtr s)
{
    Interlace * il = dynamic_cast<Interlace*>(s.get());
    if (il == nullptr)
    {
        fail("Style error","dynamic cast of Interlace");
    }

    ColorSet * cset      = il->getColorSet();
    bool    draw_outline = il->getDrawOutline();
    bool    includeTipVerts = il->getIncludeTipVertices();
    qreal   width        = il->getLineWidth();
    qreal   gap          = il->getGap();
    qreal   shadow       = il->getShadow();
    PrototypePtr proto   = il->getPrototype();
    Xform   xf           = il->getCanvasXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    setPrototype(ts,proto);
    ts << "</" << str << ">" << endl;

    str = "style.Colored";
    ts << "<" << str << ">" << endl;
    procColorSet(ts,cset);
    ts << "</" << str << ">" << endl;

    str = "style.Thick";
    ts << "<" << str << ">" << endl;
    processsStyleThick(ts,draw_outline,width);
    ts << "</" << str << ">" << endl;

    str = "style.Interlace";
    ts << "<" << str << ">" << endl;
    processsStyleInterlace(ts,gap,shadow,includeTipVerts);
    ts << "</" << str << ">" << endl;

    qDebug() << "end interlace";
    return true;
}

bool MosaicWriter::processOutline(QTextStream &ts, StylePtr s)
{
    Outline * ol = dynamic_cast<Outline*>(s.get());
    if (ol == nullptr)
    {
        fail("Style error","dynamic cast of Interlace");
    }

    ColorSet * cset      = ol->getColorSet();
    bool    draw_outline = ol->getDrawOutline();
    qreal   width        = ol->getLineWidth();
    PrototypePtr proto   = ol->getPrototype();
    Xform   xf           = ol->getCanvasXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    setPrototype(ts,proto);
    ts << "</" << str << ">" << endl;

    str = "style.Colored";
    ts << "<" << str << ">" << endl;
    procColorSet(ts,cset);
    ts << "</" << str << ">" << endl;

    str = "style.Thick";
    ts << "<" << str << ">" << endl;
    processsStyleThick(ts,draw_outline,width);
    ts << "</" << str << ">" << endl;

    qDebug() << "end outline";
    return true;
}

bool MosaicWriter::processFilled(QTextStream &ts, StylePtr s)
{
    Filled * fl = dynamic_cast<Filled*>(s.get());
    if (fl == nullptr)
    {
        fail("Style error","dynamic cast of Filled");
    }

    int algorithm           = fl->getAlgorithm();
    int cleanseLevel        = fl->getCleanseLevel();

    ColorSet * colorSetB    = fl->getBlackColorSet();
    ColorSet * colorSetW    = fl->getWhiteColorSet();
    ColorGroup * colorGroup = fl->getColorGroup();

    bool    draw_inside     = fl->getDrawInsideBlacks();
    bool    draw_outside    = fl->getDrawOutsideWhites();

    PrototypePtr proto      = fl->getPrototype();
    Xform   xf              = fl->getCanvasXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    setPrototype(ts,proto);
    ts << "</" << str << ">" << endl;

    if (colorSetB->size())
    {
        str = "ColorBlacks";
        ts << "<" << str << ">" << endl;
        procColorSet(ts,colorSetB);
        ts << "</" << str << ">" << endl;
    }

    if (colorSetW->size())
    {
        str = "ColorWhites";
        ts << "<" << str << ">" << endl;
        procColorSet(ts,colorSetW);
        ts << "</" << str << ">" << endl;
    }

    if (colorGroup->size())
    {
        str = "ColorGroup";
        ts << "<" << str << ">" << endl;
        procColorGroup(ts,colorGroup);
        ts << "</" << str << ">" << endl;
    }

    str = "style.Filled";
    ts << "<" << str << ">" << endl;
    processsStyleFilled(ts,draw_inside,draw_outside,algorithm,cleanseLevel);
    ts << "</" << str << ">" << endl;

    qDebug() << "end filled";
    return true;
}

bool MosaicWriter::processPlain(QTextStream &ts, StylePtr s)
{
    Plain * pl = dynamic_cast<Plain*>(s.get());
    if (pl == nullptr)
    {
        fail("Style error","dynamic cast of Plain");
    }

    ColorSet * cset     = pl->getColorSet();
    PrototypePtr proto  = pl->getPrototype();
    Xform   xf          = pl->getCanvasXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    setPrototype(ts,proto);
    ts << "</" << str << ">" << endl;

    str = "style.Colored";
    ts << "<" << str << ">" << endl;
    procColorSet(ts,cset);
    ts << "</" << str << ">" << endl;

    qDebug() << "end plain";
    return true;
}

bool MosaicWriter::processSketch(QTextStream &ts, StylePtr s)
{
    Sketch * sk = dynamic_cast<Sketch*>(s.get());
    if (sk == nullptr)
    {
        fail("Style error","dynamic cast of Sketch");
    }

    ColorSet * cset     = sk->getColorSet();
    PrototypePtr proto  = sk->getPrototype();
    Xform   xf          = sk->getCanvasXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    setPrototype(ts,proto);
    ts << "</" << str << ">" << endl;

    str = "style.Colored";
    ts << "<" << str << ">" << endl;
    procColorSet(ts,cset);
    ts << "</" << str << ">" << endl;

    return true;
}

bool MosaicWriter::processEmboss(QTextStream &ts, StylePtr s)
{
    Emboss * em = dynamic_cast<Emboss*>(s.get());
    if (em == nullptr)
    {
        fail("Style error","dynamic cast of Emboss");
    }

    ColorSet * cset      = em->getColorSet();
    bool    draw_outline = em->getDrawOutline();
    qreal   width        = em->getLineWidth();
    qreal   angle        = em->getAngle();
    PrototypePtr proto   = em->getPrototype();
    Xform   xf           = em->getCanvasXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    setPrototype(ts,proto);
    ts << "</" << str << ">" << endl;

    str = "style.Colored";
    ts << "<" << str << ">" << endl;
    procColorSet(ts,cset);
    ts << "</" << str << ">" << endl;

    str = "style.Thick";
    ts << "<" << str << ">" << endl;
    processsStyleThick(ts,draw_outline,width);
    ts << "</" << str << ">" << endl;

    str = "style.Emboss";
    ts << "<" << str << ">" << endl;
    processsStyleEmboss(ts,angle);
    ts << "</" << str << ">" << endl;

    qDebug() << "end emboss";
    return true;
}

bool MosaicWriter::processTileColors(QTextStream &ts, StylePtr s)
{
    TileColors * tc = dynamic_cast<TileColors*>(s.get());
    if (tc == nullptr)
    {
        fail("Style error","dynamic cast of Tile Colors");
    }

    PrototypePtr proto  = tc->getPrototype();
    Xform   xf          = tc->getCanvasXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    setPrototype(ts,proto);
    ts << "</" << str << ">" << endl;

    QColor color;
    int width;
    bool outlineEnb = tc->getOutline(color,width);
    if (outlineEnb)
    {
        ts << "<outline>" << endl;
        procColor(ts,color);
        procWidth(ts,width);
        ts << "</outline>";
    }

    return true;
}

void MosaicWriter::procesToolkitGeoLayer(QTextStream & ts, Xform & xf)
{
    ts << "<left__delta>"  << xf.getTranslateX()      << "</left__delta>"  << endl;
    ts << "<top__delta>"   << xf.getTranslateY()      << "</top__delta>"   << endl;
    ts << "<width__delta>" << xf.getScale()           << "</width__delta>" << endl;
    ts << "<theta__delta>" << xf.getRotateRadians()   << "</theta__delta>" << endl;
    QPointF pt = xf.getCenter();
    ts << "<center>" << pt.x() << "," << pt.y()       << "</center>"        << endl;
}

void MosaicWriter::procColorSet(QTextStream &ts, ColorSet * colorSet)
{
    int count = colorSet->size();
    colorSet->resetIndex();
    for (int i=0; i < count; i++)
    {
        TPColor tpcolor = colorSet->getNextColor();
        procColor(ts,tpcolor);
    }
}

void MosaicWriter::procColorGroup(QTextStream &ts, ColorGroup * colorGroup)
{
    int count = colorGroup->size();
    colorGroup->resetIndex();
    for (int i=0; i < count; i++)
    {
        ColorSet * cs = colorGroup->getNextColorSet();
        bool hide = cs->isHidden();
        QString qs = QString("<Group hideSet=\"%1\">").arg( (hide) ? "t" : "f");
        ts << qs << endl;
        procColorSet(ts,cs);
        ts << "</Group>" << endl;
    }
}

void MosaicWriter::processsStyleThick(QTextStream &ts, bool draw_outline, qreal width)
{
    QString draw = (draw_outline) ? "true" : "false";
    ts << "<draw__outline>" << draw << "</draw__outline>" << endl;
    ts << "<width>" << width << "</width>" << endl;
}

void MosaicWriter::processsStyleInterlace(QTextStream &ts, qreal gap, qreal shadow, bool includeTipVerts)
{
    QString include = (includeTipVerts) ? "true" : "false";
    ts << "<gap>" << gap << "</gap>" << endl;
    ts << "<shadow>" << shadow << "</shadow>" << endl;
    ts << "<includeTipVerts>" << include << "</includeTipVerts>" << endl;
}

void MosaicWriter::processsStyleFilled(QTextStream &ts, bool draw_inside, bool draw_outside, int algorithm, int cleanseLevel)
{
    QString drawi = (draw_inside) ? "true" : "false";
    QString drawo = (draw_outside) ? "true" : "false";
    ts << "<draw__inside>" << drawi << "</draw__inside>" << endl;
    ts << "<draw__outside>" << drawo << "</draw__outside>" << endl;
    ts << "<algorithm>" << algorithm << "</algorithm>" << endl;
    ts << "<cleanse>" << cleanseLevel << "</cleanse>" << endl;
}

void MosaicWriter::processsStyleEmboss(QTextStream &ts, qreal  angle)
{
    ts << "<angle>" << angle << "</angle>";
}

void MosaicWriter::setBoundary(QTextStream & ts, PolyPtr p)
{
    QString qsid;
    if (hasReference(p))
    {
        qsid = getPolyReference(p);
        ts << "<boundary" << qsid << "/>" << endl;
        return;
    }

    qsid = nextId();
    setPolyReference(getRef(),p);

    ts << "<boundary" << qsid << ">" << endl;

    ts << "<pts" << nextId() << ">" << endl;
    setPolygon(ts,p);
    ts << "</pts>" << endl;

    ts << "</boundary>" << endl;
}

void MosaicWriter::setPolygon(QTextStream & ts, PolyPtr pp)
{
    for (int i=0; i < pp->size(); i++)
    {
        QPointF qp = pp->at(i);
        ts << "<Point" << nextId() << ">" << endl;
        ts << "<x>"  << qp.x() << "</x>" << endl;
        ts << "<y>"  << qp.y() << "</y>" << endl;
        ts << "</Point>" << endl;
    }
}

void MosaicWriter::setPrototype(QTextStream & ts, PrototypePtr pp)
{
    QString qsid;
    if (hasReference(pp))
    {
        qsid = getProtoReference(pp);
        ts << "<prototype" << qsid << "/>" << endl;
        return;
    }

    qsid = nextId();
    setProtoReference(getRef(),pp);
    ts << "<prototype" << qsid << " serialization=\"custom\">" << endl;

    QString str = "app.Prototype";
    ts << "<" << str << ">" << endl;
    ts << "<string>" << pp->getTiling()->getName() << "</string>" << endl;

    QVector<DesignElementPtr>  dels = pp->getDesignElements();
    QVector<DesignElementPtr>::reverse_iterator it;
    for (it = dels.rbegin(); it != dels.rend(); it++)
    {
        DesignElementPtr de = *it;
        FeaturePtr feature = de->getFeature();
        FigurePtr  figure  = de->getFigure();

        if (!feature)
            fail("Feature not found in prototype","");
        if (!figure)
            fail("Figure not found in prototype", "");

        ts << "<entry>" << endl;

        setFeature(ts,feature);

        QString name;
        eFigType figType = figure->getFigType();
        switch (figType)
        {
        case FIG_TYPE_EXPLICIT:
            name = "app.ExplicitFigure";
            setExplicitFigure(ts,name,figure);
            break;

        case FIG_TYPE_STAR:
            name = "app.Star";
            setStarFigure(ts,name,figure);
            break;

        case FIG_TYPE_EXTENDED_STAR:
            name = "ExtendedStar";
            setExtendedStarFigure(ts,name,figure);
            break;

        case FIG_TYPE_EXTENDED_ROSETTE:
            name = "ExtendedRosette";
            setExtendedRosetteFigure(ts,name,figure);
            break;

        case FIG_TYPE_ROSETTE:
            name = "app.Rosette";
            setRosetteFigure(ts,name,figure);
            break;

        case FIG_TYPE_CONNECT_STAR:
            name = "app.ConnectFigure";
            setStarConnectFigure(ts,name,figure);
            break;

        case FIG_TYPE_CONNECT_ROSETTE:
            name = "app.ConnectFigure";
            setRosetteConnectFigure(ts,name,figure);
            break;

        case FIG_TYPE_RADIAL:
        case FIG_TYPE_UNDEFINED:
            fail("Unexpected figure type:", sFigType[figType]);

        case FIG_TYPE_EXPLICIT_INFER:
            name = "app.Infer";
            setExplicitFigure(ts,name,figure);
            break;

        case FIG_TYPE_EXPLICIT_ROSETTE:
            name = "app.ExplicitRosette";
            setExplicitFigure(ts,name,figure);
            break;

        case FIG_TYPE_EXPLICIT_HOURGLASS:
            name = "app.ExplicitHourglass";
            setExplicitFigure(ts,name,figure);
            break;

        case FIG_TYPE_EXPLICIT_INTERSECT:
            name = "app.ExplicitIntersect";
            setExplicitFigure(ts,name,figure);
            break;

        case FIG_TYPE_EXPLICIT_GIRIH:
            name = "app.ExplicitGirih";
            setExplicitFigure(ts,name,figure);
            break;

        case FIG_TYPE_EXPLICIT_STAR:
            name = "app.ExplicitStar";
            setExplicitFigure(ts,name,figure);
            break;

        case FIG_TYPE_EXPLICIT_FEATURE:
            name = "app.ExplicitFeature";
            setExplicitFigure(ts,name,figure);
            break;
        }
        ts << "</entry>" << endl;
    }

    ts << "</" << str << ">" << endl;
    ts << "</prototype>" << endl;
    qDebug() << "Proto created";
}

void MosaicWriter::setFeature(QTextStream & ts,FeaturePtr fp)
{
    QString str = "tile.Feature";

    QString qsid;
    if (hasReference(fp))
    {
        qsid = getFeatureReference(fp);
    }
    else
    {
        qsid = nextId();
        setFeatureReference(getRef(),fp);
    }

    ts << "<" << str << qsid << ">" << endl;
    if (fp->isRegular())
    {
        ts << "<regular>true</regular>" << endl;
    }
    else
    {
        ts << "<regular>false</regular>" << endl;
    }
    ts << "<rotation>" << fp->getRotation() << "</rotation>" << endl;
    ts << "<scale>" << fp->getScale() << "</scale>" << endl;

    ts << "<edges" << nextId() << ">" << endl;

    const EdgePoly & ep  = fp->getBase();
    setEdgePoly(ts,ep);

    ts << "</edges>" << endl;

    ts << "</" << str << ">" << endl;

}

void MosaicWriter::setFigureCommon(QTextStream & ts, FigurePtr fp)
{
    int    bs = fp->getExtBoundarySides();
    qreal bsc = fp->getExtBoundaryScale();
    qreal fsc = fp->getFigureScale();
    qreal   r = fp->getFigureRotate();

    ts << "<boundarySides>" << bs  <<"</boundarySides>"  << endl;
    ts << "<boundaryScale>" << bsc << "</boundaryScale>" << endl;
    ts << "<figureScale>"   << fsc << "</figureScale>"   << endl;
    ts << "<r>"             << r   << "</r>"             << endl;
}

void MosaicWriter::setExplicitFigure(QTextStream & ts,QString name, FigurePtr fp)
{
    ExplicitPtr ep = std::dynamic_pointer_cast<ExplicitFigure>(fp);
    if (!ep)
    {
        fail("Style error","dynamic cast of Explicit Figure");
    }

    QString qsid;
    if (hasReference(ep))
    {
        qsid = getExplicitReference(ep);
        ts << "<" << name << qsid << "/>" << endl;
        return;
   }

    qsid = nextId();
    setExplicitReference(getRef(),ep);
    ts << "<" << name << qsid << " type=\"" << fp->getFigTypeString() << "\"" << ">" << endl;

    switch(ep->getFigType())
    {
    case FIG_TYPE_UNDEFINED:
    case FIG_TYPE_RADIAL:
    case FIG_TYPE_ROSETTE:
    case FIG_TYPE_STAR:
    case FIG_TYPE_CONNECT_STAR:
    case FIG_TYPE_CONNECT_ROSETTE:
    case FIG_TYPE_EXTENDED_ROSETTE:
    case FIG_TYPE_EXTENDED_STAR:
        fail("Code Error","Not an explicit figure");

    case FIG_TYPE_EXPLICIT:
    {
        MapPtr map = ep->getFigureMap();
        setMap(ts,map);
        break;
    }

    case FIG_TYPE_EXPLICIT_INFER:
    case FIG_TYPE_EXPLICIT_FEATURE:
        // these have no parameters
        break;

    case FIG_TYPE_EXPLICIT_GIRIH:
        ts << "<sides>" << ep->getN() << "</sides>" << endl;
        ts << "<skip>"  << ep->skip  << "</skip>"  << endl;
        break;

    case FIG_TYPE_EXPLICIT_STAR:
    case FIG_TYPE_EXPLICIT_HOURGLASS:
        ts << "<s>" << ep->s << "</s>" << endl;
        ts << "<d>" << ep->d << "</d>"  << endl;
        break;

    case FIG_TYPE_EXPLICIT_ROSETTE:
        ts << "<s>" << ep->s << "</s>" << endl;
        ts << "<q>" << ep->q << "</q>"  << endl;
        ts << "<rFlexPt>" << ep->r_flexPt << "</rFlexPt>"  << endl;
        break;

    case FIG_TYPE_EXPLICIT_INTERSECT:
        ts << "<s>" << ep->s << "</s>" << endl;
        ts << "<sides>" << ep->getN() << "</sides>" << endl;
        ts << "<skip>"  << ep->skip  << "</skip>"  << endl;
        ts << "<progressive>" << ((ep->progressive) ? "t" : "f") << "</progressive>" << endl;
        break;
    }

    setFigureCommon(ts,fp);

    ts << "</" << name << ">" << endl;
}

void MosaicWriter::setStarFigure(QTextStream & ts, QString name, FigurePtr fp, bool childEnd)
{
    StarPtr sp = std::dynamic_pointer_cast<Star>(fp);
    if (!sp)
    {
        fail("Style error","dynamic cast of Star");
    }

    QString qsid;
    if (hasReference(sp))
    {
        qsid = getStarReference(sp);
    }
    else
    {
        qsid = nextId();
        setStarReference(getRef(),sp);
    }
    ts << "<" << name << qsid << ">" << endl;

    int        n = sp->getN();
    qreal      d = sp->getD();
    int        s = sp->getS();

    ts << "<n>" << n << "</n>" << endl;
    ts << "<d>" << d << "</d>" << endl;
    ts << "<s>" << s << "</s>" << endl;

    setFigureCommon(ts, fp);

    if (childEnd)
        ts << "</child>" << endl;
    else
        ts << "</" << name << ">" << endl;
}

void MosaicWriter::setExtendedStarFigure(QTextStream & ts,QString name, FigurePtr fp)
{
    ExtStarPtr sp = std::dynamic_pointer_cast<ExtendedStar>(fp);
    if (!sp)
    {
        fail("Style error","dynamic cast of ExtendedStar");
    }

    QString qsid;
    if (hasReference(sp))
    {
        qsid = getExtendedStarReference(sp);
    }
    else
    {
        qsid = nextId();
        setExtendedStarReference(getRef(),sp);
    }

    int        n = sp->getN();
    qreal      d = sp->getD();
    int        s = sp->getS();
    QString  ext_t     = (sp->getExtendPeripheralVertices())    ? "\"t\"" : "\"f\"";
    QString  ext_not_t = (sp->getExtendFreeVertices()) ? "\"t\"" : "\"f\"";

    ts << "<" << name << qsid << "  extendPeripherals=" << ext_t << "  extendFreeVertices=" << ext_not_t << ">" << endl;
    ts << "<n>" << n << "</n>" << endl;
    ts << "<d>" << d << "</d>" << endl;
    ts << "<s>" << s << "</s>" << endl;

    setFigureCommon(ts, fp);

    ts << "</" << name << ">" << endl;
}

void MosaicWriter::setRosetteFigure(QTextStream & ts,QString name, FigurePtr fp, bool childEnd)
{
    RosettePtr rp = std::dynamic_pointer_cast<Rosette>(fp);
    if (!rp)
    {
        fail("Style error","dynamic cast of Rosette");
    }

    QString qsid;
    if (hasReference(rp))
    {
        qsid = getRosetteReference(rp);
    }
    else
    {
        qsid = nextId();
        setRosetteReference(getRef(),rp);
    }
    ts << "<" << name << qsid << ">" << endl;

    int n       = rp->getN();
    int s       = rp->getS();
    qreal q     = rp->getQ();
    qreal k     = rp->getK();

    ts << "<n>" << n << "</n>" << endl;
    ts << "<q>" << q << "</q>" << endl;
    ts << "<s>" << s << "</s>" << endl;
    ts << "<k>" << k << "</k>" << endl;

    setFigureCommon(ts,fp);

    if (childEnd)
        ts << "</child>" << endl;
    else
        ts << "</" << name << ">" << endl;
}

void MosaicWriter::setExtendedRosetteFigure(QTextStream & ts,QString name, FigurePtr fp)
{
    ExtRosettePtr rp = std::dynamic_pointer_cast<ExtendedRosette>(fp);
    if (!rp)
    {
        fail("Style error","dynamic cast of ExtendedRosette");
    }

    QString qsid;
    if (hasReference(rp))
    {
        qsid = getExtendedRosetteReference(rp);
    }
    else
    {
        qsid = nextId();
        setExtendedRosetteReference(getRef(),rp);
    }

    int        n = rp->getN();
    qreal      q = rp->getQ();
    qreal      k = rp->getK();
    int        s = rp->getS();
    QString  ext_t     = (rp->getExtendPeripheralVertices())    ? "\"t\"" : "\"f\"";
    QString  ext_not_t = (rp->getExtendFreeVertices()) ? "\"t\"" : "\"f\"";
    QString  con_bnd_v = (rp->getConnectBoundaryVertices()) ? "\"t\"" : "\"f\"";

    ts << "<" << name << qsid << "  extendPeripherals=" << ext_t << "  extendFreeVertices=" << ext_not_t << "  connectBoundaryVertices=" << con_bnd_v << ">" << endl;
    ts << "<n>" << n << "</n>" << endl;
    ts << "<q>" << q << "</q>" << endl;
    ts << "<s>" << s << "</s>" << endl;
    ts << "<k>" << k << "</k>" << endl;

    setFigureCommon(ts,fp);

    ts << "</" << name << ">" << endl;
}

void MosaicWriter::setRosetteConnectFigure(QTextStream & ts,QString name, FigurePtr fp)
{
    qDebug() << fp->getFigureDesc();
    RosetteConnectPtr rcp = std::dynamic_pointer_cast<RosetteConnectFigure>(fp);
    if (!rcp)
    {
        fail("Style error","dynamic cast of RosetteConnectFigure");
    }

    QString qsid;
    if (hasReference(rcp))
    {
        qsid = getRosetteConnectReference(rcp);
    }
    else
    {
        qsid = nextId();
        setRosetteConnectReference(getRef(),rcp);
    }
    ts << "<" << name << qsid << ">" << endl;


    int n = rcp->getN();

    ts << "<n>" << n << "</n>" << endl;

    setRosetteFigure(ts,QString("child class=\"app.Rosette\""),rcp,true);

    qreal s2 = rcp->getFigureScale();
    ts << "<s>" << s2 << "</s>" << endl;
    ts << "</" << name << ">" << endl;
}

void MosaicWriter::setStarConnectFigure(QTextStream & ts,QString name, FigurePtr fp)
{
    qDebug() << fp->getFigureDesc();
    StarConnectPtr scp = std::dynamic_pointer_cast<StarConnectFigure>(fp);
    if (!scp)
    {
        fail("Style error","dynamic cast StarConnectFigure");
    }

    QString qsid;
    if (hasReference(scp))
    {
        qsid = getStarConnectReference(scp);
    }
    else
    {
        qsid = nextId();
        setStarConnectReference(getRef(),scp);
    }
    ts << "<" << name << qsid << ">" << endl;


    int n = scp->getN();

    ts << "<n>" << n << "</n>" << endl;

    setStarFigure(ts,QString("child class=\"app.Star\""),scp,true);

    qreal s2 = scp->getFigureScale();
    ts << "<s>" << s2 << "</s>" << endl;
    ts << "</" << name << ">" << endl;
}

bool MosaicWriter::setMap(QTextStream &ts, MapPtr map)
{
    qDebug().noquote() << map->summary();

    bool verify = map->verify();
    if (!verify)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        box.setText("XML Writer: Map verify failed/n/nProceed to cleanse ?");
        int ret = box.exec();
        switch (ret)
        {
        case QMessageBox::Yes :
            verify = map->cleanse(default_cleanse);     // always write a good map
            break;

        case QMessageBox::No :
            verify = true;
            break;
        }
        if (!verify)
        {
            QMessageBox box;
            box.setIcon(QMessageBox::Warning);
            box.setText("XML Writer: Map verify failed after cleanse");
            box.exec();
            return false;
        }
    }

    QString qsid;

    if (hasReference(map))
    {
        qsid = getMapReference(map);
        ts << "<map" << qsid << "/>" << endl;
        return true;
    }

    qsid = nextId();
    setMapReference(getRef(),map);
    ts << "<map" << qsid << ">" << endl;

    // vertices
    const QVector<VertexPtr> & vertices = map->getVertices();
    setVertices(ts,vertices);

    // Edges
    const QVector<EdgePtr> & edges = map->getEdges();
    setEdges(ts,edges);

    ts << "</map>" << endl;

    return true;
}


void MosaicWriter::setVertices(QTextStream & ts, const QVector<VertexPtr> & vertices)
{
    ts << "<vertices" << nextId() <<  ">" << endl;
    for (const auto & v : vertices)
    {
        setVertex(ts,v);
    }
    ts << "</vertices>" << endl;
}

void MosaicWriter::setEdges(QTextStream & ts, const QVector<EdgePtr> & edges)
{
    ts << "<edges" << nextId() <<  ">" << endl;
    for (const auto & edge : edges)
    {
        setEdge(ts,edge);
    }
    ts << "</edges>" << endl;
}

void MosaicWriter::setEdgePoly(QTextStream & ts, const EdgePoly & epoly)
{
    for (auto it = epoly.begin(); it != epoly.end(); it++)
    {
        EdgePtr ep = *it;
        VertexPtr v1 = ep->v1;
        VertexPtr v2 = ep->v2;
        if (ep->getType() == EDGETYPE_LINE)
        {
            ts << "<Line>" << endl;
            VertexPtr v1 = ep->v1;
            VertexPtr v2 = ep->v2;
            setVertexEP(ts,v1,"Point");
            setVertexEP(ts,v2,"Point");
            ts << "</Line>" << endl;
        }
        else if (ep->getType() == EDGETYPE_CURVE)
        {
            QString str = QString("<Curve convex=\"%1\">").arg(ep->isConvex() ? "t" : "f");
            ts << str << endl;
            QPointF p3 = ep->getArcCenter();
            setVertexEP(ts,v1,"Point");
            setVertexEP(ts,v2,"Point");
            setPoint(ts,p3,"Center");
            ts << "</Curve>" << endl;
        }
    }
}

void MosaicWriter::setVertexEP(QTextStream & ts,VertexPtr v, QString name)
{
    QString qsid;
    if (hasReference(v))
    {
        qsid = getVertexReference(v);
        ts << "<" << name << qsid << "/>" << endl;
        return;
    }

    qsid = nextId();
    setVertexReference(getRef(),v);

    QPointF pt = v->pt;

    ts << "<" << name << qsid << ">";
    ts << pt.x() << "," << pt.y();
    ts << "</" << name << ">" << endl;
}

void MosaicWriter::setPoint(QTextStream & ts, QPointF pt, QString name)
{
    ts << "<" << name << ">";
    ts << pt.x() << "," << pt.y();
    ts << "</" << name << ">" << endl;
}

void MosaicWriter::setVertex(QTextStream & ts, VertexPtr v, QString name)
{
    QString qsid;
    if (hasReference(v))
    {
        qsid = getVertexReference(v);
        ts << "<" << name << qsid << "/>" << endl;
        return;
    }

    qsid = nextId();
    setVertexReference(getRef(),v);
    ts << "<" << name << qsid << ">" << endl;

    // pos
    setPos(ts,v->pt);

    ts << "</" << name << ">" << endl;
}

void MosaicWriter::setEdges(QTextStream & ts, QVector<EdgePtr> & qvec)
{
    ts << "<edges>" << endl;

    for (auto it = qvec.begin(); it != qvec.end(); it++)
    {
        // edge
        EdgePtr e = *it;
        setEdge(ts,e);      // called from setNeighbour first time
    }

    ts << "</edges>" << endl;
 }


void MosaicWriter::setEdge(QTextStream & ts, EdgePtr e)
{
    qDebug() << "Edge" << e.get();
    QString qsid;
    if (hasReference(e))
    {
        qsid = getEdgeReference(e);
        ts << "<Edge" << qsid << "/>" << endl;
    }


    bool curved = (e->getType() == EDGETYPE_CURVE);

    qsid = nextId();
    qDebug() << "new edge ref=" << getRef();
    setEdgeReference(getRef(),e);

    if (curved)
    {
        QString str = QString("<curve %1 convex=\"%2\">").arg(qsid).arg(e->isConvex() ? "t" : "f");
        ts << str << endl;
    }
    else
    {
        ts << "<Edge" << qsid << ">" << endl;
    }

    // v1
    VertexPtr v1 = e->v1;
    setVertex(ts,v1,"v1");

    // v2
    VertexPtr v2 = e->v2;
    setVertex(ts,v2,"v2");

    if (curved)
    {
        QPointF p = e->getArcCenter();
        setPos(ts,p);
    }

    if (curved)
    {
        ts << "</curve>" << endl;
    }
    else
    {
        ts << "</Edge>" << endl;
    }
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

bool MosaicWriter::hasReference(PolyPtr pp)
{
    return poly_ids.contains(pp);
}

bool MosaicWriter::hasReference(PrototypePtr pp)
{
    return proto_ids.contains(pp);
}

bool MosaicWriter::hasReference(FeaturePtr fp)
{
    return feature_ids.contains(fp);
}

bool MosaicWriter::hasReference(FigurePtr fp)
{
    return figure_ids.contains(fp);
}

bool MosaicWriter::hasReference(ExplicitPtr ep)
{
    return explicit_ids.contains(ep);
}

bool MosaicWriter::hasReference(MapPtr map)
{
    return map_ids.contains(map);
}

bool MosaicWriter::hasReference(VertexPtr v)
{
    return vertex_ids.contains(v);
}

bool MosaicWriter::hasReference(RosettePtr n)
{
    return rosette_ids.contains(n);
}

bool MosaicWriter::hasReference(StarPtr n)
{
    return star_ids.contains(n);
}

bool MosaicWriter::hasReference(ExtStarPtr n)
{
    return extended_star_ids.contains(n);
}

bool MosaicWriter::hasReference(ExtRosettePtr n)
{
    return extended_rosette_ids.contains(n);
}

bool MosaicWriter::hasReference(RosetteConnectPtr n)
{
    return rosette_connect_ids.contains(n);
}

bool MosaicWriter::hasReference(StarConnectPtr n)
{
    return star_connect_ids.contains(n);
}

bool MosaicWriter::hasReference(EdgePtr e)
{
    return edge_ids.contains(e);
}

void MosaicWriter::setProtoReference(int id, PrototypePtr ptr)
{
    proto_ids[ptr] = id;
}

void MosaicWriter::setPolyReference(int id, PolyPtr ptr)
{
    poly_ids[ptr] = id;
}

void MosaicWriter::setFigureReference(int id, FigurePtr ptr)
{
   figure_ids[ptr] = id;
}

void MosaicWriter::setFeatureReference(int id,FeaturePtr ptr)
{
    feature_ids[ptr] = id;
}

void MosaicWriter::setExplicitReference(int id,ExplicitPtr ptr)
{
    explicit_ids[ptr] = id;
}

void MosaicWriter::setMapReference(int id, MapPtr ptr)
{
    map_ids[ptr] = id;
}

void MosaicWriter::setVertexReference(int id, VertexPtr ptr)
{
    vertex_ids[ptr] = id;
}

void MosaicWriter::setRosetteReference(int id, RosettePtr ptr)
{
    rosette_ids[ptr] = id;
}

void MosaicWriter::setStarReference(int id, StarPtr ptr)
{
    star_ids[ptr] = id;
}

void MosaicWriter::setExtendedStarReference(int id, ExtStarPtr ptr)
{
    extended_star_ids[ptr] = id;
}

void MosaicWriter::setExtendedRosetteReference(int id, ExtRosettePtr ptr)
{
    extended_rosette_ids[ptr] = id;
}

void MosaicWriter::setRosetteConnectReference(int id, RosetteConnectPtr ptr)
{
    rosette_connect_ids[ptr] = id;
}

void MosaicWriter::setStarConnectReference(int id, StarConnectPtr ptr)
{
    star_connect_ids[ptr] = id;
}

void MosaicWriter::setEdgeReference(int id, EdgePtr ptr)
{
    edge_ids[ptr] = id;
}

QString MosaicWriter::getPolyReference(PolyPtr ptr)
{
    int id =  poly_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getProtoReference(PrototypePtr ptr)
{
    int id =  proto_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getFeatureReference(FeaturePtr ptr)
{
    int id =  feature_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getFigureReference(FigurePtr ptr)
{
    int id =  figure_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getExplicitReference(ExplicitPtr ptr)
{
    int id =  explicit_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getMapReference(MapPtr ptr)
{
    int id =  map_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getVertexReference(VertexPtr ptr)
{
    int id =  vertex_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getRosetteReference(RosettePtr ptr)
{
    int id =  rosette_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getStarReference(StarPtr ptr)
{
    int id =  star_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getExtendedStarReference(ExtStarPtr ptr)
{
    int id =  extended_star_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getExtendedRosetteReference(ExtRosettePtr ptr)
{
    int id =  extended_rosette_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getRosetteConnectReference(RosetteConnectPtr ptr)
{
    int id =  rosette_connect_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getStarConnectReference(StarConnectPtr ptr)
{
    int id =  star_connect_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}


QString MosaicWriter::getEdgeReference(EdgePtr ptr)
{
    int id =  edge_ids.value(ptr);
    qDebug() << "edge ref" << id;
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString  MosaicWriter::id(int id)
{
    qDebug() << "id=" << id;
    QString qs = QString(" id=\"%1\"").arg(id);
    return qs;
}

QString  MosaicWriter::nextId()
{
    return id(++refId);
}

void MosaicWriter::setPos(QTextStream & ts,QPointF qpf)
{
    ts << "<pos>";
    ts << qpf.x() << "," << qpf.y();
    ts << "</pos>" << endl;
}

void MosaicWriter::fail(QString a, QString b)
{
    _failMsg = QString("%1 %2").arg(a).arg(b);
    qWarning().noquote() << _failMsg;
    throw(_failMsg);
}
