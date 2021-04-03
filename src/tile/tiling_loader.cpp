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

#include "tile/tiling.h"
#include "tile/tiling_loader.h"
#include "tile/feature_reader.h"
#include "tile/backgroundimage.h"
#include "base/mosaic_loader.h"
#include "makers/tiling_maker/tiling_maker.h"

using namespace pugi;
using std::string;

// Tiling I/O.
//
// Functions for reading tilings in from files.  No real error-checking
// is done, because I don't expect you to write these files by hand --
// they should be auto-generated from DesignerPanel.
//

TilingPtr TilingLoader::readTiling(QString file )
{
    TilingPtr t;
    QFile data(file);
    if (data.open(QFile::ReadOnly))
    {
        QTextStream str(&data);
        t = readTiling(str);
        data.close();
    }
    else
    {
        qFatal("Could not read tile file");
    }
    return t;
}

// This parses a taprats (non-XML) tiling
TilingPtr TilingLoader::readTiling(QTextStream & st)
{
    st.skipWhiteSpace();

    QString astring;
    astring = st.readLine();
    QStringList qsl;
    qsl = astring.split('"');

    if (qsl.count() != 3)
        qFatal("First line indecipherable");
    if( !qsl[0].contains("tiling"))
        qFatal("This isn't a tiling file.");

    QString name = qsl[1];

    int nf = qsl[2].toInt();    // number of features

    st.skipWhiteSpace();
    qreal x1,y1,x2,y2;
    st >> x1;
    st >> y1;
    st >> x2;
    st >> y2;

    // create the tiling
    tiling = make_shared<Tiling>(name,QPointF(x1,y1),QPointF(x2,y2));

    for( int idx = 0; idx < nf; ++idx )
    {
        FeaturePtr bf;

        st >> astring;
        bool reg = false;
        if( astring == "regular")
        {
            reg = true;
        }

        int num_sides;
        int num_xforms;
        st >> num_sides;
        st >> num_xforms;

        if (reg)
        {
            bf = make_shared<Feature>(num_sides,0);
        }
        else
        {
            QPolygonF pts;
            for( int v = 0; v < num_sides; ++v )
            {
                st >> x1;
                st >> y1;
                pts << QPointF(x1, y1);
            }
            EdgePoly ep(pts);
            bf = make_shared<Feature>(ep,0);
        }

        for( int jdx = 0; jdx < num_xforms; ++jdx )
        {
            qreal a,b,c,d,e,f;
            st >> a;
            st >> b;
            st >> c;
            st >> d;
            st >> e;
            st >> f;
            QTransform t(a,d,
                         b,e,
                         c,f);
            PlacedFeaturePtr pfp = make_shared<PlacedFeature>(bf,t);
            tiling->add(pfp);
        }
    }

    st.skipWhiteSpace();
    tiling->setDescription(readQuotedString(st));

    st.skipWhiteSpace();
    tiling->setAuthor(readQuotedString(st));

    return tiling;
}

TilingPtr TilingLoader::readTilingXML(QString file )
{
    xml_document doc;
    xml_parse_result result = doc.load_file(file.toStdString().c_str());
    if (result == false)
    {
        qWarning("Badly constructed XML file");
        return nullptr;
    }

    xml_node tiling_node = doc.first_child();
    TilingPtr tp = readTilingXML(tiling_node);
    if (tp->hasOverlaps())
        qInfo() << tiling->getName() << "HAS OVERLAPS";
    return tp;
}

