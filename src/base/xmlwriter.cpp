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

#include "xmlwriter.h"
#include "base/canvas.h"
#include "base/border.h"
#include "base/fileservices.h"
#include "base/configuration.h"
#include "base/workspace.h"
#include "panels/panel.h"
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
#include "tapp/ExtendedRosette.h"
#include "tapp/RosetteConnectFigure.h"
#include "tapp/ExplicitFigure.h"
#include "tile/featurewriter.h"

const int currentXMLVersion = 2;

XmlWriter::XmlWriter(StyledDesign & styledDesign) : design(styledDesign)
{
    config = Configuration::getInstance();
}

XmlWriter::~XmlWriter()
{
}

bool XmlWriter::writeXML(QString fileName)
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

bool XmlWriter::generateDesignNotes(QTextStream & ts)
{
    if (!design.getNotes().isEmpty())
    {
        ts << "<designNotes>" << design.getNotes() << "</designNotes>" << endl;
    }
    return true;
}

bool XmlWriter::generateVector(QTextStream & ts)
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

bool XmlWriter::processVector(QTextStream &ts)
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
    const StyleSet & sset = design.getStyleSet();
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

void XmlWriter::processDesign(QTextStream &ts)
{
    CanvasSettings info= design.getCanvasSettings();
    qreal scale        = info.getScale();
    QColor back        = info.getBackgroundColor();
    QSizeF size        = info.getSizeF();
    BorderPtr border   = info.getBorder();

    ts << "<design>" << endl;

    procScale(ts,scale);
    procSize(ts,size);
    procBackground(ts,back);
    procBorder(ts,border);

    ts << "</design>" << endl;
}

void XmlWriter::procScale(QTextStream &ts,qreal scale)
{
    ts << "<scale>" << scale << "</scale>" << endl;
}

void XmlWriter::procWidth(QTextStream &ts,qreal width)
{
    ts << "<width>" << width << "</width>" << endl;
}

void XmlWriter::procSize(QTextStream &ts,QSizeF size)
{
    ts << "<size>"   << endl;
    ts << "<width>"  << size.width()  << "</width>" << endl;
    ts << "<height>" << size.height() << "</height>" << endl;
    ts << "</size>"  << endl;
}

void XmlWriter::procBackground(QTextStream &ts,QColor color)
{
    ts << "<background>" << endl;
    procColor(ts,color);
    ts << "</background>" << endl;
}

void XmlWriter::procColor(QTextStream & ts, QColor color)
{
    ts << "<color>";
    ts << color.name(QColor::HexArgb);
    ts << "</color>" << endl;
}

void XmlWriter::procColor(QTextStream & ts, TPColor tpcolor)
{
    QString qs = QString("<color hide=\"%1\">").arg(tpcolor.hidden ? 't' : 'f');
    ts << qs << tpcolor.color.name(QColor::HexArgb) << "</color>" << endl;
}

void XmlWriter::procBorder(QTextStream &ts,BorderPtr border)
{
    if (!border)
    {
        return;
    }

    eBorderType type = border->getType();
    QString txt = QString("<border type=\"%1\">").arg(type);
    ts << txt << endl;

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
    }

    ts << "</border>" << endl;
}

bool XmlWriter::processThick(QTextStream &ts, StylePtr s)
{
    Thick * th = dynamic_cast<Thick*>(s.get());
    if (th == nullptr)
    {
        fail("Style error","dynamic cast of Thick");
    }

    QColor  color           = th->getColor();
    bool    draw_outline    = th->getDrawOutline();
    qreal   width           = th->getLineWidth();
    PrototypePtr proto      = th->getPrototype();
    PolyPtr poly            = th->getBoundary();
    Xform   xf              = th->getDeltas();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    processStyleStyle(ts,proto,poly);
    ts << "</" << str << ">" << endl;

    str = "style.Colored";
    ts << "<" << str << ">" << endl;
    procColor(ts,color);
    ts << "</" << str << ">" << endl;

    str = "style.Thick";
    ts << "<" << str << ">" << endl;
    processsStyleThick(ts,draw_outline,width);
    ts << "</" << str << ">" << endl;

    qDebug() << "end thick";
    return true;
}

