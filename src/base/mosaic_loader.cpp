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

#include "base/mosaic_loader.h"
#include "base/border.h"
#include "base/configuration.h"
#include "base/tiledpatternmaker.h"
#include "base/utilities.h"
#include "style/colored.h"
#include "style/thick.h"
#include "style/filled.h"
#include "style/interlace.h"
#include "style/outline.h"
#include "style/plain.h"
#include "style/sketch.h"
#include "style/emboss.h"
#include "style/tile_colors.h"
#include "tapp/star.h"
#include "tapp/extended_star.h"
#include "tapp/infer.h"
#include "tapp/explicit_figure.h"
#include "tile/feature_reader.h"
#include "tile/tiling_manager.h"

#undef  DEBUG_REFERENCES

MosaicLoader::MosaicLoader()
{
    // defaults
    _background = QColor(Qt::white);
    _width      = 1500;
    _height     = 1100;
    _version    = 0;

    view        = View::getInstance();
}

MosaicLoader::~MosaicLoader()
{
    qDebug() << "MosaicLoader: descructor";
}

QString MosaicLoader::getLoadedFilename()
{
    return _fileName;
}

MosaicPtr MosaicLoader::loadMosaic(QString fileName)
{
    view->dump(true);

    qDebug().noquote() << "MosaicLoader loading:" << fileName;
    _fileName = fileName;

    xml_document doc;
    xml_parse_result result = doc.load_file(fileName.toStdString().c_str());

    if (result == false)
    {
        _failMessage = result.description();
        qWarning().noquote() << _failMessage;
        _mosaic.reset();
        return _mosaic;
    }

    try
    {
        _mosaic = make_shared<Mosaic>();

        parseXML(doc);

        view->dump(true);

        return _mosaic;
    }
    catch (...)
    {
        qWarning() << "ERROR processing XML file"  << fileName;
        _mosaic.reset();
        return _mosaic;
    }
}

MapPtr MosaicLoader::loadMosaicMap(QString fileName)
{
    MapPtr map;

    view->dump(true);

    qDebug().noquote() << "MosaicLoader loading:" << fileName;
    _fileName = fileName;

    xml_document doc;
    xml_parse_result result = doc.load_file(fileName.toStdString().c_str());

    if (result == false)
    {
        _failMessage = result.description();
        qWarning().noquote() << _failMessage;
        return map;
    }

    try
    {
        qDebug() << "MosaicLoader - start parsing";
        vOrigCnt = 0;
        vRefrCnt = 0;
        eOrigCnt = 0;
        eRefrCnt = 0;
        nRefrCnt = 0;

        xml_node node = doc.first_child();
        string str = node.name();
        qDebug() << str.c_str();
        if (str  != "vector")
        {
            return map;
        }

        xml_attribute attr = node.attribute("version");
        if (attr)
        {
            QString str = attr.value();
            _version = str.toUInt();
        }

        map = getMap(node);

        view->dump(true);
    }
    catch (...)
    {
        qWarning() << "ERROR processing XML file"  << fileName;
    }

    return map;
}

void MosaicLoader::parseXML(xml_document & doc)
{
    vOrigCnt = 0;
    vRefrCnt = 0;
    eOrigCnt = 0;
    eRefrCnt = 0;
    nRefrCnt = 0;
    qDebug() << "MosaicLoader - start parsing";

    for (xml_node n = doc.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();
        if (str  == "vector")
            processVector(n);
        else
            fail("Unexpected", str.c_str());
    }

    view->dump(true);

    qDebug() << "MosaicLoader - end parsing";
}

void MosaicLoader::processDesignNotes(xml_node & node)
{
    string str = node.child_value();
    _mosaic->setNotes(str.c_str());
}

void MosaicLoader::processVector(xml_node & node)
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

    ModelSettingsPtr settings = make_shared<ModelSettings>();
    settings->setBackgroundColor(_background);
    settings->setSize(QSize(_width,_height));
    settings->setBorder(_border);
    settings->setBkgdImage(getFirstTiling()->getBackground());

    // Canvas Settings fill data defaults to FillData defaults, loader can  override these
    if (_fillData.isSet())
    {
        qDebug() << "Using Mosaic FiilData";
        settings->setFillData(_fillData);
    }
    else
    {
        qDebug() << "Using Tiling FiilData";
        FillData fd =  getFirstTiling()->getFillData();
        settings->setFillData(fd);
    }

    _mosaic->setSettings(settings);
}

void MosaicLoader::processDesign(xml_node & node)
{
    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();
        if (str == "scale")
            continue;       // scale deprecated 30DEC19
        else if (str == "size")
            procSize(n, _width, _height);
        else if (str == "background")
            _background = procBackgroundColor(n);
        else if (str == "border")
            procBorder(n);
        else if (str == "Fill")
            procFill(n);
        else
            fail("Unexpected", str.c_str());
    }
}

void MosaicLoader::processThick(xml_node & node)
{
    ColorSet  colorset;
    bool    draw_outline = false;
    qreal   width        = 0.0;
    PrototypePtr proto;
    Xform   xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf);
        else if (str == "style.Style")
            processStyleStyle(n,proto);
        else if (str == "style.Colored")
            processColorSet(n,colorset);
        else if (str == "style.Thick")
            processsStyleThick(n,draw_outline,width);
        else
            fail("Unexpected", str.c_str());
    }

    qDebug() << "Constructing Thick from prototype and poly";
    Thick * thick = new Thick(proto);
    thick->setCanvasXform(xf);
    thick->setColorSet(colorset);
    thick->setDrawOutline(draw_outline);
    thick->setLineWidth(width);

    qDebug().noquote() << "XmlServices created Style(Thick)" << thick->getInfo();

    _mosaic->addStyle(StylePtr(thick));

    qDebug() << "end thick";
}

