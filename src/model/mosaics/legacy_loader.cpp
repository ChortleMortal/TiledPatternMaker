#include "model/mosaics/legacy_loader.h"
#include <QColor>
#include <QMessageBox>

#include "model/mosaics/mosaic_reader.h"
#include "model/mosaics/reader_base.h"
#include "model/prototypes/design_element.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/reader_base.h"
#include "model/mosaics/legacy_loader.h"
#include "model/prototypes/prototype.h"
#include "model/motifs/explicit_map_motif.h"
#include "model/motifs/irregular_motif.h"
#include "model/motifs/extender.h"
#include "model/motifs/inferred_motif.h"
#include "model/motifs/motif.h"
#include "model/motifs/rosette.h"
#include "model/motifs/star.h"
#include "sys/geometry/map.h"
#include "model/mosaics/border.h"
#include "sys/sys/fileservices.h"
#include "model/styles/emboss.h"
#include "model/styles/filled.h"
#include "model/styles/interlace.h"
#include "model/styles/outline.h"
#include "model/styles/plain.h"
#include "model/styles/sketch.h"
#include "model/styles/thick.h"
#include "model/tilings/tile.h"
#include "model/tilings/tile_reader.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling_reader.h"
#include "model/tilings/tiling_manager.h"
#include "gui/top/system_view_controller.h"

#undef  DEBUG_REFERENCES
#define SUPPORT_BUGS
#define NEIGHBOURS

#define clearMe(T, map) \
        {QMap<int,T>::iterator i = map.begin(); \
        while (i != map.end()) \
        { i.value().reset(); i++;}}


LegacyLoader::LegacyLoader()
{
    qDebug() << "Constructing LegacyLoader";
    _loaded     = false;
    _debug      = false;

    vOrigCnt = 0;
    vRefrCnt = 0;
    eOrigCnt = 0;
    eRefrCnt = 0;
    nOrigCnt = 0;
    nRefrCnt = 0;
}

LegacyLoader::~LegacyLoader()
{
    qDebug() << "Destroying LegacyLoader";
}

void LegacyLoader::processTapratsVector(xml_node & node, MosaicPtr mosaic)
{
    _mosaic = mosaic;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug() << str.c_str();
        if (str == "csk.taprats.style.Thick")
            processThick(n);
        else if (str == "csk.taprats.style.Filled")
            processFilled(n);
        else if (str == "csk.taprats.style.Interlace")
            processInterlace(n);
        else if (str == "csk.taprats.style.Outline")
            processOutline(n);
        else if (str == "csk.taprats.style.Plain")
            processPlain(n);
        else if (str == "csk.taprats.style.Sketch")
            processSketch(n);
        else if (str == "csk.taprats.style.Emboss")
            processEmboss(n);
        else
            qCritical() << "Unexpected" << str.c_str();

    }
    if (_debug) qDebug() << "end vector";
    
    CanvasSettings ms = _mosaic->getCanvasSettings();
    ms.setBackgroundColor(QColor(Qt::white));
    ms.setViewSize(QSize(1500,1100));  // this is an arbitrary resizing to match with version 1.8.1
    ms.setCanvasSize(QSize(1500,1100));

    // Canvas Settings fill data defaults to FillData defaults, loader can  override these
    if (_tilings.size() > 0)
    {
        if (_debug) qDebug() << "Using Tiling FiilData";
        FillData fd =  _tilings.first()->hdr().getCanvasSettings().getFillData();
        ms.setFillData(fd);
    }
    else
    {
        if (_debug) qDebug() << "Using Default FiilData";
        FillData fd;    // default
        ms.setFillData(fd);
    }
    _mosaic->setCanvasSettings(ms);
}

void LegacyLoader::processThick(xml_node & node)
{
    QColor  color;
    eDrawOutline  draw_outline = OUTLINE_NONE;
    qreal   width        = 0.0;
    ProtoPtr proto;
    Xform   xf;
    int     zlevel        = 0;


    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug() << str.c_str();

        if (str == "csk.taprats.toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "csk.taprats.style.Style")
            processStyleStyle(n,proto);
        else if (str == "csk.taprats.style.Colored")
            color = processStyleColored(n);
        else if (str == "csk.taprats.style.Thick")
            processsStyleThick(n,draw_outline,width);
        else
            qCritical() << "Unexpected" << n.name();
    }

    if (_debug) qDebug() << "Constructing Thick from prototype and poly";
    Thick * thick = new Thick(proto);
    thick->setModelXform(xf,false,Sys::nextSigid());
    thick->setColor(color);
    thick->setDrawOutline(draw_outline);
    thick->setLineWidth(width);

    if (_debug) qDebug().noquote() << "XmlServices created Style(Thick)" << thick->getInfo();
    _mosaic->addStyle(StylePtr(thick));

    if (_debug) qDebug() << "end thick";
}