bool XmlWriter::processInterlace(QTextStream & ts, StylePtr s)
{
    Interlace * il = dynamic_cast<Interlace*>(s.get());
    if (il == nullptr)
    {
        fail("Style error","dynamic cast of Interlace");
    }

    QColor  color        = il->getColor();
    bool    draw_outline = il->getDrawOutline();
    bool    includeTipVerts= il->getIncludeTipVertices();
    qreal   width        = il->getLineWidth();
    qreal   gap          = il->getGap();
    qreal   shadow       = il->getShadow();
    PrototypePtr proto   = il->getPrototype();
    PolyPtr poly         = il->getBoundary();       // DAC - is this right
    Xform   xf           = il->getDeltas();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    processStyleStyle(ts,proto,poly);
    ts << "</" << str << ">" << endl;

    str = "style.Colored";
    ts << "<" << str << ">" << endl;
    procColor(ts,color);
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

bool XmlWriter::processOutline(QTextStream &ts, StylePtr s)
{
    Outline * ol = dynamic_cast<Outline*>(s.get());
    if (ol == nullptr)
    {
        fail("Style error","dynamic cast of Interlace");
    }

    QColor  color        = ol->getColor();
    bool    draw_outline = ol->getDrawOutline();
    qreal   width        = ol->getLineWidth();
    PrototypePtr proto   = ol->getPrototype();
    PolyPtr poly         = ol->getBoundary();
    Xform   xf           = ol->getDeltas();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    processStyleStyle(ts,proto,poly);
    ts << "</" << str << ">" << endl;

    str = "style.Colored";
    ts << "<" << str << ">" << endl;
    procColor(ts,color);
    ts << "</" << str << ">" << endl;

    str = "style.Thick";
    ts << "<" << str << ">" << endl;
    processsStyleThick(ts,draw_outline,width);
    ts << "</" << str << ">" << endl;

    qDebug() << "end outline";
    return true;
}

bool XmlWriter::processFilled(QTextStream &ts, StylePtr s)
{
    Filled * fl = dynamic_cast<Filled*>(s.get());
    if (fl == nullptr)
    {
        fail("Style error","dynamic cast of Filled");
    }

    int     algorithm       = fl->getAlgorithm();

    ColorSet & colorSetB    = fl->getBlackColorSet();
    ColorSet & colorSetW    = fl->getWhiteColorSet();
    ColorGroup & colorGroup = fl->getColorGroup();

    bool    draw_inside     = fl->getDrawInsideBlacks();
    bool    draw_outside    = fl->getDrawOutsideWhites();

    PrototypePtr proto      = fl->getPrototype();
    PolyPtr poly            = fl->getBoundary();
    Xform   xf              = fl->getDeltas();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    processStyleStyle(ts,proto,poly);
    ts << "</" << str << ">" << endl;

    if (colorSetB.size())
    {
        str = "ColorBlacks";
        ts << "<" << str << ">" << endl;
        procColorSet(ts,colorSetB);
        ts << "</" << str << ">" << endl;
    }

    if (colorSetW.size())
    {
        str = "ColorWhites";
        ts << "<" << str << ">" << endl;
        procColorSet(ts,colorSetW);
        ts << "</" << str << ">" << endl;
    }

    if (colorGroup.size())
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

bool XmlWriter::processPlain(QTextStream &ts, StylePtr s)
{
    Plain * pl = dynamic_cast<Plain*>(s.get());
    if (pl == nullptr)
    {
        fail("Style error","dynamic cast of Plain");
    }

    QColor  color       = pl->getColor();
    PrototypePtr proto  = pl->getPrototype();
    PolyPtr poly        = pl->getBoundary();
    Xform   xf          = pl->getDeltas();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    processStyleStyle(ts,proto,poly);
    ts << "</" << str << ">" << endl;

    str = "style.Colored";
    ts << "<" << str << ">" << endl;
    procColor(ts,color);
    ts << "</" << str << ">" << endl;

    qDebug() << "end plain";
    return true;
}

bool XmlWriter::processSketch(QTextStream &ts, StylePtr s)
{
    Sketch * sk = dynamic_cast<Sketch*>(s.get());
    if (sk == nullptr)
    {
        fail("Style error","dynamic cast of Sketch");
    }

    QColor  color       = sk->getColor();
    PrototypePtr proto  = sk->getPrototype();
    PolyPtr poly        = sk->getBoundary();
    Xform   xf          = sk->getDeltas();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    processStyleStyle(ts,proto,poly);
    ts << "</" << str << ">" << endl;

    str = "style.Colored";
    ts << "<" << str << ">" << endl;
    procColor(ts,color);
    ts << "</" << str << ">" << endl;

    return true;
}

bool XmlWriter::processEmboss(QTextStream &ts, StylePtr s)
{
    Emboss * em = dynamic_cast<Emboss*>(s.get());
    if (em == nullptr)
    {
        fail("Style error","dynamic cast of Emboss");
    }

    QColor  color        = em->getColor();
    bool    draw_outline = em->getDrawOutline();
    qreal   width        = em->getLineWidth();
    qreal   angle        = em->getAngle();
    PrototypePtr proto   = em->getPrototype();
    PolyPtr poly         = em->getBoundary();
    Xform   xf           = em->getDeltas();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    processStyleStyle(ts,proto,poly);
    ts << "</" << str << ">" << endl;

    str = "style.Colored";
    ts << "<" << str << ">" << endl;
    procColor(ts,color);
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

bool XmlWriter::processTileColors(QTextStream &ts, StylePtr s)
{
    TileColors * tc = dynamic_cast<TileColors*>(s.get());
    if (tc == nullptr)
    {
        fail("Style error","dynamic cast of Tile Colors");
    }

    PrototypePtr proto  = tc->getPrototype();
    PolyPtr poly        = tc->getBoundary();
    Xform   xf          = tc->getDeltas();

    QString str;

    str = "toolkit.GeoLayer";
    ts << "<" << str << ">" << endl;
    procesToolkitGeoLayer(ts,xf);
    ts << "</" << str << ">" << endl;

    str = "style.Style";
    ts << "<" << str << ">" << endl;
    processStyleStyle(ts,proto,poly);
    ts << "</" << str << ">" << endl;

    return true;
}

void XmlWriter::procesToolkitGeoLayer(QTextStream & ts, Xform & xf)
{
    ts << "<left__delta>"  << xf.translateX     << "</left__delta>"  << endl;
    ts << "<top__delta>"   << xf.translateY     << "</top__delta>"   << endl;
    ts << "<width__delta>" << (xf.scale - 1.0)  << "</width__delta>" << endl;
    ts << "<theta__delta>" << xf.rotation       << "</theta__delta>" << endl;
}

void XmlWriter::processStyleStyle(QTextStream & ts, PrototypePtr & proto, PolyPtr & poly)
{
    setBoundary(ts,poly);
    setPrototype(ts,proto);
}

void XmlWriter::procColorSet(QTextStream &ts, ColorSet & colorSet)
{
    int count = colorSet.size();
    colorSet.resetIndex();
    for (int i=0; i < count; i++)
    {
        TPColor tpcolor = colorSet.getNextColor();
        procColor(ts,tpcolor);
    }
}

void XmlWriter::procColorGroup(QTextStream &ts, ColorGroup & colorGroup)
{
    int count = colorGroup.size();
    colorGroup.resetIndex();
    for (int i=0; i < count; i++)
    {
        ColorSet & cs = colorGroup.getNextColorSet();
        bool hide = cs.isHidden();
        QString qs = QString("<Group hideSet=\"%1\">").arg( (hide) ? "t" : "f");
        ts << qs << endl;
        procColorSet(ts,cs);
        ts << "</Group>" << endl;
    }
}

void XmlWriter::processsStyleThick(QTextStream &ts, bool draw_outline, qreal width)
{
    QString draw = (draw_outline) ? "true" : "false";
    ts << "<draw__outline>" << draw << "</draw__outline>" << endl;
    ts << "<width>" << width << "</width>" << endl;
}

void XmlWriter::processsStyleInterlace(QTextStream &ts, qreal gap, qreal shadow, bool includeTipVerts)
{
    QString include = (includeTipVerts) ? "true" : "false";
    ts << "<gap>" << gap << "</gap>" << endl;
    ts << "<shadow>" << shadow << "</shadow>" << endl;
    ts << "<includeTipVerts>" << include << "</includeTipVerts>" << endl;
}

void XmlWriter::processsStyleFilled(QTextStream &ts, bool draw_inside, bool draw_outside, int algorithm)
{
    QString drawi = (draw_inside) ? "true" : "false";
    QString drawo = (draw_outside) ? "true" : "false";
    ts << "<draw__inside>" << drawi << "</draw__inside>" << endl;
    ts << "<draw__outside>" << drawo << "</draw__outside>" << endl;
    ts << "<algorithm>" << algorithm << "</algorithm>" << endl;
}

void XmlWriter::processsStyleEmboss(QTextStream &ts, qreal  angle)
{
    ts << "<angle>" << angle << "</angle>";
}

void XmlWriter::setBoundary(QTextStream & ts, PolyPtr p)
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

void XmlWriter::setPolygon(QTextStream & ts, PolyPtr pp)
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

void XmlWriter::setPrototype(QTextStream & ts, PrototypePtr pp)
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
        case FIG_TYPE_CONNECT_ROSETTE:
            name = "app.ConnectFigure";
            setRosetteConnectFigure(ts,name,figure);
            break;

        case FIG_TYPE_RADIAL:
        case FIG_TYPE_UNDEFINED:
            fail("Unexpected figure type:", sFigType[figType]);

        case FIG_TYPE_INFER:
            name = "app.Infer";
            setExplicitFigure(ts,name,figure);
            break;

        case FIG_TYPE_EXPLICIT_ROSETTE:
            name = "app.ExplicitRosette";
            setExplicitFigure(ts,name,figure);
            break;

        case FIG_TYPE_HOURGLASS:
            name = "app.ExplicitHourglass";
            setExplicitFigure(ts,name,figure);
            break;

        case FIG_TYPE_INTERSECT_PROGRESSIVE:
            name = "app.ExplicitIntersectProg";
            setExplicitFigure(ts,name,figure);
            break;

        case FIG_TYPE_INTERSECT:
            name = "app.ExplicitIntersect";
            setExplicitFigure(ts,name,figure);
            break;

        case FIG_TYPE_GIRIH:
            name = "app.ExplicitGirih";
            setExplicitFigure(ts,name,figure);
            break;

        case FIG_TYPE_EXPLICIT_STAR:
            name = "app.ExplicitStar";
            setExplicitFigure(ts,name,figure);
            break;

        case FIG_TYPE_FEATURE:
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

void XmlWriter::setFeature(QTextStream & ts,FeaturePtr fp)
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

    ts << "<edges" << nextId() << ">" << endl;

    EdgePoly & ep  = fp->getEdgePoly();

    FeatureWriter fw;
    fw.setEdgePoly(ts,ep);

    ts << "</edges>" << endl;

    ts << "</" << str << ">" << endl;

}

void XmlWriter::setFigureCommon(QTextStream & ts, FigurePtr fp)
{
    int       bs = fp->getExtBoundarySides();
    qreal    bsc = fp->getExtBoundaryScale();
    qreal    fsc = fp->getFigureScale();

    ts << "<boundarySides>" << bs << "</boundarySides>" << endl;
    ts << "<boundaryScale>" << bsc << "</boundaryScale>" << endl;
    ts << "<figureScale>" << fsc << "</figureScale>" << endl;
}

void XmlWriter::setExplicitFigure(QTextStream & ts,QString name, FigurePtr fp)
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

    _currentMap = ep->getFigureMap();
    setMap(ts,_currentMap);

    setFigureCommon(ts,fp);

    ts << "</" << name << ">" << endl;
}

void XmlWriter::setStarFigure(QTextStream & ts,QString name, FigurePtr fp)
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
    qreal      r = sp->getR();

    ts << "<n>" << n << "</n>" << endl;
    ts << "<d>" << d << "</d>" << endl;
    ts << "<s>" << s << "</s>" << endl;
    ts << "<r>" << r << "</r>" << endl;

    setFigureCommon(ts, fp);

    ts << "</" << name << ">" << endl;
}

