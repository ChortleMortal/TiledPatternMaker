#include "gui/map_editor/map_editor.h"
#include "gui/top/system_view_controller.h"
#include "model/mosaics/border.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_writer.h"
#include "model/motifs/explicit_map_motif.h"
#include "model/motifs/irregular_motif.h"
#include "model/motifs/irregular_rosette.h"
#include "model/motifs/irregular_star.h"
#include "model/motifs/rosette.h"
#include "model/motifs/rosette2.h"
#include "model/motifs/star.h"
#include "model/motifs/star2.h"
#include "model/prototypes/design_element.h"
#include "model/prototypes/prototype.h"
#include "model/settings/canvas_settings.h"
#include "model/styles/emboss.h"
#include "model/styles/filled.h"
#include "model/styles/interlace.h"
#include "model/styles/outline.h"
#include "model/styles/plain.h"
#include "model/styles/sketch.h"
#include "model/styles/thick.h"
#include "model/styles/tile_colors.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling_writer.h"
#include "sys/geometry/crop.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_verifier.h"
#include "sys/geometry/vertex.h"
#include "sys/qt/tpm_io.h"
#include "sys/sys.h"
#include "sys/sys/fileservices.h"

using std::dynamic_pointer_cast;

int MosaicWriter::currentXMLVersion = 0;

MosaicWriter::MosaicWriter()
{
    debug = false;
    //currentXMLVersion = 3;
    //currentXMLVersion = 4;  // 05OCT19 use ColorSets in Colored
    //currentXMLVersion = 5;  // 25OCT19 revised way of defining maps
    //currentXMLVersion = 6;  // 26JUL20 includes FillData
    //currentXMLVersion = 7;  // 15DEC20 Feature epolys being saved correctly
    //currentXMLVersion = 8;  // 31DEC20 Indepedent background image being saved
    //currentXMLVersion = 9;  // 03MAR21 Neighbour Map no longer needed
    //currentXMLVersion = 10; // 07APR21 Border enhancements
    //currentXMLVersion = 11; // 13NOV21 Crop separated from Border
    //currentXMLVersion = 12; // 24NOV21 Reworked border definitions
    //currentXMLVersion = 13; // 16SEP22 Extended Boundary scale/rot used
    //currentXMLVersion = 14; // 23NOV22 <n> is common for all motifs
    //currentXMLVersion = 15; // 26MAY23 add Crop parms
    //currentXMLVersion = 16; // 07OCT23 Motif scale and rotate are additive to tile
    //currentXMLVersion = 17; // 11NOV23 Border crops now saved in model units again
    //currentXMLVersion = 18; // 28FEB24 Tile Colors have colors in mosaic not in XML
    //currentXMLVersion = 19; // 08MAR24 Crops are enhanced
    //currentXMLVersion = 20; // 30MAR24 Curved Edge arc centers changed
    //currentXMLVersion = 21; // 04MAY24 <string> becomes <Tiling>
    //currentXMLVersion = 22; // 12JUN24 better extended motifs
    //currentXMLVersion = 23; // 18JUN24 add 'Geotoolkit' (aka model XForm) for borders
    //currentXMLVersion = 24; // 25JUN24 refactored ExtendedBoundary and Extender class
    //currentXMLVersion = 25; // 28JUL24 connect Rays now has a mode variable
    //currentXMLVersion = 26; // 18MAR25 cleanse level and sensitivity stored in prototype
    //currentXMLVersion = 27; // 14APR25 fixes double multiply for irregular sale and rotate
    //currentXMLVersion = 28; // 30SEP25 does not save rotate centres and legacy center conversion
      currentXMLVersion = 29; // 30OCT25 has background alignment conversion
}

MosaicWriter::~MosaicWriter()
{
}

// this saves a complete mosaic
bool MosaicWriter::writeXML(VersionedFile xfile, MosaicPtr mosaic)
{
    qDebug() << "Writing XML:" << xfile.getPathedName();
    _mosaic    = mosaic;

    QFile xml(xfile.getPathedName());
    if (!xml.open(QFile::WriteOnly | QFile::Truncate))
    {
        _failMsg = QString("Could not open file to write: %1").arg(xfile.getPathedName());
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
        _failMsg = QString("ERROR writing XML file: %1").arg(xfile.getPathedName());
        return false;
    }

    xml.close();

    if (!rv)
    {
        xml.close();
        _failMsg = QString("ERROR writing XML file: %1").arg(xfile.getPathedName());
        return false;
    }

    rv = FileServices::reformatXML(xfile);
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
    mwbase.refId    = 0;
    qDebug() << "XML Writer - start generation";

    qDebug() << "version=" << currentXMLVersion;
    QString qs = QString(" version=\"%1\"").arg(currentXMLVersion);
    ts << "<vector" << mwbase.nextId() << qs << ">" << endl;

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
    ProtoPtr  pp = sp->getPrototype();
    TilingPtr tiling = pp->getTiling();
    if (!tiling)
    {
        qWarning("Tiling not found in style");
        return false;
    }
    tiling->writeTilingXML(ts);
#endif

    //design
    processMosaic(ts);

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
            ts << "<" << str << mwbase.nextId() << " >" << endl;
            processThick(ts,s);
            break;
        case STYLE_FILLED:
            str = "style.Filled";
            ts << "<" << str << mwbase.nextId() << " >" << endl;
            processFilled(ts,s);
            break;
        case STYLE_INTERLACED:
            str = "style.Interlace";
            ts << "<" << str << mwbase.nextId() << ">" << endl;
            processInterlace(ts,s);
            break;
        case STYLE_OUTLINED:
            str = "style.Outline";
            ts << "<" << str << mwbase.nextId() << ">" << endl;
            processOutline(ts,s);
            break;
        case STYLE_EMBOSSED:
            str = "style.Emboss";
            ts << "<" << str << mwbase.nextId() << ">" << endl;
            processEmboss(ts,s);
            break;
        case STYLE_PLAIN:
            str = "style.Plain";
            ts << "<" << str << mwbase.nextId() << ">" << endl;
            processPlain(ts,s);
            break;
        case STYLE_SKETCHED:
            str = "style.Sketch";
            ts << "<" << str << mwbase.nextId() << " >" << endl;
            processSketch(ts,s);
            break;
        case STYLE_TILECOLORS:
            str = "style.TileColors";
            ts << "<" << str << mwbase.nextId() << " >" << endl;
            processTileColors(ts,s);
            break;
        case STYLE_BORDER:
            str = "style.Border";
            ts << "<" << str <<  mwbase.nextId() << " >" << endl;
            procBorder(ts,s);
            break;
        case STYLE_STYLE:
            qCritical() << "Unexpected style" << est;
            break;
        }
        ts << "</" << str << ">" << endl;
    }

    qDebug() << "end vector";
    return true;
}

