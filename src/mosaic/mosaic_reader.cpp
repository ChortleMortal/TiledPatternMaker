#include <QMessageBox>

#include "mosaic/mosaic_reader.h"
#include "mosaic_reader_base.h"
#include "mosaic/design_element.h"
#include "mosaic/mosaic.h"
#include "mosaic/mosaic_reader_base.h"
#include "mosaic/legacy_loader.h"
#include "makers/prototype_maker/prototype.h"
#include "motifs/explicit_map_motif.h"
#include "motifs/irregular_motif.h"
#include "motifs/extended_rosette.h"
#include "motifs/extended_star.h"
#include "motifs/extender.h"
#include "motifs/girih_motif.h"
#include "motifs/hourglass_motif.h"
#include "motifs/inferred_motif.h"
#include "motifs/intersect_motif.h"
#include "motifs/irregular_rosette.h"
#include "motifs/irregular_star.h"
#include "motifs/motif.h"
#include "motifs/motif_connector.h"
#include "motifs/rosette.h"
#include "motifs/rosette_connect.h"
#include "motifs/star.h"
#include "motifs/star_connect.h"
#include "motifs/tile_motif.h"
#include "geometry/loose.h"
#include "geometry/map.h"
#include "misc/border.h"
#include "misc/defaults.h"
#include "misc/tpm_io.h"
#include "panels/panel.h"
#include "settings/configuration.h"
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
#include "tile/tile.h"
#include "tile/tile_reader.h"
#include "tile/tiling.h"
#include "tile/tiling_loader.h"
#include "tile/tiling_manager.h"
#include "viewers/viewcontrol.h"

#undef  DEBUG_REFERENCES

using std::make_shared;

MosaicReader::MosaicReader() : MosaicReaderBase()
{
    // defaults
    _background   = QColor(Qt::white);
    _width        = DEFAULT_WIDTH;
    _height       = DEFAULT_HEIGHT;
    _version      = 0;
    _debug        = false;
    _cleanseLevel = 0;
    view          = ViewControl::getInstance();
}

MosaicReader::~MosaicReader()
{
    //qDebug() << "MosaicLoader: destructor";
}

QString MosaicReader::getLoadedFilename()
{
    return _fileName;
}

MosaicPtr MosaicReader::readXML(QString fileName)
{
    view->dumpRefs();

    qInfo().noquote() << "MosaicLoader::loadMosaic()" << fileName << " : start";
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
        // load the Mosaic
        _mosaic = make_shared<Mosaic>();
        parseXML(doc);

        // The InferredMotif type is dependant on its surrounding motifs.
        // So these need to be prepared before the prototype map
        // can be built

        // pass 1 - build all motif maps
        for (auto & proto : _mosaic->getPrototypes())
        {
            for (const auto & del : proto->getDesignElements())
            {
                auto motif = del->getMotif();
                motif->buildMotifMaps();
            }
        }

        // pass 2 - rebuild InferredMotifs
        for (auto & proto : _mosaic->getPrototypes())
        {
            for (const auto & del : proto->getDesignElements())
            {
                auto motif = del->getMotif();
                if (motif->getMotifType() == MOTIF_TYPE_INFERRED)
                {
                    qDebug() << motif->getMotifTypeString();
                    motif->buildMotifMaps();
                }
            }
        }
        
        view->dumpRefs();

        qInfo().noquote() << "MosaicLoader load" << fileName << " : complete";

        return _mosaic;
    }
    catch (...)
    {
        QString str =  "ERROR loading XML file"  +  fileName;
        qWarning() << str;
        _failMessage += "\n" + str;
        _mosaic.reset();
        return _mosaic;
    }
}

void MosaicReader::parseXML(xml_document & doc)
{
    nRefrCnt = 0;
    if (_debug) qDebug() << "MosaicLoader - start parsing";

    for (xml_node node = doc.first_child(); node; node = node.next_sibling())
    {
        string str = node.name();
        if (_debug) qDebug().noquote() << str.c_str();
        if (str  == "vector")
        {
            xml_attribute attr = node.attribute("version");
            if (attr)
            {
                QString str = attr.value();
                _version = str.toUInt();
            }
            if (_version > 0 || node.child("design"))
            {
                processVector(node);
            }
            else
            {
                LegacyLoader ll;
                ll.processTapratsVector(node,_mosaic);
            }
        }
        else
            fail("Unexpected", str.c_str());
    }
    
    view->dumpRefs();

    if (_debug) qDebug() << "MosaicLoader - end parsing";
}

void MosaicReader::processDesignNotes(xml_node & node)
{
    string str = node.child_value();
    _mosaic->setNotes(str.c_str());
}

void MosaicReader::processVector(xml_node & node)
{
    // pass 1
    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();
        if (str  == "designNotes")
            continue;
        else if (str == "design")
            continue;
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

    // pass 2
    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();
        if (str  == "designNotes")
            processDesignNotes(n);
        else if (str == "design")
            processDesign(n);
        else if (str == "style.Thick")
            continue;
        else if (str == "style.Filled")
            continue;
        else if (str == "style.Interlace")
            continue;
        else if (str == "style.Outline")
            continue;
        else if (str == "style.Plain")
            continue;
        else if (str == "style.Sketch")
            continue;
        else if (str == "style.Emboss")
            continue;
        else if (str == "style.TileColors")
            continue;
        else
            fail("Unexpected", str.c_str());
    }
    if (_debug) qDebug() << "end vector";

    ModelSettings & settings = _mosaic->getSettings();

    settings.setBackgroundColor(_background);
    settings.setSize(QSize(_width,_height));
    settings.setZSize(QSize(_zwidth,_zheight));

    // Canvas Settings fill data defaults to FillData defaults, loader can  override these
    if (_fillData.isSet())
    {
        if (_debug) qDebug() << "Using Mosaic FiilData";
        settings.setFillData(_fillData);
    }
    else if (_tilings.size() > 0)
    {
        if (_debug) qDebug() << "Using Tiling FiilData";
        const FillData & fd =  getFirstTiling()->getData().getFillData();
        settings.setFillData(fd);
    }
    else
    {
        if (_debug) qDebug() << "Using Default FiilData";
        FillData fd;    // default
        settings.setFillData(fd);
    }

    if (_border)
    {
        _mosaic->setBorder(_border);
    }

    if (_crop)
    {
        _mosaic->setCrop(_crop);
    }

    if (_cleanseLevel > 0)
    {
        _mosaic->setCleanseLevel(_cleanseLevel);
    }
}

void MosaicReader::processDesign(xml_node & node)
{
    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();
        if (str == "scale")
            continue;       // scale deprecated 30DEC19
        else if (str == "size")
            procSize(n, _width, _height,_zwidth, _zheight);
        else if (str == "background")
            _background = procBackgroundColor(n);
        else if (str == "border")
            procBorder(n);
        else if (str == "Crop")
            procCrop(n);
        else if (str == "Fill")
        {
            bool isSingle = false;
            xml_attribute single = n.attribute("singleton");
            if (single)
            {
                if (single.as_string() == QString("t"))
                {
                    isSingle = true;
                }
            }
            procFill(n,isSingle);
        }
        else if (str == "BackgroundImage")
            TilingLoader::getBackgroundImage(n);
        else if (str == "Cleanse")
        {
            str = n.child_value();
            bool bStatus;
            QString str2 = str.c_str();
            _cleanseLevel = str2.toUInt(&bStatus,16);
        }
        else
            fail("Unexpected", str.c_str());
    }
}