void MosaicLoader::processInterlace(xml_node & node)
{
    ColorSet  colorset;
    PrototypePtr proto;
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
            processStyleStyle(n,proto);
        else if (str == "style.Colored")
            processColorSet(n,colorset);
        else if (str == "style.Thick")
            processsStyleThick(n,draw_outline,width);
        else if (str == "style.Interlace")
            processsStyleInterlace(n,gap,shadow,includeTipVerts);
        else
            fail("Unexpected", str.c_str());
    }

    qDebug() << "Constructing Interlace (DAC) from prototype and poly";
    Interlace * interlace = new Interlace(proto);
    interlace->setCanvasXform(xf);
    interlace->setColorSet(colorset);
    interlace->setDrawOutline(draw_outline);
    interlace->setLineWidth(width);
    interlace->setGap(gap);
    interlace->setShadow(shadow);
    interlace->setIncludeTipVertices(includeTipVerts);
    qDebug().noquote() << "XmlServices created Style(Interlace)" << interlace->getInfo();

    _mosaic->addStyle(StylePtr(interlace));

    qDebug() << "end interlace";
}

void MosaicLoader::processOutline(xml_node & node)
{
    ColorSet  colorset;
    bool    draw_outline = false;
    qreal   width        = 0.0;
    PrototypePtr proto;
    Xform   xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf);
        else if (str == "style.Style")
            processStyleStyle(n,proto);
        else if (str == "style.Colored")
            processColorSet(n,colorset);
        else if (str == "style.Thick")
            processsStyleThick(n,draw_outline,width);
        else
            fail("Unexpected", str.c_str());
    }

    qDebug() << "Constructing Outline from prototype and poly";
    Outline * outline = new Outline(proto);
    outline->setCanvasXform(xf);
    outline->setColorSet(colorset);
    outline->setDrawOutline(draw_outline);
    outline->setLineWidth(width);

    qDebug().noquote() << "XmlServices created Style(Outline)" << outline->getInfo();

    _mosaic->addStyle(StylePtr(outline));

    qDebug() << "end outline";
}

void MosaicLoader::processFilled(xml_node & node)
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
            processStyleStyle(n,proto);
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

    Filled * filled = new Filled(proto,algorithm);
    filled->setCanvasXform(xf);

    if (oldFormat)
    {
        // old - redundant way of dealing with colors
        ColorSet & csetW = filled->getWhiteColorSet();
        csetW.addColor(colorSet.getColor(0));

        ColorSet & csetB = filled->getBlackColorSet();
        if (colorSet.size() >= 2)
            csetB.addColor(colorSet.getColor(1));
        else
            csetB.addColor(colorSet.getColor(0));
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
            cs.addColor(Qt::black);
            cgroup.addColorSet(cs);
        }
#endif
    }

    filled->setDrawInsideBlacks(draw_inside);
    filled->setDrawOutsideWhites(draw_outside);

    qDebug().noquote() << "XmlServices created Style(Filled)" << filled->getInfo();

    _mosaic->addStyle(StylePtr(filled));

    qDebug() << "end filled";
}

void MosaicLoader::processPlain(xml_node & node)
{
    ColorSet  colorset;
    PrototypePtr proto;
    Xform   xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf);
        else if (str == "style.Style")
            processStyleStyle(n,proto);
        else if (str == "style.Colored")
            processColorSet(n,colorset);
        else
            fail("Unexpected", str.c_str());
    }

    Plain * plain = new Plain(proto);
    plain->setCanvasXform(xf);
    plain->setColorSet(colorset);
    qDebug().noquote() << "XmlServices created Style (Plain)" << plain->getInfo();

    _mosaic->addStyle(StylePtr(plain));

    qDebug() << "end plain";
}

void MosaicLoader::processSketch(xml_node & node)
{
    ColorSet  colorset;
    PrototypePtr proto;
    Xform   xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf);
        else if (str == "style.Style")
            processStyleStyle(n,proto);
        else if (str == "style.Colored")
            processColorSet(n,colorset);
        else
            fail("Unexpected", str.c_str());
    }

    Sketch * sketch = new Sketch(proto);
    sketch->setCanvasXform(xf);
    sketch->setColorSet(colorset);
    qDebug().noquote() << "XmlServices created Style (Sketch)" << sketch->getInfo();

    _mosaic->addStyle(StylePtr(sketch));

    qDebug() << "end sketch";
}

void MosaicLoader::processEmboss(xml_node & node)
{
    ColorSet  colorset;
    bool    draw_outline = false;
    qreal   width        = 0.0;
    qreal   angle        = 0.0;
    PrototypePtr proto;
    Xform   xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf);
        else if (str == "style.Style")
            processStyleStyle(n,proto);
        else if (str == "style.Colored")
            processColorSet(n,colorset);
        else if (str == "style.Thick")
            processsStyleThick(n,draw_outline,width);
        else if (str == "style.Emboss")
            processsStyleEmboss(n,angle);
        else
            fail("Unexpected", str.c_str());
    }

    qDebug() << "Constructing Emboss from prototype and poly";
    Emboss * emboss = new Emboss(proto);
    emboss->setCanvasXform(xf);
    emboss->setColorSet(colorset);
    emboss->setDrawOutline(draw_outline);
    emboss->setLineWidth(width);
    emboss->setAngle(angle);

    qDebug().noquote() << "XmlServices created Style(Emboss)" << emboss->getInfo();

    _mosaic->addStyle(StylePtr(emboss));

    qDebug() << "end emboss";
}