void MosaicWriter::processMosaic(QTextStream &ts)
{
    ts << "<design>" << endl;

    auto & info         = _mosaic->getCanvasSettings();
    QSize  viewSize     = info.getViewSize();
    QSize canvasSize    = info.getCanvasSize();
    procSize(ts,viewSize,canvasSize);

    QColor bkgdColor    = info.getBackgroundColor();
    procBackground(ts,bkgdColor);

    CropPtr crop  = _mosaic->getCrop();
    procCrop(ts,crop,"Crop");

    CropPtr painterCrop = _mosaic->getPainterCrop();
    procCrop(ts,painterCrop,"PainterCrop");

    int minX,minY,maxX,maxY;
    bool singleton;
    FillData fd = info.getFillData();
    fd.get(singleton,minX,maxX,minY,maxY);
    if (!singleton)
    {
        ts << "<Fill singleton = \"f\">" << minX << "," << maxX << "," << minY << "," << maxY << "</Fill>" << endl;
    }
    else
    {
        ts << "<Fill singleton = \"t\">0,0,0,0</Fill>";
    }

    auto bip = _mosaic->getBkgdImage();
    if (!bip)
    {
        auto tiling = _mosaic->getTilings().first();
        bip = tiling->getBkgdImage();
    }
    if (bip)
    {
        TilingWriter::writeBackgroundImage(ts,bip);
    }

    ts << "</design>" << endl;
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

void MosaicWriter::procBorder(QTextStream &ts, StylePtr style)
{
    BorderPtr border = dynamic_pointer_cast<Border>(style);
    if (border == nullptr)
    {
        fail("Style error","dynamic cast of Border");
    }

    QString stype  = border->getBorderTypeString();
    QString sshape = border->getCropTypeString();
    bool useView   = border->getUseViewSize();

    QString txt = QString("<border type=\"%1\" shape=\"%2\" useViewSize=\"%3\" >").arg(stype).arg(sshape).arg((useView) ? "t" : "f");
    ts << txt << endl;

    QString str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,border->getModelXform(),border->getZLevel());
    ts << "</" << str << ">" << endl;

    eBorderType btype = border->getBorderType();
    eCropType ctype   = border->getCropType();
    if (btype == BORDER_PLAIN)
    {
        BorderPlain * b0 = dynamic_cast<BorderPlain*>(border.get());
        Q_ASSERT(b0);
        QColor color;
        qreal  width;
        b0->get(width, color);
        procColor(ts,color);
        procWidth(ts,width);
        if (ctype == CROP_RECTANGLE)
        {
            QRectF rect = b0->getRect();
            procRect(ts,rect);
        }
        else if (ctype == CROP_CIRCLE)
        {
            Circle c = b0->getCircle();
            procCircle(ts,c);
        }
    }
    else if (btype == BORDER_TWO_COLOR)
    {
        BorderTwoColor * b1 = dynamic_cast<BorderTwoColor*>(border.get());
        Q_ASSERT(b1);
        QColor color1,color2;
        qreal  width,length;
        b1->get(color1,color2,width,length);
        procColor(ts,color1);
        procColor(ts,color2);
        procWidth(ts,width);
        procLength(ts,length);
        QRectF rect = b1->getRect();
        procRect(ts,rect);
    }
    else if (btype == BORDER_BLOCKS)
    {
        BorderBlocks * b2 = dynamic_cast<BorderBlocks*>(border.get());
        Q_ASSERT(b2);
        QColor color;
        int    rows;
        int    cols;
        qreal  width;
        b2->get(color,rows,cols,width);
        procColor(ts,color);
        ts << "<rows>" << rows << "</rows>" << endl;
        ts << "<cols>" << cols << "</cols>" << endl;
        QRectF rect = b2->getRect();
        procRect(ts,rect);
        procWidth(ts,width);
    }

    ts << "</border>" << endl;
}

void MosaicWriter::procCrop(QTextStream &ts,CropPtr crop, QString name)
{
    if (!crop)
    {
        return;
    }

    eCropType type = crop->getCropType();
    if (type == CROP_UNDEFINED)
    {
        return;
    }

    QString parms = QString(" embed=%1 apply=%2 clip=%3").arg((crop->getEmbed()) ? "\"t\"" : "\"f\"").arg((crop->getApply()) ? "\"t\"" : "\"f\"").arg((crop->getClip()) ? "\"t\"" : "\"f\"");
    ts << "<" << name<< parms << " >" << endl;

    if (type == CROP_RECTANGLE)
    {
        QRectF rect = crop->getRect();
        procRect(ts,rect);
    }
    else if (type == CROP_CIRCLE)
    {
        Circle c = crop->getCircle();
        procCircle(ts,c);
    }
    else if (type == CROP_POLYGON)
    {
        APolygon ap = crop->getAPolygon();
        procAPolygon(ts,ap);
    }
    ts << "</" << name << ">" << endl;
}

