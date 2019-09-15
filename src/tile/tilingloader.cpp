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

#include "tile/tilingloader.h"
#include "tile/Tiling.h"
#include "tile/featurereader.h"

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

    beginTiling(qsl[1]);

    int nf = qsl[2].toInt();    // number of features

    st.skipWhiteSpace();
    qreal x1,y1,x2,y2;
    st >> x1;
    st >> y1;
    st >> x2;
    st >> y2;

    setTranslations(QPointF( x1, y1 ), QPointF( x2, y2 ) );

    for( int idx = 0; idx < nf; ++idx )
    {
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

        if( reg )
        {
            beginRegularFeature( num_sides );
        }
        else
        {
            beginPolygonFeature( num_sides );
            for( int v = 0; v < num_sides; ++v )
            {
                st >> x1;
                st >> y1;
                addPoint(QPointF(x1, y1));
            }
            commitPolygonFeature();
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
            Transform t = Transform(a, b, c, d, e, f);
            addPlacement(t.getQTransform());
        }
        endFeature();
    }

    st.skipWhiteSpace();
    b_setDescription(readQuotedString(st));

    st.skipWhiteSpace();
    b_setAuthor(readQuotedString(st));

    return endTiling();
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

    version = 0;
    xml_attribute vatt = tiling_node.attribute("version");
    if (vatt)
    {
        QString ver = vatt.value();
        version = ver.toInt();
    }

    // name
    xml_node node;
    node = tiling_node.child("Name");
    if (node)
    {
        string strname = node.child_value();
        beginTiling(strname.c_str());
    }
    else
    {
        qWarning("XML error: name not found");
        return nullptr;
    }

    // author
    node = tiling_node.child("Auth");
    if (node)
    {
        string strname = node.child_value();
        b_setAuthor(strname.c_str());
    }

    // description
    node = tiling_node.child("Desc");
    if (node)
    {
        string strname = node.child_value();
        b_setDescription(strname.c_str());
    }

    // fills - not part of taprats and not necessarily found
    xml_node t0 = tiling_node.child("Fill");
    if (t0)
    {
        string strt0 = t0.child_value();
        getFill(strt0.c_str());
    }

    // translations
    QPointF ptt1,ptt2;
    xml_node t1 = tiling_node.child("T1");
    if (t1)
    {
        string strt1 = t1.child_value();
        ptt1 = getPoint(strt1.c_str());
    }
    xml_node t2 = tiling_node.child("T2");
    if (t2)
    {
        string strt2 = t2.child_value();
        ptt2 = getPoint(strt2.c_str());
    }
    setTranslations(ptt1,ptt2);

   FeatureReader fr;
   for (xml_node feature = tiling_node.child("Feature"); feature; feature = feature.next_sibling("Feature"))
   {
       xml_attribute fatt = feature.attribute("type");
       if (!fatt)
       {
           qWarning("missing type attribute");
           continue;
       }
       string strtype = fatt.as_string();

       int sides = 0;
       if (strtype == "girih")
       {
           xml_attribute nameattr = feature.attribute("name");
           if (!nameattr)
           {
               qWarning("missing name attribute for girih shape");
               continue;
           }
           QString name = nameattr.value();
           commitGirihShapeFeature(name);
       }
       else if (strtype == "regular")
       {
           xml_attribute sidesatt = feature.attribute("sides");
           if (!sidesatt)
           {
               qWarning("missing sides attribute");
              continue;
           }
           sides = sidesatt.as_int();
           beginRegularFeature(sides);
       }
       else if (strtype == "polygon")
       {
            beginPolygonFeature(0);
            for (xml_node pnode = feature.child("Point"); pnode; pnode = pnode.next_sibling("Point"))
            {
                string txt = pnode.child_value();
                QPointF pt = getPoint(txt.c_str());
                addPoint(pt);
            }
            commitPolygonFeature();
       }
       else if (strtype == "edgepoly")
       {
           FeatureReader fr;
           EdgePoly ep = fr.getEdgePoly(feature);
           commitEdgePolyFeature(ep);
       }

       for (xml_node plnode = feature.child("Placement"); plnode; plnode = plnode.next_sibling("Placement"))
       {
           string txt    = plnode.child_value();
           Transform   T = getTransform(txt.c_str());
           addPlacement(T.getQTransform());
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
           addColors(tpcolors);
        }

       endFeature();
   }

   xml_node bkImage = tiling_node.child("BackgroundImage");
   if (bkImage)
   {
       xml_attribute attr = bkImage.attribute("name");
       if (attr)
       {
           QString str;
           b_bkName = attr.value();

           xml_node n = bkImage.child("Scale");
           if (n)
           {
                str= n.child_value();
                b_bkScale = str.toDouble();
           }

           n = bkImage.child("Rot");
           if (n)
           {
                str= n.child_value();
                b_bkRot = str.toDouble();
           }

           n = bkImage.child("X");
           if (n)
           {
                str= n.child_value();
                b_bkX = str.toDouble();
           }

           n = bkImage.child("Y");
           if (n)
           {
                str= n.child_value();
                b_bkY = str.toDouble();
           }

           n= bkImage.child("Perspective");
           if (n)
           {
               str = n.child_value();
               b_bkgdAdjust = getQTransform(str);
           }
        }
    }

   return endTiling();
}

