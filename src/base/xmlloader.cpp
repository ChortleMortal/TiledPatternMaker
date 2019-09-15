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

#include "xmlloader.h"
#include "base/canvas.h"
#include "base/border.h"
#include "base/configuration.h"
#include "base/tilingmanager.h"
#include "base/utilities.h"
#include "base/workspace.h"
#include "style/Colored.h"
#include "style/Thick.h"
#include "style/Filled.h"
#include "style/Interlace.h"
#include "style/Outline.h"
#include "style/Plain.h"
#include "style/Sketch.h"
#include "style/Emboss.h"
#include "style/TileColors.h"
#include "tapp/Star.h"
#include "tapp/ExtendedStar.h"
#include "tapp/ExplicitFigure.h"
#include "tile/featurereader.h"

#undef  DEBUG_REFERENCES

XmlLoader::XmlLoader(StyledDesign & styledDesign) : _styledDesign(styledDesign)
{
    qDebug() << "Constructing XML LOADER";

    // defaults
    _background = QColor(Qt::white);
    _width      = 1500;
    _height     = 1100;
    _scale      = 1.0;
    _version    = 0;
    _border     = nullptr;
    canvas      = Canvas::getInstance();
}

XmlLoader::~XmlLoader()
{
    qDebug() << "Destroying XML LOADER";
}

QString XmlLoader::getLoadedFilename()
{
    return _fileName;
}

bool XmlLoader::load(QString fileName)
{
    canvas->dump(true);

    qDebug().noquote() << "XmlLoader loading:" << fileName;
    _fileName = fileName;

    xml_document doc;
    xml_parse_result result = doc.load_file(fileName.toStdString().c_str());

    if (result == false)
    {
        _failMessage = result.description();
        qWarning().noquote() << _failMessage;
        return false;
    }

    try
    {
        parseXML(doc);

        canvas->dump(true);

        return true;
    }
    catch (...)
    {
        qWarning() << "ERROR processing XML file"  << fileName;
        return false;
    }
}

void XmlLoader::parseXML(xml_document & doc)
{
    vOrigCnt = 0;
    vRefrCnt = 0;
    eOrigCnt = 0;
    eRefrCnt = 0;
    nRefrCnt = 0;
    qDebug() << "XML LOADER - start parsing";

    for (xml_node n = doc.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();
        if (str  == "vector")
            processVector(n);
        else
            fail("Unexpected", str.c_str());
    }

    canvas->dump(true);

    qDebug() << "XML LOADER - end parsing";
}

void XmlLoader::processDesignNotes(xml_node & node)
{
    string str = node.child_value();
    _styledDesign.setNotes(str.c_str());
}

void XmlLoader::processVector(xml_node & node)
{

    xml_attribute attr = node.attribute("version");
    if (attr)
    {
        QString str = attr.value();
        _version = str.toUInt();
    }

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();
        if (str  == "designNotes")
            processDesignNotes(n);
        else if (str == "Tiling")
        {
            TilingLoader tm;
            _tiling = tm.readTilingXML(n);
        }
        else if (str == "design")
            processDesign(n);
        else if (str == "style.Thick")
            processThick(n);
        else if (str == "style.Filled")
            processFilled(n);
        else if (str == "style.Interlace")
            processInterlace(n);
        else if (str == "style.Outline")
            processOutline(n);
        else if (str == "style.Plain")
            processPlain(n);
        else if (str == "style.Sketch")
            processSketch(n);
        else if (str == "style.Emboss")
            processEmboss(n);
        else if (str == "style.TileColors")
            processTileColors(n);
        else
            fail("Unexpected", str.c_str());
    }
    qDebug() << "end vector";

    CanvasSettings info;
    info.setScale(_scale);
    info.setBackgroundColor(_background);
    info.setSizeF(QSizeF(_width,_height));
    info.setBorder(_border);
    _styledDesign.setupCanvas(info);

}

void XmlLoader::processDesign(xml_node & node)
{
    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "scale")
            procScale(n);
        else if (str == "size")
            procSize(n, _width, _height);
        else if (str == "background")
            _background = procBackground(n);
        else if (str == "border")
            procBorder(n);
        else
            fail("Unexpected", str.c_str());
    }
}

void XmlLoader::processThick(xml_node & node)
{
    QColor  color;
    bool    draw_outline = false;
    qreal   width        = 0.0;
    PrototypePtr proto;
    PolyPtr poly;
    Xform   xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf);
        else if (str == "style.Style")
            processStyleStyle(n,proto,poly);
        else if (str == "style.Colored")
            processColorSet(n,color);
        else if (str == "style.Thick")
            processsStyleThick(n,draw_outline,width);
        else
            fail("Unexpected", str.c_str());
    }

    qDebug() << "Constructing Thick from prototype and poly";
    Thick * thick = new Thick(proto,poly);
    thick->setDeltas(xf);
    thick->setColor(color);
    thick->setDrawOutline(draw_outline);
    thick->setLineWidth(width);

    qDebug().noquote() << "XmlServices created Style(Thick)" << thick->getInfo();
    _styledDesign.addStyle(StylePtr(thick));

    qDebug() << "end thick";
}