void LegacyLoader::processInterlace(xml_node & node)
{
    QColor  color;
    eDrawOutline  draw_outline = OUTLINE_NONE;
    qreal   width        = 0.0;
    qreal   gap          = 0.0;
    qreal   shadow       = 0.0;
    ProtoPtr proto;
    Xform   xf;
    int     zlevel        = 0;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug() << str.c_str();

        if (str == "csk.taprats.toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "csk.taprats.style.Style")
            processStyleStyle(n,proto);
        else if (str == "csk.taprats.style.Colored")
            color = processStyleColored(n);
        else if (str == "csk.taprats.style.Thick")
            processsStyleThick(n,draw_outline,width);
        else if (str == "csk.taprats.style.Interlace")
            processsStyleInterlace(n,gap,shadow);
        else
            qCritical() << "Unexpected" << n.name();
    }

    if (_debug) qDebug() << "Constructing Interlace (DAC) from prototype and poly";
    Interlace * interlace = new Interlace(proto);
    interlace->setModelXform(xf,false,Sys::nextSigid());
    interlace->setColor(color);
    interlace->setDrawOutline(draw_outline);
    interlace->setLineWidth(width);
    interlace->setGap(gap);
    interlace->setShadow(shadow);
    if (_debug) qDebug().noquote() << "XmlServices created Style(Interlace)" << interlace->getInfo();
    _mosaic->addStyle(StylePtr(interlace));

    if (_debug) qDebug() << "end interlace";
}

void LegacyLoader::processOutline(xml_node & node)
{
    QColor  color;
    eDrawOutline  draw_outline = OUTLINE_NONE;
    qreal   width        = 0.0;
    ProtoPtr proto;
    Xform   xf;
    int     zlevel        = 0;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug() << str.c_str();

        if (str == "csk.taprats.toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "csk.taprats.style.Style")
            processStyleStyle(n,proto);
        else if (str == "csk.taprats.style.Colored")
            color = processStyleColored(n);
        else if (str == "csk.taprats.style.Thick")
            processsStyleThick(n,draw_outline,width);
        else
            qCritical() << "Unexpected" << n.name();
    }

    if (_debug) qDebug() << "Constructing Outline from prototype and poly";
    Outline * outline = new Outline(proto);
    if (_debug) qDebug().noquote() << xf.info();
    outline->setModelXform(xf,false,Sys::nextSigid());
    outline->setColor(color);
    outline->setDrawOutline(draw_outline);
    outline->setLineWidth(width);

    if (_debug) qDebug().noquote() << "XmlServices created Style(Outline)" << outline->getInfo();
    _mosaic->addStyle(StylePtr(outline));

    if (_debug) qDebug() << "end outline";
}

void LegacyLoader::processFilled(xml_node & node)
{
    QColor  color;
    bool    draw_inside = false;
    bool    draw_outside = true;
    ProtoPtr proto;
    Xform   xf;
    int     zlevel        = 0;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug() << str.c_str();

        if (str == "csk.taprats.toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "csk.taprats.style.Style")
            processStyleStyle(n,proto);
        else if (str == "csk.taprats.style.Colored")
            color = processStyleColored(n);
        else if (str == "csk.taprats.style.Filled")
            processsStyleFilled(n,draw_inside,draw_outside);
        else
            qCritical() << "Unexpected" << n.name();
    }

    Filled * filled = new Filled(proto,FILL_ORIGINAL);
    filled->setModelXform(xf,false,Sys::nextSigid());
    //filled->setColor(color);
    filled->setDrawInsideBlacks(draw_inside);
    filled->setDrawOutsideWhites(draw_outside);
    if (_debug) qDebug().noquote() << "XmlServices created Style(Filled)" << filled->getInfo();

    ColorSet * csetW = filled->getWhiteColorSet();
    csetW->addColor(color);

    ColorSet * csetB = filled->getBlackColorSet();
    csetB->addColor(color);

    _mosaic->addStyle(StylePtr(filled));

    if (_debug) qDebug() << "end filled";
}