void MosaicReader::processThick(xml_node & node)
{
    ColorSet     colorset;
    eDrawOutline draw_outline = OUTLINE_NONE;
    qreal        width        = 0.0;
    Qt::PenJoinStyle pjs      = Qt::RoundJoin;
    Qt::PenCapStyle pcs       = Qt::RoundCap;
    qreal        outlineWidth = 0.05;
    QColor       outlineColor = Qt::black;
    ProtoPtr proto;
    Xform        xf;
    int          zlevel        = 0;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "style.Style")
            processStylePrototype(n,proto);
        else if (str == "style.Colored")
            processColorSet(n,colorset);
        else if (str == "style.Thick")
            processsStyleThick(n,draw_outline,width,outlineWidth,outlineColor,pjs,pcs);
        else
            fail("Unexpected", str.c_str());
    }

    if (_debug) qDebug() << "Constructing Thick from prototype and poly";
    auto thick = make_shared<Thick>(proto);
    thick->setCanvasXform(xf);
    thick->setColorSet(colorset);
    thick->setDrawOutline(draw_outline);
    if (draw_outline)
    {
        thick->setOutlineColor(outlineColor);
        thick->setOutlineWidth(outlineWidth);
    }
    thick->setLineWidth(width);
    thick->setJoinStyle(pjs);
    thick->setCapStyle(pcs);
    thick->setZValue(zlevel);

    if (_debug) qDebug().noquote() << "XmlServices created Style(Thick)" << thick->getInfo();

    _mosaic->addStyle(thick);

    if (_debug) qDebug() << "end thick";
}

void MosaicReader::processInterlace(xml_node & node)
{
    ColorSet     colorset;
    ProtoPtr proto;
    eDrawOutline draw_outline    = OUTLINE_NONE;
    bool         includeTipVerts = false;
    bool         start_under     = false;
    qreal        width           = 0.0;
    Qt::PenJoinStyle pjs         = Qt::BevelJoin;
    Qt::PenCapStyle pcs          = Qt::SquareCap;
    qreal        outlineWidth    = 0.03;
    QColor       outlineColor    = Qt::black;
    qreal        gap             = 0.0;
    qreal        shadow          = 0.0;
    int          zlevel          = 0;
    Xform        xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "style.Style")
            processStylePrototype(n,proto);
        else if (str == "style.Colored")
            processColorSet(n,colorset);
        else if (str == "style.Thick")
            processsStyleThick(n,draw_outline,width,outlineWidth,outlineColor,pjs,pcs);
        else if (str == "style.Interlace")
            processsStyleInterlace(n,gap,shadow,includeTipVerts,start_under);
        else
            fail("Unexpected", str.c_str());
    }

    if (_debug) qDebug() << "Constructing Interlace (DAC) from prototype and poly";
    auto interlace = make_shared<Interlace>(proto);
    interlace->setInitialStartUnder(start_under);
    interlace->setCanvasXform(xf);
    interlace->setColorSet(colorset);
    interlace->setDrawOutline(draw_outline);
    if (draw_outline)
    {
        interlace->setOutlineColor(outlineColor);
        interlace->setOutlineWidth(outlineWidth);
    }
    interlace->setLineWidth(width);
    interlace->setJoinStyle(pjs);
    interlace->setCapStyle(pcs);
    interlace->setGap(gap);
    interlace->setShadow(shadow);
    interlace->setIncludeTipVertices(includeTipVerts);
    interlace->setZValue(zlevel);
    if (_debug) qDebug().noquote() << "XmlServices created Style(Interlace)" << interlace->getInfo();

    _mosaic->addStyle(interlace);

    if (_debug) qDebug() << "end interlace";
}

void MosaicReader::processOutline(xml_node & node)
{
    ColorSet     colorset;
    eDrawOutline draw_outline = OUTLINE_NONE;
    qreal        width        = 0.0;
    Qt::PenJoinStyle pjs      = Qt::BevelJoin;
    Qt::PenCapStyle pcs       = Qt::SquareCap;
    qreal        outlineWidth = 0.03;
    QColor       outlineColor = Qt::black;
    int          zlevel       = 0;
    ProtoPtr proto;
    Xform        xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "style.Style")
            processStylePrototype(n,proto);
        else if (str == "style.Colored")
            processColorSet(n,colorset);
        else if (str == "style.Thick")
            processsStyleThick(n,draw_outline,width,outlineWidth,outlineColor,pjs,pcs);
        else
            fail("Unexpected", str.c_str());
    }

    if (_debug) qDebug() << "Constructing Outline from prototype and poly";
    auto outline = make_shared<Outline>(proto);
    outline->setCanvasXform(xf);
    outline->setColorSet(colorset);
    outline->setDrawOutline(draw_outline);
    if (draw_outline)
    {
        outline->setOutlineColor(outlineColor);
        outline->setOutlineWidth(outlineWidth);
    }
    outline->setLineWidth(width);
    outline->setJoinStyle(pjs);
    outline->setCapStyle(pcs);
    outline->setZValue(zlevel);

    if (_debug) qDebug().noquote() << "XmlServices created Style(Outline)" << outline->getInfo();

    _mosaic->addStyle(outline);

    if (_debug) qDebug() << "end outline";
}

void MosaicReader::processFilled(xml_node & node)
{
    bool         oldFormat      = false;
    int          zlevel         = 0;
    int          l_algorithm    = 0;
    bool         l_draw_inside  = false;
    bool         l_draw_outside = true;

    ColorSet     l_colorSet;
    ColorSet     l_colorsB;
    ColorSet     l_colorsW;
    ColorGroup   l_colorGroup;

    ProtoPtr l_proto;
    Xform        l_xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
        {
            procesToolkitGeoLayer(n,l_xf,zlevel);
        }
        else if (str == "style.Style")
        {
            processStylePrototype(n,l_proto);
        }
        else if (str == "style.Colored")
        {
            oldFormat = true;
            processColorSet(n,l_colorSet);
        }
        else if (str == "style.Filled")
        {
            processsStyleFilled(n,l_draw_inside,l_draw_outside,l_algorithm);
        }
        else if (str == "ColorBlacks")
        {
            oldFormat = false;
            processColorSet(n,l_colorsB);
        }
        else if (str == "ColorWhites")
        {
            oldFormat = false;
            processColorSet(n,l_colorsW);
        }
        else if (str == "ColorGroup")
        {
            oldFormat = false;
            processColorGroup(n,l_colorGroup);
        }
        else
        {
            fail("Unexpected", str.c_str());
        }
    }

    auto filled = make_shared<Filled>(l_proto,l_algorithm);
    filled->setCanvasXform(l_xf);
    filled->setZValue(zlevel);

    if (oldFormat)
    {
        // old - redundant way of dealing with colors
        ColorSet * csetW = filled->getWhiteColorSet();
        csetW->addColor(l_colorSet.getColor(0));

        ColorSet * csetB = filled->getBlackColorSet();
        if (l_colorSet.size() >= 2)
            csetB->addColor(l_colorSet.getColor(1));
        else
            csetB->addColor(l_colorSet.getColor(0));
    }
    else
    {
        ColorSet * csetW = filled->getWhiteColorSet();
        csetW->setColors(l_colorsW);

        ColorSet * csetB = filled->getBlackColorSet();
        csetB->setColors(l_colorsB);

        ColorGroup * cgroup = filled->getColorGroup();
        *cgroup = l_colorGroup;

        if (cgroup->size()== 0)
        {
            // for backwards compatabililts
            ColorSet ll_csw;
            if (l_colorsW.size())
            {
                ll_csw.setColors(l_colorsW);
                cgroup->addColorSet(ll_csw);
            }
            ColorSet ll_csb;
            if (l_colorsB.size())
            {
                ll_csw.setColors(l_colorsB);
                cgroup->addColorSet(ll_csb);
            }
        }
        if (cgroup->size()== 0)
        {
            // last resort
            ColorSet cs;
            cs.addColor(Qt::black);
            cgroup->addColorSet(cs);
        }
    }

    filled->setDrawInsideBlacks(l_draw_inside);
    filled->setDrawOutsideWhites(l_draw_outside);

    if (_debug) qDebug().noquote() << "XmlServices created Style(Filled)" << filled->getInfo();

    _mosaic->addStyle(filled);

    if (_debug) qDebug() << "end filled";
}

