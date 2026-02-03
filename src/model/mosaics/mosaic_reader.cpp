#include <QMessageBox>

#include "gui/top/controlpanel.h"   // required
#include "gui/top/system_view_controller.h"
#include "model/mosaics/border.h"
#include "model/mosaics/legacy_loader.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_reader.h"
#include "model/mosaics/reader_base.h"
#include "model/motifs/explicit_map_motif.h"
#include "model/motifs/extender.h"
#include "model/motifs/girih_motif.h"
#include "model/motifs/hourglass_motif.h"
#include "model/motifs/inferred_motif.h"
#include "model/motifs/intersect_motif.h"
#include "model/motifs/irregular_motif.h"
#include "model/motifs/irregular_rosette.h"
#include "model/motifs/irregular_star.h"
#include "model/motifs/motif.h"
#include "model/motifs/rosette.h"
#include "model/motifs/rosette2.h"
#include "model/motifs/star.h"
#include "model/motifs/star2.h"
#include "model/motifs/tile_motif.h"
#include "model/prototypes/design_element.h"
#include "model/prototypes/prototype.h"
#include "model/settings/canvas_settings.h"
#include "model/styles/emboss.h"
#include "model/styles/interlace.h"
#include "model/styles/outline.h"
#include "model/styles/plain.h"
#include "model/styles/sketch.h"
#include "model/styles/thick.h"
#include "model/styles/tile_colors.h"
#include "model/tilings/backgroundimage.h"
#include "model/tilings/tile.h"
#include "model/tilings/tile_reader.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling_manager.h"
#include "model/tilings/tiling_reader.h"
#include "model/tilings/tiling_writer.h"
#include "sys/geometry/loose.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_verifier.h"
#include "sys/sys/fileservices.h"

#undef  DEBUG_REFERENCES

using std::make_shared;

MosaicReader::MosaicReader(SystemViewController * vc)
{
    _vc           = vc;

    // defaults
    _background   = QColor(Qt::white);
    _viewSize     = QSize(Sys::DEFAULT_WIDTH, Sys::DEFAULT_HEIGHT);
    _version      = 0;
    _debug        = false;
    _cleanseLevel = 0;
    _cleanseSensitivity = -1;
    _legacyCenterConverted = false;
}

// called by page loader and by BMPEngine
MosaicPtr MosaicReader::readXML(VersionedFile xfile)
{
    Sys::dumpRefs();

    _xfile = xfile;
    qInfo().noquote() << "MosaicLoader::readXML()" << _xfile.getPathedName() << " : start";

    xml_document doc;
    xml_parse_result result = doc.load_file(_xfile.getPathedName().toStdString().c_str());
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

        correctMotifScaleandRotation();
        _mosaic->setLegacyModelConverted(_legacyCenterConverted);

        Sys::dumpRefs();

        qInfo().noquote() << "MosaicLoader load" << _xfile.getPathedName() << " : complete";

        return _mosaic;
    }
    catch (...)
    {
        QString str =  "ERROR loading XML file"  +  _xfile.getPathedName();
        qWarning() << str;
        _failMessage += "\n" + str;
        _mosaic.reset();
        return _mosaic;
    }
}

void MosaicReader::parseXML(xml_document & doc)
{
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

    Sys::dumpRefs();

    if (_debug) qDebug() << "MosaicLoader - end parsing";
}

void MosaicReader::correctMotifScaleandRotation()
{
    if (_version >= 16)
    {
        return;
    }

    for (auto & proto : _mosaic->getPrototypes())
    {
        for (const auto & del : std::as_const(proto->getDesignElements()))
        {
            auto tile  = del->getTile();
            auto motif = del->getMotif();

            qreal tscale = tile->getScale();
            qreal trot   = tile->getRotation();
            qreal mscale = motif->getMotifScale();
            qreal mrot   = motif->getMotifRotate();

            if (!Loose::equals(tscale,1.0))
            {
                qInfo() << "CONVERT   scale: motif" << mscale << "tile" << tscale;
                mscale /= tscale;
                motif->setMotifScale(mscale);
                qInfo() << "CONVERTED scale: motif" << mscale;
                motif->resetMotifMap();
            }

            if (!Loose::zero(trot))
            {
                qInfo() << "CONVERT rot    : motif" << mrot << "tile" << trot;
                mrot -= trot;
                motif->setMotifRotate(mrot);
                qInfo() << "CONVERTED rot  : motif" << mrot;
                motif->resetMotifMap();
            }
        }
    }
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
        else if (str == "style.Border")
            continue;
        else
            fail("Unexpected", str.c_str());
    }

        auto style       = _mosaic->getFirstStyle();
        _firstStyleXform = style->getModelXform();

    // pass 2
    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();
        if (str  == "designNotes")
            processDesignNotes(n);
        else if (str == "design")
            continue;               //processDesign(n);       // why is this here - isn't it too late
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
        else if (str == "style.Border")
            processBorder(n);
        else
            fail("Unexpected", str.c_str());
    }
    if (_debug) qDebug() << "end vector";

    CanvasSettings cs;
    cs.setBackgroundColor(_background);
    cs.setViewSize(_viewSize);
    cs.setCanvasSize(_canvasSize);

    // Canvas Settings fill data defaults to FillData defaults, loader can  override these
    if (_fillData.isSet())
    {
        if (_debug) qDebug() << "Using Mosaic FiilData";
        cs.setFillData(_fillData);
    }
    else if (_tilings.size() > 0)
    {
        if (_debug) qDebug() << "Using Tiling FiilData";
        FillData fdt =  getFirstTiling()->hdr().getCanvasSettings().getFillData();
        if (fdt.isSet())
        {
            cs.setFillData(fdt);
        }
        else
        {
            FillData fd;    // default
            fd.setLegacyDefaults();
            cs.setFillData(fd);
        }
    }
    else
    {
        if (_debug) qDebug() << "Using Default FiilData";
        FillData fd;
        fd.setLegacyDefaults();
        cs.setFillData(fd);
    }

    _mosaic->setCanvasSettings(cs);

    if (_mosbip)
    {
        _mosaic->setBkgdImage(_mosbip);
    }

    if (_border)
    {
        if (_version < 17)
        {
            // convert legacy borders which were built using screen units
            _border->setRequiresConversion(true);
        }
        _mosaic->addStyle(_border);
    }

    if (_crop)
    {
        _mosaic->setCrop(_crop);
    }

    if (_painterCrop)
    {
        _mosaic->setPainterCrop(_painterCrop);
    }

    if (_cleanseLevel > 0)
    {
        Q_ASSERT(_version < 26);
        auto protos = _mosaic->getPrototypes();
        for (auto & proto : protos)
        {
            proto->setCleanseLevel(_cleanseLevel);
            if (!Loose::equals(_cleanseSensitivity,-1.0))
            {
                proto->setCleanseSensitivity(_cleanseSensitivity);
            }
        }
    }

    _mosaic->setLoadedXMLVersion(_version);
}