void LegacyLoader::processPlain(xml_node & node)
{
    QColor  color;
    ProtoPtr proto;
    Xform   xf;
    int     zlevel        = 0;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        qDebug() << str.c_str();

        if (str == "csk.taprats.toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "csk.taprats.style.Style")
            processStyleStyle(n,proto);
        else if (str == "csk.taprats.style.Colored")
            color = processStyleColored(n);
        else
            qCritical() << "Unexpected" << n.name();
    }

    Plain * plain = new Plain(proto);
    plain->setModelXform(xf,false,Sys::nextSigid());
    plain->setColor(color);
    if (_debug) qDebug().noquote() << "XmlServices created Style (Plain)" << plain->getInfo();
    _mosaic->addStyle(StylePtr(plain));

    if (_debug) qDebug() << "end plain";
}

void LegacyLoader::processSketch(xml_node & node)
{
    QColor  color;
    ProtoPtr proto;
    Xform   xf;
    int     zlevel        = 0;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug() << str.c_str();

        if (str == "csk.taprats.toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "csk.taprats.style.Style")
            processStyleStyle(n,proto);
        else if (str == "csk.taprats.style.Colored")
            color = processStyleColored(n);
        else
            qCritical() << "Unexpected" << n.name();
    }

    Sketch * sketch = new Sketch(proto);
    sketch->setModelXform(xf,false,Sys::nextSigid());
    sketch->setColor(color);
    if (_debug) qDebug().noquote() << "XmlServices created Style (Sketch)" << sketch->getInfo();
    _mosaic->addStyle(StylePtr(sketch));

    if (_debug) qDebug() << "end sketch";
}

void LegacyLoader::processEmboss(xml_node & node)
{
    QColor          color;
    eDrawOutline    draw_outline = OUTLINE_NONE;
    qreal           width        = 0.0;
    qreal           angle        = 0.0;
    ProtoPtr    proto;
    Xform           xf;
    int             zlevel        = 0;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug() << str.c_str();

        if (str == "csk.taprats.toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "csk.taprats.style.Style")
            processStyleStyle(n,proto);
        else if (str == "csk.taprats.style.Colored")
            color = processStyleColored(n);
        else if (str == "csk.taprats.style.Thick")
            processsStyleThick(n,draw_outline,width);
        else if (str == "csk.taprats.style.Emboss")
            processsStyleEmboss(n,angle);
        else
            qCritical() << "Unexpected" << n.name();
    }

    if (_debug) qDebug() << "Constructing Emboss from prototype and poly";
    Emboss * emboss = new Emboss(proto);
    emboss->setModelXform(xf,false,Sys::nextSigid());
    ColorSet cs;
    cs.addColor(color);
    emboss->setColorSet(cs);
    emboss->setDrawOutline(draw_outline);
    emboss->setLineWidth(width);
    emboss->setAngle(angle);

    if (_debug) qDebug().noquote() << "XmlServices created Style(Emboss)" << emboss->getInfo();
    _mosaic->addStyle(StylePtr(emboss));

    if (_debug) qDebug() << "end emboss";
}

void LegacyLoader::procesToolkitGeoLayer(xml_node & node, Xform & xf, int & zlevel)
{
    xml_node def = node.child("default");
    if (!def)
    {
        qWarning("GeoLayer has no default");
        return;

    }
    QString val;
    qreal   fval;
    xml_node n = def.child("left__delta");
    if (n)
    {
        val   = n.child_value();
        fval  = val.toDouble();
        fval *= -75.0;
        xf.setTranslateX(fval);
    }

    n = def.child("top__delta");
    if (n)
    {
        val   = n.child_value();
        fval  = val.toDouble();
        fval *= -75.0;
        xf.setTranslateY(fval);
    }

    n = def.child("width__delta");
    if (n)
    {
        val  = n.child_value();
        fval = val.toDouble();
        xf.setScale(fval);
        xf.setScale(1.0);     //used as scale
    }

    n = def.child("theta__delta");
    if (n)
    {
        val  = n.child_value();
        fval = val.toDouble();
        xf.setRotateRadians(fval);
    }

    // 06OCT25 removed support for node "center" which was never used

    n = def.child("Z");
    if (n)
    {
        val          = n.child_value();
        zlevel       = val.toInt();
    }
}