void MosaicReader::processPlain(xml_node & node)
{
    ColorSet     colorset;
    ProtoPtr proto;
    Xform        xf;
    int          zlevel = 0;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "style.Style")
            processStylePrototype(n,proto);
        else if (str == "style.Colored")
            processColorSet(n,colorset);
        else
            fail("Unexpected", str.c_str());
    }

    auto plain = make_shared<Plain>(proto);
    plain->setCanvasXform(xf);
    plain->setColorSet(colorset);
    plain->setZValue(zlevel);

    if (_debug) qDebug().noquote() << "XmlServices created Style (Plain)" << plain->getInfo();

    _mosaic->addStyle(plain);

    if (_debug) qDebug() << "end plain";
}

void MosaicReader::processSketch(xml_node & node)
{
    ColorSet     colorset;
    ProtoPtr proto;
    Xform        xf;
    int          zlevel = 0;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "style.Style")
            processStylePrototype(n,proto);
        else if (str == "style.Colored")
            processColorSet(n,colorset);
        else
            fail("Unexpected", str.c_str());
    }

    auto sketch = make_shared<Sketch>(proto);
    sketch->setCanvasXform(xf);
    sketch->setColorSet(colorset);
    sketch->setZValue(zlevel);

    if (_debug) qDebug().noquote() << "XmlServices created Style (Sketch)" << sketch->getInfo();

    _mosaic->addStyle(sketch);

    if (_debug) qDebug() << "end sketch";
}

void MosaicReader::processEmboss(xml_node & node)
{
    ColorSet     colorset;
    eDrawOutline draw_outline = OUTLINE_NONE;
    qreal        width        = 0.0;
    Qt::PenJoinStyle pjs      = Qt::BevelJoin;
    Qt::PenCapStyle pcs       = Qt::SquareCap;
    qreal        outlineWidth = 0.03;
    QColor       outlineColor = Qt::black;
    qreal        angle        = 0.0;
    int          zlevel       = 0;
    ProtoPtr proto;
    Xform        xf;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "style.Style")
            processStylePrototype(n,proto);
        else if (str == "style.Colored")
            processColorSet(n,colorset);
        else if (str == "style.Thick")
            processsStyleThick(n,draw_outline,width,outlineWidth,outlineColor,pjs,pcs);
        else if (str == "style.Emboss")
            processsStyleEmboss(n,angle);
        else
            fail("Unexpected", str.c_str());
    }

    if (_debug) qDebug() << "Constructing Emboss from prototype and poly";
    auto emboss = make_shared<Emboss>(proto);
    emboss->setCanvasXform(xf);
    emboss->setColorSet(colorset);
    emboss->setDrawOutline(draw_outline);
    if (draw_outline)
    {
        emboss->setOutlineColor(outlineColor);
        emboss->setOutlineWidth(outlineWidth);
    }
    emboss->setLineWidth(width);
    emboss->setJoinStyle(pjs);
    emboss->setCapStyle(pcs);
    emboss->setAngle(angle);
    emboss->setZValue(zlevel);

    if (_debug) qDebug().noquote() << "XmlServices created Style(Emboss)" << emboss->getInfo();

    _mosaic->addStyle(emboss);

    if (_debug) qDebug() << "end emboss";
}

void MosaicReader::processTileColors(xml_node & node)
{
    ProtoPtr proto;
    Xform        xf;
    bool         outline = false;
    int          width   = 3;
    int          zlevel  = 0;
    QColor       color   = Qt::white;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
            procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "style.Style")
            processStylePrototype(n,proto);
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

    if (_debug) qDebug() << "Constructing TileColors from prototype and poly";
    auto tc  = make_shared<TileColors>(proto);
    tc->setCanvasXform(xf);
    if (outline)
    {
        tc->setOutline(true,color,width);
    }
    tc->setZValue(zlevel);

    if (_debug) qDebug().noquote() << "XmlServices created Style(TileColors)" << tc->getInfo();

    _mosaic->addStyle(tc);

    if (_debug) qDebug() << "end TileColors";
}

void MosaicReader::procesToolkitGeoLayer(xml_node & node, Xform & xf, int & zlevel)
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
        xf.setModelCenter(QPointF(x,y));
    }
    n = node.child("Z");
    if (n)
    {
        val          = n.child_value();
        zlevel       = val.toInt();
    }
}

void MosaicReader::processStylePrototype(xml_node & node, ProtoPtr & proto)
{
    xml_node n = node.child("prototype");
    if (_debug) qDebug().noquote() << n.name();
    view->dumpRefs();
    proto = getPrototype(n);
    view->dumpRefs();
    if (_debug) qDebug().noquote() << proto->getInfo();
}

#if 0
void XmlLoader::processColorSet(xml_node & node, QColor & color)
{
    xml_node n   = node.child("color");
    color        = processColor(n);
}
#endif
void MosaicReader::processColorSet(xml_node & node, ColorSet &colorSet)
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

void MosaicReader::processColorGroup(xml_node & node, ColorGroup &colorGroup)
{
    xml_node n;
    for (n = node.child("Group"); n; n = n.next_sibling("Group"))
    {
        ColorSet cset;
        processColorSet(n,cset);
        colorGroup.addColorSet(cset);
    }
}

QColor MosaicReader::processColor(xml_node & n)
{
    QString str = n.child_value();
    QColor color(str);
    return color;
}

qreal MosaicReader::procWidth(xml_node & node)
{
    QString str = node.child_value();
    return str.toDouble();
}