void MosaicReader::processDesign(xml_node & node)
{
    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        QString str = n.name();
        if (_debug) qDebug().noquote() << str;

        if (str == "scale")
        {
            continue;       // scale deprecated 30DEC19
        }
        else if (str == "size")
        {
            _viewSize = procViewSize(n);
            _canvasSize = procCanvasSize(n,_viewSize);

            Canvas & canvas = _vc->getCanvas();
            canvas.setCanvasSize(_canvasSize);
            canvas.dump();
        }
        else if (str == "background")
        {
            _background = procBackgroundColor(n);
        }
        else if (str == "border")
        {
            procBorder(n);
        }
        else if (str == "Crop")
        {
            if (_version < 19)
            {
                _crop = processOldCrop(n);
            }
            else
            {
                _crop = processCrop(n);
            }
        }
        else if (str == "PainterCrop")
        {
            _painterCrop = processCrop(n);
        }
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
        {
            _mosbip = getBackgroundImage(n);
        }
        else if (str == "Cleanse")
        {
            str = n.child_value();
            bool bStatus;
            uint val = str.toUInt(&bStatus,16);
            _cleanseLevel = val;
        }
        else if (str == "Sensitivity")
        {
            str = n.child_value();
            qreal val = str.toDouble();
            _cleanseSensitivity = val;
        }
        else
            fail("Unexpected", str);
    }
}

BkgdImagePtr MosaicReader::getBackgroundImage(xml_node & node)
{
    xml_attribute attr = node.attribute("name");
    QString name       = attr.value();

    BkgdImagePtr bip = std::make_shared<BackgroundImage>();
    bool loaded = bip->load(name);
    if (!loaded)
    {
        bip.reset();
        return bip;
    }

    Xform xf;

    xml_node n = node.child("Scale");
    if (n)
    {
        QString str = n.child_value();
        xf.setScale(str.toDouble());
    }

    n = node.child("Rot");
    if (n)
    {
        QString str = n.child_value();
        xf.setRotateRadians(str.toDouble());
    }

    n = node.child("X");
    if (n)
    {
        QString str= n.child_value();
        xf.setTranslateX(str.toDouble());
    }

    n = node.child("Y");
    if (n)
    {
        QString str = n.child_value();
        xf.setTranslateY(str.toDouble());
    }

    QPointF legacyCenter;
    n = node.child("Center");
    if (n)
    {
        // depracted
        QString str = n.child_value();
        QStringList qsl = str.split(",");
        qreal x = qsl[0].toDouble();
        qreal y = qsl[1].toDouble();
        legacyCenter = QPointF(x,y);
    }

    bip->setModelXform(xf,false,Sys::nextSigid());

    if (_version < 29)
    {
        if (bip->correctLegacyLayer(legacyCenter))
            _legacyCenterConverted = true;

    }
    qDebug().noquote() << "mosaic background image xform:" << xf.info();

    n= node.child("Perspective");
    bool usePerspective = false;
    if (n)
    {
        QString str = n.child_value();
        QTransform t = getQTransform(str);

        if (!t.isIdentity())
        {
            bip->setAdjustedTransform(t);
            bip->createAdjustedImage();
            usePerspective = true;
        }
    }
    bip->setUseAdjusted(usePerspective);
    bip->createPixmap();
    return bip;
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
    ProtoPtr     proto;
    Xform        xf;
    eZLevel      zlevel       = STANDARD_ZLEVEL;
    QPointF      legacyCenter;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
            legacyCenter = procesToolkitGeoLayer(n,xf,zlevel);
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
    thick->setViewController(_vc);
    thick->setModelXform(xf,false,Sys::nextSigid());
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
    thick->setZLevel(zlevel);

    if (_version < 29)
    {
        qInfo() << "legacy correction for tiling" << _xfile.getVersionedName().get() << legacyCenter;
        if (thick->correctLegacyLayer(legacyCenter))
            _legacyCenterConverted = true;
    }

    if (_debug) qDebug().noquote() << "XmlServices created Style(Thick)" << thick->getInfo();

    _mosaic->addStyle(thick);

    if (_debug) qDebug() << "end thick";
}

void MosaicReader::processInterlace(xml_node & node)
{
    ColorSet     colorset;
    ProtoPtr     proto;
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
    eZLevel      zlevel       = STANDARD_ZLEVEL;
    Xform        xf;
    QPointF      legacyCenter;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
            legacyCenter = procesToolkitGeoLayer(n,xf,zlevel);
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
    interlace->setViewController(_vc);
    interlace->setInitialStartUnder(start_under);
    interlace->setModelXform(xf,false,Sys::nextSigid());
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
    interlace->setZLevel(zlevel);

    if (_version < 29)
    {
        qInfo() << "legacy correction for" << _xfile.getVersionedName().get();
        if (interlace->correctLegacyLayer(legacyCenter))
            _legacyCenterConverted = true;
    }

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
    eZLevel      zlevel       = STANDARD_ZLEVEL;
    ProtoPtr     proto;
    Xform        xf;
    QPointF      legacyCenter;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
            legacyCenter = procesToolkitGeoLayer(n,xf,zlevel);
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
    outline->setViewController(_vc);
    outline->setModelXform(xf,false,Sys::nextSigid());
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
    outline->setZLevel(zlevel);

     if (_version < 29)
    {
        qInfo() << "legacy correction for" << _xfile.getVersionedName().get();
        if (outline->correctLegacyLayer(legacyCenter))
            _legacyCenterConverted = true;
    }

    if (_debug) qDebug().noquote() << "XmlServices created Style(Outline)" << outline->getInfo();

    _mosaic->addStyle(outline);

    if (_debug) qDebug() << "end outline";
}