void LegacyLoader::processStyleStyle(xml_node & node, ProtoPtr & proto)
{
    xml_node def = node.child("default");
    xml_node n   = def.child("boundary");
    if (_debug) qDebug() << n.name();
    //poly = getBoundary(n);

    n = def.child("prototype");
    if (_debug) qDebug() << n.name();
    proto = getPrototype(n);
    if (_debug) qDebug().noquote() << proto->info();
}

QColor LegacyLoader::processStyleColored(xml_node & node)
{
    xml_node def = node.child("default");
    xml_node n   = def.child("color");

    QColor color;

    xml_node c = n.child("red");
    QString col = c.child_value();
    color.setRed(col.toUInt());

    c = n.child("green");
    col = c.child_value();
    color.setGreen(col.toUInt());

    c = n.child("blue");
    col = c.child_value();
    color.setBlue(col.toUInt());

    c = n.child("alpha");
    col = c.child_value();
    color.setAlpha(col.toUInt());

    return color;
}

void LegacyLoader::processsStyleThick(xml_node & node, eDrawOutline & draw_outline, qreal & width)
{
    xml_node def = node.child("default");

    QString str  = def.child_value("draw__outline");
    if (str == "true")
        draw_outline = OUTLINE_DEFAULT;
    else
        draw_outline = OUTLINE_NONE;

    QString w = def.child_value("width");
    width = w.toDouble();
 }

void LegacyLoader::processsStyleInterlace(xml_node & node, qreal & gap, qreal & shadow)
{
    xml_node def = node.child("default");

    QString str  = def.child_value("gap");
    gap =  str.toDouble();

    str = def.child_value("shadow");
    shadow = str.toDouble();
}

void LegacyLoader::processsStyleFilled(xml_node & node, bool & draw_inside, bool & draw_outside)
{
    xml_node def = node.child("default");

    QString str;
    str = def.child_value("draw__inside");
    draw_inside  = (str == "true");

    str = def.child_value("draw__outside");
    draw_outside  = (str == "true");
}

void LegacyLoader::processsStyleEmboss(xml_node & node, qreal & angle)
{
    xml_node def = node.child("default");

    QString str  = def.child_value("angle");
    angle =  str.toDouble();
}

PolyPtr LegacyLoader::getBoundary(xml_node & node)
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

PolyPtr LegacyLoader::getPolygon(xml_node & node)
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