void XmlLoader::processInterlace(xml_node & node)
{
    QColor  color;
    PrototypePtr proto;
    PolyPtr poly;
    bool    draw_outline    = false;
    bool    includeTipVerts   = false;
    qreal   width           = 0.0;
    qreal   gap             = 0.0;
    qreal   shadow          = 0.0;
    Xform   xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf);
        else if (str == "style.Style")
            processStyleStyle(n,proto,poly);
        else if (str == "style.Colored")
            processColorSet(n,color);
        else if (str == "style.Thick")
            processsStyleThick(n,draw_outline,width);
        else if (str == "style.Interlace")
            processsStyleInterlace(n,gap,shadow,includeTipVerts);
        else
            fail("Unexpected", str.c_str());
    }

    qDebug() << "Constructing Interlace (DAC) from prototype and poly";
    Interlace * interlace = new Interlace(proto,poly);
    interlace->setDeltas(xf);
    interlace->setColor(color);
    interlace->setDrawOutline(draw_outline);
    interlace->setLineWidth(width);
    interlace->setGap(gap);
    interlace->setShadow(shadow);
    interlace->setIncludeTipVertices(includeTipVerts);
    qDebug().noquote() << "XmlServices created Style(Interlace)" << interlace->getInfo();
    _styledDesign.addStyle(StylePtr(interlace));

    qDebug() << "end interlace";
}

void XmlLoader::processOutline(xml_node & node)
{
    QColor  color;
    bool    draw_outline = false;
    qreal   width        = 0.0;
    PrototypePtr proto;
    PolyPtr poly;
    Xform   xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf);
        else if (str == "style.Style")
            processStyleStyle(n,proto,poly);
        else if (str == "style.Colored")
            processColorSet(n,color);
        else if (str == "style.Thick")
            processsStyleThick(n,draw_outline,width);
        else
            fail("Unexpected", str.c_str());
    }

    qDebug() << "Constructing Outline from prototype and poly";
    Outline * outline = new Outline(proto,poly);
    outline->setDeltas(xf);
    outline->setColor(color);
    outline->setDrawOutline(draw_outline);
    outline->setLineWidth(width);

    qDebug().noquote() << "XmlServices created Style(Outline)" << outline->getInfo();
    _styledDesign.addStyle(StylePtr(outline));

    qDebug() << "end outline";
}

void XmlLoader::processFilled(xml_node & node)
{
    int     algorithm   = 0;
    bool    oldFormat   = false;

    ColorSet colorSet;
    ColorSet colorsB;
    ColorSet colorsW;
    ColorGroup colorGroup;

    bool    draw_inside  = false;
    bool    draw_outside = true;

    PrototypePtr proto;
    PolyPtr poly;

    Xform xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "toolkit.GeoLayer")
        {
            procesToolkitGeoLayer(n,xf);
        }
        else if (str == "style.Style")
        {
            processStyleStyle(n,proto,poly);
        }
        else if (str == "style.Colored")
        {
            oldFormat = true;
            processColorSet(n,colorSet);
        }
        else if (str == "style.Filled")
        {
            processsStyleFilled(n,draw_inside,draw_outside,algorithm);
        }
        else if (str == "ColorBlacks")
        {
            oldFormat = false;
            processColorSet(n,colorsB);
        }
        else if (str == "ColorWhites")
        {
            oldFormat = false;
            processColorSet(n,colorsW);
        }
        else if (str == "ColorGroup")
        {
            oldFormat = false;
            processColorGroup(n,colorGroup);
        }
        else
        {
            fail("Unexpected", str.c_str());
        }
    }

    Filled * filled = new Filled(proto,poly,algorithm);
    filled->setDeltas(xf);

    if (oldFormat)
    {
        // old - redundant way of dealing with colors
        ColorSet & csetW = filled->getWhiteColorSet();
        csetW.setColor(colorSet.getColor(0));

        ColorSet & csetB = filled->getBlackColorSet();
        if (colorSet.size() >= 2)
            csetB.setColor(colorSet.getColor(1));
        else
            csetB.setColor(colorSet.getColor(0));
    }
    else
    {
        ColorSet & csetW = filled->getWhiteColorSet();
        csetW.setColors(colorsW);

        ColorSet & csetB = filled->getBlackColorSet();
        csetB.setColors(colorsB);

        ColorGroup & cgroup = filled->getColorGroup();
        cgroup = colorGroup;
#if 1
        if (cgroup.size()== 0)
        {
            // for backwards compatabililts
            ColorSet csw;
            if (colorsW.size())
            {
                csw.setColors(colorsW);
                cgroup.addColorSet(csw);
            }
            ColorSet csb;
            if (colorsB.size())
            {
                csw.setColors(colorsB);
                cgroup.addColorSet(csb);
            }
        }
        if (cgroup.size()== 0)
        {
            // last resort
            ColorSet cs;
            cs.setColor(Qt::black);
            cgroup.addColorSet(cs);
        }
#endif
    }

    filled->setDrawInsideBlacks(draw_inside);
    filled->setDrawOutsideWhites(draw_outside);

    qDebug().noquote() << "XmlServices created Style(Filled)" << filled->getInfo();
    _styledDesign.addStyle(StylePtr(filled));

    qDebug() << "end filled";
}

