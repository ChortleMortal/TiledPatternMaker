#include "mosaic/mosaic_writer.h"
#include "mosaic/design_element.h"
#include "mosaic/mosaic.h"
#include "makers/prototype_maker/prototype.h"
#include "motifs/explicit_map_motif.h"
#include "motifs/irregular_motif.h"
#include "motifs/extended_rosette.h"
#include "motifs/extended_star.h"
#include "motifs/rosette_connect.h"
#include "motifs/star.h"
#include "motifs/star_connect.h"
#include "motifs/irregular_rosette.h"
#include "motifs/irregular_star.h"
#include "geometry/crop.h"
#include "geometry/edge.h"
#include "geometry/map.h"
#include "geometry/vertex.h"
#include "makers/map_editor/map_editor.h"
#include "misc/backgroundimage.h"
#include "misc/border.h"
#include "misc/tpm_io.h"
#include "misc/fileservices.h"
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
#include "tile/tiling.h"
#include "tile/tiling_writer.h"
#include "viewers/viewcontrol.h"

using std::dynamic_pointer_cast;

MosaicWriter::MosaicWriter() : MosaicWriterBase()
{
    config = Configuration::getInstance();

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
      currentXMLVersion = 14; // 23NOV22 <n> is common for all motifs
}

MosaicWriter::~MosaicWriter()
{
}

// this saves a complete mosaic
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
        _failMsg = QString("ERROR writing XML file: %1").arg(fileName);
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
            ts << "<" << str << nextId() << " >" << endl;
            processThick(ts,s);
            break;
        case STYLE_FILLED:
            str = "style.Filled";
            ts << "<" << str << nextId() << " >" << endl;
            processFilled(ts,s);
            break;
        case STYLE_INTERLACED:
            str = "style.Interlace";
            ts << "<" << str << nextId() << ">" << endl;
            processInterlace(ts,s);
            break;
        case STYLE_OUTLINED:
            str = "style.Outline";
            ts << "<" << str << nextId() << ">" << endl;
            processOutline(ts,s);
            break;
        case STYLE_EMBOSSED:
            str = "style.Emboss";
            ts << "<" << str << nextId() << ">" << endl;
            processEmboss(ts,s);
            break;
        case STYLE_PLAIN:
            str = "style.Plain";
            ts << "<" << str << nextId() << ">" << endl;
            processPlain(ts,s);
            break;
        case STYLE_SKETCHED:
            str = "style.Sketch";
            ts << "<" << str << nextId() << " >" << endl;
            processSketch(ts,s);
            break;
        case STYLE_TILECOLORS:
            str = "style.TileColors";
            ts << "<" << str << nextId() << " >" << endl;
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
    ModelSettings & info  = _mosaic->getSettings();
    QColor bkgdColor      = info.getBackgroundColor();
    QSize  size           = info.getSize();
    QSize  zsize          = info.getZSize();
    BorderPtr border      = _mosaic->getBorder();
    CropPtr crop          = _mosaic->getCrop();
    BkgdImgPtr bip        = BackgroundImage::getSharedInstance();
    uint cleanseLevel     = _mosaic->getCleanseLevel();

    ts << "<design>" << endl;
    procSize(ts,size,zsize);
    procBackground(ts,bkgdColor);
    procBorder(ts,border);
    procCrop(ts,crop);
    int minX,minY,maxX,maxY;
    bool singleton;
    info.getFillData().get(singleton,minX,maxX,minY,maxY);
    if (!singleton)
    {
        ts << "<Fill singleton = \"f\">" << minX << "," << maxX << "," << minY << "," << maxY << "</Fill>" << endl;
    }
    else
    {
        ts << "<Fill singleton = \"t\">0,0,0,0</Fill>";
    }
    TilingWriter::writeBackgroundImage(ts,bip);

    if (cleanseLevel > 0)
    {
        ts << "<Cleanse>" << QString::number(cleanseLevel,16) << "</Cleanse>" << endl;
    }

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