void XmlWriter::setExtendedStarFigure(QTextStream & ts,QString name, FigurePtr fp)
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
    qreal      r = sp->getR();
    QString  ext_t     = (sp->getExtendPeripheralVertices())    ? "\"t\"" : "\"f\"";
    QString  ext_not_t = (sp->getExtendFreeVertices()) ? "\"t\"" : "\"f\"";

    ts << "<" << name << qsid << "  extendPeripherals=" << ext_t << "  extendFreeVertices=" << ext_not_t << ">" << endl;
    ts << "<n>" << n << "</n>" << endl;
    ts << "<d>" << d << "</d>" << endl;
    ts << "<s>" << s << "</s>" << endl;
    ts << "<r>" << r << "</r>" << endl;

    setFigureCommon(ts, fp);

    ts << "</" << name << ">" << endl;
}

void XmlWriter::setRosetteFigure(QTextStream & ts,QString name, FigurePtr fp, bool childEnd)
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
    qreal r     = rp->getR();

    ts << "<n>" << n << "</n>" << endl;
    ts << "<q>" << q << "</q>" << endl;
    ts << "<s>" << s << "</s>" << endl;
    ts << "<r>" << r << "</r>" << endl;

    setFigureCommon(ts,fp);

    if (childEnd)
        ts << "</child>" << endl;
    else
        ts << "</" << name << ">" << endl;
}