void XmlLoader::processPlain(xml_node & node)
{
    QColor  color;
    PrototypePtr proto;
    PolyPtr poly;
    Xform   xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf);
        else if (str == "style.Style")
            processStyleStyle(n,proto,poly);
        else if (str == "style.Colored")
            processColorSet(n,color);
        else
            fail("Unexpected", str.c_str());
    }

    Plain * plain = new Plain(proto,poly);
    plain->setDeltas(xf);
    plain->setColor(color);
    qDebug().noquote() << "XmlServices created Style (Plain)" << plain->getInfo();
    _styledDesign.addStyle(StylePtr(plain));

    qDebug() << "end plain";
}

void XmlLoader::processSketch(xml_node & node)
{
    QColor  color;
    PrototypePtr proto;
    PolyPtr poly;
    Xform   xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf);
        else if (str == "style.Style")
            processStyleStyle(n,proto,poly);
        else if (str == "style.Colored")
            processColorSet(n,color);
        else
            fail("Unexpected", str.c_str());
    }

    Sketch * sketch = new Sketch(proto,poly);
    sketch->setDeltas(xf);
    sketch->setColor(color);
    qDebug().noquote() << "XmlServices created Style (Sketch)" << sketch->getInfo();
    _styledDesign.addStyle(StylePtr(sketch));

    qDebug() << "end sketch";
}

void XmlLoader::processEmboss(xml_node & node)
{
    QColor  color;
    bool    draw_outline = false;
    qreal   width        = 0.0;
    qreal   angle        = 0.0;
    PrototypePtr proto;
    PolyPtr poly;
    Xform   xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf);
        else if (str == "style.Style")
            processStyleStyle(n,proto,poly);
        else if (str == "style.Colored")
            processColorSet(n,color);
        else if (str == "style.Thick")
            processsStyleThick(n,draw_outline,width);
        else if (str == "style.Emboss")
            processsStyleEmboss(n,angle);
        else
            fail("Unexpected", str.c_str());
    }

    qDebug() << "Constructing Emboss from prototype and poly";
    Emboss * emboss = new Emboss(proto,poly);
    emboss->setDeltas(xf);
    emboss->setColor(color);
    emboss->setDrawOutline(draw_outline);
    emboss->setLineWidth(width);
    emboss->setAngle(angle);

    qDebug().noquote() << "XmlServices created Style(Emboss)" << emboss->getInfo();
    _styledDesign.addStyle(StylePtr(emboss));

    qDebug() << "end emboss";
}

void XmlLoader::processTileColors(xml_node & node)
{
    PrototypePtr proto;
    PolyPtr poly;
    Xform   xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf);
        else if (str == "style.Style")
            processStyleStyle(n,proto,poly);
        else
            fail("Unexpected", str.c_str());
    }

    qDebug() << "Constructing TileColors from prototype and poly";
    TileColors * tc  = new TileColors(proto,poly);
    tc->setDeltas(xf);

    qDebug().noquote() << "XmlServices created Style(TileColors)" << tc->getInfo();
    _styledDesign.addStyle(StylePtr(tc));

    qDebug() << "end emboss";
}

void XmlLoader::procesToolkitGeoLayer(xml_node & node, Xform & xf)
{
    QString val;

    xml_node n = node.child("left__delta");
    if (n)
    {
        val           = n.child_value();
        xf.translateX = val.toDouble();
    }

    n = node.child("top__delta");
    if (n)
    {
        val           = n.child_value();
        xf.translateY = val.toDouble();
    }

    n = node.child("width__delta");
    if (n)
    {
        val          = n.child_value();
        xf.scale     = val.toDouble() + 1.0;     //used as scale
    }

    n = node.child("theta__delta");
    if (n)
    {
        val          = n.child_value();
        xf.rotation  = val.toDouble();
    }
}

void XmlLoader::processStyleStyle(xml_node & node, PrototypePtr & proto, PolyPtr & poly)
{
    xml_node n   = node.child("boundary");
    qDebug() << n.name();
    poly = getBoundary(n);

    n = node.child("prototype");
    qDebug() << n.name();
    canvas->dump(true);
    proto = getPrototype(n);
    canvas->dump(true);
    qDebug().noquote() << proto->getInfo();
}

void XmlLoader::processColorSet(xml_node & node, QColor & color)
{
    xml_node n   = node.child("color");
    color        = processColor(n);
}

void XmlLoader::processColorSet(xml_node & node, ColorSet &colorSet)
{
    bool hide = false;
    xml_attribute attr = node.attribute("hideSet");
    if (attr)
    {
        QString str = attr.value();
        if (str == "t")
        {
            hide = true;
        }
    }
    colorSet.hide(hide);

    xml_node n;
    for (n = node.child("color"); n; n = n.next_sibling("color"))
    {
        bool hide = false;
        if (_version >= 1)
        {
            xml_attribute attr = n.attribute("hide");
            if (attr)
            {
                QString str = attr.value();
                hide = (str == "t");
            }
        }
        QColor color = processColor(n);
        if (_version == 0)
        {
            qreal alpha = color.alphaF();
            if (alpha < 1.0)
            {
                hide = true;
            }
        }
        colorSet.addColor(color,hide);
    }
}