void MosaicReader::processsStyleThick(xml_node & node, eDrawOutline & draw_outline, qreal & width, qreal & outlineWidth, QColor & outlineColor, Qt::PenJoinStyle & pjs, Qt::PenCapStyle &pcs)
{
    QString w = node.child_value("width");
    width = w.toDouble();

    QString str  = node.child_value("draw__outline");
    if (str == "true")
    {
        draw_outline = OUTLINE_DEFAULT;    // black, width 1

        auto cnode = node.child("outline_width");
        if (cnode)
        {
            QString ow = cnode.child_value();
            bool ok;
            qreal qow = ow.toDouble(&ok);
            if (ok)
            {
                outlineWidth = qow;
                draw_outline = OUTLINE_SET;
            }
        }
        auto cnode2 = node.child("outline_color");
        if (cnode2)
        {
            QString col = cnode2.child_value();
            outlineColor  = QColor(col);
        }
    }

    auto jsnode = node.child("join");
    if (jsnode)
    {
        QString str = jsnode.child_value();
        if (str == "miter")
        {
            pjs = Qt::MiterJoin;
        }
        else if (str == "bevel")
        {
            pjs = Qt::BevelJoin;
        }
        else if (str == "round")
        {
            pjs = Qt::RoundJoin;
        }
    }

    auto csnode = node.child("cap");
    if (jsnode)
    {
        QString str = csnode.child_value();
        if (str == "square")
        {
            pcs = Qt::SquareCap;
        }
        else if (str == "flat")
        {
            pcs = Qt::FlatCap;
        }
        else if (str == "round")
        {
            pcs = Qt::RoundCap;
        }
    }
 }

void MosaicReader::processsStyleInterlace(xml_node & node, qreal & gap, qreal & shadow, bool & includeTipVerts, bool & start_under)
{
    QString str  = node.child_value("gap");
    gap =  str.toDouble();

    str = node.child_value("shadow");
    shadow = str.toDouble();

    str = node.child_value("includeTipVerts");
    includeTipVerts = (str == "true");

    str = node.child_value("startUnder");
    start_under = (str == "true");
}

void MosaicReader::processsStyleFilled(xml_node & node, bool & draw_inside, bool & draw_outside, int & algorithm)
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

void MosaicReader::processsStyleEmboss(xml_node & node, qreal & angle)
{
    QString str  = node.child_value("angle");
    angle =  str.toDouble();
}

PolyPtr MosaicReader::getBoundary(xml_node & node)
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

PolyPtr MosaicReader::getPolygon(xml_node & node)
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

QPointF MosaicReader::getPos(xml_node & node)
{
    QString txt = node.child_value();
    QStringList qsl;
    qsl = txt.split(',');
    qreal x = qsl[0].toDouble();
    qreal y = qsl[1].toDouble();
    return QPointF(x,y);
}

QRectF  MosaicReader::getRectangle(xml_node node)
{
    QString x,y,width,height;

    xml_node n = node.child("X");
    x = n.child_value();
    n = node.child("Y");
    y = n.child_value();
    n = node.child("width");
    width = n.child_value();
    n = node.child("height");
    height = n.child_value();
    return QRectF(x.toDouble(),y.toDouble(),width.toDouble(),height.toDouble());
}

Circle MosaicReader::getCircle(xml_node node)
{
    QString x,y,radius;

    xml_node n = node.child("X");
    x = n.child_value();
    n = node.child("Y");
    y = n.child_value();
    n = node.child("radius");
    radius = n.child_value();
    return Circle(QPointF(x.toDouble(),y.toDouble()),radius.toDouble());
}