void XmlWriter::setExtendedRosetteFigure(QTextStream & ts,QString name, FigurePtr fp)
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
    qreal      r = rp->getR();
    QString  ext_t     = (rp->getExtendPeripheralVertices())    ? "\"t\"" : "\"f\"";
    QString  ext_not_t = (rp->getExtendFreeVertices()) ? "\"t\"" : "\"f\"";
    QString  con_bnd_v = (rp->getConnectBoundaryVertices()) ? "\"t\"" : "\"f\"";

    ts << "<" << name << qsid << "  extendPeripherals=" << ext_t << "  extendFreeVertices=" << ext_not_t << "  connectBoundaryVertices=" << con_bnd_v << ">" << endl;
    ts << "<n>" << n << "</n>" << endl;
    ts << "<q>" << q << "</q>" << endl;
    ts << "<k>" << k << "</k>" << endl;
    ts << "<s>" << s << "</s>" << endl;
    ts << "<r>" << r << "</r>" << endl;

    setFigureCommon(ts,fp);

    ts << "</" << name << ">" << endl;
}

void XmlWriter::setRosetteConnectFigure(QTextStream & ts,QString name, FigurePtr fp)
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

void XmlWriter::setMap(QTextStream &ts, MapPtr map)
{
    QString qsid;
    if (hasReference(map))
    {
        qsid = getMapReference(map);
        ts << "<map" << qsid << "/>" << endl;
        return;
    }

    qsid = nextId();
    setMapReference(getRef(),map);
    ts << "<map" << qsid << ">" << endl;

    // vertices
    const QVector<VertexPtr> * vertices = map->getVertices();
    setVertices(ts,vertices);

    // Edges
    const QVector<EdgePtr> * edges = map->getEdges();
    setEdges(ts,edges);

    ts << "</map>" << endl;
}