void XmlLoader::processColorGroup(xml_node & node, ColorGroup &colorGroup)
{
    xml_node n;
    for (n = node.child("Group"); n; n = n.next_sibling("Group"))
    {
        ColorSet cset;
        processColorSet(n,cset);
        colorGroup.addColorSet(cset);
    }
}

QColor XmlLoader::processColor(xml_node & n)
{
    QString str = n.child_value();
    QColor color(str);
    return color;
}

qreal XmlLoader::procWidth(xml_node & node)
{
    QString str = node.child_value();
    return str.toDouble();
}

void XmlLoader::processsStyleThick(xml_node & node, bool & draw_outline, qreal & width)
{
    QString str  = node.child_value("draw__outline");
    draw_outline = (str == "true");

    QString w = node.child_value("width");
    width = w.toDouble();
 }

void XmlLoader::processsStyleInterlace(xml_node & node, qreal & gap, qreal & shadow, bool & includeTipVerts)
{
    QString str  = node.child_value("gap");
    gap =  str.toDouble();

    str = node.child_value("shadow");
    shadow = str.toDouble();

    str = node.child_value("includeTipVerts");
    includeTipVerts = (str == "true");
}

void XmlLoader::processsStyleFilled(xml_node & node, bool & draw_inside, bool & draw_outside, int & algorithm)
{
    QString str;
    str = node.child_value("draw__inside");
    draw_inside  = (str == "true");

    str = node.child_value("draw__outside");
    draw_outside  = (str == "true");

    xml_node n = node.child("algorithm");
    if (n)
    {
        QString algo = n.child_value();
        algorithm = algo.toInt();
    }
}

void XmlLoader::processsStyleEmboss(xml_node & node, qreal & angle)
{
    QString str  = node.child_value("angle");
    angle =  str.toDouble();
}

PolyPtr XmlLoader::getBoundary(xml_node & node)
{
    if (hasReference(node))
    {
        return getPolyReferencedPtr(node);
    }

    xml_node poly = node.child("pts");

    PolyPtr b = getPolygon(poly);
    setPolyReference(node,b);
    return b;
}

PolyPtr XmlLoader::getPolygon(xml_node & node)
{
    PolyPtr poly = make_shared<QPolygonF>();
    xml_node point;
    for (point = node.child("Point"); point; point = point.next_sibling("Point"))
    {
        QString x = point.child_value("x");
        QString y = point.child_value("y");
        poly->push_back(QPointF(x.toDouble(),y.toDouble()));
    }
    return poly;
}

QPointF XmlLoader::getPos(xml_node & node)
{
    QString txt = node.child_value();
    QStringList qsl;
    qsl = txt.split(',');
    qreal x = qsl[0].toDouble();
    qreal y = qsl[1].toDouble();
    return QPointF(x,y);
}

PrototypePtr XmlLoader::getPrototype(xml_node & node)
{
    qDebug() << node.name();
    if (hasReference(node))
    {
        return getProtoReferencedPtr(node);
    }

    xml_node protonode = node.child("app.Prototype");
    qDebug() << protonode.name();

    QString tilingName = protonode.child_value("string");
    qDebug() << "string=" << tilingName;

    if (_tiling && _tiling->getName() == tilingName)
    {
        qDebug() << "Using tiling from styled design";
    }
    else
    {
        TilingManager * tm = TilingManager::getInstance();
        qDebug() << "loading named tiling" << tilingName;
        _tiling = tm->loadTiling(tilingName);
        if (!_tiling)
        {
            fail("Tiling not loaded: ",tilingName);
        }
    }

    //qDebug().noquote() << _tiling->dump();
    qDebug() << "Creating new prototype";

    PrototypePtr proto = make_shared<Prototype>(_tiling);
    setProtoReference(node,proto);
    canvas->dump(true);

    xml_node entry;
    for (entry = protonode.child("entry"); entry; entry = entry.next_sibling("entry"))
    {
        bool dc_found = false;

        FeaturePtr feature;
        FigurePtr  figure;
        QString name;

        xml_node xmlfeature = entry.first_child();
        name = xmlfeature.name();
        qDebug() << name;
        if ( name == "tile.Feature")
        {
            qDebug() << "adding Feature";
            feature = getFeature(xmlfeature);
        }
        else
        {
           fail("feature not found", "");
        }

        canvas->dump(true);
        xml_node xmlfigure  = xmlfeature.next_sibling();
        name = xmlfigure.name();
        qDebug() << name;
        if (name == "app.ExplicitFigure")
        {
            qDebug() << "adding ExplicitFigure";
            figure = getExplicitFigure(xmlfigure, FIG_TYPE_EXPLICIT);
            dc_found = true;
            canvas->dump(true);
        }
        else if (name == "app.Star")
        {
            qDebug() << "adding Star Figure";
            figure =  getStarFigure(xmlfigure);
            dc_found = true;
        }
        else if (name == "ExtendedStar")
        {
            qDebug() << "adding ExtendedStar Figure";
            figure =  getExtendedStarFigure(xmlfigure);
            dc_found = true;
        }
        else if (name == "app.Rosette")
        {
            qDebug() << "adding Rosette Figure";
            figure =  getRosetteFigure(xmlfigure);
            dc_found = true;
        }
        else if (name == "ExtendedRosette")
        {
            qDebug() << "adding ExtendedRosette Figure";
            figure =  getExtendedRosetteFigure(xmlfigure);
            dc_found = true;
        }
        else if (name == "app.ConnectFigure")
        {
            qDebug() << "adding Connect Figure";
            figure =  getRosetteConnectFigure(xmlfigure);
            dc_found = true;
        }
        else if (name == "app.Infer")
        {
            qDebug() << "adding Infer";
            figure = getExplicitFigure(xmlfigure,FIG_TYPE_INFER);
            dc_found = true;
            canvas->dump(true);
        }
        else if (name == "app.ExplicitFeature")
        {
            qDebug() << "adding Infer";
            figure = getExplicitFigure(xmlfigure,FIG_TYPE_FEATURE);
            dc_found = true;
            canvas->dump(true);
        }
        else
        {
            fail("Figure not found:", name);
        }

        canvas->dump(true);
        if (dc_found)
        {
            // if the found feature is identical to the one in the known tiling
            // then use that
            // DAC 27MAY17 - imprtant that this code not removed or Design View will fail
            for (auto it = _tiling->getPlacedFeatures().begin(); it != _tiling->getPlacedFeatures().end(); it++)
            {
                PlacedFeaturePtr pf = *it;
                FeaturePtr       fp = pf->getFeature();
                if (fp->equals(feature))
                {
                    feature = fp;
                }
            }
            qDebug() << "adding to Proto" << figure->getFigureDesc();
            DesignElementPtr  dep = make_shared<DesignElement>(feature, figure);
            proto->addElement(dep);
            qDebug() << "design element:" << dep->toString();
        }
        //p->walk();
    }

    proto->createProtoMap();
    qDebug() << "Proto created";
    return proto;
}

