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
// PlacedFeature.java
//
// A PlacedFeature is a Feature together with a transform matrix.
// It allows us to share an underlying feature while putting several
// copies together into a tiling.  A tiling is represented as a
// collection of PlacedFeatures (that may share Features) that together
// make up a translational unit.

#include "tile/placed_feature.h"
#include "geometry/transform.h"
#include "base/configuration.h"
#include "tile/feature_writer.h"
#include "tile/feature_reader.h"
#include "base/utilities.h"
#include "base/fileservices.h"


PlacedFeature::PlacedFeature()
{
    _show = true;
} // default

PlacedFeature::PlacedFeature(FeaturePtr feature, QTransform T )
{
    this->feature = feature;
    this->T       = T;
    _show = true;
    //qDebug() << "setTransform1=" << Transform::toInfoString(T);
}

PlacedFeature::PlacedFeature(PlacedFeaturePtr other)
{
    feature = other->feature;
    T       = other->T;
    _show = true;
    //qDebug() << "setTransform2=" << Transform::toInfoString(T);
}

void PlacedFeature::setFeature(FeaturePtr feature)
{
    this->feature = feature;
}

FeaturePtr PlacedFeature::getFeature()
{
    return feature;
}

QTransform PlacedFeature::getTransform()
{
    return T;
}

EdgePoly  PlacedFeature::getPlacedEdgePoly()
{
    const EdgePoly & ep = feature->getEdgePoly();
#if 1
    EdgePoly ep2  = ep.recreate();
    ep2.mapD(T);
#else
    EdgePoly ep2  = ep.map(T);
#endif
    return ep2;
}

void PlacedFeature::setTransform(QTransform newT)
{
    //qDebug() << "setTransform3 before =" << Transform::toInfoString(T);
    T = newT;
    //qDebug() << "setTransform3 after =" << Transform::toInfoString(T);
}

bool PlacedFeature::saveAsGirihShape(QString name)
{
    Configuration * config = Configuration::getInstance();
    QString root     = config->rootMediaDir;
    QString filename = root + "girih_shapes" + "/" + name + ".xml";

    QFile data(filename);
    if (data.open(QFile::WriteOnly))
    {
        qDebug() << "Writing:"  << data.fileName();
        QTextStream str(&data);
        str.setRealNumberPrecision(16);
        saveGirihShape(str,name);
        data.close();

        if (FileServices::reformatXML(filename))
        {
            girihShapeName = name;
            return true;
        }
    }
    qWarning() << "Could not write tile file:"  << filename;

    return false;
}

void PlacedFeature::saveGirihShape(QTextStream & ts, QString name)
{
    FeatureWriter fw;

    ts << "<?xml version=\"1.0\"?>" << endl;

    if (feature->isRegular())
    {
        QString str = QString("<Poly name=\"%1\" type=\"regular\" sides=\"%2\">").arg(name).arg(feature->numPoints());
        ts << str << endl;
    }
    else
    {
        QString str = QString("<Poly name=\"%1\">").arg(name);
        ts << str << endl;
        fw.setEdgePoly(ts,feature->getEdgePoly());
    }
    fw.setTransform(ts,T);
    ts << "</Poly>" << endl;
}

bool PlacedFeature::loadFromGirihShape(QString name)
{
    Configuration * config = Configuration::getInstance();
    QString root     = config->rootMediaDir;
    QString filename = root + "girih_shapes" + "/" + name + ".xml";

    xml_document doc;
    xml_parse_result result = doc.load_file(filename.toStdString().c_str());
    if (result == false)
    {
        qWarning("Badly constructed XML file");
        return false;
    }

    xml_node tiling_node = doc.first_child();
    QString node_name = tiling_node.name();
    if (node_name != "Poly")
    {
        qWarning() << "Unexpected node name = "  << node_name;
        return false;
    }

    xml_attribute attr = tiling_node.attribute("type");
    if (attr)
    {
        QString str = attr.value();
        if (str == "regular")
        {
            xml_attribute attr2 = tiling_node.attribute("sides");
            Q_ASSERT(attr2);
            QString val = attr2.value();
            int sides = val.toInt();
            loadGirihShape(sides,tiling_node);
        }
    }
    else
    {
        xml_node n = tiling_node.first_child();
        string nname = n.name();
        if (nname == "Point")
        {
            // load old format (no longer used)
            loadGirihShapeOld(tiling_node);
        }
        else
        {
            Q_ASSERT(nname == "Line" || nname == "Curve");
            loadGirihShape(tiling_node);
        }
    }
    girihShapeName = name;

    return true;
}

void PlacedFeature::loadGirihShape(xml_node & poly_node)
{
    FeatureReader fr;
    EdgePoly ep = fr.getEdgePoly(poly_node);
    feature     = make_shared<Feature>(ep,0);
    T           = fr.getTransform(poly_node);
}

void PlacedFeature::loadGirihShape(int sides, pugi::xml_node & poly_node)
{
    FeatureReader fr;
    feature     = make_shared<Feature>(sides,0);
    T           = fr.getTransform(poly_node);
}

void PlacedFeature::loadGirihShapeOld(xml_node & poly_node)
{
    QPolygonF poly;
    for (xml_node n = poly_node.first_child(); n; n = n.next_sibling())
    {
        QString str = n.child_value();
        QStringList qsl;
        qsl = str.split(',');
        qreal a = qsl[0].toDouble();
        qreal b = qsl[1].toDouble();
        QPointF pt(a,b);
        poly << pt;
    }

    EdgePoly epoly(poly);
    feature = make_shared<Feature>(epoly,0);
}