ProtoPtr LegacyLoader::getPrototype(xml_node & node)
{
    if (hasReference(node))
    {
        return getProtoReferencedPtr(node);
    }

    xml_node proto = node.child("csk.taprats.app.Prototype");

    TilingPtr tp;
    xml_node tnode = proto.child("string");
    if (tnode)
    {
        QString tilingName = tnode.child_value();
        qDebug().noquote() << "Tiling Name:" << tilingName;

        // has tiling been already loaded by another prototype
        tp = findTiling(tilingName);
        if (!tp)
        {
            // load tiling
            qDebug().noquote() << "loading named tiling" << tilingName;
#if 0
            TilingManager tm;
            tp = tm.loadTiling(tilingName,TILM_LOAD_FROM_MOSAIC);
#else
            VersionedName vn(tilingName);
            VersionedFile filename = FileServices::getFile(vn,FILE_TILING);
            TilingReader tm(Sys::viewController);
            ReaderBase mrbase;
            tp  = tm.readTilingXML(filename,&mrbase);
#endif
                if (tp)
            {
                _tilings.push_back(tp);
            }
            else
            {
                qInfo() << "Tiling not loaded :" << tilingName;
            }
        }
    }
    if (!tp)
    {
        // 18JAN2022 - adds this case for mosaics with maps but no tiling
        tp = make_shared<Tiling>();
        _tilings.push_back(tp);
    }

    ProtoPtr p = make_shared<Prototype>(tp,_mosaic);
    setProtoReference(node,p);

    xml_node hash = proto.child("hashtable");
    xml_node entry;
    for (entry = hash.child("entry"); entry; entry = entry.next_sibling("entry"))
    {
        bool dc_found = false;

        FeaturePtr feature;
        FigurePtr  figure;
        QString name;

        xml_node xmlfeature = entry.first_child();
        name = xmlfeature.name();
        if ( name == "csk.taprats.tile.Feature")
        {
            if (_debug) qDebug() << "adding Feature";
            feature = getFeature(xmlfeature);
        }
        else
        {
            qCritical() << "feature not found";
        }

        xml_node xmlfigure  = xmlfeature.next_sibling();
        name = xmlfigure.name();
        if (name == "csk.taprats.app.ExplicitFigure")
        {
            if (_debug) qDebug() << "adding ExplicitFigure";
            figure = getExplicitFigure(xmlfigure,feature);
            dc_found = true;
        }
        else if (name == "csk.taprats.app.Star")
        {
            if (_debug) qDebug() << "adding Star Figure";
            figure =  getStarFigure(xmlfigure);
            dc_found = true;
        }
        else if (name == "csk.taprats.app.Rosette")
        {
            if (_debug) qDebug() << "adding Rosette Figure";
            figure =  getRosetteFigure(xmlfigure);
            dc_found = true;
        }
        else if (name == "csk.taprats.app.ConnectFigure")
        {
            if (_debug) qDebug() << "adding Connect Figure";
            figure =  getConnectFigure(xmlfigure);
            dc_found = true;
        }
        else
        {
            qCritical() << "figure not found" << name;
        }

        if (dc_found)
        {
            // if the found feature is identical to the one in the known tiling
            // then use that
            // DAC 27MAY17 - imprtant that this code not removed or Design View will fail
            const PlacedTiles tilingUnit = tp->unit().getIncluded();
            for (const auto & pf : std::as_const(tilingUnit))
            {
                if (*pf->getTile().get() == *feature.get())
                {
                    feature = pf->getTile();
                }
            }
            if (_debug) qDebug() << "adding to Proto" << figure->getMotifDesc();
            DesignElementPtr  dep = make_shared<DesignElement>(feature, figure);
            p->addDesignElement(dep);
        }
        //p->walk();
    }

    if (_debug) qDebug() << "Proto created";
    return p;
}

FeaturePtr LegacyLoader::getFeature(xml_node & node)
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

    PolyPtr b = getPolygon(poly);

    FeaturePtr f = make_shared<Tile>(b);
    setFeatureReference(node,f);
    f->setRegular(regular);

    return f;
}

ExplicitPtr LegacyLoader::getExplicitFigure(xml_node & node, FeaturePtr tile)
{
    if (hasReference(node))
    {
        ExplicitPtr f = getExplicitReferencedPtr(node);
        return f;
    }

    MapPtr map = getMap(node);
    //map->verify("XML Explicit figure",false);

    ExplicitPtr figure = make_shared<ExplicitMapMotif>(map);
    figure->setTile(tile);
    setExplicitReference(node,figure);
    return(figure);
}

StarPtr LegacyLoader::getStarFigure(xml_node & node)
{
    if (hasReference(node))
    {
        StarPtr f = getStarReferencedPtr(node);
        return f;
    }

    static int count = 0;
    MapPtr map = getMap(node);
    if (_debug) qDebug() << "XML Input - verifying Map" << count++;
    //map->verify("XML Star figure",false);

    QString str;
    str = node.child_value("n");
    int n = str.toInt();

    str = node.child_value("d");
    qreal d = str.toDouble();

    str = node.child_value("s");
    int s = str.toInt();

    StarPtr star = make_shared<Star>(n, d, s);
    setStarReference(node,star);
    return star;
}

RosettePtr LegacyLoader::getRosetteFigure(xml_node & node)
{
    if (hasReference(node))
    {
        RosettePtr f = getRosetteReferencedPtr(node);
        return f;
    }

    QString str;
    str = node.child_value("n");
    int n = str.toInt();

    str = node.child_value("dn");
    qreal dn = str.toDouble();
    Q_UNUSED(dn);

    str = node.child_value("don");
    qreal don = str.toDouble();
    Q_UNUSED(don);

    str = node.child_value("q");
    qreal q = str.toDouble();

    str = node.child_value("s");
    int s = str.toInt();

    RosettePtr rosette = make_shared<Rosette>(n, q, s);
    setRosetteReference(node,rosette);
    return rosette;
}