bool MosaicWriter::processThick(QTextStream &ts, StylePtr s)
{
    Thick * th = dynamic_cast<Thick*>(s.get());
    if (th == nullptr)
    {
        fail("Style error","dynamic cast of Thick");
    }

    ColorSet * cset         = th->getColorSet();
    eDrawOutline outline    = th->getDrawOutline();
    qreal   width           = th->getLineWidth();
    Qt::PenJoinStyle pjs    = th->getJoinStyle();
    Qt::PenCapStyle pcs     = th->getCapStyle();
    qreal   outline_width   = th->getOutlineWidth();
    QColor  outline_color   = th->getOutlineColor();
    ProtoPtr proto          = th->getPrototype();
    Xform   xf              = th->getModelXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,s->getZLevel());
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
    processsStyleThick(ts,outline,width,outline_width,outline_color,pjs,pcs);
    ts << "</" << str << ">" << endl;

    if (debug) qDebug() << "end thick";
    return true;
}

bool MosaicWriter::processInterlace(QTextStream & ts, StylePtr s)
{
    Interlace * il = dynamic_cast<Interlace*>(s.get());
    if (il == nullptr)
    {
        fail("Style error","dynamic cast of Interlace");
    }

    ColorSet * cset         = il->getColorSet();
    eDrawOutline outline    = il->getDrawOutline();
    bool    includeTipVerts = il->getIncludeTipVertices();
    qreal   width           = il->getLineWidth();
    Qt::PenJoinStyle pjs    = il->getJoinStyle();
    Qt::PenCapStyle pcs     = il->getCapStyle();
    qreal   outline_width   = il->getOutlineWidth();
    QColor  outline_color   = il->getOutlineColor();
    qreal   gap             = il->getGap();
    qreal   shadow          = il->getShadow();
    ProtoPtr proto          = il->getPrototype();
    Xform   xf              = il->getModelXform();
    bool    startUnder      = il->getInitialStartUnder();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,il->getZLevel());
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
    processsStyleThick(ts,outline,width,outline_width,outline_color,pjs,pcs);
    ts << "</" << str << ">" << endl;

    str = "style.Interlace";
    ts << "<" << str << ">" << endl;
    processsStyleInterlace(ts,gap,shadow,includeTipVerts,startUnder);
    ts << "</" << str << ">" << endl;

    if (debug) qDebug() << "end interlace";
    return true;
}

bool MosaicWriter::processOutline(QTextStream &ts, StylePtr s)
{
    Outline * ol = dynamic_cast<Outline*>(s.get());
    if (ol == nullptr)
    {
        fail("Style error","dynamic cast of Interlace");
    }

    ColorSet * cset       = ol->getColorSet();
    eDrawOutline outline  = ol->getDrawOutline();
    qreal   width         = ol->getLineWidth();
    Qt::PenJoinStyle pjs  = ol->getJoinStyle();
    Qt::PenCapStyle pcs   = ol->getCapStyle();
    qreal   outline_width = ol->getOutlineWidth();
    QColor  outline_color = ol->getOutlineColor();
    ProtoPtr proto        = ol->getPrototype();
    Xform   xf            = ol->getModelXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,ol->getZLevel());
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
    processsStyleThick(ts,outline,width,outline_width,outline_color,pjs,pcs);
    ts << "</" << str << ">" << endl;

    if (debug) qDebug() << "end outline";
    return true;
}

bool MosaicWriter::processFilled(QTextStream &ts, StylePtr s)
{
    Filled * filled = dynamic_cast<Filled*>(s.get());
    if (filled == nullptr)
    {
        fail("Style error","dynamic cast of Filled");
    }

    eFillType algorithm     = filled->getAlgorithm();

    ColorSet * colorSetB    = filled->getBlackColorSet();
    ColorSet * colorSetW    = filled->getWhiteColorSet();
    ColorGroup * colorGroup = filled->getColorGroup();

    bool    draw_inside     = filled->getDrawInsideBlacks();
    bool    draw_outside    = filled->getDrawOutsideWhites();

    ProtoPtr proto          = filled->getPrototype();
    Xform   xf              = filled->getModelXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,filled->getZLevel());
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
    processsStyleFilled(ts,draw_inside,draw_outside,algorithm);
    if (algorithm == FILL_DIRECT_FACE)
    {
        processsStyleFilledFaces(ts,filled);
    }
    ts << "</" << str << ">" << endl;

    if (debug) qDebug() << "end filled";
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
    ProtoPtr proto      = pl->getPrototype();
    Xform   xf          = pl->getModelXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,s->getZLevel());
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    setPrototype(ts,proto);
    ts << "</" << str << ">" << endl;

    str = "style.Colored";
    ts << "<" << str << ">" << endl;
    procColorSet(ts,cset);
    ts << "</" << str << ">" << endl;

    if (debug) qDebug() << "end plain";
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
    ProtoPtr proto      = sk->getPrototype();
    Xform   xf          = sk->getModelXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,s->getZLevel());
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

    ColorSet * cset       = em->getColorSet();
    eDrawOutline outline  = em->getDrawOutline();
    qreal   width         = em->getLineWidth();
    Qt::PenJoinStyle pjs  = em->getJoinStyle();
    Qt::PenCapStyle pcs   = em->getCapStyle();
    qreal   outline_width = em->getOutlineWidth();
    QColor  outline_color = em->getOutlineColor();
    qreal   angle         = em->getAngle();
    ProtoPtr proto        = em->getPrototype();
    Xform   xf            = em->getModelXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,em->getZLevel());
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
    processsStyleThick(ts,outline,width,outline_width,outline_color,pjs,pcs);
    ts << "</" << str << ">" << endl;

    str = "style.Emboss";
    ts << "<" << str << ">" << endl;
    processsStyleEmboss(ts,angle);
    ts << "</" << str << ">" << endl;

    if (debug) qDebug() << "end emboss";
    return true;
}

bool MosaicWriter::processTileColors(QTextStream &ts, StylePtr style)
{
    // Note: tile colors are stored in the tiling
    // Here we just specifu that those colors are to be used in this tiling
    TileColors * tc = dynamic_cast<TileColors*>(style.get());
    if (tc == nullptr)
    {
        fail("Style error","dynamic cast of Tile Colors");
    }

    ProtoPtr proto  = tc->getPrototype();
    Xform   xf          = tc->getModelXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,style->getZLevel());
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

    ColorGroup &  group = tc->getTileColors();
    for (ColorSet & set : group)
    {
        QString s = "<BkgdColors>";
        set.resetIndex();
        QColor color = set.getNextTPColor().color;
        s += color.name();
        for (int i=1; i < set.size(); i++)
        {
            s += ",";
            QColor color = set.getNextTPColor().color;
            s += color.name();
        }
        s += "</BkgdColors>";
        ts << s << endl;
    }

    return true;
}