void MosaicLoader::processTileColors(xml_node & node)
{
    PrototypePtr proto;
    Xform   xf;
    bool outline = false;
    int  width   = 3;
    QColor color = Qt::white;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf);
        else if (str == "style.Style")
            processStyleStyle(n,proto);
        else if (str == "outline")
        {
            outline = true;
            QString w = n.child_value("width");
            width = w.toInt();
            w      = n.child_value("color");
            color  = QColor(w);
        }
        else
            fail("Unexpected", str.c_str());
    }

    qDebug() << "Constructing TileColors from prototype and poly";
    TileColors * tc  = new TileColors(proto);
    tc->setCanvasXform(xf);
    if (outline)
    {
        tc->setOutline(true,color,width);
    }

    qDebug().noquote() << "XmlServices created Style(TileColors)" << tc->getInfo();

    _mosaic->addStyle(StylePtr(tc));

    qDebug() << "end emboss";
}

void MosaicLoader::procesToolkitGeoLayer(xml_node & node, Xform & xf)
{
    QString val;
    qreal   fval;
    xml_node n = node.child("left__delta");
    if (n)
    {
        val           = n.child_value();
        fval          = val.toDouble();
        if (!Loose::zero(fval) && _version < 3)
        {
            fval *= -75.0;
        }
        xf.setTranslateX(fval);
    }

    n = node.child("top__delta");
    if (n)
    {
        val           = n.child_value();
        fval          = val.toDouble();
        if (!Loose::zero(fval) && _version < 3)
        {
            fval *= -75.0;
        }
        xf.setTranslateY(fval);
    }

    n = node.child("width__delta");
    if (n)
    {
        val          = n.child_value();
        fval         = val.toDouble();
        xf.setScale(fval);
        if (_version < 3)
        {
            xf.setScale(1.0);     //used as scale
        }
    }

    n = node.child("theta__delta");
    if (n)
    {
        val          = n.child_value();
        fval         = val.toDouble();
        xf.setRotateRadians(fval);
    }

    n = node.child("center");
    if (n)
    {
        val          = n.child_value();
        QStringList qsl = val.split(",");
        qreal x = qsl[0].toDouble();
        qreal y = qsl[1].toDouble();
        xf.setCenter(QPointF(x,y));
    }
}

void MosaicLoader::processStyleStyle(xml_node & node, PrototypePtr & proto)
{
    xml_node n = node.child("prototype");
    qDebug() << n.name();
    view->dump(true);
    proto = getPrototype(n);
    view->dump(true);
    qDebug().noquote() << proto->getInfo();
}

#if 0
void XmlLoader::processColorSet(xml_node & node, QColor & color)
{
    xml_node n   = node.child("color");
    color        = processColor(n);
}
#endif
void MosaicLoader::processColorSet(xml_node & node, ColorSet &colorSet)
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

void MosaicLoader::processColorGroup(xml_node & node, ColorGroup &colorGroup)
{
    xml_node n;
    for (n = node.child("Group"); n; n = n.next_sibling("Group"))
    {
        ColorSet cset;
        processColorSet(n,cset);
        colorGroup.addColorSet(cset);
    }
}

QColor MosaicLoader::processColor(xml_node & n)
{
    QString str = n.child_value();
    QColor color(str);
    return color;
}

qreal MosaicLoader::procWidth(xml_node & node)
{
    QString str = node.child_value();
    return str.toDouble();
}

void MosaicLoader::processsStyleThick(xml_node & node, bool & draw_outline, qreal & width)
{
    QString str  = node.child_value("draw__outline");
    draw_outline = (str == "true");

    QString w = node.child_value("width");
    width = w.toDouble();
 }

void MosaicLoader::processsStyleInterlace(xml_node & node, qreal & gap, qreal & shadow, bool & includeTipVerts)
{
    QString str  = node.child_value("gap");
    gap =  str.toDouble();

    str = node.child_value("shadow");
    shadow = str.toDouble();

    str = node.child_value("includeTipVerts");
    includeTipVerts = (str == "true");
}

void MosaicLoader::processsStyleFilled(xml_node & node, bool & draw_inside, bool & draw_outside, int & algorithm)
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

void MosaicLoader::processsStyleEmboss(xml_node & node, qreal & angle)
{
    QString str  = node.child_value("angle");
    angle =  str.toDouble();
}

PolyPtr MosaicLoader::getBoundary(xml_node & node)
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

PolyPtr MosaicLoader::getPolygon(xml_node & node)
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

QPointF MosaicLoader::getPos(xml_node & node)
{
    QString txt = node.child_value();
    QStringList qsl;
    qsl = txt.split(',');
    qreal x = qsl[0].toDouble();
    qreal y = qsl[1].toDouble();
    return QPointF(x,y);
}