void MosaicReader::processFilled(xml_node & node)
{
    bool         oldFormat      = false;
    eZLevel      zlevel         = STANDARD_ZLEVEL;
    eFillType    l_algorithm    = FILL_ORIGINAL;
    bool         l_draw_inside  = false;
    bool         l_draw_outside = true;

    ColorSet     l_colorSet;
    ColorSet     l_colorsB;
    ColorSet     l_colorsW;
    ColorGroup   l_colorGroup;

    ProtoPtr     l_proto;
    Xform        l_xf;
    QPointF      legacyCenter;

    QVector<int> faceColorIndices;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
        {
             legacyCenter = procesToolkitGeoLayer(n,l_xf,zlevel);
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
            if (l_algorithm == FILL_DIRECT_FACE)
            {
                processsStyleFilledFaces(n,faceColorIndices);
            }
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

    FillPtr filled = make_shared<Filled>(l_proto,l_algorithm);
    filled->setViewController(_vc);
    filled->setModelXform(l_xf,false,Sys::nextSigid());
    filled->setZLevel(zlevel);

     if (_version < 29)
    {
        qInfo() << "legacy correction for" << _xfile.getVersionedName().get();
        if (filled->correctLegacyLayer(legacyCenter))
            _legacyCenterConverted = true;
    }

    if (oldFormat)
    {
        // old - redundant way of dealing with colors
        ColorSet * csetW = filled->getWhiteColorSet();
        csetW->addColor(l_colorSet.getTPColor(0));

        ColorSet * csetB = filled->getBlackColorSet();
        if (l_colorSet.size() >= 2)
            csetB->addColor(l_colorSet.getTPColor(1));
        else
            csetB->addColor(l_colorSet.getTPColor(0));
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

    if (l_algorithm == FILL_DIRECT_FACE)
    {
        filled->setPaletteIndices(faceColorIndices);
    }

    if (_debug) qDebug().noquote() << "XmlServices created Style(Filled)" << filled->getInfo();

    _mosaic->addStyle(filled);

    if (_debug) qDebug() << "end filled";
}

void MosaicReader::processPlain(xml_node & node)
{
    ColorSet     colorset;
    ProtoPtr     proto;
    Xform        xf;
    eZLevel      zlevel = STANDARD_ZLEVEL;
    QPointF      legacyCenter;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
            legacyCenter = procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "style.Style")
            processStylePrototype(n,proto);
        else if (str == "style.Colored")
            processColorSet(n,colorset);
        else
            fail("Unexpected", str.c_str());
    }

    auto plain = make_shared<Plain>(proto);
    plain->setViewController(_vc);
    plain->setModelXform(xf,false,Sys::nextSigid());
    plain->setColorSet(colorset);
    plain->setZLevel(zlevel);

    if (_version < 29)
    {
        qInfo() << "legacy correction for" << _xfile.getVersionedName().get();
        if (plain->correctLegacyLayer(legacyCenter))
            _legacyCenterConverted = true;
    }

    if (_debug) qDebug().noquote() << "XmlServices created Style (Plain)" << plain->getInfo();

    _mosaic->addStyle(plain);

    if (_debug) qDebug() << "end plain";
}

void MosaicReader::processSketch(xml_node & node)
{
    ColorSet     colorset;
    ProtoPtr     proto;
    Xform        xf;
    eZLevel      zlevel = STANDARD_ZLEVEL;
    QPointF      legacyCenter;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
            legacyCenter = procesToolkitGeoLayer(n,xf,zlevel);
        else if (str == "style.Style")
            processStylePrototype(n,proto);
        else if (str == "style.Colored")
            processColorSet(n,colorset);
        else
            fail("Unexpected", str.c_str());
    }

    auto sketch = make_shared<Sketch>(proto);
    sketch->setViewController(_vc);
    sketch->setModelXform(xf,false,Sys::nextSigid());
    sketch->setColorSet(colorset);
    sketch->setZLevel(zlevel);

     if (_version < 29)
    {
        qInfo() << "legacy correction for" << _xfile.getVersionedName().get();
        if (sketch->correctLegacyLayer(legacyCenter))
            _legacyCenterConverted = true;
    }

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
    eZLevel      zlevel       = STANDARD_ZLEVEL;
    ProtoPtr     proto;
    Xform        xf;
    QPointF      legacyCenter;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
            legacyCenter = procesToolkitGeoLayer(n,xf,zlevel);
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
    emboss->setViewController(_vc);
    emboss->setModelXform(xf,false,Sys::nextSigid());
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
    emboss->setZLevel(zlevel);

    if (_version < 29)
    {
        qInfo() << "legacy correction for" << _xfile.getVersionedName().get();
        if (emboss->correctLegacyLayer(legacyCenter))
            _legacyCenterConverted = true;
    }

    if (_debug) qDebug().noquote() << "XmlServices created Style(Emboss)" << emboss->getInfo();

    _mosaic->addStyle(emboss);

    if (_debug) qDebug() << "end emboss";
}

void MosaicReader::processTileColors(xml_node & node)
{
    ProtoPtr     proto;
    Xform        xf;
    bool         outline = false;
    int          width   = 3;
    eZLevel      zlevel  = STANDARD_ZLEVEL;
    QColor       color   = Qt::white;
    QPointF      legacyCenter;

    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string str = n.name();
        if (_debug) qDebug().noquote() << str.c_str();

        if (str == "toolkit.GeoLayer")
            legacyCenter = procesToolkitGeoLayer(n,xf,zlevel);
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
        else if (str == "BkgdColors")
        {
            QString str = n.child_value();
            QStringList qsl = str.split(',');
            ColorSet cset;
            for (const auto & str2 : std::as_const(qsl))
            {
                QColor c(str2);
                cset.addColor(c);
            }
            _tilingColorGroup.addColorSet(cset);
        }
        else
            fail("Unexpected", str.c_str());
    }

    if (_debug) qDebug() << "Constructing TileColors from prototype and poly";

    auto tc  = make_shared<TileColors>(proto);
    tc->setViewController(_vc);

    auto & colorGroup = tc->getTileColors();
    colorGroup = _tilingColorGroup;

    tc->setModelXform(xf,false,Sys::nextSigid());

    if (outline)
    {
        tc->setOutline(true,color,width);
    }

    tc->setZLevel(zlevel);

    if (_version < 29)
    {
        qInfo() << "legacy correction for" << _xfile.getVersionedName().get();
        if (tc->correctLegacyLayer(legacyCenter))
            _legacyCenterConverted = true;
    }

    if (_debug) qDebug().noquote() << "XmlServices created Style(TileColors)" << tc->getInfo();

    _mosaic->addStyle(tc);

    if (_debug) qDebug() << "end TileColors";
}

void MosaicReader::processBorder(xml_node & node)
{
    xml_node n = node.child("border");
    procBorder(n);
}

QPointF MosaicReader::procesToolkitGeoLayer(xml_node & node, Xform & xf, eZLevel & zlevel)
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

    QPointF legacyCenter;
    n = node.child("center");
    if (n)
    {
        // deprecated
        val          = n.child_value();
        QStringList qsl = val.split(",");
        qreal x = qsl[0].toDouble();
        qreal y = qsl[1].toDouble();
        legacyCenter    = QPointF(x,y);
        // this use of center was bogus
        // to correct legacy XML this center is converted
        // and appled to the model transfrom translate
    }
    n = node.child("Z");
    if (n)
    {
        val          = n.child_value();
        zlevel       = (eZLevel)val.toInt();
    }

    return legacyCenter;
}

void MosaicReader::processStylePrototype(xml_node & node, ProtoPtr & proto)
{
    xml_node n = node.child("prototype");
    if (_debug) qDebug().noquote() << n.name();
    Sys::dumpRefs();
    proto = getPrototype(n);
    Sys::dumpRefs();
    if (_debug) qDebug().noquote() << proto->info();
}

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

qreal MosaicReader::procLength(xml_node & node)
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

void MosaicReader::processsStyleFilled(xml_node & node, bool & draw_inside, bool & draw_outside, eFillType & algorithm)
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
        algorithm = eFillType(algo.toInt());
    }
}