FeaturePtr XmlLoader::getFeature(xml_node & node)
{
    if (hasReference(node))
    {
        FeaturePtr f = getFeatureReferencedPtr(node);
        return f;
    }

    QString str;
    str = node.child_value("regular");
    bool regular = (str == "true");

    xml_node poly = node.child("points");
    FeaturePtr f;
    if (poly)
    {
        PolyPtr b    = getPolygon(poly);
        EdgePoly ep(b);

        f = make_shared<Feature>(ep);
        setFeatureReference(node,f);
        f->setRegular(regular);
        return f;
    }
    poly = node.child("edges");
    if (poly)
    {
        FeatureReader fr;
        EdgePoly ep = fr.getEdgePoly(poly);

        f = make_shared<Feature>(ep);
        setFeatureReference(node,f);
        f->setRegular(regular);
        return f;
    }
    return f;
}

void XmlLoader::getFigureCommon(xml_node & node, FigurePtr fig)
{
    QString str;
    if (node.child("boundarySides"))
    {
        str = node.child_value("boundarySides");
        int bsides = str.toInt();
        fig->setExtBoundarySides(bsides);
    }

    if (node.child("boundaryScale"))
    {
        str = node.child_value("boundaryScale");
        qreal bscale = str.toDouble();
        fig->setExtBoundaryScale(bscale);
    }

    if (node.child("figureScale"))
    {
        str = node.child_value("figureScale");
        qreal fscale = str.toDouble();
        fig->setFigureScale(fscale);
    }
}

ExplicitPtr XmlLoader::getExplicitFigure(xml_node & node, eFigType figType)
{
    if (hasReference(node))
    {
        ExplicitPtr f = getExplicitReferencedPtr(node);
        return f;
    }

    MapPtr map = getMap(node);
    map->verify("XML Explicit figure",false,true);

    ExplicitPtr figure = make_shared<ExplicitFigure>(map,figType);
    setExplicitReference(node,figure);

    getFigureCommon(node, figure);

    return(figure);
}

StarPtr XmlLoader::getStarFigure(xml_node & node)
{
    if (hasReference(node))
    {
        StarPtr f = getStarReferencedPtr(node);
        return f;
    }

    static int count = 0;
    MapPtr map = getMap(node);
    qDebug() << "XML Input - verifying Map" << count++;
    map->verify("XML Star figure",false);

    QString str;
    str = node.child_value("n");
    int n = str.toInt();

    str = node.child_value("d");
    qreal d = str.toDouble();

    str = node.child_value("s");
    int s = str.toInt();

    qreal r = 0.0;
    str = node.child_value("r");
    if (!str.isEmpty())
        r = str.toDouble();

    StarPtr star = make_shared<Star>(n, d, s,r);
    setStarReference(node,star);

    getFigureCommon(node,star);

    return star;
}