void XmlWriter::setVertices(QTextStream & ts, const QVector<VertexPtr> * vertices)
{
    ts << "<vertices" << nextId() <<  ">" << endl;
    for (auto it = vertices->begin(); it != vertices->end(); it++)
    {
        VertexPtr v = *it;
        setVertex(ts,v);
    }
    ts << "</vertices>" << endl;
}

void XmlWriter::setEdges(QTextStream & ts, const QVector<EdgePtr> * edges )
{
    ts << "<edges" << nextId() <<  ">" << endl;
    for (auto it = edges->begin(); it != edges->end(); it++)
    {
        EdgePtr e = *it;
        setEdge(ts,e,true);
    }
    ts << "</edges>" << endl;
}

void XmlWriter::setVertex(QTextStream & ts,VertexPtr v, QString name)
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
    setPos(ts,v->getPosition());

    // edges
    QVector<EdgePtr> & edges = v->getNeighbours();
    setEdges(ts,edges);

    ts << "</" << name << ">" << endl;
}

void XmlWriter::setEdges(QTextStream & ts, QVector<EdgePtr> & qvec)
{
    ts << "<edges2>" << endl;

    for (auto it = qvec.begin(); it != qvec.end(); it++)
    {
        // edge
        EdgePtr e = *it;
        setEdge(ts,e);      // called from setNeighbour first time
    }

    ts << "</edges2>" << endl;
 }


 void XmlWriter::setEdge(QTextStream & ts, EdgePtr e, bool capitalE)
{
    qDebug() << "edge" << e.get();
    QString qsid;
    if (hasReference(e))
    {
        qsid = getEdgeReference(e);

        if (capitalE)
            ts << "<Edge" << qsid << "/>" << endl;
        else
            ts << "<edge" << qsid << "/>" << endl;
        return;
    }


    bool curved = (e->getType() == EDGE_CURVE);

    qsid = nextId();
    qDebug() << "new edge ref=" << getRef();
    setEdgeReference(getRef(),e);
    if (capitalE)
    {
        ts << "<Edge" << qsid << ">" << endl;
    }
    else
    {
        if (curved)
        {
            QString str = QString("<curve %1 convex=\"%2\">").arg(qsid).arg(e->isConvex() ? "t" : "f");
            ts << str << endl;
        }
        else
        {
            ts << "<edge" << qsid << ">" << endl;
        }
    }

    // map
    setMap(ts,_currentMap);

    // v1
    VertexPtr v1 = e->getV1();
    setVertex(ts,v1,"v1");

    // v2
    VertexPtr v2 = e->getV2();
    setVertex(ts,v2,"v2");

    if (curved)
    {
        QPointF p = e->getArcCenter();
        setPos(ts,p);
    }

    if (capitalE)
    {
        ts << "</Edge>" << endl;
    }
    else
    {
        if (curved)
        {
            ts << "</curve>" << endl;
        }
        else
        {
            ts << "</edge>" << endl;
        }
    }
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

bool XmlWriter::hasReference(PolyPtr pp)
{
    return poly_ids.contains(pp);
}

bool XmlWriter::hasReference(PrototypePtr pp)
{
    return proto_ids.contains(pp);
}

bool XmlWriter::hasReference(FeaturePtr fp)
{
    return feature_ids.contains(fp);
}

bool XmlWriter::hasReference(FigurePtr fp)
{
    return figure_ids.contains(fp);
}

bool XmlWriter::hasReference(ExplicitPtr ep)
{
    return explicit_ids.contains(ep);
}

bool XmlWriter::hasReference(MapPtr map)
{
    return map_ids.contains(map);
}

bool XmlWriter::hasReference(VertexPtr v)
{
    return vertex_ids.contains(v);
}

bool XmlWriter::hasReference(NeighbourPtr n)
{
    return neighbour_ids.contains(n);
}

bool XmlWriter::hasReference(RosettePtr n)
{
    return rosette_ids.contains(n);
}

bool XmlWriter::hasReference(StarPtr n)
{
    return star_ids.contains(n);
}

bool XmlWriter::hasReference(ExtStarPtr n)
{
    return extended_star_ids.contains(n);
}

bool XmlWriter::hasReference(ExtRosettePtr n)
{
    return extended_rosette_ids.contains(n);
}

bool XmlWriter::hasReference(RosetteConnectPtr n)
{
    return rosette_connect_ids.contains(n);
}

bool XmlWriter::hasReference(EdgePtr e)
{
    return edge_ids.contains(e);
}

void XmlWriter::setProtoReference(int id, PrototypePtr ptr)
{
    proto_ids[ptr] = id;
}

void XmlWriter::setPolyReference(int id, PolyPtr ptr)
{
    poly_ids[ptr] = id;
}

void XmlWriter::setFigureReference(int id, FigurePtr ptr)
{
   figure_ids[ptr] = id;
}

void XmlWriter::setFeatureReference(int id,FeaturePtr ptr)
{
    feature_ids[ptr] = id;
}

void XmlWriter::setExplicitReference(int id,ExplicitPtr ptr)
{
    explicit_ids[ptr] = id;
}

void XmlWriter::setMapReference(int id, MapPtr ptr)
{
    map_ids[ptr] = id;
}

void XmlWriter::setVertexReference(int id, VertexPtr ptr)
{
    vertex_ids[ptr] = id;
}

void XmlWriter::setNeighbourReference(int id, NeighbourPtr ptr)
{
    neighbour_ids[ptr] = id;
}

void XmlWriter::setRosetteReference(int id, RosettePtr ptr)
{
    rosette_ids[ptr] = id;
}

void XmlWriter::setStarReference(int id, StarPtr ptr)
{
    star_ids[ptr] = id;
}

void XmlWriter::setExtendedStarReference(int id, ExtStarPtr ptr)
{
    extended_star_ids[ptr] = id;
}

void XmlWriter::setExtendedRosetteReference(int id, ExtRosettePtr ptr)
{
    extended_rosette_ids[ptr] = id;
}

void XmlWriter::setRosetteConnectReference(int id, RosetteConnectPtr ptr)
{
    rosette_connect_ids[ptr] = id;
}

void XmlWriter::setEdgeReference(int id, EdgePtr ptr)
{
    edge_ids[ptr] = id;
}

QString XmlWriter::getPolyReference(PolyPtr ptr)
{
    int id =  poly_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString XmlWriter::getProtoReference(PrototypePtr ptr)
{
    int id =  proto_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString XmlWriter::getFeatureReference(FeaturePtr ptr)
{
    int id =  feature_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString XmlWriter::getFigureReference(FigurePtr ptr)
{
    int id =  figure_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString XmlWriter::getExplicitReference(ExplicitPtr ptr)
{
    int id =  explicit_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString XmlWriter::getMapReference(MapPtr ptr)
{
    int id =  map_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString XmlWriter::getVertexReference(VertexPtr ptr)
{
    int id =  vertex_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString XmlWriter::getNeighbourReference(NeighbourPtr ptr)
{
    int id =  neighbour_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString XmlWriter::getRosetteReference(RosettePtr ptr)
{
    int id =  rosette_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString XmlWriter::getStarReference(StarPtr ptr)
{
    int id =  star_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString XmlWriter::getExtendedStarReference(ExtStarPtr ptr)
{
    int id =  extended_star_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString XmlWriter::getExtendedRosetteReference(ExtRosettePtr ptr)
{
    int id =  extended_rosette_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString XmlWriter::getRosetteConnectReference(RosetteConnectPtr ptr)
{
    int id =  rosette_connect_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString XmlWriter::getEdgeReference(EdgePtr ptr)
{
    int id =  edge_ids.value(ptr);
    qDebug() << "edge ref" << id;
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString  XmlWriter::id(int id)
{
    qDebug() << "id=" << id;
    QString qs = QString(" id=\"%1\"").arg(id);
    return qs;
}

QString  XmlWriter::nextId()
{
    return id(++refId);
}

void XmlWriter::setTransform(QTextStream & ts,Transform T)
{
    QVector<qreal> vals = T.get();
    Q_ASSERT (vals.size() >= 6);
    ts << "<Tr" << nextId() << ">" << endl;
    ts << "<a>" << vals[0] << "</a>" << endl;
    ts << "<b>" << vals[1] << "</b>" << endl;
    ts << "<c>" << vals[2] << "</c>" << endl;
    ts << "<d>" << vals[3] << "</d>" << endl;
    ts << "<e>" << vals[4] << "</e>" << endl;
    ts << "<f>" << vals[5] << "</f>" << endl;
    ts << "</Tr>" << endl;
}

void XmlWriter::setPos(QTextStream & ts,QPointF qpf)
{
    ts << "<pos>";
    ts << QString::number(qpf.x(),'g',16) << "," << QString::number(qpf.y(),'g',16);
    ts << "</pos>" << endl;
}

void XmlWriter::fail(QString a, QString b)
{
    _failMsg = QString("%1 %2").arg(a).arg(b);
    qWarning().noquote() << _failMsg;
    throw(_failMsg);
}