void MosaicWriter::procesToolkitGeoLayer(QTextStream & ts, const Xform & xf, eZLevel zlevel)
{
    ts << "<left__delta>"  << xf.getTranslateX()      << "</left__delta>"  << endl;
    ts << "<top__delta>"   << xf.getTranslateY()      << "</top__delta>"   << endl;
    ts << "<width__delta>" << xf.getScale()           << "</width__delta>" << endl;
    ts << "<theta__delta>" << xf.getRotateRadians()   << "</theta__delta>" << endl;
    ts << "<Z>"            << zlevel                  << "</Z>"        << endl;
}

void MosaicWriter::procColorSet(QTextStream &ts, ColorSet * colorSet)
{
    int count = colorSet->size();
    colorSet->resetIndex();
    for (int i=0; i < count; i++)
    {
        TPColor tpcolor = colorSet->getNextTPColor();
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

void MosaicWriter::processsStyleThick(QTextStream &ts, eDrawOutline draw_outline, qreal width, qreal outlineWidth, QColor outlineColor, Qt::PenJoinStyle join_style, Qt::PenCapStyle cap_style)
{
    QString draw = (draw_outline == OUTLINE_NONE) ? "false" : "true";
    ts << "<draw__outline>" << draw << "</draw__outline>" << endl;
    if (draw_outline == OUTLINE_SET)
    {
        ts << "<outline_width>" << outlineWidth << "</outline_width>" << endl;
        ts << "<outline_color>" << outlineColor.name(QColor::HexArgb) << "</outline_color>" << endl;
    }
    ts << "<width>" << width << "</width>" << endl;
    switch(join_style)
    {
    case Qt::MiterJoin:
        ts << "<join>miter</join>" << endl;
        break;
    case Qt::BevelJoin:
        ts << "<join>bevel</join>" << endl;
        break;
    default:
    case Qt::RoundJoin:
        ts << "<join>round</join>" << endl;
        break;
    }
    switch(cap_style)
    {
    case Qt::SquareCap:
        ts << "<cap>square</cap>" << endl;
        break;
    case Qt::FlatCap:
        ts << "<cap>flat</cap>" << endl;
        break;
    default:
    case Qt::RoundCap:
        ts << "<cap>round</cap>" << endl;
        break;
    }
}

void MosaicWriter::processsStyleInterlace(QTextStream &ts, qreal gap, qreal shadow, bool includeTipVerts,bool startUnder)
{
    QString include = (includeTipVerts) ? "true" : "false";
    QString sunder  = (startUnder) ? "true" : "false";
    ts << "<gap>" << gap << "</gap>" << endl;
    ts << "<shadow>" << shadow << "</shadow>" << endl;
    ts << "<includeTipVerts>" << include << "</includeTipVerts>" << endl;
    ts << "<startUnder>" << sunder << "</startUnder>" << endl;
}

void MosaicWriter::processsStyleFilled(QTextStream &ts, bool draw_inside, bool draw_outside, eFillType algorithm)
{
    QString drawi = (draw_inside) ? "true" : "false";
    QString drawo = (draw_outside) ? "true" : "false";
    ts << "<draw__inside>" << drawi << "</draw__inside>" << endl;
    ts << "<draw__outside>" << drawo << "</draw__outside>" << endl;
    ts << "<algorithm>" << algorithm << "</algorithm>" << endl;
}

void MosaicWriter::processsStyleFilledFaces(QTextStream &ts, class Filled * filled)
{
    DCELPtr dcel = filled->getPrototype()->getDCEL();
    if (!dcel)
    {
        qWarning() << "DCEL not found - cannot save face data";
        return;
    }
    FaceSet & faces = dcel->getFaceSet();
    int size        = faces.size();

    QString str = QString("<FaceColors faces=\"%1\">").arg(size);
    ts << str << endl;

    // get these color indices directly from the visible faces
    int count = 0;
    while (count < size)
    {
        ts << QString::number(faces[count]->iPalette);
        count++;
        if (count < size)
        {
            ts << ",";
        }
    }
    ts << endl;
    ts << "</FaceColors>" << endl;
}

void MosaicWriter::processsStyleEmboss(QTextStream &ts, qreal  angle)
{
    ts << "<angle>" << angle << "</angle>";
}

void MosaicWriter::setPrototype(QTextStream & ts, ProtoPtr pp)
{
    QString qsid;
    if (hasReference(pp))
    {
        qsid = getProtoReference(pp);
        ts << "<prototype" << qsid << "/>" << endl;
        return;
    }

    qsid = mwbase.nextId();
    setProtoReference(mwbase.getRef(),pp);
    ts << "<prototype" << qsid << " >" << endl;

    QString str = "app.Prototype";
    ts << "<" << str << ">" << endl;

    auto tiling = pp->getTiling();
    if (tiling)
    {
        ts << "<Tiling>" << pp->getTiling()->getVName().get() << "</Tiling>" << endl;
    }
    else
    {
        qWarning("Saving mosiaic with no tiling");
    }

    uint cleanseLevel   = pp->getCleanseLevel();
    qreal sensitivity   = pp->getCleanseSensitivity();

    if (cleanseLevel > 0)
    {
        ts << "<Cleanse>" << QString::number(cleanseLevel,16) << "</Cleanse>" << endl;
        ts << "<Sensitivity>" << sensitivity << "</Sensitivity>" << endl;
    }

    if (pp->getDistortionEnable())
    {
        QTransform t = pp->getDistortion();
        ts << "<Distort>" << endl;
        ts << "<scalex>" << t.m11() << "</scalex>" << endl;
        ts << "<scaley>" << t.m22() << "</scaley>" << endl;
        ts << "</Distort>" << endl;
    }

    QVector<DELPtr>  dels = pp->getDesignElements();
    QVector<DELPtr>::reverse_iterator it;
    for (it = dels.rbegin(); it != dels.rend(); it++)
    {
        DELPtr de = *it;
        TilePtr tile = de->getTile();
        MotifPtr  motif  = de->getMotif();

        if (!tile)
            fail("Tile not found in prototype","");
        if (!motif)
            fail("Moitf not found in prototype", "");

        ts << "<entry>" << endl;

        setTile(ts,tile);

        eMotifType motifType = motif->getMotifType();
        switch (motifType)
        {
        case MOTIF_TYPE_STAR:
            setStar(ts,Sys::XMLgetMotifName(motifType),motif);
            break;

        case MOTIF_TYPE_STAR2:
            setStar2(ts,Sys::XMLgetMotifName(motifType),motif);
            break;

        case MOTIF_TYPE_ROSETTE:
            setRosette(ts,Sys::XMLgetMotifName(motifType),motif);
            break;

        case MOTIF_TYPE_ROSETTE2:
            setRosette2(ts,Sys::XMLgetMotifName(motifType),motif);
            break;

        case MOTIF_TYPE_RADIAL:
        case MOTIF_TYPE_UNDEFINED:
            fail("Unexpected figure type:", sMotifType[motifType]);
            
        case MOTIF_TYPE_EXPLICIT_MAP:
        case MOTIF_TYPE_IRREGULAR_NO_MAP:
        case MOTIF_TYPE_INFERRED:
        case MOTIF_TYPE_IRREGULAR_ROSETTE:
        case MOTIF_TYPE_HOURGLASS:
        case MOTIF_TYPE_INTERSECT:
        case MOTIF_TYPE_GIRIH:
        case MOTIF_TYPE_IRREGULAR_STAR:
        case MOTIF_TYPE_EXPLCIT_TILE:
            setExplicitMotif(ts,Sys::XMLgetMotifName(motifType),motif);
            break;
        }

        ts << "</entry>" << endl;
    }

    ts << "</" << str << ">" << endl;
    ts << "</prototype>" << endl;
    if (debug) qDebug() << "Proto created";
}

void MosaicWriter::setTile(QTextStream & ts, TilePtr tile)
{
    QString str = "Tile";

    QString qsid;
    if (hasReference(tile))
    {
        qsid = getTileReference(tile);
    }
    else
    {
        qsid = mwbase.nextId();
        setTileReference(mwbase.getRef(),tile);
    }

    ts << "<" << str << qsid << ">" << endl;
    if (tile->isRegular())
    {
        ts << "<regular>true</regular>" << endl;
    }
    else
    {
        ts << "<regular>false</regular>" << endl;
    }
    ts << "<rotation>" << tile->getRotation() << "</rotation>" << endl;
    ts << "<scale>" << tile->getScale() << "</scale>" << endl;

    ts << "<edges" << mwbase.nextId() << ">" << endl;

    const EdgeSet & ep  = tile->getBase();
    setEdgePoly(ts,ep);

    ts << "</edges>" << endl;

    ts << "</" << str << ">" << endl;

}

void MosaicWriter::setMotifCommon(QTextStream & ts, MotifPtr motif)
{
    qreal fsc = motif->getMotifScale();
    qreal   r = motif->getMotifRotate();
    int     n = motif->getN();
    int ver   = motif->getVersion();

    if (ver > 1)
    ts << "<version>"        << ver << "</version>"        << endl;
    ts << "<figureScale>"    << fsc << "</figureScale>"    << endl;
    ts << "<r>"              << r   << "</r>"              << endl;
    ts << "<n>"              << n   << "</n>"              << endl;

    for (const ExtenderPtr & ep : motif->getExtenders())
    {
        QString  extRays    = (ep->getExtendRays())              ? "\"t\"" : "\"f\"";
        QString  extTips2B  = (ep->getExtendTipsToBound())       ? "\"t\"" : "\"f\"";
        QString  extTips2T  = (ep->getExtendBoundaryToTile())    ? "\"t\"" : "\"f\"";
        QString  conRays    = "\""+ QString::number(ep->getConnectRays()) + "\"";
        QString  embedBound = (ep->getEmbedBoundary())           ? "\"t\"" : "\"f\"";
        QString  embedTile  = (ep->getEmbedTile())               ? "\"t\"" : "\"f\"";

        ts << "<Extender"
           << "  extRays="      << extRays
           << "  extTips2B="    << extTips2B
           << "  extTips2T="    << extTips2T
           << "  conRays="      << conRays
           << "  embedBound="   << embedBound
           << "  embedTile="    << embedTile;
        ts << ">" << endl;

        const ExtendedBoundary & eb = ep->getExtendedBoundary();
        int    bs = eb.getSides();
        qreal bsc = eb.getScale();
        qreal brt = eb.getRotate();
        ts << "<boundarySides>"  << bs  << "</boundarySides>"  << endl;
        ts << "<boundaryScale>"  << bsc << "</boundaryScale>"  << endl;
        ts << "<boundaryRotate>" << brt << "</boundaryRotate>" << endl;

        ts << "</Extender>" << endl;
    }

    if (motif->getRadialConnector())
    {
        ts << "<Connector/>" << endl;
    }

    if (motif->getCleanse() > 0)
    {
        ts << "<Cleanse>" << motif->getCleanse() << "</Cleanse>" << endl;
        ts << "<Sensitivity>" << motif->getCleanseSensitivity() << "</Sensitivity>" << endl;
    }
}

void MosaicWriter::setExplicitMotif(QTextStream & ts, QString name, MotifPtr motif)
{
    auto ep = dynamic_pointer_cast<IrregularMotif>(motif);
    if (!ep)
    {
        fail("MosaicWriter Style error","dynamic cast of Explicit Motif");
    }

    QString qsid;
    if (hasReference(ep))
    {
        qsid = getExplicitReference(ep);
        ts << "<" << name << qsid << "/>" << endl;
        return;
   }

    qsid = mwbase.nextId();
    setExplicitReference(mwbase.getRef(),ep);
    ts << "<" << name << qsid << " type=\"" << motif->getMotifTypeString() << "\"" << ">" << endl;

    setMotifCommon(ts,motif);

    switch(ep->getMotifType())
    {
    case MOTIF_TYPE_UNDEFINED:
    case MOTIF_TYPE_RADIAL:
    case MOTIF_TYPE_ROSETTE:
    case MOTIF_TYPE_ROSETTE2:
    case MOTIF_TYPE_STAR:
    case MOTIF_TYPE_STAR2:
        fail("Code Error","Not an explicit motif");
        
    case MOTIF_TYPE_EXPLICIT_MAP:
    {
        auto emm = dynamic_pointer_cast<ExplicitMapMotif>(motif);
        MapPtr map = emm->getExplicitMap();
        setMap(ts,map);
        break;
    }

    case MOTIF_TYPE_INFERRED:
    case MOTIF_TYPE_EXPLCIT_TILE:
    case MOTIF_TYPE_IRREGULAR_NO_MAP:
        // these have no parameters
        break;

    case MOTIF_TYPE_GIRIH:
        ts << "<skip>"  << ep->skip  << "</skip>"  << endl;
        break;

    case MOTIF_TYPE_IRREGULAR_STAR:
    {
        ts << "<s>" << ep->s << "</s>" << endl;
        ts << "<d>" << ep->d << "</d>"  << endl;
        auto star = dynamic_pointer_cast<IrregularStar>(ep);
    }   break;

    case MOTIF_TYPE_HOURGLASS:
        ts << "<s>" << ep->s << "</s>" << endl;
        ts << "<d>" << ep->d << "</d>"  << endl;

        break;

    case MOTIF_TYPE_IRREGULAR_ROSETTE:
    {
        ts << "<s>" << ep->s << "</s>" << endl;
        ts << "<q>" << ep->q << "</q>"  << endl;
        ts << "<rFlexPt>" << ep->r << "</rFlexPt>"  << endl;
        auto rose = dynamic_pointer_cast<IrregularRosette>(ep);
    }   break;

    case MOTIF_TYPE_INTERSECT:
        ts << "<s>" << ep->s << "</s>" << endl;
        ts << "<skip>"  << ep->skip  << "</skip>"  << endl;
        ts << "<progressive>" << ((ep->progressive) ? "t" : "f") << "</progressive>" << endl;
        break;
    }

    ts << "</" << name << ">" << endl;
}

void MosaicWriter::setStar(QTextStream & ts, QString name, MotifPtr fp)
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
        qsid = mwbase.nextId();
        setStarReference(mwbase.getRef(),sp);
    }

    ts << "<" << name << qsid;
    if (sp->getOnPoint())
    {
        ts << " onpoint=\"t\"";
    }
    if (sp->getInscribe())
    {
        ts << " inscribed=\"t\"";
    }
    ts << ">" << endl;

    setMotifCommon(ts,fp);
    setStarCommon(ts,sp);
}