PrototypePtr MosaicLoader::getPrototype(xml_node & node)
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

    TilingPtr tp = findTiling(tilingName);
    if (!tp)
    {
        qDebug() << "loading named tiling" << tilingName;
        TilingManager tm;
        tp = tm.loadTiling(tilingName,SM_LOAD_FROM_MOSAIC);
        if (!tp)
        {
            fail("Tiling not loaded: ",tilingName);
        }
        _tilings.push_back(tp);
    }
    //qDebug().noquote() << tp->dump();

    qDebug() << "Creating new prototype";

    PrototypePtr proto = make_shared<Prototype>(tp);
    setProtoReference(node,proto);

    QVector<FeaturePtr> uniqueFeatures = tp->getUniqueFeatures();
    int numFeatures = uniqueFeatures.size();

    QVector<FeaturePtr> usedFeatures;
    xml_node entry;
    for (entry = protonode.child("entry"); entry; entry = entry.next_sibling("entry"))
    {
        bool found = false;

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

        xml_node xmlfigure  = xmlfeature.next_sibling();
        name = xmlfigure.name();
        qDebug() << name;
        if (name == "app.ExplicitFigure")
        {
            qDebug() << "adding ExplicitFigure";
            figure = getExplicitFigure(xmlfigure, FIG_TYPE_EXPLICIT);
            found = true;
        }
        else if (name == "app.Star")
        {
            qDebug() << "adding Star Figure";
            figure =  getStarFigure(xmlfigure);
            found = true;
        }
        else if (name == "ExtendedStar")
        {
            qDebug() << "adding ExtendedStar Figure";
            figure =  getExtendedStarFigure(xmlfigure);
            found = true;
        }
        else if (name == "app.Rosette")
        {
            qDebug() << "adding Rosette Figure";
            figure =  getRosetteFigure(xmlfigure);
            found = true;
        }
        else if (name == "ExtendedRosette")
        {
            qDebug() << "adding ExtendedRosette Figure";
            figure =  getExtendedRosetteFigure(xmlfigure);
            found = true;
        }
        else if (name == "app.ConnectFigure")
        {
            qDebug() << "adding Connect Figure";
            figure =  getConnectFigure(xmlfigure);
            found = true;
        }
        else if (name == "app.Infer")
        {
            qDebug() << "adding Infer";
            figure = getExplicitFigure(xmlfigure,FIG_TYPE_EXPLICIT_INFER);
            found = true;
        }
        else if (name == "app.ExplicitFeature")
        {
            qDebug() << "adding Explicit Feature";
            figure = getExplicitFigure(xmlfigure,FIG_TYPE_EXPLICIT_FEATURE);
            found = true;
        }
        else if (name == "app.ExplicitGirih")
        {
            qDebug() << "adding ExplicitGirih";
            figure = getExplicitFigure(xmlfigure,FIG_TYPE_EXPLICIT_GIRIH);
            found = true;
        }
        else if (name == "app.ExplicitStar")
        {
            qDebug() << "adding ExplicitStar";
            figure = getExplicitFigure(xmlfigure,FIG_TYPE_EXPLICIT_STAR);
            found = true;
        }
        else if (name == "app.ExplicitRosette")
        {
            qDebug() << "adding ExplicitRosette";
            figure = getExplicitFigure(xmlfigure,FIG_TYPE_EXPLICIT_ROSETTE);
            found = true;
        }
        else if (name == "app.ExplicitHourglass")
        {
            qDebug() << "adding ExplicitHourglass";
            figure = getExplicitFigure(xmlfigure,FIG_TYPE_EXPLICIT_HOURGLASS);
            found = true;
        }
        else if (name == "app.ExplicitIntersect")
        {
            qDebug() << "adding ExplicitIntersect";
            figure = getExplicitFigure(xmlfigure,FIG_TYPE_EXPLICIT_INTERSECT);
            found = true;
        }
        else
        {
            fail("Figure not found:", name);
        }

        if (found)
        {
            // if the found feature is identical to the one in the known tiling
            // then use that
            // DAC 27MAY17 - imprtant that this code not removed or Design View will fail
            bool found2 = false;
            for (auto tilingFeature :  uniqueFeatures)
            {
                if (usedFeatures.contains(tilingFeature))
                {
                    continue;
                }

                if (tilingFeature->equals(feature))
                {
                    usedFeatures.push_back(tilingFeature);
                    feature = tilingFeature;
                    qDebug() << "adding to Proto" << figure->getFigureDesc();
                    DesignElementPtr  dep = make_shared<DesignElement>(feature, figure);
                    proto->addElement(dep);
                    qDebug() << "design element:" << dep->toString();
                    found2 = true;
                    break;
                }
            }
            if (!found2)
            {
                qWarning() << "Design feature not matching tiling feature" << _fileName;
            }
        }
        //p->walk();
    }

    QVector<DesignElementPtr>  & designElements = proto->getDesignElements();
    int numDesignElements = designElements.size();
    if (numFeatures != numDesignElements)
    {
        qWarning() <<  "Num unqiue features = " << numFeatures << "num DesignElements = " << numDesignElements;
        qWarning("Feature/DesignElement MISMATCH");
    }


    // create explicit maps
    for (auto del : designElements)
    {
        FigurePtr figp = del->getFigure();
        FeaturePtr featp = del->getFeature();
        switch (figp->getFigType())
        {
        case FIG_TYPE_UNDEFINED:
        case FIG_TYPE_RADIAL:
        case FIG_TYPE_ROSETTE:
        case FIG_TYPE_STAR:
        case FIG_TYPE_CONNECT_STAR:
        case FIG_TYPE_CONNECT_ROSETTE:
        case FIG_TYPE_EXTENDED_ROSETTE:
        case FIG_TYPE_EXTENDED_STAR:
            // not explicit
            break;

        case FIG_TYPE_EXPLICIT:
            // already has a map
            break;

        case FIG_TYPE_EXPLICIT_INFER:
        {
            ExplicitPtr ep = std::dynamic_pointer_cast<ExplicitFigure>(figp);
            InferPtr inf = make_shared<Infer>(proto);
            MapPtr map =  inf->infer(featp);
            ep->setExplicitMap(map);
            break;
        }

        case FIG_TYPE_EXPLICIT_ROSETTE:
        {
            ExplicitPtr ep = std::dynamic_pointer_cast<ExplicitFigure>(figp);
            InferPtr inf = make_shared<Infer>(proto);
            MapPtr map =  inf->inferRosette(featp, ep->q, ep->s, ep->r_flexPt);
            ep->setExplicitMap(map);
            break;
        }

        case FIG_TYPE_EXPLICIT_HOURGLASS:
        {
            ExplicitPtr ep = std::dynamic_pointer_cast<ExplicitFigure>(figp);
            InferPtr inf = make_shared<Infer>(proto);
            MapPtr map =  inf->inferHourglass(featp, ep->d, ep->s);
            ep->setExplicitMap(map);
            break;
        }

        case FIG_TYPE_EXPLICIT_INTERSECT:
        {
            ExplicitPtr ep = std::dynamic_pointer_cast<ExplicitFigure>(figp);
            InferPtr inf = make_shared<Infer>(proto);
            MapPtr map;
            if (ep->progressive)
                map =  inf->inferIntersectProgressive(featp, ep->getN(), ep->skip,ep->s);
            else
                map =  inf->inferIntersect(featp, ep->getN(), ep->skip, ep->s);
            ep->setExplicitMap(map);
            break;
        }

        case FIG_TYPE_EXPLICIT_STAR:
        {
            ExplicitPtr ep = std::dynamic_pointer_cast<ExplicitFigure>(figp);
            InferPtr inf = make_shared<Infer>(proto);
            MapPtr map =  inf->inferStar(featp, ep->d, ep->s);
            ep->setExplicitMap(map);
            break;
        }
        case FIG_TYPE_EXPLICIT_FEATURE:
        {
            ExplicitPtr ep = std::dynamic_pointer_cast<ExplicitFigure>(figp);
            InferPtr inf = make_shared<Infer>(proto);
            MapPtr map =  inf->inferFeature(featp);
            ep->setExplicitMap(map);
            break;
        }

        case FIG_TYPE_EXPLICIT_GIRIH:
        {
            ExplicitPtr ep = std::dynamic_pointer_cast<ExplicitFigure>(figp);
            InferPtr inf = make_shared<Infer>(proto);
            MapPtr map =  inf->inferGirih(featp, ep->getN(), ep->skip);
            ep->setExplicitMap(map);
        }
        }
    }

    qDebug() << "Proto created";

    return proto;
}

