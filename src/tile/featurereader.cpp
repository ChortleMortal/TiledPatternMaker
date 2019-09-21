#include "featurereader.h"
#include "geometry/xform.h"
#include <QTransform>

FeatureReader::FeatureReader()
{
    vOrigCnt = 0;
    vRefrCnt = 0;
}

EdgePoly FeatureReader::getEdgePoly(xml_node & node)
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
            xml_node n2  = n.child("Point");
            xml_node n3  = n2.next_sibling("Point");
            xml_node n4  = n.child("Center");
            VertexPtr v1 = getVertex(n2);
            VertexPtr v2 = getVertex(n3);
            QPointF   c0 = getPoint(n4);
            bool convex = true; // default
            xml_attribute conv = n.attribute("convex");
            if (conv)
            {
                QString val = conv.value();
                convex = (val == "t") ? true : false;
            }
            ep.push_back(make_shared<Edge>(v1,v2,c0,convex));
        }
    }
    return ep;
}

QTransform  FeatureReader::getTransform(xml_node & node)
{
    QString val;
    Xform xf;

    xml_node n = node.child("tx");
    if (n)
    {
        val           = n.child_value();
        xf.translateX = val.toDouble();
    }

    n = node.child("ty");
    if (n)
    {
        val           = n.child_value();
        xf.translateY = val.toDouble();
    }

    n = node.child("scale");
    if (n)
    {
        val          = n.child_value();
        xf.scale     = val.toDouble();
    }

    n = node.child("rot");
    if (n)
    {
        val          = n.child_value();
        xf.rotationRadians  = val.toDouble();
    }

    return xf.getTransform();
}

VertexPtr FeatureReader::getVertex(xml_node & node)
{
    if (hasReference(node))
    {
        vRefrCnt++;
        return getVertexReferencedPtr(node);
    }

    // pos
    QString txt = node.child_value();

    QStringList qsl;
    qsl = txt.split(',');
    qreal x = qsl[0].toDouble();
    qreal y = qsl[1].toDouble();

    vOrigCnt++;
    VertexPtr v = make_shared<Vertex>(QPointF(x,y));
    setVertexReference(node,v);

    return v;
}

QPointF FeatureReader::getPoint(xml_node & node)
{
    QString txt = node.child_value();
    QStringList qsl;
    qsl = txt.split(',');
    qreal x = qsl[0].toDouble();
    qreal y = qsl[1].toDouble();
    return QPointF(x,y);
}

bool FeatureReader::hasReference(xml_node & node)
{
    xml_attribute ref;
    ref = node.attribute("reference");
    return (ref);
}

void FeatureReader::setVertexReference(xml_node & node, VertexPtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        vertex_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref vertex:" << i;
#endif
    }
}

VertexPtr FeatureReader::getVertexReferencedPtr(xml_node & node)
{
    VertexPtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = VertexPtr(vertex_ids[id]);
        //if (retval == NULL) qCritical() << "reference:" << id << "- NOT FOUND";
    }
    return retval;
}
