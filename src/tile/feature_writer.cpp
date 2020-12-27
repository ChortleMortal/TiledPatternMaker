#include "feature_writer.h"
#include "geometry/xform.h"
#include <QTransform>


FeatureWriter::FeatureWriter()
{
    refId    = 0;
}

void FeatureWriter::setEdgePoly(QTextStream & ts, const EdgePoly & epoly)
{
    for (auto it = epoly.begin(); it != epoly.end(); it++)
    {
        EdgePtr ep = *it;
        VertexPtr v1 = ep->getV1();
        VertexPtr v2 = ep->getV2();
        if (ep->getType() == EDGETYPE_LINE)
        {
            ts << "<Line>" << endl;
            VertexPtr v1 = ep->getV1();
            VertexPtr v2 = ep->getV2();
            setVertex(ts,v1,"Point");
            setVertex(ts,v2,"Point");
            ts << "</Line>" << endl;
        }
        else if (ep->getType() == EDGETYPE_CURVE)
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

void FeatureWriter::setTransform(QTextStream & ts, QTransform T)
{
    Xform xf;
    xf.setTransform(T);

    ts << "<tx>"    << xf.getTranslateX()    << "</tx>"    << endl;
    ts << "<ty>"    << xf.getTranslateY()    << "</ty>"    << endl;
    ts << "<scale>" << xf.getScale()         << "</scale>" << endl;
    ts << "<rot>"   << xf.getRotateRadians() << "</rot>"   << endl;
}

void FeatureWriter::setVertex(QTextStream & ts,VertexPtr v, QString name)
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

void FeatureWriter::setPoint(QTextStream & ts, QPointF pt, QString name)
{
    ts << "<" << name << ">";
    ts << pt.x() << "," << pt.y();
    ts << "</" << name << ">" << endl;
}

QString  FeatureWriter::id(int id)
{
    //qDebug() << "id=" << id;
    QString qs = QString(" id=\"%1\"").arg(id);
    return qs;
}


QString  FeatureWriter::nextId()
{
    return id(++refId);
}

QString FeatureWriter::getVertexReference(VertexPtr ptr)
{
    int id =  vertex_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

void FeatureWriter::setVertexReference(int id, VertexPtr ptr)
{
    vertex_ids[ptr] = id;
}


bool FeatureWriter::hasReference(VertexPtr v)
{
    return vertex_ids.contains(v);
}