FeaturePtr MosaicLoader::getFeature(xml_node & node)
{
    if (hasReference(node))
    {
        FeaturePtr f = getFeatureReferencedPtr(node);
        return f;
    }

    QString str;
    str = node.child_value("regular");
    bool regular = (str == "true");

    qreal rotation = 0.0;
    xml_node r = node.child("rotation");
    if (r)
    {
        str = r.child_value();
        rotation = str.toDouble();
    }

    qreal scale = 1.0;
    xml_node s = node.child("scale");
    if (s)
    {
        str = s.child_value();
        scale = str.toDouble();
    }

    xml_node poly = node.child("points");
    FeaturePtr f;
    if (poly)
    {
        PolyPtr b    = getPolygon(poly);
        EdgePoly ep(b);

        f = make_shared<Feature>(ep,rotation,scale);
        setFeatureReference(node,f);
        f->setRegular(regular);
        return f;
    }
    poly = node.child("edges");
    if (poly)
    {
        FeatureReader fr;
        EdgePoly ep = fr.getEdgePoly(poly);
        f = make_shared<Feature>(ep,rotation,scale);
        f->setRegular(regular);
        if (((_version == 5) || (_version ==6)) && (!Loose::zero(rotation) || !Loose::equals(scale,1.0)))
        {
            qWarning() << "Decomposing Feature for backwards compatability"  << _fileName;
            f->decompose();
        }

        setFeatureReference(node,f);
        return f;
    }
    return f;
}

void MosaicLoader::getFigureCommon(xml_node & node, FigurePtr fig)
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

ExplicitPtr MosaicLoader::getExplicitFigure(xml_node & node, eFigType figType)
{
    ExplicitPtr ep;
    if (hasReference(node))
    {
        ep = getExplicitReferencedPtr(node);
        return ep;
    }

    qDebug() << "getExplicitFigure";

    ep = make_shared<ExplicitFigure>(_currentMap,figType,10);

    switch (figType)
    {
    case FIG_TYPE_UNDEFINED:
    case FIG_TYPE_RADIAL:
    case FIG_TYPE_ROSETTE:
    case FIG_TYPE_STAR:
    case FIG_TYPE_CONNECT_STAR:
    case FIG_TYPE_CONNECT_ROSETTE:
    case FIG_TYPE_EXTENDED_ROSETTE:
    case FIG_TYPE_EXTENDED_STAR:
        qFatal("Not an explicit figure");

    case FIG_TYPE_EXPLICIT:
        // only the old original taprats formats have maps
        _currentMap = getMap(node);
        ep->setExplicitMap(_currentMap);
        break;

    case FIG_TYPE_EXPLICIT_INFER:
    case FIG_TYPE_EXPLICIT_FEATURE:
        // these have no parameters
        break;

    case FIG_TYPE_EXPLICIT_GIRIH:
    {
        QString str;
        str = node.child_value("sides");
        int sides = str.toInt();

        str = node.child_value("skip");
        qreal skip  = str.toDouble();

        ep->setN(sides);
        ep->skip  = skip;
        break;
    }
    case FIG_TYPE_EXPLICIT_STAR:
    {
        QString str;
        str = node.child_value("s");
        int s = str.toInt();

        str = node.child_value("d");
        qreal d  = str.toDouble();

        ep->s = s;
        ep->d = d;
        break;
    }
    case FIG_TYPE_EXPLICIT_ROSETTE:
    {
        QString str;
        str = node.child_value("s");
        int s = str.toInt();

        str = node.child_value("q");
        qreal q  = str.toDouble();

        str = node.child_value("r");
        qreal r  = str.toDouble();

        ep->s = s;
        ep->q = q;
        ep->setFigureRotate(r);
        break;
    }
    case FIG_TYPE_EXPLICIT_HOURGLASS:
    {
        QString str;
        str = node.child_value("s");
        int s = str.toInt();

        str = node.child_value("d");
        qreal d  = str.toDouble();

        ep->s = s;
        ep->d = d;
        break;
    }
    case FIG_TYPE_EXPLICIT_INTERSECT:
    {
        QString str;
        str = node.child_value("s");
        int s = str.toInt();

        str = node.child_value("sides");
        int sides = str.toInt();

        str = node.child_value("skip");
        qreal skip  = str.toDouble();

        str = node.child_value("progressive");
        bool prog = (str == "t");

        ep->s     = s;
        ep->setN(sides);
        ep->skip  = skip;
        ep->progressive = prog;
        break;
    }
    }
    setExplicitReference(node,ep);

    getFigureCommon(node,ep);

    return(ep);
}