TilingPtr TilingLoader::readTilingXML(xml_node & tiling_node)
{
    string str = tiling_node.name();
    if (str != "Tiling")
    {
        qDebug() << "XML Error: node is:" << tiling_node.name();
        qWarning("<Tiling> not found");
        return nullptr;
    }

    int version = 0;
    xml_attribute vatt = tiling_node.attribute("version");
    if (vatt)
    {
        QString ver = vatt.value();
        version = ver.toInt();
    }

    // name
    xml_node node;
    QString name;
    node = tiling_node.child("Name");
    if (node)
    {
        name = node.child_value();
    }
    else
    {
        qWarning("XML error: name not found");
        return nullptr;
    }

    // translations
    QPointF ptt1,ptt2;
    xml_node t1 = tiling_node.child("T1");
    if (t1)
    {
        QString strt1 = t1.child_value();
        ptt1 = getPoint(strt1);
    }
    xml_node t2 = tiling_node.child("T2");
    if (t2)
    {
        QString strt2 = t2.child_value();
        ptt2 = getPoint(strt2);
    }

    // create the tiling
    tiling = make_shared<Tiling>(name, ptt1, ptt2);
    tiling->setVersion(version);

    // view settings
    getViewSettings(tiling_node);

    // author
    node = tiling_node.child("Auth");
    if (node)
    {
        QString author = node.child_value();
        tiling->setAuthor(author);
    }

    // description
    node = tiling_node.child("Desc");
    if (node)
    {
        QString description = node.child_value();
        tiling->setDescription(description);
    }

    // fills - not part of taprats and not necessarily found
    // 26JUL2020 fillData moved to Mosaic
    // 13SEP2020 fillData can be in both Mosaic and tiling
    xml_node t0 = tiling_node.child("Fill");
    if (t0)
    {
        QString strt0 = t0.child_value();
        FillData fd = getFill(strt0);
        tiling->setFillData(fd);
    }

    FeatureReader fr;   // must be located here to bw used for all features

    for (xml_node feature = tiling_node.child("Feature"); feature; feature = feature.next_sibling("Feature"))
    {
        FeaturePtr bf;

        xml_attribute fatt = feature.attribute("type");
        if (!fatt)
        {
            qWarning("missing type attribute");
            continue;
        }

        QString strtype = fatt.as_string();

        if (strtype == "girih")
        {
            xml_attribute nameattr = feature.attribute("name");
            if (!nameattr)
            {
                qWarning("missing name attribute for girih shape");
                continue;
            }
            QString name = nameattr.value();
            PlacedFeaturePtr pfp = make_shared<PlacedFeature>();
            if (pfp->loadFromGirihShape(name))
            {
                bf = pfp->getFeature();
            }
        }
        else if (strtype == "regular")
        {
            int sides      = 0;
            xml_attribute sidesatt = feature.attribute("sides");
            if (!sidesatt)
            {
                qWarning("missing sides attribute");
                continue;
            }
            sides = sidesatt.as_int();

            qreal rotation = 0.0;
            xml_attribute rotatt = feature.attribute("rotation");
            if (rotatt)
            {
                rotation = rotatt.as_double();
            }

            qreal scale = 1.0;
            xml_attribute scaleatt = feature.attribute("scale");
            if (scaleatt)
            {
                scale = scaleatt.as_double();
            }

            bf = make_shared<Feature>(sides,rotation,scale);
        }
        else if (strtype == "polygon")
        {
            // older tilings have this, newer tilings use edgepoly
            QPolygonF pts;
            for (xml_node pnode = feature.child("Point"); pnode; pnode = pnode.next_sibling("Point"))
            {
                string txt = pnode.child_value();
                QPointF pt = getPoint(txt.c_str());
                pts << pt;
            }
            EdgePoly ep(pts);
            bf = make_shared<Feature>(ep);
        }
        else if (strtype == "edgepoly")
        {
            EdgePoly ep = fr.getEdgePoly(feature);

            qreal rotation = 0.0;
            xml_attribute rotatt = feature.attribute("rotation");
            if (rotatt)
            {
                rotation = rotatt.as_double();
            }

            qreal scale = 1.0;
            xml_attribute scaleatt = feature.attribute("scale");
            if (scaleatt)
            {
                scale = scaleatt.as_double();
            }

            bf = make_shared<Feature>(ep,rotation,scale);
        }

        for (xml_node plnode = feature.child("Placement"); plnode; plnode = plnode.next_sibling("Placement"))
        {
            QTransform T;
            if (version <= 5)
            {
                QString txt = plnode.child_value();
                T = getAffineTransform(txt);
            }
            else
            {
                Q_ASSERT(version >= 6);
                T = getAffineTransform(plnode);
            }

            PlacedFeaturePtr pfp = make_shared<PlacedFeature>(bf,T);
            tiling->add(pfp);
        }

        xml_node bcolor = feature.child("BkgdColors");
        if (bcolor)
        {
            QVector<TPColor> tpcolors;
            QString txt = bcolor.child_value();
            QStringList txtList = txt.split(',');
            for (auto it = txtList.begin(); it != txtList.end(); it++)
            {
                QString acolor = *it;
                tpcolors.push_back(TPColor(acolor,false));

            }
            ColorSet * bkgds = bf->getBkgdColors();
            bkgds->setColors(tpcolors);
        }
    }

    xml_node bkImage = tiling_node.child("BackgroundImage");
    if (bkImage)
    {
       BkgdImgPtr bip = getBackgroundImage(bkImage);
       if (bip && bip->isLoaded())
       {
           tiling->setBackground(bip);
       }
    }

    // tiling->dump();
    return tiling;
}