void MosaicWriter::procRect(QTextStream &ts, QRectF rect)
{
    ts << "<rect>"    << endl;
    ts << "<X>"       << rect.x()       << "</X>" << endl;
    ts << "<Y>"       << rect.y()       << "</Y>" << endl;
    ts << "<width>"   << rect.width()   << "</width>" << endl;
    ts << "<height>"  << rect.height()  << "</height>" << endl;
    ts << "</rect>"   << endl;
}

void MosaicWriter::procCircle(QTextStream &ts,CirclePtr c)
{
    ts << "<circle>"    << endl;
    ts << "<radius>"    << c->radius     << "</radius>" << endl;
    ts << "<X>"         << c->centre.x() << "</X>" << endl;
    ts << "<Y>"         << c->centre.y() << "</Y>" << endl;
    ts << "</circle>"   << endl;
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

    QString stype  = border->getBorderTypeString();
    QString sshape = border->getCropTypeString();
    QString txt = QString("<border type=\"%1\" shape=\"%2\">").arg(stype).arg(sshape);
    ts << txt << endl;

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
            auto c = b0->getCircle();
            procCircle(ts,c);
        }
    }
    else if (btype == BORDER_TWO_COLOR)
    {
        BorderTwoColor * b1 = dynamic_cast<BorderTwoColor*>(border.get());
        Q_ASSERT(b1);
        QColor color1,color2;
        qreal  width;
        b1->get(color1,color2,width);
        procColor(ts,color1);
        procColor(ts,color2);
        procWidth(ts,width);
        QRectF rect = b1->getRect();
        procRect(ts,rect);
    }
    else if (btype == BORDER_BLOCKS)
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
        QRectF rect = b2->getRect();
        procRect(ts,rect);
    }

    ts << "</border>" << endl;
}

void MosaicWriter::procCrop(QTextStream &ts,CropPtr crop)
{
    if (!crop)
    {
        return;
    }

    QRectF r = crop->getRect();
    if (r.isValid())
    {
        ts << "<Crop>" << endl;
        ts << "<boundary>" << r.x() << "," << r.y() << "," << r.width() << "," << r.height() << "</boundary>" << endl;
        ts << "</Crop>" << endl;
    }
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
    ProtoPtr proto      = th->getPrototype();
    Xform   xf              = th->getCanvasXform();
    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,s->zValue());
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
    ProtoPtr proto      = il->getPrototype();
    Xform   xf              = il->getCanvasXform();
    bool    startUnder      = il->getInitialStartUnder();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,il->zValue());
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

    ColorSet * cset       = ol->getColorSet();
    eDrawOutline outline  = ol->getDrawOutline();
    qreal   width         = ol->getLineWidth();
    Qt::PenJoinStyle pjs  = ol->getJoinStyle();
    Qt::PenCapStyle pcs   = ol->getCapStyle();
    qreal   outline_width = ol->getOutlineWidth();
    QColor  outline_color = ol->getOutlineColor();
    ProtoPtr proto    = ol->getPrototype();
    Xform   xf            = ol->getCanvasXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,ol->zValue());
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

    ColorSet * colorSetB    = fl->getBlackColorSet();
    ColorSet * colorSetW    = fl->getWhiteColorSet();
    ColorGroup * colorGroup = fl->getColorGroup();

    bool    draw_inside     = fl->getDrawInsideBlacks();
    bool    draw_outside    = fl->getDrawOutsideWhites();

    ProtoPtr proto      = fl->getPrototype();
    Xform   xf              = fl->getCanvasXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,fl->zValue());
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
    ProtoPtr proto  = pl->getPrototype();
    Xform   xf          = pl->getCanvasXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,s->zValue());
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
    ProtoPtr proto  = sk->getPrototype();
    Xform   xf          = sk->getCanvasXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,s->zValue());
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
    ProtoPtr proto    = em->getPrototype();
    Xform   xf            = em->getCanvasXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,em->zValue());
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

    ProtoPtr proto  = tc->getPrototype();
    Xform   xf          = tc->getCanvasXform();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf,s->zValue());
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