void MosaicWriter::setStar2(QTextStream & ts, QString name, MotifPtr fp)
{
    Star2Ptr sp = std::dynamic_pointer_cast<Star2>(fp);
    if (!sp)
    {
        fail("Style error","dynamic cast of Star");
    }

    QString qsid;
    if (hasReference(sp))
    {
        qsid = getStar2Reference(sp);
    }
    else
    {
        qsid = mwbase.nextId();
        setStar2Reference(mwbase.getRef(),sp);
    }

    ts << "<" << name << qsid;
    if (sp->getOnPoint())
    {
        ts << " onpoint=\"t\"";
    }
    if (sp->getInscribe())
    {
        ts << " inscribed=\"t\"";
    }
    ts << ">" << endl;

    setMotifCommon(ts,fp);
    setStar2Common(ts,sp);
}

void MosaicWriter::setRosette(QTextStream & ts, QString name, MotifPtr motif)
{
    RosettePtr rp = std::dynamic_pointer_cast<Rosette>(motif);
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
        qsid = mwbase.nextId();
        setRosetteReference(mwbase.getRef(),rp);
    }

    ts << "<" << name << qsid;
    if (rp->getOnPoint())
    {
        ts << " onpoint=\"t\"";
    }
    if (rp->getInscribe())
    {
        ts << " inscribed=\"t\"";
    }
    ts << ">" << endl;

    setMotifCommon(ts,motif);
    setRosetteCommon(ts,rp);
}