StarPtr MosaicLoader::getStarFigure(xml_node & node)
{
    if (hasReference(node))
    {
        StarPtr f = getStarReferencedPtr(node);
        return f;
    }

    qDebug() << "getStarFigure";

    _currentMap = getMap(node);

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

ExtStarPtr  MosaicLoader::getExtendedStarFigure(xml_node & node)
{
    if (hasReference(node))
    {
        ExtStarPtr f = getExtStarReferencedPtr(node);
        return f;
    }

    qDebug() << "getExtendedStarFigure";

    _currentMap = getMap(node);

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

RosettePtr MosaicLoader::getRosetteFigure(xml_node & node)
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

    qreal k = 0.0;
    str = node.child_value("k");
    if (!str.isEmpty())
        k = str.toDouble();

    RosettePtr rosette = make_shared<Rosette>(n, q, s, k, r);
    setRosetteReference(node,rosette);

    getFigureCommon(node,rosette);

    return rosette;
}

ExtRosettePtr  MosaicLoader::getExtendedRosetteFigure(xml_node & node)
{
    if (hasReference(node))
    {
        ExtRosettePtr f = getExtRosetteReferencedPtr(node);
        return f;
    }

    qDebug() << "getExtendedRosetteFigure";

    _currentMap = getMap(node);

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

    str = node.child_value("s");
    int s = str.toInt();

    qreal r = 0.0;
    str = node.child_value("r");
    if (!str.isEmpty())
        r = str.toDouble();

    qreal k = 0.0;
    str = node.child_value("k");
    if (!str.isEmpty())
        k = str.toDouble();

    ExtRosettePtr rosette = make_shared<ExtendedRosette>(n, q, s, k, r, extendPeripherals, extendFreeVertices, connectBoundaryVertices);
    setExtRosetteReference(node,rosette);

    getFigureCommon(node,rosette);
    return rosette;
}

FigurePtr MosaicLoader::getConnectFigure(xml_node & node)
{
    FigurePtr fp;
    xml_node child = node.child("child");
    if (child)
    {
        xml_attribute class1 = child.attribute("class");
        qDebug() << class1.value();
        if (QString(class1.value()) == "app.Rosette")
        {
            fp = getRosetteConnectFigure(node);
        }
        else if (QString(class1.value()) == "app.Star")
        {
            fp = getStarConnectFigure(node);
        }
        else
        {
            fail("Connect Figure child","");
        }
    }
    else
    {
        fail("Connect Figure","");
    }
    return fp;
}

RosetteConnectPtr MosaicLoader::getRosetteConnectFigure(xml_node & node)
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
            fail("Rosette Connect figure not based on Rosette","");
        }
        rcp = make_shared<RosetteConnectFigure>(rp->getN(),
                                                rp->getQ(),
                                                rp->getS(),
                                                rp->getK(),
                                                rp->getFigureRotate());
        setRosetteConnectReference(node,rcp);
        qDebug() << rcp->getFigureDesc();
    }
    else
    {
        fail("Connect Figure","");
    }
    return rcp;
}

StarConnectPtr MosaicLoader::getStarConnectFigure(xml_node & node)
{
    if (hasReference(node))
    {
        StarConnectPtr f = getStarConnectReferencedPtr(node);
        return f;
    }

    QString str;
    str = node.child_value("n");
    int n = str.toInt();

    str = node.child_value("s");
    qreal s = str.toDouble();

    StarPtr sp;
    StarConnectPtr scp;
    xml_node child = node.child("child");
    if (child)
    {
        xml_attribute class1 = child.attribute("class");
        qDebug() << class1.value();
        if (QString(class1.value()) == "app.Star")
        {
            sp = getStarFigure(child);
            Q_ASSERT(sp->getN() == n);
            //Q_ASSERT(r->getS() == s);
            qDebug() << "connect s:" <<  sp->getS() << s;
        }
        else
        {
            fail("Connect figure not based on Star","");
        }
        scp = make_shared<StarConnectFigure>(sp->getN(),
                                             sp->getD(),
                                             sp->getS(),
                                             sp->getFigureRotate());
        setStarConnectReference(node,scp);
        qDebug() << scp->getFigureDesc();
    }
    else
    {
        fail("Connect Figure","");
    }
    return scp;
}

