#include "tile/tiling.h"
#include "tile/tiling_loader.h"
#include "tile/tile_reader.h"
#include "misc/backgroundimage.h"
#include "mosaic/mosaic_reader.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "tile/placed_tile.h"
#include "tile/tile.h"
#include "misc/defaults.h"

using namespace pugi;
using std::string;
using std::make_shared;

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

    int nf = qsl[2].toInt();    // number of tiles

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
        TilePtr bf;

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
            bf = make_shared<Tile>(num_sides,0);
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
            bf = make_shared<Tile>(ep,0);
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
            PlacedTilePtr pfp = make_shared<PlacedTile>(tiling.get(),bf,t);
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
    qDebug().noquote() << "TilingLoader::readTilingXML" << file;

    xml_document doc;
    xml_parse_result result = doc.load_file(file.toStdString().c_str());
    if (result == false)
    {
        qWarning("Badly constructed XML file");
        return nullptr;
    }

    xml_node tiling_node = doc.first_child();
    TilingPtr tp = readTilingXML(tiling_node);
    if (tp->hasIntrinsicOverlaps())
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

	xml_node view = tiling_node.child("View");
    if (view)
    {
        Xform xf = getXform(view);
        tiling->setCanvasXform(xf);
        qDebug().noquote() << "tiling canvas xform" << xf.toInfoString();
    }

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
    // 03AUG2020 fillData can be singleton
    xml_node t0 = tiling_node.child("Fill");
    if (t0)
    {
        bool isSingle = false;
        xml_attribute single = t0.attribute("singleton");
        if (single)
        {
            if (single.as_string() == QString("t"))
            {
                isSingle = true;
            }
        }
        QString strt0 = t0.child_value();
        FillData fd = getFill(strt0,isSingle);
        FillData & fdata = tiling->getDataAccess().getFillDataAccess();
        fdata = fd;
    }

    TileReader tr;   // must be located here to bw used for all tiles

    for (xml_node tile = tiling_node.child("Feature"); tile; tile = tile.next_sibling("Feature"))
    {
        TilePtr bf;

        xml_attribute fatt = tile.attribute("type");
        if (!fatt)
        {
            qWarning("missing type attribute");
            continue;
        }

        QString strtype = fatt.as_string();

        if (strtype == "girih")
        {
            xml_attribute nameattr = tile.attribute("name");
            if (!nameattr)
            {
                qWarning("missing name attribute for girih shape");
                continue;
            }
            QString name = nameattr.value();
            PlacedTilePtr pfp = make_shared<PlacedTile>(tiling.get());
            if (pfp->loadFromGirihShape(name))
            {
                bf = pfp->getTile();
            }
        }
        else if (strtype == "regular")
        {
            int sides      = 0;
            xml_attribute sidesatt = tile.attribute("sides");
            if (!sidesatt)
            {
                qWarning("missing sides attribute");
                continue;
            }
            sides = sidesatt.as_int();

            qreal rotation = 0.0;
            xml_attribute rotatt = tile.attribute("rotation");
            if (rotatt)
            {
                rotation = rotatt.as_double();
            }

            qreal scale = 1.0;
            xml_attribute scaleatt = tile.attribute("scale");
            if (scaleatt)
            {
                scale = scaleatt.as_double();
            }

            bf = make_shared<Tile>(sides,rotation,scale);
        }
        else if (strtype == "polygon")
        {
            // older tilings have this, newer tilings use edgepoly
            QPolygonF pts;
            for (xml_node pnode = tile.child("Point"); pnode; pnode = pnode.next_sibling("Point"))
            {
                string txt = pnode.child_value();
                QPointF pt = getPoint(txt.c_str());
                pts << pt;
            }
            EdgePoly ep(pts);
            bf = make_shared<Tile>(ep);
        }
        else if (strtype == "edgepoly")
        {
            EdgePoly ep = tr.getEdgePoly(tile);

            qreal rotation = 0.0;
            xml_attribute rotatt = tile.attribute("rotation");
            if (rotatt)
            {
                rotation = rotatt.as_double();
            }

            qreal scale = 1.0;
            xml_attribute scaleatt = tile.attribute("scale");
            if (scaleatt)
            {
                scale = scaleatt.as_double();
            }

            bf = make_shared<Tile>(ep,rotation,scale);
        }

        for (xml_node plnode = tile.child("Placement"); plnode; plnode = plnode.next_sibling("Placement"))
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

            PlacedTilePtr pfp = make_shared<PlacedTile>(tiling.get(),bf,T);
            tiling->add(pfp);
        }

        xml_node bcolor = tile.child("BkgdColors");
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
            bf->getTileColors()->setColors(tpcolors);
        }
    }

    xml_node bkImage = tiling_node.child("BackgroundImage");
    if (bkImage)
    {
        getBackgroundImage(bkImage);
    }

    tiling->purgeStack();   // always start clean

    // tiling->dump();
    return tiling;
}

void TilingLoader::getBackgroundImage(xml_node & node)
{
    xml_attribute attr = node.attribute("name");
    QString name       = attr.value();

    BkgdImgPtr bip = BackgroundImage::getSharedInstance();
    bip->load(name);
    if (bip->isLoaded())
    {
        Xform xf = bip->getCanvasXform();

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

        n = node.child("Center");
        if (n)
        {
            QString str = n.child_value();
            QStringList qsl = str.split(",");
            qreal x = qsl[0].toDouble();
            qreal y = qsl[1].toDouble();
            xf.setModelCenter(QPointF(x,y));
        }

        bip->setCanvasXform(xf);
        qDebug().noquote() << "background image xform:" << xf.toInfoString();

        n= node.child("Perspective");
        if (n)
        {
            QString str = n.child_value();
            bip->perspective = getQTransform(str);

            if (!bip->perspective.isIdentity())
            {
                bip->createAdjustedImage();
            }
        }

        bip->createPixmap();
    }
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
    xf.setModelCenter(getPoint(str));

    return  xf;
}

FillData TilingLoader::getFill(QString txt,bool isSingle)
{
    FillData fd;
    QStringList qsl;
    qsl = txt.split(',');
    fd.set(isSingle,qsl[0].toInt(),qsl[1].toInt(),qsl[2].toInt(),qsl[3].toInt());
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

    int width  = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;
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

    ModelSettings & ms = tiling->getDataAccess().getSettingsAccess();

    QSize size(width,height);
    ms.setSize(size);
    qDebug() << "tiling size:" << size;

    int zwidth  = width;
    int zheight = height;
    xml_node n2 = view.child("zwidth");
    if (n2)
    {
        QString str = n2.child_value();
        zwidth = str.toInt();
    }

    n2 = view.child("zheight");
    if (n2)
    {
        QString str = n2.child_value();
        zheight = str.toInt();
    }

    QSize zsize(zwidth,zheight);
    ms.setZSize(zsize);
    qDebug() << "tiling zoom size:" << zsize;

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
        xf.setModelCenter(QPointF(x,y));
    }

    tiling->setCanvasXform(xf);
    qDebug().noquote() << "tiling canvas xform:" << xf.toInfoString();
}