void  MosaicReader::processsStyleFilledFaces(xml_node & node, QVector<int> &paletteIndices)
{
    xml_node fnode = node.child("FaceColors");
    xml_attribute attr = fnode.attribute("faces");
    int numFaces = 0;
    if (attr)
    {
        QString str = attr.value();
        numFaces = str.toInt();
    }
    QString str = fnode.child_value();
    QStringList qsl = str.split(',');
    for (auto & str : qsl)
    {
        paletteIndices.push_back(str.toInt());
    }
    if (numFaces && numFaces != paletteIndices.size())
    {
        qWarning() << "MosaicReader::processsStyleFilledFaces count mismatch" << numFaces << paletteIndices.size();
    }
}

void MosaicReader::processsStyleEmboss(xml_node & node, qreal & angle)
{
    QString str  = node.child_value("angle");
    angle =  str.toDouble();
}

void MosaicReader::getExtendedData(xml_node & node, ExtenderPtr ep)
{
    if (_version >= 22)
    {
        bool extendRays              = getAttributeTF(node,"extRays");
        bool extTipsToBoundary       = getAttributeTF(node,"extTips2B");
        bool extendBoundaryToTile    = getAttributeTF(node,"extTips2T");
        bool embedBound              = getAttributeTF(node,"embedBound");
        bool embedTile               = getAttributeTF(node,"embedTile");

        ep->setExtendRays(extendRays);
        ep->setExtendTipsToBound(extTipsToBoundary);
        ep->setExtendTipsToTile(extendBoundaryToTile);
        ep->setEmbedBoundary(embedBound);
        ep->setEmbedTile(embedTile);

        if (_version >= 25)
        {
            uint connectBoundaryVertices = getAttributeUint(node,"conRays");
            ep->setConnectRays(connectBoundaryVertices);
        }
        else
        {
            bool connectBoundaryVertices = getAttributeTF(node,"conRays");
            uint crays = 0;
            if (connectBoundaryVertices)
                crays = 1;
            ep->setConnectRays(crays);
        }
    }
    else
    {
        bool extendPeripherals       = getAttributeTF(node,"extendPeripherals");
        bool extendFreeVertices      = getAttributeTF(node,"extendFreeVertices");

        ep->setExtendRays(extendPeripherals);
        ep->setExtendTipsToBound(extendFreeVertices);

        bool connectBoundaryVertices = getAttributeTF(node,"connectBoundaryVertices");
        uint crays = 0;
        if (connectBoundaryVertices)
            crays = 1;
        ep->setConnectRays(crays);
    }
}

void  MosaicReader::getExtendedBoundary(xml_node & node, ExtendedBoundary & eb)
{
    QString str;
    xml_node child;
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

    if (node.child("boundaryRotate"))
    {
        str = node.child_value("boundaryRotate");
        qreal brotate = str.toDouble();
        eb.setRotate(brotate);
    }
}

PolyPtr MosaicReader::getPolygonV1(xml_node & node)
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

APolygon MosaicReader::getApoly(xml_node & node)
{
    APolygon ap;

    xml_attribute source;
    source = node.attribute("source");
    Q_ASSERT(source);
    QString str = source.value();
    bool hasSource= (str == "t");
    if (hasSource)
    {
        QPolygonF p = getPolygonV2(node);
        ap.set(p);
    }
    else
    {
        xml_node n = node.child("n");
        QString str = n.child_value();
        int val = str.toInt();
        ap.setSides(val);
    }

    xml_node anode;

    anode = node.child("rot");
    str = anode.child_value();
    qreal rot = str.toDouble();
    ap.setRotate(rot);

    anode = node.child("scale");
    str = anode.child_value();
    qreal sc = str.toDouble();
    ap.setScale(sc);

    anode = node.child("pos");
    QPointF pos = getPos(anode);
    ap.setPos(pos);

    return ap;
}

QPolygonF MosaicReader::getPolygonV2(xml_node & node)
{
    QPolygonF poly;
    for (xml_node anode = node.child("pos"); anode; node = anode.next_sibling())
    {
        QPointF pos = getPos(anode);
        poly << pos;
    }
    return poly;
}