ExtStarPtr  XmlLoader::getExtendedStarFigure(xml_node & node)
{
    if (hasReference(node))
    {
        ExtStarPtr f = getExtStarReferencedPtr(node);
        return f;
    }

    static int count = 0;
    MapPtr map = getMap(node);
    qDebug() << "XML Input - verifying Map" << count++;
    map->verify("XML Star figure",false);

    bool extendPeripherals   = false;   // default
    bool extendFreeVertices  = true;    // default

    QString str;

    xml_attribute ext_t = node.attribute("extendPeripherals");
    if (ext_t)
    {
        str = (ext_t.value());
        extendPeripherals = (str == "t");
    }

    xml_attribute ext_non_t = node.attribute("extendFreeVertices");
    if (ext_non_t)
    {
        str = (ext_non_t.value());
        extendFreeVertices = (str == "t");
    }

    str = node.child_value("n");
    int n = str.toInt();

    str = node.child_value("d");
    qreal d = str.toDouble();

    str = node.child_value("s");
    int s = str.toInt();

    qreal r = 0.0;
    str = node.child_value("r");
    if (!str.isEmpty())
        r = str.toDouble();

    ExtStarPtr star = make_shared<ExtendedStar>(n, d, s, r, extendPeripherals, extendFreeVertices);
    setExtStarReference(node,star);

    getFigureCommon(node,star);

    return star;
}

RosettePtr XmlLoader::getRosetteFigure(xml_node & node)
{
    if (hasReference(node))
    {
        RosettePtr f = getRosetteReferencedPtr(node);
        return f;
    }

    QString str;
    str = node.child_value("n");
    int n = str.toInt();

    str = node.child_value("q");
    qreal q = str.toDouble();

    str = node.child_value("s");
    int s = str.toInt();

    qreal r = 0.0;
    str = node.child_value("r");
    if (!str.isEmpty())
        r = str.toDouble();

    RosettePtr rosette = make_shared<Rosette>(n, q, s, r);
    setRosetteReference(node,rosette);

    getFigureCommon(node,rosette);

    return rosette;
}

ExtRosettePtr  XmlLoader::getExtendedRosetteFigure(xml_node & node)
{
    if (hasReference(node))
    {
        ExtRosettePtr f = getExtRosetteReferencedPtr(node);
        return f;
    }

    static int count = 0;
    MapPtr map = getMap(node);
    qDebug() << "XML Input - verifying Map" << count++;
    map->verify("XML Extended Rosette figure",false);

    bool extendPeripherals       = false;     // default
    bool extendFreeVertices      = true;    // default
    bool connectBoundaryVertices = false;    // default

    QString str;

    xml_attribute ext_t = node.attribute("extendPeripherals");
    if (ext_t)
    {
        str = (ext_t.value());
        extendPeripherals = (str == "t");
    }

    xml_attribute ext_non_t = node.attribute("extendFreeVertices");
    if (ext_non_t)
    {
        str = (ext_non_t.value());
        extendFreeVertices = (str == "t");
    }

    xml_attribute con_bnd_v = node.attribute("connectBoundaryVertices");
    if (con_bnd_v)
    {
        str = (con_bnd_v.value());
        connectBoundaryVertices = (str == "t");
    }

    str = node.child_value("n");
    int n = str.toInt();

    str = node.child_value("q");
    qreal q = str.toDouble();

    qreal k = 0.0;
    str = node.child_value("k");
    if (!str.isEmpty())
        k = str.toDouble();

    str = node.child_value("s");
    int s = str.toInt();

    qreal r = 0.0;
    str = node.child_value("r");
    if (!str.isEmpty())
        r = str.toDouble();

    ExtRosettePtr rosette = make_shared<ExtendedRosette>(n, q, s, k, r, extendPeripherals, extendFreeVertices, connectBoundaryVertices);
    setExtRosetteReference(node,rosette);

    getFigureCommon(node,rosette);
    return rosette;
}

RosetteConnectPtr XmlLoader::getRosetteConnectFigure(xml_node & node)
{
    if (hasReference(node))
    {
        RosetteConnectPtr f = getRosetteConnectReferencedPtr(node);
        return f;
    }

    QString str;
    str = node.child_value("n");
    int n = str.toInt();

    str = node.child_value("s");
    qreal s = str.toDouble();

    RosetteConnectPtr rcp;
    RosettePtr rp;
    xml_node child = node.child("child");
    if (child)
    {
        xml_attribute class1 = child.attribute("class");
        qDebug() << class1.value();
        if (QString(class1.value()) == "app.Rosette")
        {
            rp = getRosetteFigure(child);
            Q_ASSERT(rp->getN() == n);
            //Q_ASSERT(r->getS() == s);
            qDebug() << "connect s:" <<  rp->getS() << s;
        }
        else
        {
            fail("Connect figure not based on Rosette","");
        }
        rcp = make_shared<RosetteConnectFigure>(rp->getN(),
                                                rp->getQ(),
                                                rp->getS(),
                                                rp->getK(),
                                                rp->getR());
        setRosetteConnectReference(node,rcp);
        qDebug() << rcp->getFigureDesc();
    }
    else
    {
        fail("Connect Figure","");
    }
    return rcp;
}

