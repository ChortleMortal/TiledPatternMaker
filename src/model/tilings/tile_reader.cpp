#include "model/tilings/tile_reader.h"
#include "sys/geometry/xform.h"
#include "sys/geometry/edge.h"
#include "model/mosaics/mosaic_reader_base.h"
#include <QTransform>

using std::make_shared;

TileReader::TileReader()
{
    base = std::make_shared<MosaicReaderBase>();
}

TileReader::TileReader(MRBasePtr base)
{
    this->base = base;
}

EdgePoly TileReader::getEdgePoly(xml_node & node, bool legacyFlip)
{
    EdgePoly ep;
    for (xml_node n = node.first_child(); n; n = n.next_sibling())
    {
        string name = n.name();
        if (name == "Line")
        {
            xml_node n2  = n.child("Point");
            xml_node n3  = n2.next_sibling("Point");
            VertexPtr v1 = getVertex(n2);
            VertexPtr v2 = getVertex(n3);
            ep.push_back(make_shared<Edge>(v1,v2));
        }
        else if (name == "Curve")
        {
            xml_node n2   = n.child("Point");
            xml_node n3    = n2.next_sibling("Point");
            xml_node n4    = n.child("Center");
            VertexPtr v1   = getVertex(n2);
            VertexPtr v2   = getVertex(n3);
            QPointF center = getPoint(n4);
            bool convex = true; // default
            xml_attribute conv = n.attribute("convex");
            if (conv)
            {
                QString val = conv.value();
                convex = (val == "t") ? true : false;
            }
            if (legacyFlip && !convex)
            {
                center = ArcData::reflectCentreOverEdge(v1->pt,v2->pt,center);
            }
            EdgePtr eptr = make_shared<Edge>(v1,v2,center,convex,false);
            ep.push_back(eptr);
        }
        else if (name == "Chord")
        {
            xml_node n2    = n.child("Point");
            xml_node n3    = n2.next_sibling("Point");
            xml_node n4    = n.child("Center");
            VertexPtr v1   = getVertex(n2);
            VertexPtr v2   = getVertex(n3);
            QPointF center = getPoint(n4);
            bool convex = true; // default
            xml_attribute conv = n.attribute("convex");
            if (conv)
            {
                QString val = conv.value();
                convex = (val == "t") ? true : false;
            }
            if (legacyFlip && !convex)
            {
                center = ArcData::reflectCentreOverEdge(v1->pt,v2->pt,center);
            }
            EdgePtr eptr = make_shared<Edge>(v1,v2,center,convex,true);
            ep.push_back(eptr);
        }

    }
    return ep;
}

QTransform  TileReader::getTransform(xml_node & node)
{
    QString val;
    Xform xf;

    xml_node n = node.child("tx");
    if (n)
    {
        val = n.child_value();
        xf.setTranslateX(val.toDouble());
    }

    n = node.child("ty");
    if (n)
    {
        val = n.child_value();
        xf.setTranslateY(val.toDouble());
    }

    n = node.child("scale");
    if (n)
    {
        val = n.child_value();
        xf.setScale(val.toDouble());
    }

    n = node.child("rot");
    if (n)
    {
        val = n.child_value();
        xf.setRotateRadians(val.toDouble());
    }

    return xf.getTransform();
}

VertexPtr TileReader::getVertex(xml_node & node)
{
    if (base->hasReference(node))
    {
        return base->getVertexReferencedPtr(node);
    }

    // pos
    QString txt = node.child_value();

    QStringList qsl;
    qsl = txt.split(',');
    qreal x = qsl[0].toDouble();
    qreal y = qsl[1].toDouble();

    VertexPtr v = make_shared<Vertex>(QPointF(x,y));
    base->setVertexReference(node,v);

    return v;
}

QPointF TileReader::getPoint(xml_node & node)
{
    QString txt = node.child_value();
    QStringList qsl;
    qsl = txt.split(',');
    qreal x = qsl[0].toDouble();
    qreal y = qsl[1].toDouble();
    return QPointF(x,y);
}
