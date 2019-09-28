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



// Feature management.
// Added feature are embedded into a PlacedFeature.
void Tiling::add(const PlacedFeaturePtr pf )
{
    placed_features.push_back(pf);
}

void Tiling::add(FeaturePtr f, QTransform  T)
{
    add(make_shared<PlacedFeature>(f, T));
}

void Tiling::remove(PlacedFeaturePtr pf)
{
    placed_features.removeOne(pf);
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

QString Tiling::dump() const
{
    QString astring;
    QDebug  deb(&astring);

    deb << "name=" << name  << "t1=" << t1 << "t2=" << t2 << "num features=" << placed_features.size() << endl;
    for (int i=0; i < placed_features.size(); i++)
    {
        PlacedFeaturePtr  pf = placed_features[i];
        deb << "poly" << i << "points=" << pf->getFeature()->numPoints()  << "transform= " << Transform::toInfoString(pf->getTransform()) << endl;
        deb << pf->getFeaturePolygon() << endl;
    }

    return astring;
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
        Q_UNUSED(save)
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
        str.setRealNumberPrecision(16);
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

void Tiling::writeTilingXML(QTextStream & out)
{
    refId    = 0;
    vertex_ids.clear();

    // Regroup features by their translation so that we write each feature only once.
    FeatureGroup fgroup = regroupFeatures();

    out << "<?xml version=\"1.0\"?>" << endl;
    out << "<Tiling version=\"2\">" << endl;
    out << "<Name>" << name << "</Name>" << endl;

    // fill paratmeters not part of original taprats
    int minX,minY,maxX,maxY;
    fillData.get(minX,maxX,minY,maxY);
    out << "<Fill>" << minX << "," << maxX << ","
                    << minY << "," << maxY << "</Fill>" << endl;

    out << "<T1>" <<  t1.x() << "," << t1.y() << "</T1>" << endl;
    out << "<T2>" <<  t2.x() << "," << t2.y() << "</T2>" << endl;

    // tiling maker view settings
    out << "<View>"  << endl;
    out << "<Scale>" << viewXform.getScale()          << "</Scale>" << endl;
    out << "<Rot>"   << viewXform.getRotateRadians()  << "</Rot>" << endl;
    out << "<X>"     << viewXform.getTranslateX()     << "</X>" << endl;
    out << "<Y>"     << viewXform.getTranslateY()     << "</Y>" << endl;
    setPoint(out,viewXform.getRotateCenter(),"Center");
    out << "</View>" << endl;


    //structure is feature then placements. so feature is not duplicated. I dont know if this adds any value
    for (auto it = fgroup.begin(); it != fgroup.end(); it++)
    {
        QPair<FeaturePtr,QList<PlacedFeaturePtr>> & apair = *it;
        FeaturePtr f                     = apair.first;
        QList<PlacedFeaturePtr> & qvpfp = apair.second;
        PlacedFeaturePtr pfp = qvpfp.first();

        if (pfp->isGirihShape())    // TODO verify this code works
        {
            out << "<Feature type=\"girih\" name=\"" << pfp->getGirishShapeName() << "\">" << endl;

        }
        else if (f->isRegular())
        {
            out << "<Feature type=\"regular\" sides=\"" << f->numPoints() << "\">" << endl;
        }
        else
        {
            out << "<Feature type=\"edgepoly\">" << endl;
            EdgePoly epoly = f->getEdgePoly();
            setEdgePoly(out,epoly);
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

        // placements - still using Kaplan's affine transform class for read/write
        for(auto it= qvpfp.begin(); it != qvpfp.end(); it++ )
        {
            PlacedFeaturePtr & pf = *it;
            QTransform t = pf->getTransform();
            QVector<qreal> ds;
            ds << t.m11();
            ds << t.m21();
            ds << t.dx();
            ds << t.m12();
            ds << t.m22();
            ds << t.dy();
            Q_ASSERT(ds.size() == 6);
            out << "<Placement>";
            for (int j=0; j<5; j++)
            {
                out << ds[j] << ",";
            }
            out << ds[5];
            out << "</Placement>" << endl;
        }

        out << "</Feature>" << endl;
    }

    out << "<Desc>" << desc << "</Desc>" << endl;
    out << "<Auth>" << author << "</Auth>" << endl;

    if (!bkgd.bkgdName.isEmpty())
    {
        QString astring = QString("<BackgroundImage name=\"%1\">").arg(bkgd.bkgdName);
        out << astring << endl;

        out << "<Scale>" << bkgd.scale << "</Scale>" << endl;
        out << "<Rot>"   << bkgd.rot   << "</Rot>"  << endl;
        out << "<X>"     << bkgd.x     << "</X>" << endl;
        out << "<Y>"     << bkgd.y     << "</Y>" << endl;

        if (!bkgd.perspective.isIdentity())
        {
            out << "<Perspective>";
            out << Transform::toString(bkgd.perspective);
            out << "</Perspective>" << endl;
        }

        out << "</BackgroundImage>" << endl;
    }

    out << "</Tiling>" << endl;
}

void Tiling::setEdgePoly(QTextStream & ts, EdgePoly & epoly)
{
    for (auto it = epoly.begin(); it != epoly.end(); it++)
    {
        EdgePtr ep = *it;
        VertexPtr v1 = ep->getV1();
        VertexPtr v2 = ep->getV2();
        if (ep->getType() == EDGE_LINE)
        {
            ts << "<Line>" << endl;
            VertexPtr v1 = ep->getV1();
            VertexPtr v2 = ep->getV2();
            setVertex(ts,v1,"Point");
            setVertex(ts,v2,"Point");
            ts << "</Line>" << endl;
        }
        else if (ep->getType() == EDGE_CURVE)
        {
            QString str = QString("<Curve convex=\"%1\">").arg(ep->isConvex() ? "t" : "f");
            ts << str << endl;
            QPointF p3 = ep->getArcCenter();
            setVertex(ts,v1,"Point");
            setVertex(ts,v2,"Point");
            setPoint(ts,p3,"Center");
            ts << "</Curve>" << endl;
        }
    }
}

void Tiling::setVertex(QTextStream & ts,VertexPtr v, QString name)
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

    QPointF pt = v->getPosition();

    ts << "<" << name << qsid << ">";
    ts << pt.x() << "," << pt.y();
    ts << "</" << name << ">" << endl;
}

void Tiling::setPoint(QTextStream & ts, QPointF pt, QString name)
{
    ts << "<" << name << ">";
    ts << pt.x() << "," << pt.y();
    ts << "</" << name << ">" << endl;
}

QString Tiling::getVertexReference(VertexPtr ptr)
{
    int id =  vertex_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString  Tiling::id(int id)
{
    //qDebug() << "id=" << id;
    QString qs = QString(" id=\"%1\"").arg(id);
    return qs;
}

QString  Tiling::nextId()
{
    return id(++refId);
}

void Tiling::setVertexReference(int id, VertexPtr ptr)
{
    vertex_ids[ptr] = id;
}

bool Tiling::hasReference(VertexPtr v)
{
    return vertex_ids.contains(v);
}