MapPtr MosaicLoader::getMap(xml_node &node)
{
    //qDebug() << node.name();
    //qDebug() << "use count=" << map.use_count();
    xml_node xmlmap = node.child("map");

    if (hasReference(xmlmap))
    {
        _currentMap =  getMapReferencedPtr(xmlmap);
        return _currentMap;
    }
    else
    {
        _currentMap = make_shared<Map>("currentMap");
        //qDebug() << "use count=" << map.use_count();
        setMapReference(xmlmap,_currentMap);
    }

    // vertices
    xml_node vertices = xmlmap.child("vertices");
    for (xml_node vertex = vertices.child("Vertex"); vertex; vertex = vertex.next_sibling("Vertex"))
    {
        VertexPtr v = getVertex(vertex);
        _currentMap->vertices.push_back(v);
    }

    if (_version < 5)
    {
        // Edges
        xml_node edges = xmlmap.child("edges");
        for (xml_node edge = edges.child("Edge"); edge; edge = edge.next_sibling("Edge"))
        {
            EdgePtr e = getEdge(edge);
            _currentMap->edges.push_back(e);
        }
    }
    else
    {
        // Edges
        xml_node edges = xmlmap.child("edges");
        for (xml_node e = edges.first_child(); e; e= e.next_sibling())
        {
            QString name = e.name();
            if (name == "Edge")
            {
                EdgePtr ep = getEdge(e);
                _currentMap->edges.push_back(ep);
            }
            else if (name == "curve")
            {
                EdgePtr ep = getCurve(e);
                _currentMap->edges.push_back(ep);
            }
        }

        // Neighbours
        NeighbourMap & nmap = _currentMap->getNeighbourMap();
        QMap<VertexPtr,NeighboursPtr> & qnmap = nmap.get();
        xml_node neighbours = xmlmap.child("neighbours");
        for (xml_node set = neighbours.child("neighbourset"); set; set = set.next_sibling("neighbourset"))
        {
            VertexPtr v = getVertexReferencedPtr(set);
            NeighboursPtr n = make_shared<Neighbours>(v);
            QString nbs = set.child_value();
            QStringList nbslst = nbs.split(",");
            for (const auto & str : nbslst)
            {
                int id = str.toInt();
                EdgePtr e = EdgePtr(edge_ids[id]);
                Q_ASSERT(e);
                n->insertEdgeSimple(e);
            }
            qnmap[v] = n;
        }
    }

    if (!_currentMap->verifyMap("XML Loader"))
    {
        _currentMap->cleanse();
        if (!_currentMap->verifyMap("XML Loader - cleanse"))
        {
            QMessageBox box;
            box.setIcon(QMessageBox::Warning);
            box.setText("XML Loader: map verify failed after cleanse");
            box.exec();
        }
    }

    return _currentMap;
}


VertexPtr MosaicLoader::getVertex(xml_node & node)
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

    if (_version >= 5)
    {
        return v;
    }

    // the old way
    NeighbourMap & nmap = _currentMap->getNeighbourMap();
    nmap.insertVertex(v);

    NeighboursPtr np = nmap.getNeighbours(v);

    // edges = neighbour
    xml_node edges2 = node.child("edges2");
    if (edges2)
    {
        xml_node e;
        for (e = edges2.first_child(); e; e= e.next_sibling())
        {
            QString name = e.name();
            if (name == "edge")
            {
                EdgePtr ep = getEdge(e);
                np->insertEdgeSimple(ep);
            }
            else if (name == "curve")
            {
                EdgePtr ep = getCurve(e);
                np->insertEdgeSimple(ep);
            }
        }
    }
    else
    {
        qWarning("edges2 not found");
    }


    return v;
}

EdgePtr MosaicLoader::getEdge(xml_node & node)
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
    //qDebug() << "created Edge" << edge.get();

    if (_version < 5)
    {
        _currentMap = getMap(node);
    }

    xml_node v1node = node.child("v1");
    VertexPtr v1 = getVertex(v1node);

    xml_node v2node = node.child("v2");
    VertexPtr v2 = getVertex(v2node);

#if 0
    Neighbours nbs = _currentMap->getNeighbours()[v1];
    nbs.setVertex(v2);
    nbs = _currentMap->getNeighbours()[v2];
    nbs.setVertex(v1);
#endif
    edge->setV1(v1);
    edge->setV2(v2);
    qDebug() << "Edge:" << edge.get() << "added" << v1->getPosition() << v2->getPosition();

    return edge;
}

EdgePtr MosaicLoader::getCurve(xml_node & node)
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
    //qDebug() << "created Edge" << edge.get();

    if (_version < 5)
    {
        _currentMap = getMap(node);
    }

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

void MosaicLoader::procSize(xml_node & node, int & width, int & height)
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

QColor MosaicLoader::procBackgroundColor(xml_node & node)
{
    xml_node n   = node.child("color");
    if (n)
        return processColor(n);
    else
        return _background;     // default;
}

QTransform MosaicLoader::getQTransform(QString txt)
{
    QStringList qsl;
    qsl = txt.split(',');
    qreal m11 = qsl[0].toDouble();
    qreal m12 = qsl[1].toDouble();
    qreal m13 = qsl[2].toDouble();
    qreal m21 = qsl[3].toDouble();
    qreal m22 = qsl[4].toDouble();
    qreal m23 = qsl[5].toDouble();
    qreal m31 = qsl[6].toDouble();
    qreal m32 = qsl[7].toDouble();
    qreal m33 = qsl[8].toDouble();
    return QTransform(m11,m12,m13,m21,m22,m23,m31,m32,m33);
}