ProtoPtr MosaicReader::getPrototype(xml_node & node)
{
    if (_debug) qDebug().noquote() << node.name();
    if (hasReference(node))
    {
        return getProtoReferencedPtr(node);
    }

    xml_node protonode = node.child("app.Prototype");
    if (_debug) qDebug().noquote() << protonode.name();

    // load tiling
    bool localTiling = false;     // indicates whether there is an associated tiling
    TilingPtr tp;
    xml_node tnode = protonode.child("string");
    if (tnode)
    {
        QString tilingName = tnode.child_value();
        qDebug().noquote() << "Tiling Name:" << tilingName;
#if 0
        // has tiling been already loaded by another prototype
        tp = findTiling(tilingName);
        if (!tp)
        {
            // load tiling
            qDebug().noquote() << "Loading tiling" << tilingName;
            TilingManager tm;
            tp = tm.loadTiling(tilingName,TILM_LOAD_FROM_MOSAIC);
            if (tp)
            {
                _tilings.push_back(tp);
            }
            else
            {
                qInfo() << "Tiling not loaded :" << tilingName;
            }
        }
#else
        // load tiling
        qDebug().noquote() << "Loading tiling" << tilingName;
        TilingManager tm;
        tp = tm.loadTiling(tilingName,TILM_LOAD_FROM_MOSAIC);
        if (tp)
        {
            _tilings.push_back(tp);
        }
        else
        {
            qInfo() << "Tiling not loaded :" << tilingName;
        }
#endif
    }

    if (!tp)
    {
        // 18JAN2022 - adds this case for mosaics with maps but no tiling
        tp = make_shared<Tiling>();
        _tilings.push_back(tp);
        localTiling = true;
    }

    if (_debug) qDebug() << "Creating new prototype";

    ProtoPtr proto = make_shared<Prototype>(tp);
    setProtoReference(node,proto);

    QVector<TilePtr> uniqueTiles = tp->getUniqueTiles();
    int numTiles = uniqueTiles.size();

    QVector<TilePtr> usedTiles;
    TileReader tr;
    xml_node entry;
    for (entry = protonode.child("entry"); entry; entry = entry.next_sibling("entry"))
    {
        bool found = false;

        TilePtr  tile;
        MotifPtr motif;
        QString  name;

        xml_node xmlTile = entry.first_child();
        name = xmlTile.name();
        qDebug().noquote() << name;
        if (name == "tile.Feature")
        {
            if (_debug) qDebug() << "adding Tile";
            tile = getTile(tr,xmlTile);
        }
        else
        {
           fail("tile not found", "");
        }

        int fsides = tile->numSides();

        xml_node xmlMotif  = xmlTile.next_sibling();
        name = xmlMotif.name();
        if (_debug) qDebug().noquote() << name;

        if (name == "app.Star")
        {
            if (_debug) qDebug() << "adding Star Motif";
            motif =  getStar(xmlMotif,fsides);
            found = true;
        }
        else if (name == "ExtendedStar")
        {
            if (_debug) qDebug() << "adding ExtendedStar Motif";
            motif =  getExtendedStar(xmlMotif,fsides);
            found = true;
        }
        else if (name == "app.Rosette")
        {
            if (_debug) qDebug() << "adding Rosette Motif";
            motif =  getRosette(xmlMotif,fsides);
            found = true;
        }
        else if (name == "ExtendedRosette")
        {
            if (_debug) qDebug() << "adding ExtendedRosette Motif";
            motif =  getExtendedRosette(xmlMotif,fsides);
            found = true;
        }
        else if (name == "app.ConnectFigure")
        {
            if (_debug) qDebug() << "adding Connect Motif";
            motif =  getConnectMotif(xmlMotif,fsides);
            found = true;
        }
        else if (  name == "app.ExplicitFigure" || name == "app.Infer"           || name == "app.ExplicitFeature"    || name == "app.ExplicitGirih"
                || name == "app.ExplicitStar"   || name == "app.ExplicitRosette" || name == "app.ExplicitHourglass"  || name ==  "app.ExplicitIntersect"
                || name == "IrregularNoMotif")
        {
            if (_debug) qDebug() << "adding Irregular Motif";
            motif = getIrregularMotif(xmlMotif, fsides,motifRepresentation.key(name));
            found = true;
        }
        else
        {
            _failMessage = "Motif type not found: " + name;
            qWarning() << _failMessage;
            throw(_failMessage);
        }

        if (found)
        {
            // if the found tile is identical to the one in the known tiling then use that
            // DAC 27MAY17 - imprtant that this code not removed or Mosaic View will fail
            bool found2 = false;
            for (auto & utile :  qAsConst(uniqueTiles))
            {
                if (usedTiles.contains(utile))
                    continue;

                if (utile->equals(tile))
                {
                    usedTiles.push_back(utile);
                    tile = utile;
                    if (_debug) qDebug().noquote() << "adding to Proto" << motif->getMotifDesc();
                    DesignElementPtr  dep = make_shared<DesignElement>(tile, motif);
                    proto->addElement(dep);
                    if (_debug) qDebug().noquote() << "design element:" << dep->toString();
                    found2 = true;
                    break;
                }
            }
            if (!found2)
            {
                if (!localTiling)
                {
                    _failMessage = "Mismatch between Design and Tiling";
                    qWarning() << _failMessage;
                    qWarning().noquote() << "No match for Mosaic tile in tiling" << _fileName << motif->getMotifDesc();

                    if (!Configuration::getInstance()->localCycle)
                    {
                        QMessageBox box(ControlPanel::getInstance());
                        box.setIcon(QMessageBox::Critical);
                        box.setText(_failMessage);
                        box.addButton(new QPushButton("Try to fix"),QMessageBox::AcceptRole);
                        box.addButton(QMessageBox::Abort);
                        int rv = box.exec();
                        if (rv == QMessageBox::Abort)
                        {
                            // this mismatch cannot be handled
                            throw(_failMessage);
                        }
                    }
                }
                DesignElementPtr  del = make_shared<DesignElement>(tile, motif);
                proto->addElement(del);
            }
        }
    }

    QVector<DesignElementPtr>  & designElements = proto->getDesignElements();
    int numDesignElements = designElements.size();
    if (numTiles && (numTiles != numDesignElements) && !Configuration::getInstance()->localCycle)
    {
        QString str1 = "Tile/DesignElement MISMATCH";
        QString str2;
        QDebug  deb(&str2);
        deb <<  "Num Unqiue Tiles =" << numTiles << endl << "Num DesignElements =" << numDesignElements;
        qWarning() << str1;
        qWarning() << str2;
        QMessageBox box(ControlPanel::getInstance());
        box.setWindowTitle(_fileName);
        int len = _fileName.length();
        box.setIcon(QMessageBox::Critical);
        int len2 = str1.length();
        if (len2 < len)
        {
            int diff = len-len2;
            str1.resize(len + (diff*2),' ');
        }
        box.setText(str1);
        box.setInformativeText(str2);
        box.exec();
    }

    // create explicit maps
    for (auto & del : qAsConst(designElements))
    {
        MotifPtr motif = del->getMotif();
        TilePtr  tile  = del->getTile();
        QPolygonF poly = tile->getPolygon();

        if (motif->isExplicit())
        {
            switch (motif->getMotifType())
            {
            case MOTIF_TYPE_EXPLICIT_MAP:
            {
                // already has a map
                auto exp = std::dynamic_pointer_cast<ExplicitMapMotif>(motif);
                exp->setup(tile);
                exp->setMotifBoundary(poly);
                exp->createExtendedBoundary(tile);
            }   break;

            case MOTIF_TYPE_IRREGULAR_NO_MAP:
            {
                auto irr = std::dynamic_pointer_cast<IrregularMotif>(motif);
                irr->setup(tile);
                irr->setMotifBoundary(poly);
                irr->createExtendedBoundary(tile);
            }   break;

            case MOTIF_TYPE_INFERRED:
            {
                auto infer = std::dynamic_pointer_cast<InferredMotif>(motif);
                infer->setup(tile);
                infer->setupInfer(proto);
                infer->setMotifBoundary(poly);
                infer->createExtendedBoundary(tile);
            }   break;

            case MOTIF_TYPE_IRREGULAR_ROSETTE:
            {
                auto rose = std::dynamic_pointer_cast<IrregularRosette>(motif);
                rose->setup(tile);
                rose->setMotifBoundary(poly);
                rose->createExtendedBoundary(tile);
            }   break;

            case MOTIF_TYPE_HOURGLASS:
            {
                auto hour = std::dynamic_pointer_cast<HourglassMotif>(motif);
                hour->setup(tile);
                hour->setMotifBoundary(poly);
                hour->createExtendedBoundary(tile);
            }   break;

            case MOTIF_TYPE_INTERSECT:
            {
                auto expl = std::dynamic_pointer_cast<IntersectMotif>(motif);
                expl->setup(tile);
                expl->setMotifBoundary(poly);
                expl->createExtendedBoundary(tile);
            }   break;

            case MOTIF_TYPE_IRREGULAR_STAR:
            {
                auto star = std::dynamic_pointer_cast<IrregularStar>(motif);
                star->setup(tile);
                star->setMotifBoundary(poly);
                star->createExtendedBoundary(tile);
            }   break;

            case MOTIF_TYPE_EXPLCIT_TILE:
            {
                auto etile = std::dynamic_pointer_cast<TileMotif>(motif);
                etile->setup(tile);
                etile->setMotifBoundary(poly);
                etile->createExtendedBoundary(tile);
            }   break;

            case MOTIF_TYPE_GIRIH:
            {
                auto girih = std::dynamic_pointer_cast<GirihMotif>(motif);
                girih->setup(tile);
                girih->setMotifBoundary(poly);
                girih->createExtendedBoundary(tile);
            }   break;

            default:
                qWarning("Unxpected explicit type");
            }
        }
    }

    if (_debug) qDebug() << "Proto created";

    return proto;
}