MapPtr XmlLoader::getMap(xml_node &node)
{
    //qDebug() << node.name();
    MapPtr map;
    //qDebug() << "use count=" << map.use_count();
    xml_node xmlmap = node.child("map");

    if (hasReference(xmlmap))
    {
        map =  getMapReferencedPtr(xmlmap);
        return map;
    }
    else
    {
        map = make_shared<Map>();
        //qDebug() << "use count=" << map.use_count();
        setMapReference(xmlmap,map);
    }

    // vertices
    xml_node vertices = xmlmap.child("vertices");
    xml_node vertex;
    for (vertex = vertices.child("Vertex"); vertex; vertex = vertex.next_sibling("Vertex"))
    {
        VertexPtr v = getVertex(vertex);
        map->vertices.push_back(v);
    }

    // Edges
    xml_node edges = xmlmap.child("edges");
    xml_node edge;
    for (edge = edges.child("Edge"); edge; edge = edge.next_sibling("Edge"))
    {
        EdgePtr e = getEdge(edge);
        map->edges.push_back(e);
    }

    // sort neigbours
    map->sortAllNeighboursByAngle();

    return map;
}


VertexPtr XmlLoader::getVertex(xml_node & node)
{
   if (hasReference(node))
   {
       vRefrCnt++;
       return getVertexReferencedPtr(node);
   }

   // pos
   xml_node pos = node.child("pos");
   QPointF pt;
   if (_version < 2)
   {
        QString str;
        str = pos.child_value("x");
        qreal x = str.toDouble();
        str = pos.child_value("y");
        qreal y = str.toDouble();
        pt = QPointF(x,y);
   }
   else
   {
       pt = getPos(pos);
   }
   vOrigCnt++;
   VertexPtr v = make_shared<Vertex>(pt);
   setVertexReference(node,v);

   // edges = neighbour
   xml_node edges2 = node.child("edges2");
   if (edges2)
   {
       xml_node e;
       for (e = edges2.first_child(); e; e= e.next_sibling())
       {
           QString name = e.name();
           EdgePtr ep;
           if (name == "edge")
           {
                ep = getEdge(e);
           }
           else if (name == "curve")
           {
               ep = getCurve(e);
           }
           v->insertEdgeSimple(ep);
       }
   }
   else
       qWarning("edges2 not found");

   return v;
}

EdgePtr XmlLoader::getEdge(xml_node & node)
{
    //qDebug() << node.name();

    if (hasReference(node))
    {
        eRefrCnt++;
        return getEdgeReferencedPtr(node);
    }

    eOrigCnt++;
    EdgePtr edge = make_shared<Edge>();
    setEdgeReference(node,edge);        // early for recursion
    //qDebug() << "created Edge" << Utils::addr(edge.get());

    MapPtr map = getMap(node);

    xml_node v1node = node.child("v1");
    VertexPtr v1 = getVertex(v1node);

    xml_node v2node = node.child("v2");
    VertexPtr v2 = getVertex(v2node);

    edge->setV1(v1);
    edge->setV2(v2);
    //qDebug() << "created Edge" << Utils::addr(edge.get()) << Utils::addr(v1.get()) << Utils::addr(v2.get());

    return edge;
}

EdgePtr XmlLoader::getCurve(xml_node & node)
{
    //qDebug() << node.name();

    if (hasReference(node))
    {
        eRefrCnt++;
        return getEdgeReferencedPtr(node);
    }

    eOrigCnt++;
    EdgePtr edge = make_shared<Edge>();
    setEdgeReference(node,edge);        // early for recursion
    //qDebug() << "created Edge" << Utils::addr(edge.get());

    MapPtr map = getMap(node);

    xml_node v1node = node.child("v1");
    VertexPtr v1 = getVertex(v1node);

    xml_node v2node = node.child("v2");
    VertexPtr v2 = getVertex(v2node);

    xml_node pnode = node.child("pos");
    QPointF p = getPos(pnode);

    xml_attribute attr = node.attribute("convex");
    QString val = attr.value();
    bool convex = (val == "t") ? true : false;

    edge->setV1(v1);
    edge->setV2(v2);
    edge->setArcCenter(p,convex);

    return edge;
}

void XmlLoader::procScale(xml_node & node)
{
    QString val = node.child_value();
    _scale = val.toDouble();
}

void XmlLoader::procSize(xml_node & node, int & width, int & height)
{
    QString val;
    xml_node w = node.child("width");
    if (w)
    {
        val = w.child_value();
        width = val.toInt();
    }
    xml_node h = node.child("height");
    if (h)
    {
        val = h.child_value();
        height = val.toInt();
    }
}

QColor XmlLoader::procBackground(xml_node & node)
{
    xml_node n   = node.child("color");
    if (n)
        return processColor(n);
    else
        return _background;     // default;
}

void XmlLoader::procBorder(xml_node & node)
{
    xml_attribute atype = node.attribute("type");
    if (!atype)  return;

    eBorderType type = static_cast<eBorderType>(atype.as_int());
    switch (type)
    {
    case BORDER_PLAIN:
        procBorderPlain(node);
        break;
    case BORDER_TWO_COLOR:
        procBorderTwoColor(node);
        break;
    case BORDER_BLOCKS:
        procBorderBlocks(node);
        break;
    case BORDER_NONE:
        break;
    }
}

void XmlLoader::procBorderPlain(xml_node & node)
{
    xml_node cnode = node.child("color");
    QColor col1 = processColor(cnode);

    qreal bwidth;
    xml_node wnode = node.child("width");
    if (wnode)
        bwidth = procWidth(wnode);
    else
        bwidth = 20.0; // default

    _border = make_shared<BorderPlain>(bwidth, col1);
}