void MosaicLoader::procFill(xml_node & node)
{
    QString txt = node.child_value();
    QStringList qsl;
    qsl = txt.split(',');
    _fillData.set(qsl[0].toInt(),qsl[1].toInt(),qsl[2].toInt(),qsl[3].toInt());
}

void MosaicLoader::procBorder(xml_node & node)
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



void MosaicLoader::procBorderPlain(xml_node & node)
{
    xml_node cnode = node.child("color");
    QColor col1 = processColor(cnode);

    qreal bwidth = 20.0;
    xml_node wnode = node.child("width");
    if (wnode)
        bwidth = procWidth(wnode);

    _border = make_shared<BorderPlain>(QSize(_width,_height),bwidth, col1);
    _border->construct();
}

void MosaicLoader::procBorderTwoColor(xml_node & node)
{
    xml_node cnode = node.child("color");
    QColor col1 = processColor(cnode);

    cnode = cnode.next_sibling("color");
    QColor col2 = processColor(cnode);

    qreal bwidth = 20;
    xml_node wnode = node.child("width");
    if (wnode)
        bwidth = procWidth(wnode);


    _border = make_shared<BorderTwoColor>(QSize(_width,_height),col1, col2, bwidth);
    _border->construct();
}

void MosaicLoader::procBorderBlocks(xml_node & node)
{
    xml_node cnode = node.child("color");
    QColor col1 = processColor(cnode);

    qreal bwidth = 20;
    xml_node wnode = node.child("width");
    if (wnode)
        bwidth = procWidth(wnode);

    int rows = 3;
    wnode = node.child("rows");
    if (wnode)
    {
        QString str = wnode.child_value();
        rows =  str.toInt();
    }

    int cols = 3;
    wnode = node.child("cols");
    if (wnode)
    {
        QString str = wnode.child_value();
        cols =  str.toInt();
    }

    _border = make_shared<BorderBlocks>(QSize(_width,_height),col1, bwidth, rows, cols);
    _border->construct();
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

bool MosaicLoader::hasReference(xml_node & node)
{
    xml_attribute ref;
    ref = node.attribute("reference");
    return (ref);
}

void MosaicLoader::setProtoReference(xml_node & node, PrototypePtr ptr)
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

PrototypePtr MosaicLoader::getProtoReferencedPtr(xml_node & node)
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

void MosaicLoader::setVertexReference(xml_node & node, VertexPtr ptr)
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

VertexPtr MosaicLoader::getVertexReferencedPtr(xml_node & node)
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
        if (!retval)
        {
            qCritical() << "reference id:" << id << "- NOT FOUND";
        }
    }
    return retval;
}

void MosaicLoader::setEdgeReference(xml_node & node, EdgePtr ptr)
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

EdgePtr MosaicLoader::getEdgeReferencedPtr(xml_node & node)
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

void   MosaicLoader::setPolyReference(xml_node & node, PolyPtr ptr)
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

void   MosaicLoader::setFeatureReference(xml_node & node, FeaturePtr ptr)
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
void   MosaicLoader::setFigureReference(xml_node & node, FigurePtr ptr)
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
void   MosaicLoader::setExplicitReference(xml_node & node, ExplicitPtr ptr)
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

void   MosaicLoader::setStarReference(xml_node & node, StarPtr ptr)
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

void  MosaicLoader::setExtStarReference(xml_node & node, ExtStarPtr ptr)
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

void   MosaicLoader::setRosetteReference(xml_node & node, RosettePtr ptr)
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

void  MosaicLoader::setExtRosetteReference(xml_node & node, ExtRosettePtr ptr)
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

void   MosaicLoader::setRosetteConnectReference(xml_node & node, RosetteConnectPtr ptr)
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

void   MosaicLoader::setStarConnectReference(xml_node & node, StarConnectPtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        star_connect_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref connect:" << i;
#endif
    }
}

void   MosaicLoader::setMapReference(xml_node & node, MapPtr ptr)
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

PolyPtr MosaicLoader::getPolyReferencedPtr(xml_node & node)
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

FeaturePtr MosaicLoader::getFeatureReferencedPtr(xml_node & node)
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

FigurePtr MosaicLoader::getFigureReferencedPtr(xml_node & node)
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

ExplicitPtr MosaicLoader::getExplicitReferencedPtr(xml_node & node)
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

StarPtr MosaicLoader::getStarReferencedPtr(xml_node & node)
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

ExtStarPtr MosaicLoader::getExtStarReferencedPtr(xml_node & node)
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


RosettePtr MosaicLoader::getRosetteReferencedPtr(xml_node & node)
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

ExtRosettePtr MosaicLoader::getExtRosetteReferencedPtr(xml_node & node)
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

RosetteConnectPtr MosaicLoader::getRosetteConnectReferencedPtr(xml_node & node)
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

StarConnectPtr MosaicLoader::getStarConnectReferencedPtr(xml_node & node)
{
    StarConnectPtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = StarConnectPtr(star_connect_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

MapPtr MosaicLoader::getMapReferencedPtr(xml_node & node)\
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

void MosaicLoader::fail(QString a, QString b)
{
    _failMessage = QString("%1 %2").arg(a,b);
    qWarning().noquote() << _failMessage;
    throw(_failMessage);
}


TilingPtr MosaicLoader::findTiling(QString name)
{
    for (auto tiling : qAsConst(_tilings))
    {
        if (tiling->getName() == name)
        {
            return tiling;
        }
    }
    TilingPtr tp;
    return tp;  // empty
}