void MosaicWriter::procesToolkitGeoLayer(QTextStream & ts, const Xform & xf, int zlevel)
{
    ts << "<left__delta>"  << xf.getTranslateX()      << "</left__delta>"  << endl;
    ts << "<top__delta>"   << xf.getTranslateY()      << "</top__delta>"   << endl;
    ts << "<width__delta>" << xf.getScale()           << "</width__delta>" << endl;
    ts << "<theta__delta>" << xf.getRotateRadians()   << "</theta__delta>" << endl;
    QPointF pt = xf.getModelCenter();
    ts << "<center>" << pt.x() << "," << pt.y()       << "</center>"        << endl;
    ts << "<Z>"            << zlevel                  << "</Z>"        << endl;
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

void MosaicWriter::processsStyleFilled(QTextStream &ts, bool draw_inside, bool draw_outside, int algorithm)
{
    QString drawi = (draw_inside) ? "true" : "false";
    QString drawo = (draw_outside) ? "true" : "false";
    ts << "<draw__inside>" << drawi << "</draw__inside>" << endl;
    ts << "<draw__outside>" << drawo << "</draw__outside>" << endl;
    ts << "<algorithm>" << algorithm << "</algorithm>" << endl;
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

void MosaicWriter::setPrototype(QTextStream & ts, ProtoPtr pp)
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
    ts << "<prototype" << qsid << " >" << endl;

    QString str = "app.Prototype";
    ts << "<" << str << ">" << endl;

    auto tiling = pp->getTiling();
    if (tiling)
    {
        ts << "<string>" << pp->getTiling()->getName() << "</string>" << endl;
    }
    else
    {
        qWarning("Saving mosiaic with no tiling");
    }

    QVector<DesignElementPtr>  dels = pp->getDesignElements();
    QVector<DesignElementPtr>::reverse_iterator it;
    for (it = dels.rbegin(); it != dels.rend(); it++)
    {
        DesignElementPtr de = *it;
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
            setStar(ts,motifRepresentation[motifType],motif);
            break;

        case MOTIF_TYPE_EXTENDED_STAR:
            setExtendedStar(ts,motifRepresentation[motifType],motif);
            break;

        case MOTIF_TYPE_EXTENDED_ROSETTE:
            setExtendedRosette(ts,motifRepresentation[motifType],motif);
            break;

        case MOTIF_TYPE_ROSETTE:
            setRosette(ts,motifRepresentation[motifType],motif);
            break;

        case MOTIF_TYPE_CONNECT_STAR:
            setStarConnect(ts,motifRepresentation[motifType],motif);
            break;

        case MOTIF_TYPE_CONNECT_ROSETTE:
            setRosetteConnect(ts,motifRepresentation[motifType],motif);
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
            setExplicitMotif(ts,motifRepresentation[motifType],motif);
            break;
        }

        ts << "</entry>" << endl;
    }

    ts << "</" << str << ">" << endl;
    ts << "</prototype>" << endl;
    qDebug() << "Proto created";
}