void XmlLoader::procBorderTwoColor(xml_node & node)
{
    xml_node cnode = node.child("color");
    QColor col1 = processColor(cnode);

    cnode = cnode.next_sibling("color");
    QColor col2 = processColor(cnode);

    qreal bwidth;
    xml_node wnode = node.child("width");
    if (wnode)
        bwidth = procWidth(wnode);
    else
        bwidth = 20.0; // default

    _border = make_shared<BorderTwoColor>(col1, col2, bwidth);
}

void XmlLoader::procBorderBlocks(xml_node & node)
{
    Q_UNUSED(node);  // FIXME border2
    //void addBorder2(QColor color, qreal diameter, int rows, int cols);
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

bool XmlLoader::hasReference(xml_node & node)
{
    xml_attribute ref;
    ref = node.attribute("reference");
    return (ref);
}

void XmlLoader::setProtoReference(xml_node & node, PrototypePtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        proto_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref proto:" << i;
#endif
    }
}

PrototypePtr XmlLoader::getProtoReferencedPtr(xml_node & node)
{
    PrototypePtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = PrototypePtr(proto_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

void XmlLoader::setVertexReference(xml_node & node, VertexPtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        vertex_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref vertex:" << i;
#endif
    }
}

VertexPtr XmlLoader::getVertexReferencedPtr(xml_node & node)
{
    VertexPtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = VertexPtr(vertex_ids[id]);
        //if (retval == NULL) qCritical() << "reference:" << id << "- NOT FOUND";
    }
    return retval;
}

void XmlLoader::setEdgeReference(xml_node & node, EdgePtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        edge_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref edge:" << i;
#endif
    }
}

EdgePtr XmlLoader::getEdgeReferencedPtr(xml_node & node)
{
    EdgePtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = EdgePtr(edge_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

void   XmlLoader::setPolyReference(xml_node & node, PolyPtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        poly_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref poly:" << i;
#endif
    }
}

void   XmlLoader::setFeatureReference(xml_node & node, FeaturePtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        feature_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref feature:" << i;
#endif
    }
}
void   XmlLoader::setFigureReference(xml_node & node, FigurePtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        figure_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref figure:" << i;
#endif
    }
}
void   XmlLoader::setExplicitReference(xml_node & node, ExplicitPtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        explicit_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref explicit:" << i;
#endif
    }
}

void   XmlLoader::setStarReference(xml_node & node, StarPtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        star_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref star:" << i;
#endif
    }
}

void  XmlLoader::setExtStarReference(xml_node & node, ExtStarPtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        ext_star_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref ext_star:" << i;
#endif
    }
}

void   XmlLoader::setRosetteReference(xml_node & node, RosettePtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        rosette_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref rosette:" << i;
#endif
    }
}

void  XmlLoader::setExtRosetteReference(xml_node & node, ExtRosettePtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        ext_rosette_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref ext_rosette:" << i;
#endif
    }
}

void   XmlLoader::setRosetteConnectReference(xml_node & node, RosetteConnectPtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        rosette_connect_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref connect:" << i;
#endif
    }
}
void   XmlLoader::setMapReference(xml_node & node, MapPtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        map_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref map:" << i;
#endif
    }
}

PolyPtr XmlLoader::getPolyReferencedPtr(xml_node & node)
{
    PolyPtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = PolyPtr(poly_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

FeaturePtr XmlLoader::getFeatureReferencedPtr(xml_node & node)
{
    FeaturePtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = FeaturePtr(feature_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

FigurePtr XmlLoader::getFigureReferencedPtr(xml_node & node)
{
    FigurePtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = FigurePtr(figure_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

ExplicitPtr XmlLoader::getExplicitReferencedPtr(xml_node & node)
{
    ExplicitPtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = ExplicitPtr(explicit_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

StarPtr XmlLoader::getStarReferencedPtr(xml_node & node)
{
    StarPtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = StarPtr(star_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

ExtStarPtr XmlLoader::getExtStarReferencedPtr(xml_node & node)
{
    ExtStarPtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = ExtStarPtr(ext_star_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}


RosettePtr XmlLoader::getRosetteReferencedPtr(xml_node & node)
{
    RosettePtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = RosettePtr(rosette_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

ExtRosettePtr XmlLoader::getExtRosetteReferencedPtr(xml_node & node)
{
    ExtRosettePtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = ExtRosettePtr(ext_rosette_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

RosetteConnectPtr XmlLoader::getRosetteConnectReferencedPtr(xml_node & node)
{
    RosetteConnectPtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = RosetteConnectPtr(rosette_connect_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

MapPtr XmlLoader::getMapReferencedPtr(xml_node & node)\
{
    MapPtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = MapPtr(map_ids[id]);
        //if (retval == NULL) qCritical() << "reference:" << id << "- NOT FOUND";
    }
    return retval;
}

void XmlLoader::fail(QString a, QString b)
{
    _failMessage = QString("%1 %2").arg(a).arg(b);
    qWarning().noquote() << _failMessage;
    throw(_failMessage);
}