TilePtr MosaicReader::getTile(TileReader & fr, xml_node & node)
{
    if (hasReference(node))
    {
        TilePtr f = getTileReferencedPtr(node);
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
    TilePtr f;
    if (poly)
    {
        PolyPtr b    = getPolygon(poly);
        EdgePoly ep(b);

        f = make_shared<Tile>(ep,rotation,scale);
        setTileReference(node,f);
        f->setRegular(regular);
        return f;
    }
    poly = node.child("edges");
    if (poly)
    {
        EdgePoly ep = fr.getEdgePoly(poly);
        f = make_shared<Tile>(ep,rotation,scale);
        f->setRegular(regular);
        if (((_version == 5) || (_version ==6)) && (!Loose::zero(rotation) || !Loose::equals(scale,1.0)))
        {
            qWarning() << "Decomposing Tile for backwards compatability"  << _fileName;
            f->decompose();
        }

        setTileReference(node,f);
        return f;
    }
    return f;
}

void MosaicReader::getMotifCommon(xml_node & node, MotifPtr motif, int tile_sides)
{
    QString str;
    ExtendedBoundary & eb = motif->getRWExtendedBoundary();

    if (node.child("boundarySides"))
    {
        str = node.child_value("boundarySides");
        int bsides = str.toInt();
        eb.setSides(bsides);
    }

    if (node.child("boundaryScale"))
    {
        str = node.child_value("boundaryScale");
        qreal bscale = str.toDouble();
        eb.setScale(bscale);
    }

    if (motif->isRadial() || _version >= 13)
    {
        if (node.child("figureScale"))
        {
            str = node.child_value("figureScale");
            qreal fscale = str.toDouble();
            motif->setMotifScale(fscale);
        }

        if (node.child("r"))
        {
            str = node.child_value("r");
            qreal r = str.toDouble();
            motif->setMotifRotate(r);
        }
    }

    if (node.child("n"))
    {
        str = node.child_value("n");
        int sides = str.toInt();
        motif->setN(sides);
    }
    else
    {
        motif->setN(tile_sides);
    }

    int version = 1;
    if (node.child("version"))
    {
        str = node.child_value("version");
        version = str.toInt();
    }
    motif->setVersion(version);
}

IrregularPtr MosaicReader::getIrregularMotif(xml_node & node, int tile_sides, eMotifType type)
{
    IrregularPtr ep;

    if (hasReference(node))
    {
        ep = getExplicitReferencedPtr(node);
        return ep;
    }

    if (_debug) qDebug().noquote() << "getIrregularMotif type:" << sMotifType[type];

    if (type == MOTIF_TYPE_EXPLICIT_MAP)
    {
        // only the old original taprats formats have maps
        _currentMap = getMap(node);
        ep = make_shared<ExplicitMapMotif>(_currentMap);
        getMotifCommon(node,ep,tile_sides);
    }
    else if (type == MOTIF_TYPE_IRREGULAR_NO_MAP)
    {
        ep = make_shared<IrregularMotif>();
        getMotifCommon(node,ep,tile_sides);
    }
    else if (type == MOTIF_TYPE_INFERRED)
    {
        ep = make_shared<InferredMotif>();
        getMotifCommon(node,ep,tile_sides);
    }
    else if (type == MOTIF_TYPE_EXPLCIT_TILE)
    {
        ep = make_shared<TileMotif>();
        getMotifCommon(node,ep,tile_sides);
    }
    else if  (type == MOTIF_TYPE_GIRIH)
    {
        auto exp = make_shared<GirihMotif>();
        getMotifCommon(node,exp,tile_sides);

        QString str;
        if (node.child("sides"))
        {
            Q_ASSERT(_version < 14);
            str = node.child_value("sides");
            int sides = str.toInt();
            exp->setN(sides);
        }

        str = node.child_value("skip");
        qreal skip  = str.toDouble();

        exp->init(skip);
        ep = exp;
    }
    else if (type == MOTIF_TYPE_IRREGULAR_STAR)
    {
        auto star = make_shared<IrregularStar>();
        getMotifCommon(node,star,tile_sides);

        QString str;
        str = node.child_value("s");
        int s = str.toInt();

        str = node.child_value("d");
        qreal d  = str.toDouble();

        star->init(d,s);

        ep = star;
    }
    else if (type == MOTIF_TYPE_IRREGULAR_ROSETTE)
    {
        auto rose = make_shared<IrregularRosette>();
        getMotifCommon(node,rose,tile_sides);

        QString str;
        str = node.child_value("s");
        int s = str.toInt();

        str = node.child_value("q");
        qreal q  = str.toDouble();

        qreal r_flexPt  = 0.5;
        if (node.child("rFlexPt"))
        {
            str = node.child_value("rFlexPt");
            r_flexPt = str.toDouble();
        }

        rose->init(q,r_flexPt,s);

        ep = rose;
    }
    else if (type == MOTIF_TYPE_HOURGLASS)
    {
        auto hour = make_shared<HourglassMotif>();
        getMotifCommon(node,hour,tile_sides);

        QString str;
        str = node.child_value("s");
        int s = str.toInt();

        str = node.child_value("d");
        qreal d  = str.toDouble();

        hour->init(d,s);
        ep = hour;
    }
    else if (type == MOTIF_TYPE_INTERSECT)
    {
        auto isect = make_shared<IntersectMotif>();
        getMotifCommon(node,isect,tile_sides);

        QString str;

        str = node.child_value("s");
        int s = str.toInt();

        str = node.child_value("skip");
        qreal skip  = str.toDouble();

        str = node.child_value("progressive");
        bool prog = (str == "t");

        if (node.child("sides"))
        {
            Q_ASSERT(_version < 14);
            str = node.child_value("sides");
            int sides = str.toInt();
            isect->setN(sides);
        }

        isect->init(skip,s,prog);
        ep = isect;
    }

    Q_ASSERT(ep);
    setExplicitReference(node,ep);

    return(ep);
}

StarPtr MosaicReader::getStar(xml_node & node, int tile_sides)
{
    if (hasReference(node))
    {
        StarPtr f = getStarReferencedPtr(node);
        return f;
    }

    if (_debug) qDebug() << "getStar";

    _currentMap = getMap(node);

    QString str;
    str = node.child_value("n");
    int n = str.toInt();

    str = node.child_value("d");
    qreal d = str.toDouble();

    str = node.child_value("s");
    int s = str.toInt();

    StarPtr star = make_shared<Star>(n, d, s);
    setStarReference(node,star);

    auto & eb = star->getRWExtendedBoundary();
    eb.setSides(tile_sides);      // default

    getMotifCommon(node,star,tile_sides);

    return star;
}

ExtStarPtr  MosaicReader::getExtendedStar(xml_node & node, int tile_sides)
{
    if (hasReference(node))
    {
        ExtStarPtr f = getExtStarReferencedPtr(node);
        return f;
    }

    if (_debug) qDebug() << "getExtendedStar";

    _currentMap = getMap(node);

    bool extendPeripherals       = false;   // default
    bool extendFreeVertices      = false;   // default
    bool connectBoundaryVertices = false;   // default

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

    if (_version >= 13)
    {
        xml_attribute con_bnd_v = node.attribute("connectBoundaryVertices");
        if (con_bnd_v)
        {
            str = (con_bnd_v.value());
            connectBoundaryVertices = (str == "t");
        }
    }

    str = node.child_value("n");
    int n = str.toInt();

    str = node.child_value("d");
    qreal d = str.toDouble();

    str = node.child_value("s");
    int s = str.toInt();

    ExtStarPtr star = make_shared<ExtendedStar>(n, d, s);

    auto & extender = star->getExtender();
    extender.setExtendPeripheralVertices(extendPeripherals);
    extender.setExtendFreeVertices(extendFreeVertices);
    extender.setConnectBoundaryVertices(connectBoundaryVertices);

    auto & eb = star->getRWExtendedBoundary();
    eb.setSides(tile_sides);      // default

    getMotifCommon(node,star,tile_sides);

    setExtStarReference(node,star);

    return star;
}

RosettePtr MosaicReader::getRosette(xml_node & node, int tile_sides)
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

    qreal k = 0.0;
    str = node.child_value("k");
    if (!str.isEmpty())
        k = str.toDouble();

    RosettePtr rosette = make_shared<Rosette>(n, q, s, k);

    auto & eb = rosette->getRWExtendedBoundary();
    eb.setSides(tile_sides);      // default

    getMotifCommon(node,rosette,tile_sides);

    setRosetteReference(node,rosette);

    return rosette;
}