void MosaicWriter::setTile(QTextStream & ts, TilePtr tile)
{
    QString str = "tile.Feature";

    QString qsid;
    if (hasReference(tile))
    {
        qsid = getTileReference(tile);
    }
    else
    {
        qsid = nextId();
        setTileReference(getRef(),tile);
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

    ts << "<edges" << nextId() << ">" << endl;

    const EdgePoly & ep  = tile->getBase();
    setEdgePoly(ts,ep);

    ts << "</edges>" << endl;

    ts << "</" << str << ">" << endl;

}

void MosaicWriter::setMotifCommon(QTextStream & ts, MotifPtr motif)
{
    const ExtendedBoundary & eb = motif->getExtendedBoundary();

    int    bs = eb.sides;
    qreal bsc = eb.scale;
    qreal fsc = motif->getMotifScale();
    qreal   r = motif->getMotifRotate();
    int     n = motif->getN();
    int ver   = motif->getVersion();

    if (ver > 1)
        ts << "<version>"       << ver << "</version>"       << endl;
    ts << "<boundarySides>" << bs  <<"</boundarySides>"  << endl;
    ts << "<boundaryScale>" << bsc << "</boundaryScale>" << endl;
    ts << "<figureScale>"   << fsc << "</figureScale>"   << endl;
    ts << "<r>"             << r   << "</r>"             << endl;
    ts << "<n>"             << n   << "</n>"             << endl;
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

    qsid = nextId();
    setExplicitReference(getRef(),ep);
    ts << "<" << name << qsid << " type=\"" << motif->getMotifTypeString() << "\"" << ">" << endl;

    setMotifCommon(ts,motif);

    switch(ep->getMotifType())
    {
    case MOTIF_TYPE_UNDEFINED:
    case MOTIF_TYPE_RADIAL:
    case MOTIF_TYPE_ROSETTE:
    case MOTIF_TYPE_STAR:
    case MOTIF_TYPE_CONNECT_STAR:
    case MOTIF_TYPE_CONNECT_ROSETTE:
    case MOTIF_TYPE_EXTENDED_ROSETTE:
    case MOTIF_TYPE_EXTENDED_STAR:
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

void MosaicWriter::setStar(QTextStream & ts, QString name, MotifPtr fp, bool childEnd)
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

    setMotifCommon(ts, fp);

    qreal      d = sp->getD();
    int        s = sp->getS();
    ts << "<d>" << d << "</d>" << endl;
    ts << "<s>" << s << "</s>" << endl;

    if (childEnd)
        ts << "</child>" << endl;
    else
        ts << "</" << name << ">" << endl;
}

void MosaicWriter::setExtendedStar(QTextStream & ts, QString name, MotifPtr motif)
{
    ExtStarPtr sp = std::dynamic_pointer_cast<ExtendedStar>(motif);
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

    qreal      d = sp->getD();
    int        s = sp->getS();

    auto & extender = sp->getExtender();
    QString  ext_t     = (extender.getExtendPeripheralVertices())    ? "\"t\"" : "\"f\"";
    QString  ext_not_t = (extender.getExtendFreeVertices()) ? "\"t\"" : "\"f\"";
    QString  con_bnd_v = (extender.getConnectBoundaryVertices()) ? "\"t\"" : "\"f\"";

    ts << "<" << name << qsid << "  extendPeripherals=" << ext_t << "  extendFreeVertices=" << ext_not_t << "  connectBoundaryVertices=" << con_bnd_v << ">" << endl;

    setMotifCommon(ts, motif);

    ts << "<d>" << d << "</d>" << endl;
    ts << "<s>" << s << "</s>" << endl;

    ts << "</" << name << ">" << endl;
}

void MosaicWriter::setRosette(QTextStream & ts, QString name, MotifPtr motif, bool childEnd)
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
        qsid = nextId();
        setRosetteReference(getRef(),rp);
    }
    ts << "<" << name << qsid << ">" << endl;

    setMotifCommon(ts,motif);

    int s       = rp->getS();
    qreal q     = rp->getQ();
    qreal k     = rp->getK();

    ts << "<q>" << q << "</q>" << endl;
    ts << "<s>" << s << "</s>" << endl;
    ts << "<k>" << k << "</k>" << endl;

    if (childEnd)
        ts << "</child>" << endl;
    else
        ts << "</" << name << ">" << endl;
}