void MosaicWriter::setRosette2(QTextStream & ts, QString name, MotifPtr motif)
{
    Rosette2Ptr rp = std::dynamic_pointer_cast<Rosette2>(motif);
    if (!rp)
    {
        fail("Style error","dynamic cast of Rosette");
    }

    QString qsid;
    if (hasReference(rp))
    {
        qsid = getRosette2Reference(rp);
    }
    else
    {
        qsid = mwbase.nextId();
        setRosette2Reference(mwbase.getRef(),rp);
    }

    QString  constrain = (rp->getConstrain()) ? "\"t\"" : "\"f\"";

    ts << "<" << name << qsid << "  constrain=" << constrain;
    if (rp->getOnPoint())
    {
        ts << " onpoint=\"t\"";
    }
    if (rp->getInscribe())
    {
        ts << " inscribed=\"t\"";
    }
    ts << ">" << endl;

    setMotifCommon(ts,motif);
    setRosette2Common(ts,rp);
}

void  MosaicWriter::setStarCommon(QTextStream & ts, StarPtr star)
{
    qreal      d = star->getD();
    int        s = star->getS();

    ts << "<d>" << d << "</d>" << endl;
    ts << "<s>" << s << "</s>" << endl;

    ts << "</" << Sys::XMLgetMotifName(star->getMotifType()) << ">" << endl;
}

