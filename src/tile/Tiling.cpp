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

////////////////////////////////////////////////////////////////////////////
//
// Tiling.java
//
// The representation of a tiling, which will serve as the skeleton for
// Islamic designs.  A Tiling has two translation vectors and a set of
// PlacedFeatures that make up a translational unit.  The idea is that
// the whole tiling can be replicated across the plane by placing
// a copy of the translational unit at every integer linear combination
// of the translation vectors.  In practice, we only draw at those
// linear combinations within some viewport.

#include "tile/Tiling.h"
#include "base/configuration.h"
#include "base/fileservices.h"
#include <QtWidgets>

using namespace pugi;
using std::string;

Tiling::Tiling()
{
    name   = "The Unnamed";
    author = "Author";
    desc   = "Description";
}

Tiling::Tiling(QString name, QPointF t1, QPointF t2)
{
    this->t1 = t1;
    this->t2 = t2;
    //this.placed_features = new HashSet<PlacedFeature>();
    if (!name.isEmpty())
    {
        this->name = name;
    }
    else
    {
        name   = "The Unnamed";
    }
    author = "Author";
    desc   = "Description";
}

Tiling::Tiling(Tiling * other)
{
    t1 = other->t1;
    t2 = other->t2;

    for (auto it = other->placed_features.begin(); it != other->placed_features.end(); it++)
    {
        PlacedFeaturePtr pf = make_shared<PlacedFeature>(*it);
        placed_features.push_back(pf);
    }
    //placed_features = other->placed_features;

    name = other->name;
    desc = other->desc;
    author = other->author;

    fillData = other->fillData;
}

Tiling::~Tiling()
{
    placed_features.clear();
}

// Tiling I/O.
//
// Functions for reading tilings in from files.  No real error-checking
// is done, because I don't expect you to write these files by hand --
// they should be auto-generated from DesignerPanel.
//

TilingPtr Tiling::readTiling(QString file )
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

TilingPtr Tiling::readTiling(QTextStream & st)
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
            addPlacement(Transform( a, b, c, d, e, f ) );
        }
        endFeature();
    }

    st.skipWhiteSpace();
    b_setDescription(readQuotedString(st));

    st.skipWhiteSpace();
    b_setAuthor(readQuotedString(st));

    return EndTiling();
}

// Feature management.
// Added feature are embedded into a PlacedFeature.
void Tiling::add(const PlacedFeaturePtr pf )
{
    placed_features.push_back(pf);
}

void Tiling::add(FeaturePtr f, Transform & T )
{
    add(make_shared<PlacedFeature>(f, T));
}

void Tiling::remove(PlacedFeaturePtr pf)
{
    placed_features.removeOne(pf);
}

bool Tiling::writeTilingXML()
{
    Configuration * config = Configuration::getInstance();

    // the name is in the tiling
    QString filename = FileServices::getTilingFile(name);

    if (!filename.isEmpty())
    {
        // file already exists
        QMessageBox msgBox;
        msgBox.setText(QString("The tiling %1 already exists").arg(name));
        msgBox.setInformativeText("Do you want to bump version (Bump) or overwrite (Save)?");
        QPushButton * bump   = msgBox.addButton("Bump",QMessageBox::ApplyRole);
        QPushButton * save   = msgBox.addButton(QMessageBox::Save);
        QPushButton * cancel = msgBox.addButton(QMessageBox::Cancel);
        msgBox.setDefaultButton(bump);

        msgBox.exec();

        if (msgBox.clickedButton() == cancel)
        {
            return false;
        }
        else if (msgBox.clickedButton() == bump)
        {
            // appends a version
            name = FileServices::getNextVersion(name,true);
            filename = config->newTileDir + "/" + name + ".xml";
        }
        // save drops thru
        Q_UNUSED(save);
    }
    else
    {
        // new file
        filename = config->newTileDir + "/" + name + ".xml";
    }

    QFile data(filename);
    if (data.open(QFile::WriteOnly))
    {
        qDebug() << "Writing:"  << data.fileName();
        QTextStream str(&data);
        writeTilingXML(str);
        data.close();

        bool rv = FileServices::reformatXML(filename);
        if (rv)
        {
            QMessageBox box;
            box.setIcon(QMessageBox::Information);
            box.setText(QString("Saved: %1 - OK").arg(data.fileName()));
            box.exec();
            return true;
        }
    }

    qWarning() << "Could not write tile file:"  << data.fileName();
    QMessageBox box;
    box.setIcon(QMessageBox::Critical);
    box.setText(QString("Error saving: %1 - FAILED").arg(data.fileName()));
    box.exec();
    return false;

}