RosettePtr LegacyLoader::getConnectFigure(xml_node & node)
{
    if (hasReference(node))
    {
        RosettePtr f = getRosetteReferencedPtr(node);
        return f;
    }

    QString str;
    str = node.child_value("n");
    int n = str.toInt();

//    str = node.child_value("dn");
//    qreal dn = str.toDouble();

//    str = node.child_value("don");
//    qreal don = str.toDouble();

    str = node.child_value("s");
    qreal s = str.toDouble();

    RosettePtr rose;
    xml_node child = node.child("child");
    if (child)
    {
        xml_attribute class1 = child.attribute("class");
        if (_debug) qDebug() << class1.value();
        if (QString(class1.value()) == "csk.taprats.app.Rosette")
        {
            rose = getRosetteFigure(child);
            Q_ASSERT(rose->getN() == n);
            //Q_ASSERT(qFuzzyCompare(r->get_dn(),dn));
            //Q_ASSERT(qFuzzyCompare(r->get_don(),don));
            //Q_ASSERT(r->getS() == s);
            if (_debug) qDebug() << "connect s:" <<  rose->getS() << s;
        }
        else
        {
            qFatal("Connect figure not based on Rosette");
        }
        rose->addRadialConnector();
        if (_debug) qDebug() << rose->getMotifDesc();
    }
    else
    {
        qFatal("Connect Figure");
    }
    return rose;
}

MapPtr LegacyLoader::getMap(xml_node &node)
{
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
        map = make_shared<Map>("Irregular Motif Map");
        //qDebug() << "use count=" << map.use_count();
        setMapReference(xmlmap,map);
    }

    // vertices
    xml_node vertices = xmlmap.child("vertices");
    xml_node vertex;
    for (vertex = vertices.child("Vertex"); vertex; vertex = vertex.next_sibling("Vertex"))
    {
        VertexPtr v = getVertex(vertex);
        map->XmlInsertDirect(v);
    }

    // Edges
    xml_node edges = xmlmap.child("edges");
    xml_node edge;
    for (edge = edges.child("Edge"); edge; edge = edge.next_sibling("Edge"))
    {
        EdgePtr e = getEdge(edge);
        map->XmlInsertDirect(e);
    }

    return map;
}

VertexPtr LegacyLoader::getVertex(xml_node & node)
{
   if (hasReference(node))
   {
       vRefrCnt++;
       return getVertexReferencedPtr(node);
   }

   // pos
   xml_node pos = node.child("pos");
   QString str;
   str = pos.child_value("x");
   qreal x = str.toDouble();
   str = pos.child_value("y");
   qreal y = str.toDouble();

   vOrigCnt++;
   VertexPtr v = make_shared<Vertex>(QPointF(x,y));
   setVertexReference(node,v);

#ifdef NEIGHBOURS
   // edges = neighbour
   xml_node edges = node.child("edges");
   if (edges)
   {
       NeighbourPtr n = getNeighbour(edges,v);
       if (n)
       {
            //v->insertNeighbour(n);
            //qDebug().noquote() << v->dumpNeighbours();
       }
   }
   else
       qWarning("edges not found");
#endif
   return v;
}

#ifdef NEIGHBOURS
NeighbourPtr LegacyLoader::getNeighbour(xml_node & node, VertexPtr v)
{
    if (hasReference(node))
    {
        // at end of forward-linked list.  return null pointer
        return NeighbourPtr();
    }

    xml_node edge  = node.child("edge");
    if (!edge)
    {
        qCritical("Edge not found");
        NeighbourPtr np;
        return np;
    }
    EdgePtr e = getEdge(edge);

    nOrigCnt++;
    NeighbourPtr retval = make_shared<Neighbour>();
    //retval->setEdge(e);

    setNeighbourReference(node,retval);

#ifdef SUPPORT_BUGS
    xml_node angleNode = node.child("angle");
    if (!angleNode)
    {
        qWarning("There is no angle");
        return retval;
    }
#endif

    //QString str = node.child_value("angle");
    //qreal angle = str.toDouble();
    //retval->setAngle(angle);

    xml_node edges2 = node.child("next");
    if (!edges2)
    {
        qWarning("edges2 not found");
        return retval;
    }

    NeighbourPtr n = getNeighbour(edges2,v);
    //if (n && n->getEdge())
    //{
        //v->insertNeighbour(n);
        //qDebug().noquote() << v->dumpNeighbours();
    //}

    return retval;
}
#endif