void  MosaicWriter::setStar2Common(QTextStream & ts, Star2Ptr star)
{
    qreal  theta = star->getTheta();
    int        s = star->getS();

    ts << "<angle>" << theta << "</angle>" << endl;
    ts << "<s>" << s << "</s>" << endl;

    ts << "</" << Sys::XMLgetMotifName(star->getMotifType()) << ">" << endl;
}

void  MosaicWriter::setRosetteCommon( QTextStream & ts, RosettePtr rose)
{
    int s       = rose->getS();
    qreal q     = rose->getQ();

    ts << "<q>" << q << "</q>" << endl;
    ts << "<s>" << s << "</s>" << endl;

    ts << "</" << Sys::XMLgetMotifName(rose->getMotifType()) << ">" << endl;
}

void  MosaicWriter::setRosette2Common(QTextStream & ts, Rosette2Ptr rose)
{
    int s        = rose->getS();
    qreal x      = rose->getKneeX();
    qreal y      = rose->getKneeY();
    qreal k      = rose->getK();
    uint  types  = rose->getTipTypes();
    eTipMode tmode = rose->getTipMode();

    QString tiptypes;
    if (types & TIP_TYPE2_OUTER)
        tiptypes += "outer|";
    if (types & TIP_TYPE2_INNER)
        tiptypes += "inner|";
    if (types & TIP_TYPE2_FLIPPED)
        tiptypes += "flipped";

    QString mode;
    if (tmode == TIP_MODE_REGULAR)
        mode = "regular";
    else
        mode = "alternate";

    ts << "<x>" << x << "</x>" << endl;
    ts << "<y>" << y << "</y>" << endl;
    ts << "<k>" << k << "</k>" << endl;
    ts << "<s>" << s << "</s>" << endl;
    ts << "<tiptypes>" << tiptypes << "</tiptypes>" << endl;
    ts << "<tipmode>" << mode << "</tipmode>" << endl;

    ts << "</" << Sys::XMLgetMotifName(rose->getMotifType()) << ">" << endl;
}

bool MosaicWriter::setMap(QTextStream &ts, MapPtr map)
{
    if (debug) qDebug().noquote() << "Writing map" << map->summary();

    MapVerifier mv(map);
    bool rv = mv.verifyAndFix(true,true);
    if (!rv)
        return false;
    
    if (debug) qDebug().noquote() << "Writing map" << map->summary();

    QString qsid;

    if (hasReference(map))
    {
        qsid = getMapReference(map);
        ts << "<map" << qsid << "/>" << endl;
        return true;
    }

    qsid = mwbase.nextId();
    setMapReference(mwbase.getRef(),map);
    ts << "<map" << qsid << ">" << endl;

    // vertices
    const QVector<VertexPtr> & vertices = map->getVertices();
    setVertices(ts,vertices);

    // Edges
    const EdgeSet & edges = map->getEdges();
    setEdges(ts,edges);

    ts << "</map>" << endl;

    return true;
}

void MosaicWriter::setVertices(QTextStream & ts, const QVector<VertexPtr> & vertices)
{
    ts << "<vertices" << mwbase.nextId() <<  ">" << endl;
    for (const auto & v : std::as_const(vertices))
    {
        setVertex(ts,v);
    }
    ts << "</vertices>" << endl;
}

void MosaicWriter::setEdges(QTextStream & ts, const EdgeSet & edges)
{
    ts << "<edges" << mwbase.nextId() <<  ">" << endl;
    for (const auto & edge : std::as_const(edges))
    {
        setEdge(ts,edge);
    }
    ts << "</edges>" << endl;
}