BkgdImgPtr TilingLoader::getBackgroundImage(xml_node & node)
{
    xml_attribute attr = node.attribute("name");
    QString name       = attr.value();

    BkgdImgPtr bi = make_shared<BackgroundImage>(name);
    if (bi->isLoaded())
    {
        Xform xf = bi->getCanvasXform();

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

        bi->updateBkgdXform(xf);  // does not set center

        n= node.child("Perspective");
        if (n)
        {
            QString str = n.child_value();
            bi->perspective = getQTransform(str);

            if (!bi->perspective.isIdentity())
            {
                bi->createAdjustedImage();
            }
        }

        bi->createPixmap();
    }
    else
    {
        bi.reset();
    }

    return bi;
}

Xform  TilingLoader::getXform(xml_node & node)
{
    Xform xf;
    QString str;

    str = node.child_value("Scale");
    xf.setScale(str.toDouble());

    str = node.child_value("Rot");
    xf.setRotateRadians(str.toDouble());

    str = node.child_value("X");
    xf.setTranslateX(str.toDouble());

    str = node.child_value("Y");
    xf.setTranslateY(str.toDouble());

    str = node.child_value("Center");
    xf.setCenter(getPoint(str));

    return  xf;
}

FillData TilingLoader::getFill(QString txt)
{
    FillData fd;
    QStringList qsl;
    qsl = txt.split(',');
    fd.set(qsl[0].toInt(),qsl[1].toInt(),qsl[2].toInt(),qsl[3].toInt());
    return fd;
}

QString TilingLoader::readQuotedString(QTextStream &str)
{
    QString astring;
    QString bstring;

    //look for start
    do
    {
        str >> astring;
    } while (astring.at(0) != '"');

    if (astring.count('"') > 1)
    {
        return astring;
    }

    // get others
    do
    {
        str >> bstring;
        astring += " ";
        astring += bstring;
    }
    while (!bstring.contains('"'));

    astring.remove('"');

    return astring;
}

QPointF TilingLoader::getPoint(QString txt)
{
    QStringList qsl;
    qsl = txt.split(',');
    qreal a = qsl[0].toDouble();
    qreal b = qsl[1].toDouble();
    return QPointF(a,b);
}

QTransform TilingLoader::getAffineTransform(QString txt)
{
    QStringList qsl;
    qsl = txt.split(',');
    qreal a = qsl[0].toDouble();
    qreal b = qsl[1].toDouble();
    qreal c = qsl[2].toDouble();
    qreal d = qsl[3].toDouble();
    qreal e = qsl[4].toDouble();
    qreal f = qsl[5].toDouble();
    return QTransform(a,d,
                      b,e,
                      c,f);
}

QTransform TilingLoader::getAffineTransform(xml_node & node)
{
    xml_node  n = node.child("scale");
    QString   s = n.child_value();
    qreal scale = s.toDouble();

    n           = node.child("rot");
    s           = n.child_value();
    qreal rot   = s.toDouble();

    n           = node.child("tranX");
    s           = n.child_value();
    qreal x     = s.toDouble();

    n           = node.child("tranY");
    s           = n.child_value();
    qreal y     = s.toDouble();

    QTransform t;
    t.translate(x,y);
    t.rotate(rot);
    t.scale(scale,scale);
    return t;
}

QTransform TilingLoader::getQTransform(QString txt)
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

void TilingLoader::getViewSettings(xml_node & node)
{
    xml_node view = node.child("ViewSettings");
    if (!view)
    {
        return;
    }

    int width  = 1500;   // default
    int height = 1100;   // default
    xml_node n = view.child("width");
    if (n)
    {
        QString str = n.child_value();
        width = str.toInt();
    }

    n = view.child("height");
    if (n)
    {
        QString str = n.child_value();
        height = str.toInt();
    }

    tiling->setSize(QSize(width,height));

    Xform xf;
    QString val;
    qreal   fval;

    n = view.child("left__delta");
    if (n)
    {
        val           = n.child_value();
        fval          = val.toDouble();
        xf.setTranslateX(fval);
    }

    n = view.child("top__delta");
    if (n)
    {
        val           = n.child_value();
        fval          = val.toDouble();
        xf.setTranslateY(fval);
    }

    n = view.child("width__delta");
    if (n)
    {
        val          = n.child_value();
        fval         = val.toDouble();
        xf.setScale(fval);
    }

    n = view.child("theta__delta");
    if (n)
    {
        val          = n.child_value();
        fval         = val.toDouble();
        xf.setRotateRadians(fval);
    }

    n = view.child("center");
    if (n)
    {
        val          = n.child_value();
        QStringList qsl = val.split(",");
        qreal x = qsl[0].toDouble();
        qreal y = qsl[1].toDouble();
        xf.setCenter(QPointF(x,y));
    }

    tiling->setCanvasXform(xf);
}