ProtoPtr MosaicReader::getPrototype(xml_node & node)
{
    if (_debug) qDebug().noquote() << node.name();
    if (mrbase.hasReference(node))
    {
        return getProtoReferencedPtr(node);
    }

    xml_node protonode = node.child("app.Prototype");
    if (_debug) qDebug().noquote() << protonode.name();

    // load tiling from its XML file by name
    bool localTiling = false;     // indicates whether there is an associated tiling
    TilingPtr loadedTiling;
    xml_node tnode;
    if (_version >= 21)
        tnode = protonode.child("Tiling");
    else
        tnode = protonode.child("string");
    if (tnode)
    {
        QString tilingName = tnode.child_value();
        VersionedName vn(tilingName);
        qDebug().noquote() << "Loading tiling" << vn.get();

        VersionedFile vfile = FileServices::getFile(vn,FILE_TILING);
        if (!vfile.isEmpty())
        {
            // load tiling
            TilingReader tm(_vc);
            loadedTiling = tm.readTilingXML(vfile,&mrbase);

            if (loadedTiling)
            {
                _tilings.push_back(loadedTiling);
                if (!_mosbip)
                {
                    // use tiling bip since there is no mosaic bip
                    _mosbip = loadedTiling->getBkgdImage();
                }
                if (loadedTiling->getVersion() < 8)
                {
                    _tilingColorGroup = tm.getTileColors();
                }
#ifdef LEGACY_CONVERT_XML
                if (loadedTiling->legacyModelConverted())
                {
                    TilingWriter writer;
                    writer.writeTilingXML(vfile,loadedTiling);
                }
#endif
            }
        }
    }
    if (!loadedTiling)
    {
        // 18JAN2022 - adds this case for mosaics with maps but no tiling
        qInfo() << "No tiling loaded - using empty tiling";
        loadedTiling = make_shared<Tiling>();
        _tilings.push_back(loadedTiling);
        localTiling = true;
    }

    if (_debug) qDebug() << "Creating new prototype";

    Q_ASSERT(_mosaic);

    ProtoPtr proto = make_shared<Prototype>(loadedTiling,_mosaic);
    setProtoReference(node,proto);

    xml_node dnode = protonode.child("Distort");
    if (dnode)
    {

        bool bStatus;
        QString str = dnode.child_value("scalex");
        qreal valx = str.toDouble(&bStatus);
        str = dnode.child_value("scaley");
        qreal valy = str.toDouble(&bStatus);
        QTransform t = QTransform::fromScale(valx,valy);
        proto->setDistortion(t);
        proto->enableDistortion(true);
    }

    xml_node cnode = protonode.child("Cleanse");
    if (cnode)
    {
        QString str = cnode.child_value();
        bool bStatus;
        uint val = str.toUInt(&bStatus,16);
        proto->setCleanseLevel(val);
    }

    cnode = protonode.child("Sensitivity");
    if (cnode)
    {
        QString str = cnode.child_value();
        qreal val = str.toDouble();
        proto->setCleanseSensitivity(val);
    }

    QVector<TilePtr> uniqueLoadedTiles = loadedTiling->unit().getUniqueTiles();
    int numLoadedTiles                 = uniqueLoadedTiles.size();

    QVector<TilePtr> usedTiles;
    TileReader tr(&mrbase);

    // each mosaic "entry" is a design element
    int del = 0;
    for (xml_node entry = protonode.child("entry"); entry; entry = entry.next_sibling("entry"))
    {
        TilePtr  entryTile;

        if (_debug) qDebug() << "Design element" << del++;
        bool motifFound = false;

        xml_node tileNode = entry.first_child();
        QString  tileName = tileNode.name();
        if (_debug) qDebug().noquote() << tileName;

        if (tileName == "Tile" || tileName == "tile.Feature")
        {
            if (_debug) qDebug() << "adding Tile";
            entryTile = getTile(tr,tileNode);
        }
        else
        {
           fail("tile not found", "");
        }

        int sides = entryTile->numEdges();

        xml_node motifNode  = tileNode.next_sibling();
        QString  motifType  = motifNode.name();
        eMotifType type     = Sys::XMLgetMotifType(motifType);

        MotifPtr motif;
        switch(type)
        {
        case MOTIF_TYPE_STAR:
            motif = getStar(motifNode,sides);
            motifFound = true;
            break;

        case MOTIF_TYPE_STAR2:
            motif = getStar2(motifNode,sides);
            motifFound = true;
            break;

        case MOTIF_TYPE_ROSETTE:
            motif =  getRosette(motifNode,sides);
            motifFound = true;
            break;

        case MOTIF_TYPE_ROSETTE2:
            motif =  getRosette2(motifNode,sides);
            motifFound = true;
            break;

        case MOTIF_TYPE_EXPLICIT_MAP:
        case MOTIF_TYPE_INFERRED:
        case MOTIF_TYPE_IRREGULAR_ROSETTE:
        case MOTIF_TYPE_HOURGLASS:
        case MOTIF_TYPE_INTERSECT:
        case MOTIF_TYPE_GIRIH:
        case MOTIF_TYPE_IRREGULAR_STAR:
        case MOTIF_TYPE_EXPLCIT_TILE:
        case MOTIF_TYPE_IRREGULAR_NO_MAP:
            motif = getIrregularMotif(motifNode, sides, type);
            motifFound = true;
            break;

        case MOTIF_TYPE_UNDEFINED:
        case MOTIF_TYPE_RADIAL:
            _failMessage = "Motif type not found: " + tileName;
            qWarning() << _failMessage;
            throw(_failMessage);
            break;
        }

        // Match a tile defined in the mosaic
        if (motifFound)
        {
            Q_ASSERT(entryTile->isCorrect());
            if (_debug) qDebug() << "sides" << sides;

            // if the found tile is identical to the one in the known tiling then use that
            // DAC 27MAY17 - imprtant that this code not removed or Mosaic View will fail
            bool tileFound = false;
            for (TilePtr & loadedTile : uniqueLoadedTiles)
            {
                Q_ASSERT(loadedTile->isCorrect());

                if (usedTiles.contains(loadedTile))
                    continue;
                if (_debug) qDebug() << "loadedTile" << loadedTile->info();
                if (_debug) qDebug() << "entryTile"  << entryTile->info();

                if (*loadedTile.get() == *entryTile.get())
                {
                    if (_debug) qDebug() << "match";
                    // use the loaded tile in place of the entry tile
                    // since this correlates with what will be in the tiling maker
                    usedTiles.push_back(loadedTile);
                    entryTile = loadedTile;

                    //create the desing element and put into the prototype
                    if (_debug) qDebug().noquote() << "adding to Proto" << motif->getMotifDesc();
                    DELPtr  dep = make_shared<DesignElement>(loadedTiling,entryTile, motif);

                    if (_version < 27 && motif->isIrregular())
                    {
                        fixupIrregularTransform(dep);
                    }

                    proto->addDesignElement(dep);
                    if (_debug) qDebug().noquote() << "design element:" << dep->toString();
                    tileFound = true;
                    break;
                }
                else if (_debug) qDebug() << "no match";
            }
            if (!tileFound)
            {
                if (!localTiling)
                {
                    _failMessage = "Mismatch between Design and Tiling";
                    qWarning().noquote() << _failMessage;
                    qWarning().noquote() << "No match for Mosaic tile in tiling";

                    if (!Sys::localCycle)
                    {
                        QMessageBox box(Sys::controlPanel);
                        box.setIcon(QMessageBox::Critical);
                        box.setText(_failMessage);
                        QPushButton btn("Try to fix");
                        box.addButton(&btn,QMessageBox::AcceptRole);
                        box.addButton(QMessageBox::Cancel);
                        int rv = box.exec();
                        if (rv == QMessageBox::Cancel)
                        {
                            // this mismatch cannot be handled
                            throw(_failMessage);
                        }
                    }
                }
                DELPtr  del = make_shared<DesignElement>(loadedTiling,entryTile, motif);
                proto->addDesignElement(del);
            }
        }
    }

    QVector<DELPtr> & designElements = proto->getDesignElements();
    int numDesignElements                      = designElements.size();

    if (numLoadedTiles >> numDesignElements)
    {
        // need to create design elements for the unused tiles
        qWarning() << "Mosaic does not match tiling. Creating new Design elements. DELs:" << numDesignElements << "Tiles:" << numLoadedTiles;

        for (const auto & tile : std::as_const(uniqueLoadedTiles))
        {
            Q_ASSERT(tile->isCorrect());
            if (usedTiles.contains(tile))
            {
                continue;
            }
            // create new desigin element
            DELPtr  del = make_shared<DesignElement>(loadedTiling,tile);
            proto->addDesignElement(del);
        }
        numDesignElements = designElements.size(); // bumps the count
    }

    if (numLoadedTiles && (numLoadedTiles != numDesignElements) && !Sys::localCycle)
    {
        QString str1 = "Mosaic does not match tiling.";
        QString str2;
        QDebug  deb(&str2);
        deb <<  "Unqiue Tiles:" << numLoadedTiles << "Design Elements:" << numDesignElements;

        qWarning().noquote() << str1 << str2;

        QMessageBox box(Sys::controlPanel);
        box.setWindowTitle(_xfile.getVersionedName().get());
        int len = _xfile.getPathedName().length();
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
    for (auto & del : std::as_const(designElements))
    {
        MotifPtr motif = del->getMotif();
        TilePtr  tile  = del->getTile();
        QPolygonF poly = tile->getPolygon();

        if (motif->isIrregular())
        {
            switch (motif->getMotifType())
            {
            case MOTIF_TYPE_EXPLICIT_MAP:
            {
                // already has a map
                auto exp = std::dynamic_pointer_cast<ExplicitMapMotif>(motif);
                exp->setTile(tile);
            }   break;

            case MOTIF_TYPE_IRREGULAR_NO_MAP:
            {
                auto irr = std::dynamic_pointer_cast<IrregularNoMap>(motif);
                irr->setTile(tile);
            }   break;

            case MOTIF_TYPE_INFERRED:
            {
                auto infer = std::dynamic_pointer_cast<InferredMotif>(motif);
                infer->setTile(tile);
                infer->setupInfer(proto);
            }   break;

            case MOTIF_TYPE_IRREGULAR_ROSETTE:
            {
                auto rose = std::dynamic_pointer_cast<IrregularRosette>(motif);
                rose->setTile(tile);
            }   break;

            case MOTIF_TYPE_HOURGLASS:
            {
                auto hour = std::dynamic_pointer_cast<HourglassMotif>(motif);
                hour->setTile(tile);
            }   break;

            case MOTIF_TYPE_INTERSECT:
            {
                auto expl = std::dynamic_pointer_cast<IntersectMotif>(motif);
                expl->setTile(tile);
            }   break;

            case MOTIF_TYPE_IRREGULAR_STAR:
            {
                auto star = std::dynamic_pointer_cast<IrregularStar>(motif);
                star->setTile(tile);
            }   break;

            case MOTIF_TYPE_EXPLCIT_TILE:
            {
                auto etile = std::dynamic_pointer_cast<TileMotif>(motif);
                etile->setTile(tile);
            }   break;

            case MOTIF_TYPE_GIRIH:
            {
                auto girih = std::dynamic_pointer_cast<GirihMotif>(motif);
                girih->setTile(tile);
            }   break;

            case MOTIF_TYPE_UNDEFINED:
            case MOTIF_TYPE_RADIAL:
            case MOTIF_TYPE_ROSETTE:
            case MOTIF_TYPE_ROSETTE2:
            case MOTIF_TYPE_STAR:
            case MOTIF_TYPE_STAR2:
                qWarning() << "Unxpected irregular type" << sMotifType[motif->getMotifType()];
            }
        }
    }

    if (_debug) qDebug() << "Proto created";

    _prototype = proto;
    return proto;
}

TilePtr MosaicReader::getTile(TileReader & treader, xml_node & node)
{
    if (mrbase.hasReference(node))
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

    TilePtr tile;

    xml_node pointsNode = node.child("points");
    if (pointsNode)
    {
        PolyPtr b = getPolygonV1(pointsNode);
        EdgePoly ep(b);
        Q_ASSERT(ep.isCorrect());

        tile = make_shared<Tile>(ep);
        tile->setRegular(regular);
        tile->setRotate(rotation);
        tile->setScale(scale);
        tile->compose();
        Q_ASSERT(tile->isCorrect());
        setTileReference(node,tile);
        return tile;
    }

    xml_node polyNode = node.child("edges");
    if (polyNode)
    {
        bool legacyFlipConcave = (_version < 20);

        EdgeSet eset = treader.getEdgeSet(polyNode,legacyFlipConcave);

        tile = make_shared<Tile>(eset);
        tile->setRegular(regular);
        tile->setScale(scale);
        tile->setRotate(rotation);
        tile->compose();
        if (((_version == 5) || (_version ==6)) && (!Loose::zero(rotation) || !Loose::equals(scale,1.0)))
        {
            qWarning() << "Decomposing Tile for backwards compatability";
            tile->decompose();
        }

        Q_ASSERT(tile->isCorrect());
        setTileReference(node,tile);
        return tile;
    }
    return tile;
}

void MosaicReader::getMotifCommon(xml_node & node, MotifPtr motif, int tile_sides)
{
    QString str;

    // set N first so Extended Boundary can be set (changed) afterwards
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

    if (_version >= 24)
    {
        xml_node exnode;
        for (exnode = node.child("Extender"); exnode; exnode = exnode.next_sibling("Extender"))
        {
            ExtenderPtr ep = motif->addExtender();
            getExtendedData(exnode,ep);

            ExtendedBoundary & eb = ep->getRWExtendedBoundary();
            eb.setSides(tile_sides);    // default
            getExtendedBoundary(exnode,eb);
        }
        exnode = node.child("Connector");
        if (exnode)
        {
            motif->addRadialConnector();
        }
    }
    else
    {
        ExtenderPtr ep = motif->addExtender();
        getExtendedData(node,ep);

        ExtendedBoundary & eb = ep->getRWExtendedBoundary();
        eb.setSides(tile_sides);    // default
        getExtendedBoundary(node,eb);
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

    int version = 1;
    if (node.child("version"))
    {
        str = node.child_value("version");
        version = str.toInt();
    }
    motif->setVersion(version);

    if (node.child("Cleanse"))
    {
        str = node.child_value("Cleanse");
        int val = str.toInt();
        motif->setCleanse(val);

        if (node.child("Sensitivity"))
        {
            str = node.child_value("Sensitivity");
            qreal val = str.toDouble();
            motif->setCleanseSensitivity(val);
        }
    }
}

void MosaicReader::fixupIrregularTransform(DELPtr del)
{
    // prior to version 27, motifs were unnecessarily transformed
    // by the tile transform, when actually the infer() method
    // already took the tile transform into account

    qDebug() <<  "MosaicReader::fixupIrregularTransform";

    TilePtr tile   = del->getTile();
    qreal tscale   = tile->getScale();
    qreal trot     = tile->getRotation();

    MotifPtr motif = del->getMotif();
    qreal scale    = motif->getMotifScale();
    qreal rotate   = motif->getMotifRotate();

    scale  *= tscale;
    rotate += trot;

    motif->setMotifScale(scale);
    motif->setMotifRotate(rotate);
}

IrregularPtr MosaicReader::getIrregularMotif(xml_node & node, int tile_sides, eMotifType type)
{
    IrregularPtr ep;

    if (mrbase.hasReference(node))
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
        ep = make_shared<IrregularNoMap>();
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
    if (mrbase.hasReference(node))
    {
        StarPtr f = getStarReferencedPtr(node);
        return f;
    }

    xml_node snode = node;
    bool legacy    = false;

    xml_node anode = node.child("child");
    if (anode)
    {
        Q_ASSERT(anode);
        Q_ASSERT(Sys::XMLgetMotifType(anode.attribute("class").value()) == MOTIF_TYPE_STAR);
        legacy = true;
        snode = anode;
    }

    int n,s;
    qreal d;
    getStarCommon(snode,n,d,s);

    StarPtr star = make_shared<Star>(n, d, s);

    xml_attribute onpoint = node.attribute("onpoint");
    if (onpoint)
    {
        Q_ASSERT(onpoint.as_string() == QString("t"));
        star->setOnPoint(true);
    }

    xml_attribute inscribed = node.attribute("inscribed");
    if (inscribed)
    {
        Q_ASSERT(inscribed.as_string() == QString("t"));
        star->setInscribe(true);
    }

    getMotifCommon(snode,star,tile_sides);

    if (legacy)
    {
        star->addRadialConnector();
    }

    setStarReference(snode,star);

    return star;
}

Star2Ptr MosaicReader::getStar2(xml_node & node, int tile_sides)
{
    if (mrbase.hasReference(node))
    {
        Star2Ptr f = getStar2ReferencedPtr(node);
        return f;
    }

    if (_debug) qDebug() << "getStar2";

    //_currentMap = getMap(node);

    int n, s;
    qreal theta;
    getStar2Common(node,n,theta,s);

    Star2Ptr star = make_shared<Star2>(n, theta, s);

    xml_attribute onpoint = node.attribute("onpoint");
    if (onpoint)
    {
        Q_ASSERT(onpoint.as_string() == QString("t"));
        star->setOnPoint(true);
    }

    xml_attribute inscribed = node.attribute("inscribed");
    if (inscribed)
    {
        Q_ASSERT(inscribed.as_string() == QString("t"));
        star->setInscribe(true);
    }

    getMotifCommon(node,star,tile_sides);

    setStar2Reference(node,star);

    return star;
}

RosettePtr MosaicReader::getRosette(xml_node & node, int tile_sides)
{
    if (mrbase.hasReference(node))
    {
        RosettePtr f = getRosetteReferencedPtr(node);
        return f;
    }

    xml_node rnode = node;
    bool legacy = false;

    xml_node anode = node.child("child");
    if (anode)
    {
        Q_ASSERT(rnode);
        Q_ASSERT(Sys::XMLgetMotifType(anode.attribute("class").value()) == MOTIF_TYPE_ROSETTE);
        legacy = true;
        rnode  = anode;
    }

    int n,s;
    qreal q;
    getRosetteCommon(rnode,n,q,s);

    RosettePtr rosette = make_shared<Rosette>(n, q, s);

    xml_attribute onpoint = node.attribute("onpoint");
    if (onpoint)
    {
        Q_ASSERT(onpoint.as_string() == QString("t"));
        rosette->setOnPoint(true);
    }

    xml_attribute inscribed = node.attribute("inscribed");
    if (inscribed)
    {
        Q_ASSERT(inscribed.as_string() == QString("t"));
        rosette->setInscribe(true);
    }

    getMotifCommon(rnode,rosette,tile_sides);

    if (legacy)
    {
        rosette->addRadialConnector();
    }

    setRosetteReference(rnode,rosette);

    return rosette;
}

Rosette2Ptr MosaicReader::getRosette2(xml_node & node, int tile_sides)
{
    if (mrbase.hasReference(node))
    {
        Rosette2Ptr f = getRosette2ReferencedPtr(node);
        return f;
    }

    int n,s;
    qreal x,y,k = 0;
    uint tipTypes;
    eTipMode tipMode;
    bool constrain;
    getRosette2Common(node,n,x,y,k,s,tipTypes,tipMode,constrain);

    Rosette2Ptr rosette = make_shared<Rosette2>(n, x, y, s, k, constrain);
    rosette->setTipMode(tipMode);
    rosette->setTipTypes(tipTypes);

    xml_attribute onpoint = node.attribute("onpoint");
    if (onpoint)
    {
        Q_ASSERT(onpoint.as_string() == QString("t"));
        rosette->setOnPoint(true);
    }

    xml_attribute inscribed = node.attribute("inscribed");
    if (inscribed)
    {
        Q_ASSERT(inscribed.as_string() == QString("t"));
        rosette->setInscribe(true);
    }

    getMotifCommon(node,rosette,tile_sides);

    setRosette2Reference(node,rosette);

    return rosette;
}

void  MosaicReader::getStarCommon(xml_node & node, int & n, qreal & d, int & s)
{
    QString str;
    str = node.child_value("n");
    n = str.toInt();

    str = node.child_value("d");
    d = str.toDouble();

    str = node.child_value("s");
    s = str.toInt();
}

void  MosaicReader::getStar2Common(xml_node & node, int & n, qreal & theta, int & s)
{
    QString str;
    str = node.child_value("n");
    n = str.toInt();

    str = node.child_value("angle");
    theta = str.toDouble();

    str = node.child_value("s");
    s = str.toInt();
}

void  MosaicReader::getRosetteCommon(xml_node & node, int & n, qreal & q, int & s)
{
    QString str;
    str = node.child_value("n");
    n = str.toInt();

    str = node.child_value("q");
    q = str.toDouble();

    str = node.child_value("s");
    s = str.toInt();

    str = node.child_value("k");
    if (!str.isEmpty())
    {
        // k is no longer supported
        qreal k = str.toDouble();
        if (!Loose::zero(k))
            qWarning().noquote() << "MosaicReader::getRosette parameter k - is DEPRECATED" << str;
    }
}

void  MosaicReader::getRosette2Common(xml_node & node, int & n, qreal & x, qreal & y, qreal & k, int & s, uint &tipTypes, eTipMode &tipMode, bool & constrain)
{
    QString str;

    constrain  = getAttributeTF(node,"constrain");

    str = node.child_value("n");
    n = str.toInt();

    str = node.child_value("x");
    x = str.toDouble();

    str = node.child_value("y");
    y = str.toDouble();

    str = node.child_value("k");
    if (!str.isEmpty())
        k = str.toDouble();

    str = node.child_value("s");
    s = str.toInt();

    if (node.child("tip"))
    {

        tipTypes = TIP_TYPE2_OUTER;
        tipMode  = TIP_MODE_REGULAR;

        str = node.child_value("tip");
        if (str == "alt")
        {
            tipMode = TIP_MODE_ALTERNATE;
            tipTypes = TIP_TYPE2_OUTER | TIP_TYPE2_INNER;
        }
        else if (str == "in")
        {
            tipTypes = TIP_TYPE2_INNER;
        }
    }
    else
    {
        tipTypes = 0;
        str = node.child_value("tiptypes");
        QStringList qsl = str.split('|');
        if (qsl.contains("outer"))
        {
            tipTypes |= TIP_TYPE2_OUTER;
        }
        if (qsl.contains("inner"))
        {
            tipTypes |= TIP_TYPE2_INNER;
        }
        if (qsl.contains("flipped"))
        {
            tipTypes |= TIP_TYPE2_FLIPPED;
        }

        str = node.child_value("tipmode");
        if (str == "regular")
        {
            tipMode = TIP_MODE_REGULAR;
        }
        else if (str == "alternate")
        {
            tipMode = TIP_MODE_ALTERNATE;
        }
    }
}

MapPtr MosaicReader::getMap(xml_node & node)
{
    xml_node mapnode = node.child("map");

    if (mrbase.hasReference(mapnode))
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

    MapVerifier mv(_currentMap);
    mv.verifyAndFix();

    qDebug().noquote() << _currentMap->summary();

    return _currentMap;
}

VertexPtr MosaicReader::getVertex(xml_node & node)
{
    if (mrbase.hasReference(node))
    {
        VertexPtr vp = mrbase.getVertexReferencedPtr(node);
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
    mrbase.setVertexReference(node,v);

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

    if (mrbase.hasReference(node))
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

    if (mrbase.hasReference(node))
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
    eCurveType ctype = (val == "t") ? CURVE_CONVEX : CURVE_CONCAVE;

    edge->setV1(v1);
    edge->setV2(v2);
    edge->chgangeToCurvedEdge(p,ctype);

    return edge;
}

QSize MosaicReader::procViewSize(xml_node & node)
{
    QSize   sz;
    QString val;
    xml_node w = node.child("width");
    if (w)
    {
        val = w.child_value();
        sz.setWidth(val.toInt());
    }
    xml_node h = node.child("height");
    if (h)
    {
        val = h.child_value();
        sz.setHeight(val.toInt());
    }
    return sz;
}

QSize MosaicReader::procCanvasSize(xml_node & node, QSize & defaultSize)
{
    QSize   sz = defaultSize;   // for legacy cases with no canvas size
    QString val;
    xml_node w2 = node.child("zwidth");
    if (w2)
    {
        val = w2.child_value();
        sz.setWidth(val.toInt());
    }
    xml_node h2 = node.child("zheight");
    if (h2)
    {
        val = h2.child_value();
        sz.setHeight(val.toInt());
    }
    return sz;
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

bool MosaicReader::getAttributeTF(xml_node & node, const char_t* name)
{
    xml_attribute xatt = node.attribute(name);
    if (xatt)
    {
        QString str(xatt.value());
        return str == "t";
    }
    else
    {
        return false;
    }
}

uint MosaicReader::getAttributeUint(xml_node & node, const char_t* name)
{
    xml_attribute xatt = node.attribute(name);
    if (xatt)
    {
        QString str(xatt.value());
        uint val = str.toUInt();
        return val;
    }
    else
    {
        return 0;
    }
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
    // this is executed in pass 2

    // Note: version 17 uses model units
    // Note: version 23 includes model transform (toolkit.GeoLayer)

    xml_attribute atype = node.attribute("type");
    if (!atype)  return;

    bool useViewSize = false;
    xml_attribute vsize = node.attribute("useViewSize");
    if (vsize)
    {
        QString val = vsize.value();
        useViewSize = (val == "t") ? true : false;
    }

    if (_version < 12)
    {
        // legacy version code
        int iType = atype.as_int();
        if (iType == 4 && _version < 11)
        {
            _crop = processOldCrop(node);
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
        // uses name, not type enum number
        Q_ASSERT(_version >= 12);
        QString type = atype.as_string();
        if (type == "Plain")
            procBorderPlain(node);
        else if (type == "2Color")
            procBorderTwoColor(node);
        else  if (type == "Blocks")
            procBorderBlocks(node);
    }

    if (_border)
    {
        Xform xf;
        eZLevel zlevel = STANDARD_ZLEVEL;

        if (_version  >= 23)
        {
            xml_node n = node.child("toolkit.GeoLayer");
            procesToolkitGeoLayer(n,xf,zlevel);
            _border->setModelXform(xf,false,Sys::nextSigid());
            _border->setZLevel(zlevel);
        }
        else
        {
            _border->setModelXform(_firstStyleXform,false,Sys::nextSigid());
        }

        _border->setUseViewSize(useViewSize);
        _border->createStyleRepresentation();
    }
}

// the old way of handling crops
CropPtr MosaicReader::processOldCrop(xml_node & node)
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

    auto crop = make_shared<Crop>();
    crop->setRect(rect);
    if (_version < 15)
    {
        crop->setEmbed(true);
        crop->setApply(true);
    }
    else
    {
        bool embed = false;
        bool apply = false;
        bool clip  = false;
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
        attr = node.attribute("clip");
        if (attr)
        {
            QString val = attr.value();
            clip = (val == "t") ? true : false;
        }
        crop->setEmbed(embed);
        crop->setApply(apply);
        crop->setClip(clip);
    }
    return crop;
}

// the current way of handling crops
CropPtr MosaicReader::processCrop(xml_node & node)
{
    auto crop = make_shared<Crop>();

    bool embed = false;
    bool apply = false;
    bool clip  = false;
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
    attr = node.attribute("clip");
    if (attr)
    {
        QString val = attr.value();
        clip = (val == "t") ? true : false;
    }
    crop->setEmbed(embed);
    crop->setApply(apply);
    crop->setClip(clip);

    xml_node type = node.child("rect");
    if (type)
    {
        QRectF rect = getRectangle(type);
        crop->setRect(rect);
        return crop;
    }

    type = node.child("circle");
    if (type)
    {
        Circle c = getCircle(type);
        crop->setCircle(c);
        return crop;
    }

    type = node.child("APoly");
    if (type)
    {
        APolygon ap = getApoly(type);
        crop->setPolygon(ap);
        return crop;
    }

    qWarning() << "Unexpedcted crop type";
    return crop;
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
            _border = make_shared<BorderPlain>(_mosaic.get(),rect, bwidth, col1);
        }
        else if (shape == "Circle")
        {
            xml_node rnode = node.child("circle");
            auto c = getCircle(rnode);
            _border = make_shared<BorderPlain>(_mosaic.get(),c, bwidth, col1);
        }
    }
    else
    {
        _border = make_shared<BorderPlain>(_mosaic.get(),_viewSize, bwidth, col1);
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

    qreal blen = 1.5;
    xml_node lnode = node.child("length");
    if (lnode)
        blen =  procLength(lnode);

    QRectF rect(QPointF(),_viewSize);

    xml_attribute attr = node.attribute("shape") ;
    if (attr)
    {
        QString shape = attr.as_string();
        if (shape == "Rectangle")
        {
            xml_node rnode = node.child("rect");
            rect = getRectangle(rnode);
        }
    }

    _border = make_shared<BorderTwoColor>(_mosaic.get(), rect, col1, col2, bwidth, blen);
}

void MosaicReader::procBorderBlocks(xml_node & node)
{
    xml_node cnode = node.child("color");
    QColor col1 = processColor(cnode);

    int rows = 3;
    xml_node wnode = node.child("rows");
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

    QRectF rect(QPointF(),_viewSize);

    xml_attribute attr = node.attribute("shape") ;
    if (attr)
    {
        QString shape = attr.as_string();
        if (shape == "Rectangle")
        {
            xml_node rnode = node.child("rect");
            rect = getRectangle(rnode);
        }
    }

    qreal bwidth = 2.0;
    wnode = node.child("width");
    if (wnode)
        bwidth = procWidth(wnode);

    _border = make_shared<BorderBlocks>(_mosaic.get(),rect, col1, rows, cols, bwidth);
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

void   MosaicReader::setStar2Reference(xml_node & node, Star2Ptr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        star2_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref star:" << i;
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

void MosaicReader::setRosette2Reference(xml_node & node, Rosette2Ptr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        rosette2_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref rosette:" << i;
#endif
    }
}

void MosaicReader::setMapReference(xml_node & node, MapPtr ptr)
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

Star2Ptr MosaicReader::getStar2ReferencedPtr(xml_node & node)
{
    Star2Ptr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = Star2Ptr(star2_ids[id]);
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

Rosette2Ptr MosaicReader::getRosette2ReferencedPtr(xml_node & node)
{
    Rosette2Ptr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = Rosette2Ptr(rosette2_ids[id]);
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