ExtRosettePtr  MosaicReader::getExtendedRosette(xml_node & node, int tile_sides)
{
    if (hasReference(node))
    {
        ExtRosettePtr f = getExtRosetteReferencedPtr(node);
        return f;
    }

    if (_debug) qDebug() << "getExtendedRosette";

    _currentMap = getMap(node);

    bool extendPeripherals       = false;    // default
    bool extendFreeVertices      = false;     // default
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

    qreal k = 0.0;
    str = node.child_value("k");
    if (!str.isEmpty())
        k = str.toDouble();

    ExtRosettePtr rosette = make_shared<ExtendedRosette>(n, q, s, k);
    setExtRosetteReference(node,rosette);

    auto & extender = rosette->getExtender();
    extender.setExtendPeripheralVertices(extendPeripherals);
    extender.setExtendFreeVertices(extendFreeVertices);
    extender.setConnectBoundaryVertices(connectBoundaryVertices);

    auto & eb = rosette->getRWExtendedBoundary();
    eb.setSides(tile_sides);      // default

    getMotifCommon(node,rosette,tile_sides);
    return rosette;
}

MotifPtr MosaicReader::getConnectMotif(xml_node & node, int tile_sides)
{
    MotifPtr fp;
    xml_node child = node.child("child");
    if (child)
    {
        xml_attribute class1 = child.attribute("class");
        //qDebug() << class1.value();
        if (QString(class1.value()) == "app.Rosette")
        {
            fp = getRosetteConnect(node,tile_sides);
        }
        else if (QString(class1.value()) == "app.Star")
        {
            fp = getStarConnect(node,tile_sides);
        }
        else
        {
            fail("Connect Motif child","");
        }
    }
    else
    {
        fail("Connect Motifz","");
    }
    return fp;
}

RosetteConnectPtr MosaicReader::getRosetteConnect(xml_node & node, int tile_sides)
{
    if (hasReference(node))
    {
        RosetteConnectPtr f = getRosetteConnectReferencedPtr(node);
        return f;
    }

#if 0
    QString str;
    str = node.child_value("n");
    int n = str.toInt();

    str = node.child_value("s");
    qreal s = str.toDouble();
#endif

    RosetteConnectPtr rcp;
    RosettePtr rp;
    xml_node child = node.child("child");
    if (child)
    {
        xml_attribute class1 = child.attribute("class");
        //qDebug() << class1.value();
        if (QString(class1.value()) == "app.Rosette")
        {
            rp = getRosette(child,tile_sides);
            //Q_ASSERT(rp->getN() == n);
            //Q_ASSERT(rp->getS() == s);
            //qDebug() << "connect s:" <<  rp->getS() << s;
        }
        else
        {
            fail("Rosette Connect figure not based on Rosette","");
        }

        rcp = make_shared<RosetteConnect>(rp->getN(),
                                          rp->getQ(),
                                          rp->getS(),
                                          rp->getK());
        auto & eb = rcp->getRWExtendedBoundary();
        eb.setSides(tile_sides);      // default

        getMotifCommon(child,rcp,tile_sides);
        setRosetteConnectReference(node,rcp);
        //qDebug() << rcp->getFigureDesc();
    }
    else
    {
        fail("Connect Motif","");
    }
    return rcp;
}

StarConnectPtr MosaicReader::getStarConnect(xml_node & node, int tile_sides)
{
    if (hasReference(node))
    {
        StarConnectPtr f = getStarConnectReferencedPtr(node);
        return f;
    }

#if 0
    QString str;
    str = node.child_value("n");
    int n = str.toInt();

    str = node.child_value("s");
    qreal s = str.toDouble();
#endif

    StarPtr sp;
    StarConnectPtr scp;
    xml_node child = node.child("child");
    if (child)
    {
        xml_attribute class1 = child.attribute("class");
        //qDebug() << class1.value();
        if (QString(class1.value()) == "app.Star")
        {
            sp = getStar(child,tile_sides);
            //Q_ASSERT(sp->getN() == n);
            //Q_ASSERT(sp->getS() == s);
            //qDebug() << "connect s:" <<  sp->getS() << s;
        }
        else
        {
            fail("Connect figure not based on Star","");
        }
        scp = make_shared<StarConnect>(sp->getN(), sp->getD(), sp->getS());

        auto & eb = scp->getRWExtendedBoundary();
        eb.setSides(tile_sides);      // default

        getMotifCommon(node,scp,tile_sides);
        setStarConnectReference(node,scp);
        //qDebug() << scp->getFigureDesc();
    }
    else
    {
        fail("Connect Motif","");
    }
    return scp;
}

MapPtr MosaicReader::getMap(xml_node & node)
{
    xml_node mapnode = node.child("map");

    if (hasReference(mapnode))
    {
        _currentMap =  getMapReferencedPtr(mapnode);
        return _currentMap;
    }

    _currentMap = make_shared<Map>("loaded map");
    setMapReference(mapnode,_currentMap);

    // vertices
    xml_node vertices = mapnode.child("vertices");
    for (xml_node vertex = vertices.child("Vertex"); vertex; vertex = vertex.next_sibling("Vertex"))
    {
        VertexPtr v = getVertex(vertex);
    }

    if (_version < 5)
    {
        // Edges
        xml_node edges = mapnode.child("edges");
        for (xml_node edge = edges.child("Edge"); edge; edge = edge.next_sibling("Edge"))
        {
            EdgePtr e = getEdge(edge);
            _currentMap->XmlInsertDirect(e);
        }
    }
    else
    {
        // Edges
        xml_node edges = mapnode.child("edges");
        for (xml_node e = edges.first_child(); e; e= e.next_sibling())
        {
            QString name = e.name();
            EdgePtr ep;
            if (name == "Edge")
            {
                ep = getEdge(e);
            }
            else if (name == "curve")
            {
                ep = getCurve(e);
            }
            else if (name == "chord")
            {
                ep = getCurve(e);
            }
            Q_ASSERT(ep);
            //_currentMap->insertEdge(ep);
            _currentMap->XmlInsertDirect(ep);
        }
    }

    _currentMap->resetNeighbourMap();
    _currentMap->getNeighbourMap();     // rebuilds
    _currentMap->verifyAndFix();

    qDebug().noquote() << _currentMap->namedSummary();

    return _currentMap;
}

VertexPtr MosaicReader::getVertex(xml_node & node)
{
    if (hasReference(node))
    {
        VertexPtr vp = getVertexReferencedPtr(node);
        _currentMap->XmlInsertDirect(vp);      // into UniqueQVector
        return vp;
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
    VertexPtr v = make_shared<Vertex>(pt);
    setVertexReference(node,v);

     _currentMap->XmlInsertDirect(v);

    if (_version >= 5)
    {
        return v;
    }

    // the old way
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
            else if (name == "chord")
            {
                ep = getChord(e);
            }
            _currentMap->XmlInsertDirect(ep);
        }
    }
    else
    {
        qWarning("edges2 not found");
    }

    return v;
}

EdgePtr MosaicReader::getEdge(xml_node & node)
{
    //qDebug() << node.name();

    if (hasReference(node))
    {
        return getEdgeReferencedPtr(node);
    }

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

    edge->setV1(v1);
    edge->setV2(v2);
    if (_debug) qDebug().noquote() << "Edge:" << edge.get() << "added" << v1->pt << v2->pt;

    return edge;
}

EdgePtr MosaicReader::getCurve(xml_node & node)
{
    //qDebug() << node.name();

    if (hasReference(node))
    {
        return getEdgeReferencedPtr(node);
    }

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
    edge->setArcCenter(p,convex,false);

    return edge;
}