void MosaicWriter::setEdgePoly(QTextStream & ts, const EdgeSet & epoly)
{
    for (auto & ep : std::as_const(epoly))
    {
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
            QString str = QString("<Curve convex=\"%1\">").arg((ep->getCurveType() == CURVE_CONVEX) ? "t" : "f");
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
    if (mwbase.hasReference(v))
    {
        qsid = mwbase.getVertexReference(v);
        ts << "<" << name << qsid << "/>" << endl;
        return;
    }

    qsid = mwbase.nextId();
    mwbase.setVertexReference(mwbase.getRef(),v);

    QPointF pt = v->pt;

    ts << "<" << name << qsid << ">";
    ts << pt.x() << "," << pt.y();
    ts << "</" << name << ">" << endl;
}

void MosaicWriter::setVertex(QTextStream & ts, VertexPtr v, QString name)
{
    QString qsid;
    if (mwbase.hasReference(v))
    {
        qsid = mwbase.getVertexReference(v);
        ts << "<" << name << qsid << "/>" << endl;
        return;
    }

    qsid = mwbase.nextId();
    mwbase.setVertexReference(mwbase.getRef(),v);
    ts << "<" << name << qsid << ">" << endl;

    // pos
    setPos(ts,v->pt);

    ts << "</" << name << ">" << endl;
}

void MosaicWriter::setEdges(QTextStream & ts, EdgeSet & qvec)
{
    ts << "<edges>" << endl;

    for (const auto & e : std::as_const(qvec))
    {
        // edge
        setEdge(ts,e);      // called from setNeighbour first time
    }

    ts << "</edges>" << endl;
 }

void MosaicWriter::setEdge(QTextStream & ts, EdgePtr e)
{
    if (debug) qDebug() << "Edge" << e.get();
    QString qsid;
    if (hasReference(e))
    {
        qsid = getEdgeReference(e);
        ts << "<Edge" << qsid << "/>" << endl;
    }

    auto type = e->getType();

    qsid = mwbase.nextId();
    if (debug) qDebug() << "new edge ref=" << mwbase.getRef();
    setEdgeReference(mwbase.getRef(),e);

    if (type == EDGETYPE_CURVE)
    {
        QString str = QString("<curve %1 convex=\"%2\">").arg(qsid).arg((e->getCurveType() == CURVE_CONVEX) ? "t" : "f");
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

    if (type == EDGETYPE_CURVE)
    {
        QPointF p = e->getArcCenter();
        setPos(ts,p);
    }

    if (type == EDGETYPE_CURVE)
    {
        ts << "</curve>" << endl;
    }
    else
    {
        ts << "</Edge>" << endl;
    }
}

void MosaicWriter::procWidth(QTextStream &ts,qreal width)
{
    ts << "<width>" << width << "</width>" << endl;
}

void MosaicWriter::procLength(QTextStream &ts,qreal length)
{
    ts << "<length>" << length << "</length>" << endl;
}

void MosaicWriter::procSize(QTextStream &ts,QSize viewSize, QSize canvasSize)
{
    ts << "<size>"    << endl;
    ts << "<width>"   << viewSize.width()    << "</width>" << endl;
    ts << "<height>"  << viewSize.height()   << "</height>" << endl;
    ts << "<zwidth>"  << canvasSize.width()  << "</zwidth>" << endl;
    ts << "<zheight>" << canvasSize.height() << "</zheight>" << endl;
    ts << "</size>"   << endl;
}

void MosaicWriter::procRect(QTextStream &ts, QRectF &rect)
{
    ts << "<rect>"    << endl;
    ts << "<X>"       << rect.x()       << "</X>" << endl;
    ts << "<Y>"       << rect.y()       << "</Y>" << endl;
    ts << "<width>"   << rect.width()   << "</width>" << endl;
    ts << "<height>"  << rect.height()  << "</height>" << endl;
    ts << "</rect>"   << endl;
}

void MosaicWriter::procCircle(QTextStream &ts, Circle & c)
{
    ts << "<circle>"    << endl;
    ts << "<radius>"    << c.radius     << "</radius>" << endl;
    ts << "<X>"         << c.centre.x() << "</X>" << endl;
    ts << "<Y>"         << c.centre.y() << "</Y>" << endl;
    ts << "</circle>"   << endl;
}

void MosaicWriter::setPoint(QTextStream & ts, QPointF pt, QString name)
{
    ts << "<" << name << ">";
    ts << pt.x() << "," << pt.y();
    ts << "</" << name << ">" << endl;
}

void MosaicWriter::procAPolygon(QTextStream &ts,APolygon & apoly)
{
    QString str = QString("<APoly source=\"%1\">").arg(apoly.usesSource() ? "t" : "f");
    ts << str << endl;

    if (apoly.usesSource())
    {
        auto poly = apoly.getSource();
        procPolygon(ts, poly);
    }
    else
    {
        ts << "<n>" << apoly.getSides() << "</n>" << endl;
    }

    ts << "<rot>" << apoly.getRotate() << "</rot>" << endl;
    ts << "<scale>" << apoly.getScale() << "</scale>" << endl;
    setPos(ts,apoly.getPos());
    ts << "</APoly>" << endl;
}

void MosaicWriter::procPolygon(QTextStream &ts,QPolygonF & poly)
{
    ts << "<Poly>" << endl;
    for (int i=0; i < poly.size(); i++)
    {
        setPos(ts,poly[i]);
    }
    ts << "</Poly>" << endl;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

bool MosaicWriter::hasReference(PolyPtr pp)
{
    return poly_ids.contains(pp);
}

bool MosaicWriter::hasReference(ProtoPtr pp)
{
    return proto_ids.contains(pp);
}

bool MosaicWriter::hasReference(TilePtr fp)
{
    return tile_ids.contains(fp);
}

bool MosaicWriter::hasReference(MotifPtr fp)
{
    return motif_ids.contains(fp);
}

bool MosaicWriter::hasReference(ExplicitPtr ep)
{
    return explicit_ids.contains(ep);
}

bool MosaicWriter::hasReference(MapPtr map)
{
    return map_ids.contains(map);
}

bool MosaicWriter::hasReference(RosettePtr n)
{
    return rosette_ids.contains(n);
}

bool MosaicWriter::hasReference(Rosette2Ptr n)
{
    return rosette2_ids.contains(n);
}

bool MosaicWriter::hasReference(StarPtr n)
{
    return star_ids.contains(n);
}

bool MosaicWriter::hasReference(Star2Ptr n)
{
    return star2_ids.contains(n);
}

bool MosaicWriter::hasReference(EdgePtr e)
{
    return edge_ids.contains(e);
}

void MosaicWriter::setProtoReference(int id, ProtoPtr ptr)
{
    proto_ids[ptr] = id;
}

void MosaicWriter::setPolyReference(int id, PolyPtr ptr)
{
    poly_ids[ptr] = id;
}

void MosaicWriter::setMotifReference(int id, MotifPtr ptr)
{
   motif_ids[ptr] = id;
}

void MosaicWriter::setTileReference(int id,TilePtr ptr)
{
    tile_ids[ptr] = id;
}

void MosaicWriter::setExplicitReference(int id, ExplicitPtr ptr)
{
    explicit_ids[ptr] = id;
}

void MosaicWriter::setMapReference(int id, MapPtr ptr)
{
    map_ids[ptr] = id;
}

void MosaicWriter::setRosetteReference(int id, RosettePtr ptr)
{
    rosette_ids[ptr] = id;
}

void MosaicWriter::setRosette2Reference(int id, Rosette2Ptr ptr)
{
    rosette2_ids[ptr] = id;
}

void MosaicWriter::setStarReference(int id, StarPtr ptr)
{
    star_ids[ptr] = id;
}

void MosaicWriter::setStar2Reference(int id, Star2Ptr ptr)
{
    star2_ids[ptr] = id;
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

QString MosaicWriter::getProtoReference(ProtoPtr ptr)
{
    int id =  proto_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getTileReference(TilePtr ptr)
{
    int id =  tile_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getFMotifReference(MotifPtr ptr)
{
    int id =  motif_ids.value(ptr);
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

QString MosaicWriter::getRosetteReference(RosettePtr ptr)
{
    int id =  rosette_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getRosette2Reference(Rosette2Ptr ptr)
{
    int id =  rosette2_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getStarReference(StarPtr ptr)
{
    int id =  star_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getStar2Reference(Star2Ptr ptr)
{
    int id =  star2_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString MosaicWriter::getEdgeReference(EdgePtr ptr)
{
    int id =  edge_ids.value(ptr);
    if (debug) qDebug() << "edge ref" << id;
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
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