void MosaicWriter::setExtendedRosette(QTextStream & ts, QString name, MotifPtr motif)
{
    ExtRosettePtr rp = std::dynamic_pointer_cast<ExtendedRosette>(motif);
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

    qreal      q = rp->getQ();
    qreal      k = rp->getK();
    int        s = rp->getS();

    auto & extender = rp->getExtender();
    QString  ext_t     = (extender.getExtendPeripheralVertices())    ? "\"t\"" : "\"f\"";
    QString  ext_not_t = (extender.getExtendFreeVertices()) ? "\"t\"" : "\"f\"";
    QString  con_bnd_v = (extender.getConnectBoundaryVertices()) ? "\"t\"" : "\"f\"";

    ts << "<" << name << qsid << "  extendPeripherals=" << ext_t << "  extendFreeVertices=" << ext_not_t << "  connectBoundaryVertices=" << con_bnd_v << ">" << endl;

    setMotifCommon(ts,motif);

    ts << "<q>" << q << "</q>" << endl;
    ts << "<s>" << s << "</s>" << endl;
    ts << "<k>" << k << "</k>" << endl;

    ts << "</" << name << ">" << endl;
}

void MosaicWriter::setRosetteConnect(QTextStream & ts, QString name, MotifPtr motif)
{
    qDebug() << motif->getMotifDesc();
    RosetteConnectPtr rcp = std::dynamic_pointer_cast<RosetteConnect>(motif);
    if (!rcp)
    {
        fail("Style error","dynamic cast of RosetteConnect");
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

    setRosette(ts,QString("child class=\"app.Rosette\""),rcp,true);

    qreal s2 = rcp->getMotifScale();
    ts << "<s>" << s2 << "</s>" << endl;
    ts << "</" << name << ">" << endl;
}

void MosaicWriter::setStarConnect(QTextStream & ts, QString name, MotifPtr motif)
{
    qDebug() << motif->getMotifDesc();
    StarConnectPtr scp = std::dynamic_pointer_cast<StarConnect>(motif);
    if (!scp)
    {
        fail("Style error","dynamic cast StarConnect");
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

    setStar(ts,QString("child class=\"app.Star\""),scp,true);

    qreal s2 = scp->getMotifScale();
    ts << "<s>" << s2 << "</s>" << endl;
    ts << "</" << name << ">" << endl;
}

bool MosaicWriter::setMap(QTextStream &ts, MapPtr map)
{
    qDebug().noquote() << "Writing map" << map->namedSummary();

    bool rv = map->verifyAndFix(true,true);
    if (!rv)
        return false;

    qDebug().noquote() << "Writing map" << map->namedSummary();

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
        else if (ep->getType() == EDGETYPE_CHORD)
        {
            QString str = QString("<Chord convex=\"%1\">").arg(ep->isConvex() ? "t" : "f");
            ts << str << endl;
            QPointF p3 = ep->getArcCenter();
            setVertexEP(ts,v1,"Point");
            setVertexEP(ts,v2,"Point");
            setPoint(ts,p3,"Center");
            ts << "</Chord>" << endl;
        }
    }
}

void MosaicWriter::setVertexEP(QTextStream & ts,VertexPtr v, QString name)
{
    QString qsid;
    if (MosaicWriterBase::hasReference(v))
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
    if (MosaicWriterBase::hasReference(v))
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

    auto type = e->getType();

    qsid = nextId();
    qDebug() << "new edge ref=" << getRef();
    setEdgeReference(getRef(),e);

    if (type == EDGETYPE_CURVE)
    {
        QString str = QString("<curve %1 convex=\"%2\">").arg(qsid).arg(e->isConvex() ? "t" : "f");
        ts << str << endl;
    }
    else if (type == EDGETYPE_CHORD)
    {
        QString str = QString("<chord %1 convex=\"%2\">").arg(qsid).arg(e->isConvex() ? "t" : "f");
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

    if (type == EDGETYPE_CURVE || type == EDGETYPE_CHORD)
    {
        QPointF p = e->getArcCenter();
        setPos(ts,p);
    }

    if (type == EDGETYPE_CURVE)
    {
        ts << "</curve>" << endl;
    }
    else if (type == EDGETYPE_CHORD)
    {
        ts << "</chord>" << endl;
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