void Tiling::writeTilingXML(QTextStream & out )
{
    // Regroup features by their translation so that we write each feature only once.
    FeatureGroup fgroup = regroupFeatures();

    out << "<?xml version=\"1.0\"?>" << endl;
    out << "<Tiling>" << endl;
    out << "<Name>" << name << "</Name>" << endl;

    // fill paratmeters not part of original taprats
    int minX,minY,maxX,maxY;
    fillData.get(minX,maxX,minY,maxY);
    out << "<Fill>" << QString::number(minX) << "," << QString::number(maxX) << ","
                    << QString::number(minY) << "," << QString::number(maxY) << "</Fill>" << endl;

    out << "<T1>" <<  QString::number(t1.x(),'g',16) << "," << QString::number(t1.y(),'g',16) << "</T1>" << endl;
    out << "<T2>" <<  QString::number(t2.x(),'g',16) << "," << QString::number(t2.y(),'g',16) << "</T2>" << endl;

    //structure is feature then placements. so feature is not duplicated. I dont know if this adds any value
    for (auto it = fgroup.begin(); it != fgroup.end(); it++)
    {
        QPair<FeaturePtr,QList<PlacedFeaturePtr>> & apair = *it;
        FeaturePtr f                     = apair.first;
        QList<PlacedFeaturePtr> & qvpfp = apair.second;

        if (f->isRegular())
        {
            out << "<Feature type=\"regular\" sides=\"" << f->numPoints() << "\">" << endl;
        }
        else
        {
            out << "<Feature type=\"polygon\">" << endl;
            QPolygonF & pts = f->getPolygon();
            for( int idx = 0; idx < pts.size(); ++idx )
            {
                QPointF p = pts[idx];
                out << "<Point>" <<  QString::number(p.x(),'g',16) << "," << QString::number(p.y(),'g',16) << "</Point>" << endl;
            }
        }

        // background colors
        ColorSet & bkgdColors  = f->getBkgdColors();
        int sz = bkgdColors.size();
        if (sz)
        {
            QString s = "<BkgdColors>";
            for (int i = 0; i < (sz-1); i++)
            {
                QColor color = bkgdColors.getColor(i).color;
                s += color.name();
                s += ",";
            }
            QColor color = bkgdColors.getColor(sz-1).color;
            s += color.name();
            s += "</BkgdColors>";
            out << s << endl;
        }

        // placements
        for(auto it= qvpfp.begin(); it != qvpfp.end(); it++ )
        {
            PlacedFeaturePtr & pf = *it;
            Transform T           = pf->getTransform();

            qreal ds[6];
            T.get(ds);
            out << "<Placement>";
            for (int j=0; j<5; j++)
            {
                out << QString::number(ds[j],'g',16) << ",";
            }
            out << QString::number(ds[5],'g',16);
            out << "</Placement>" << endl;
        }

        out << "</Feature>" << endl;
    }

    out << "<Desc>" << desc << "</Desc>" << endl;
    out << "<Auth>" << author << "</Auth>" << endl;
    out << "</Tiling>" << endl;
}

TilingPtr Tiling::readTilingXML(QString file )
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

TilingPtr Tiling::readTilingXML(xml_node & tiling_node)
{
    string str = tiling_node.name();
    if (str != "Tiling")
    {
        qDebug() << "XML Error: node is:" << tiling_node.name();
        qWarning("<Tiling> not found");
        return nullptr;
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
       if (strtype == "regular")
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

       if (strtype == "polygon")
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

       for (xml_node plnode = feature.child("Placement"); plnode; plnode = plnode.next_sibling("Placement"))
       {
           string txt = plnode.child_value();
           Transform T = getTransform(txt.c_str());
           addPlacement(T);
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
   return EndTiling();
}

QString Tiling::readQuotedString(QTextStream &str)
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

QPointF Tiling::getPoint(QString txt)
{
    QStringList qsl;
    qsl = txt.split(',');
    qreal a = qsl[0].toDouble();
    qreal b = qsl[1].toDouble();
    return QPointF(a,b);
}

Transform Tiling::getTransform(QString txt)
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

QList<FeaturePtr> Tiling::getUniqueFeatures()
{
    QList<FeaturePtr> fs;

    for (auto it = placed_features.begin(); it != placed_features.end(); it++)
    {
        PlacedFeaturePtr pfp = *it;
        FeaturePtr fp = pfp->getFeature();
        if (fs.contains(fp))
        {
            continue;
        }
        fs.push_back(fp);
    }

    return fs;
}

// Regroup features by their translation so that we write each feature only once.
FeatureGroup Tiling::regroupFeatures()
{
    FeatureGroup featureGroup;
    for(auto it = placed_features.begin(); it != placed_features.end(); it++ )
    {
        PlacedFeaturePtr pf = *it;
        FeaturePtr f = pf->getFeature();
        if (featureGroup.containsFeature(f))
        {
            QList<PlacedFeaturePtr>  & v = featureGroup.getPlacements(f);
            v.push_back(pf);
        }
        else
        {
            QList<PlacedFeaturePtr> v;
            v.push_back(pf);
            featureGroup.push_back(qMakePair(f,v));
        }
    }
    return featureGroup;
}

void Tiling::dump() const
{
    qDebug().noquote() << "name=" << name  << "t1=" << t1 << "t2=" << t2 << "num features=" << placed_features.size();
    for (int i=0; i < placed_features.size(); i++)
    {
        PlacedFeaturePtr  pf = placed_features[i];
        qDebug().noquote() << "poly" << i << "points=" << pf->getFeature()->numPoints()  << "transform= " << pf->getTransform().toString();
    }
}

bool FeatureGroup::containsFeature(FeaturePtr fp)
{
    for (auto it = begin(); it != end(); it++)
    {
        QPair<FeaturePtr,QList<PlacedFeaturePtr>> & apair = *it;
        FeaturePtr f = apair.first;
        if (f == fp)
        {
            return true;
        }
    }
    return false;
}

QList<PlacedFeaturePtr> & FeatureGroup::getPlacements(FeaturePtr fp)
{
    Q_ASSERT(containsFeature(fp));
    for (auto it = begin(); it != end(); it++)
    {
        QPair<FeaturePtr,QList<PlacedFeaturePtr>> & apair = *it;
        FeaturePtr f = apair.first;
        if (f == fp)
        {
            return apair.second;
        }
    }

    qFatal("should never reach here");
    static QList<PlacedFeaturePtr> qvpf;
    return qvpf;
}