EdgePtr MosaicReader::getChord(xml_node & node)
{
    //qDebug() << node.name();

    if (hasReference(node))
    {
        return getEdgeReferencedPtr(node);
    }

    EdgePtr edge = make_shared<Edge>();
    setEdgeReference(node,edge);        // early for recursion
    //qDebug() << "created Edge" << edge.get();

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
    edge->setArcCenter(p,convex,true);

    return edge;
}

void MosaicReader::procSize(xml_node & node, int & width, int & height, int & zwidth, int & zheight)
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

    zwidth  = width;
    zheight = height;
    xml_node w2 = node.child("zwidth");
    if (w2)
    {
        val = w2.child_value();
        zwidth = val.toInt();
    }
    xml_node h2 = node.child("zheight");
    if (h2)
    {
        val = h2.child_value();
        zheight = val.toInt();
    }
}

QColor MosaicReader::procBackgroundColor(xml_node & node)
{
    xml_node n   = node.child("color");
    if (n)
        return processColor(n);
    else
        return _background;     // default;
}

QTransform MosaicReader::getQTransform(QString txt)
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

void MosaicReader::procFill(xml_node & node, bool isSingle)
{
    QString txt = node.child_value();
    QStringList qsl;
    qsl = txt.split(',');
    _fillData.set(isSingle,qsl[0].toInt(),qsl[1].toInt(),qsl[2].toInt(),qsl[3].toInt());
}

void MosaicReader::procBorder(xml_node & node)
{
    xml_attribute atype = node.attribute("type");
    if (!atype)  return;

    if (_version < 12)
    {
        // legacy version code
        int iType = atype.as_int();
        if (iType == 4 && _version < 11)
        {
            procCrop(node);
        }
        else
        {
            eBorderType type = static_cast<eBorderType>(iType);
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
    }
    else
    {
        // current version code
        Q_ASSERT(_version >= 12);
        QString type = atype.as_string();
        if (type == "Plain")
            procBorderPlain(node);
        else if (type == "2Color")
            procBorderTwoColor(node);
        else  if (type == "Blocks")
            procBorderBlocks(node);
    }
}

void MosaicReader::procCrop(xml_node & node)
{
    QRectF rect;
    xml_node bdry = node.child("boundary");
    if (bdry)
    {
        QString str = bdry.child_value();
        QStringList blist = str.split(",");

        QString s = blist[0];
        qreal   x = s.toDouble();

        s = blist[1];
        qreal y = s.toDouble();

        s= blist[2];
        qreal w = s.toDouble();

        s = blist[3];
        qreal h = s.toDouble();

        rect = QRectF(x,y,w,h);
    }

    _crop = make_shared<Crop>();
    _crop->setRect(rect);
    if (_version < 15)
    {
        _crop->setEmbed(true);
        _crop->setApply(true);
    }
    else
    {
        bool embed = false;
        bool apply = false;
        xml_attribute attr;
        attr = node.attribute("embed");
        if (attr)
        {
            QString val = attr.value();
            embed = (val == "t") ? true : false;
        }
        attr = node.attribute("apply");
        if (attr)
        {
            QString val = attr.value();
            apply = (val == "t") ? true : false;
        }
        _crop->setEmbed(embed);
        _crop->setApply(apply);
    }
}

void MosaicReader::procBorderPlain(xml_node & node)
{
    xml_node cnode = node.child("color");
    QColor col1 = processColor(cnode);

    qreal bwidth = 20.0;
    xml_node wnode = node.child("width");
    if (wnode)
        bwidth = procWidth(wnode);

    xml_attribute attr = node.attribute("shape") ;
    if (attr)
    {
        QString shape = attr.as_string();
        if (shape == "Rectangle")
        {
            xml_node rnode = node.child("rect");
            QRectF rect = getRectangle(rnode);
            _border = make_shared<BorderPlain>(rect,bwidth, col1);
            _border->construct();
        }
        else if (shape == "Circle")
        {
            xml_node rnode = node.child("circle");
            auto c = getCircle(rnode);
            _border = make_shared<BorderPlain>(c,bwidth, col1);
            _border->construct();
        }
    }
    else
    {
        _border = make_shared<BorderPlain>(QSize(_width,_height),bwidth, col1);
        _border->construct();
    }
}

void MosaicReader::procBorderTwoColor(xml_node & node)
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

void MosaicReader::procBorderBlocks(xml_node & node)
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

void MosaicReader::setProtoReference(xml_node & node, ProtoPtr ptr)
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

ProtoPtr MosaicReader::getProtoReferencedPtr(xml_node & node)
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
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

void MosaicReader::setEdgeReference(xml_node & node, EdgePtr ptr)
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

EdgePtr MosaicReader::getEdgeReferencedPtr(xml_node & node)
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

void   MosaicReader::setPolyReference(xml_node & node, PolyPtr ptr)
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

void   MosaicReader::setTileReference(xml_node & node, TilePtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        tile_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref tile:" << i;
#endif
    }
}
void   MosaicReader::setFMotifReference(xml_node & node, MotifPtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        motif_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref motif:" << i;
#endif
    }
}
void   MosaicReader::setExplicitReference(xml_node & node, IrregularPtr ptr)
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

void   MosaicReader::setStarReference(xml_node & node, StarPtr ptr)
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

void  MosaicReader::setExtStarReference(xml_node & node, ExtStarPtr ptr)
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

void   MosaicReader::setRosetteReference(xml_node & node, RosettePtr ptr)
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

void  MosaicReader::setExtRosetteReference(xml_node & node, ExtRosettePtr ptr)
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

void   MosaicReader::setRosetteConnectReference(xml_node & node, RosetteConnectPtr ptr)
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

void   MosaicReader::setStarConnectReference(xml_node & node, StarConnectPtr ptr)
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

void   MosaicReader::setMapReference(xml_node & node, MapPtr ptr)
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

PolyPtr MosaicReader::getPolyReferencedPtr(xml_node & node)
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

TilePtr MosaicReader::getTileReferencedPtr(xml_node & node)
{
    TilePtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = TilePtr(tile_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

MotifPtr MosaicReader::getMotifReferencedPtr(xml_node & node)
{
    MotifPtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = MotifPtr(motif_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

IrregularPtr MosaicReader::getExplicitReferencedPtr(xml_node & node)
{
    IrregularPtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = IrregularPtr(explicit_ids[id]);
        if (!retval)
            fail("reference NOT FOUND:",QString::number(id));
    }
    return retval;
}

StarPtr MosaicReader::getStarReferencedPtr(xml_node & node)
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

ExtStarPtr MosaicReader::getExtStarReferencedPtr(xml_node & node)
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


RosettePtr MosaicReader::getRosetteReferencedPtr(xml_node & node)
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

ExtRosettePtr MosaicReader::getExtRosetteReferencedPtr(xml_node & node)
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

RosetteConnectPtr MosaicReader::getRosetteConnectReferencedPtr(xml_node & node)
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

StarConnectPtr MosaicReader::getStarConnectReferencedPtr(xml_node & node)
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

MapPtr MosaicReader::getMapReferencedPtr(xml_node & node)\
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

void MosaicReader::fail(QString a, QString b)
{
    _failMessage = QString("%1 %2").arg(a).arg(b);
    qWarning().noquote() << _failMessage;
    throw(_failMessage);
}


TilingPtr MosaicReader::findTiling(QString name)
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