void TilingLoader::beginTiling( QString name )
{
    b_name = name;

    b_desc.clear();
    b_author.clear();
    b_pfs.clear();
    b_pts.clear();
    b_t1 = QPointF();
    b_t2 = QPointF();
    b_f  = nullptr;
}

TilingPtr TilingLoader::endTiling()
{
    TilingPtr tiling = make_shared<Tiling>(b_name, b_t1, b_t2);
    for( int idx = 0; idx < b_pfs.size(); ++idx )
    {
        tiling->add( b_pfs[idx]);
    }

    tiling->setDescription( b_desc );
    tiling->setAuthor( b_author );
    tiling->setFillData(b_fillData);

    if (!b_bkName.isEmpty())
    {
        BackgroundImage & bi = tiling->getBackground();
        bi.bkgdName     = b_bkName;
        bi.scale        = b_bkScale;
        bi.rot          = b_bkRot;
        bi.x            = b_bkX;
        bi.y            = b_bkY;
        bi.perspective  = b_bkgdAdjust;
    }

    //tiling->dump();
    return tiling;
}

void TilingLoader::setTranslations( QPointF t1, QPointF t2 )
{
    b_t1 = t1;
    b_t2 = t2;
}

void TilingLoader::setFillData(FillData fd)
{
    b_fillData = fd;
}

void TilingLoader::beginPolygonFeature( int n )
{
    Q_UNUSED(n);
    b_pts.clear();
}

void TilingLoader::addPoint( QPointF pt )
{
    b_pts << pt;
}

void TilingLoader::commitPolygonFeature()
{
    EdgePoly ep(b_pts);
    b_f = make_shared<Feature>(ep);
}

void TilingLoader::commitEdgePolyFeature(EdgePoly &ep)
{
    b_f = make_shared<Feature>(ep);
}

void TilingLoader::commitGirihShapeFeature(QString name)
{
    PlacedFeaturePtr pfp = make_shared<PlacedFeature>();
    pfp->loadFromGirihShape(name);
    b_f = pfp->getFeature();
}

void TilingLoader::addPlacement( QTransform T )
{
    b_pfs.push_back(make_shared<PlacedFeature>(b_f, T));
}

void TilingLoader::beginRegularFeature(int n)
{
    b_f = make_shared<Feature>(n);
}

void TilingLoader::b_setDescription( QString t )
{
    b_desc = t;
}

void TilingLoader::b_setAuthor( QString t )
{
    b_author = t;
}

void TilingLoader::addColors(QVector<TPColor> & colors)
{
    ColorSet & bkgds = b_f->getBkgdColors();
    bkgds.setColors(colors);
}

void TilingLoader::getFill(QString txt)
{
    QStringList qsl;
    qsl = txt.split(',');
    b_fillData.set(qsl[0].toInt(),qsl[1].toInt(),qsl[2].toInt(),qsl[3].toInt());
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

Transform TilingLoader::getTransform(QString txt)
{
    QStringList qsl;
    qsl = txt.split(',');
    qreal a = qsl[0].toDouble();
    qreal b = qsl[1].toDouble();
    qreal c = qsl[2].toDouble();
    qreal d = qsl[3].toDouble();
    qreal e = qsl[4].toDouble();
    qreal f = qsl[5].toDouble();
    return Transform(a,b,c,d,e,f);
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

FillData::FillData()
{
    minX = -4;
    minY = -4;
    maxX = 4;
    maxY = 4;
}

void FillData::set(int minX, int maxX, int minY, int maxY)
{
    this->minX = minX;
    this->minY = minY;
    this->maxX = maxX;
    this->maxY = maxY;
}

void FillData::set(FillData & fdata)
{
    minX = fdata.minX;
    minY = fdata.minY;
    maxX = fdata.maxX;
    maxY = fdata.maxY;
}

void FillData::get(int & minX, int & maxX, int & minY, int & maxY)
{
    minX = this->minX;
    minY = this->minY;
    maxX = this->maxX;
    maxY = this->maxY;
}