EdgePtr LegacyLoader::getEdge(xml_node & node)
{
    if (hasReference(node))
    {
        eRefrCnt++;
        return getEdgeReferencedPtr(node);
    }

    eOrigCnt++;
    EdgePtr edge = make_shared<Edge>();
    setEdgeReference(node,edge);        // early for recursion

    MapPtr map = getMap(node);

    xml_node v1node = node.child("v1");
    VertexPtr v1 = getVertex(v1node);

    xml_node v2node = node.child("v2");
    VertexPtr v2 = getVertex(v2node);

    edge->setV1(v1);
    edge->setV2(v2);

    return edge;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

bool LegacyLoader::hasReference(xml_node & node)
{
    xml_attribute ref;
    ref = node.attribute("reference");
    return (ref);
}

void LegacyLoader::setProtoReference(xml_node & node, ProtoPtr ptr)
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

ProtoPtr LegacyLoader::getProtoReferencedPtr(xml_node & node)
{
    ProtoPtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = ProtoPtr(proto_ids[id]);
        if (!retval)
            qCritical() << "reference:" << id << "- NOT FOUND";
    }
    return retval;
}

void LegacyLoader::setVertexReference(xml_node & node, VertexPtr ptr)
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

VertexPtr LegacyLoader::getVertexReferencedPtr(xml_node & node)
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

void LegacyLoader::setEdgeReference(xml_node & node, EdgePtr ptr)
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

EdgePtr LegacyLoader::getEdgeReferencedPtr(xml_node & node)
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
            qCritical() << "reference:" << id << "- NOT FOUND";
    }
    return retval;
}

#if 1
void LegacyLoader::setNeighbourReference(xml_node & node, NeighbourPtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        neighbour_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref neighbour:" << i;
#endif
    }
}

NeighbourPtr LegacyLoader::getNeighbourReferencedPtr(xml_node & node)
{
    NeighbourPtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = NeighbourPtr(neighbour_ids[id]);
        if (!retval)
            qCritical() << "reference:" << id << "- NOT FOUND";
    }
    return retval;
}
#endif
void   LegacyLoader::setPolyReference(xml_node & node, PolyPtr ptr)
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

void   LegacyLoader::setFeatureReference(xml_node & node, FeaturePtr ptr)
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
void   LegacyLoader::setFigureReference(xml_node & node, FigurePtr ptr)
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
void   LegacyLoader::setExplicitReference(xml_node & node, ExplicitPtr ptr)
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

void   LegacyLoader::setStarReference(xml_node & node, StarPtr ptr)
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

void   LegacyLoader::setRosetteReference(xml_node & node, RosettePtr ptr)
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

void   LegacyLoader::setMapReference(xml_node & node, MapPtr ptr)
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

PolyPtr LegacyLoader::getPolyReferencedPtr(xml_node & node)
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
            qCritical() << "reference:" << id << "- NOT FOUND";
    }
    return retval;
}

FeaturePtr LegacyLoader::getFeatureReferencedPtr(xml_node & node)
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
            qCritical() << "reference:" << id << "- NOT FOUND";
    }
    return retval;
}

FigurePtr LegacyLoader::getFigureReferencedPtr(xml_node & node)
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
            qCritical() << "reference:" << id << "- NOT FOUND";
    }
    return retval;
}

ExplicitPtr LegacyLoader::getExplicitReferencedPtr(xml_node & node)
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
            qCritical() << "reference:" << id << "- NOT FOUND";
    }
    return retval;
}
StarPtr LegacyLoader::getStarReferencedPtr(xml_node & node)
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
            qCritical() << "reference:" << id << "- NOT FOUND";
    }
    return retval;
}

RosettePtr LegacyLoader::getRosetteReferencedPtr(xml_node & node)
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
            qCritical() << "reference:" << id << "- NOT FOUND";
    }
    return retval;
}

MapPtr LegacyLoader::getMapReferencedPtr(xml_node & node)\
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

TilingPtr LegacyLoader::findTiling(QString name)
{
    for (auto & tiling : _tilings)
    {
        if (tiling->getVName().get() == name)
        {
            return tiling;
        }
    }
    TilingPtr tp;
    return tp;  // empty
}
