#include <QTransform>
#include "model/tilings/tile_writer.h"
#include "sys/geometry/xform.h"
#include "sys/geometry/edge.h"
#include "sys/qt/tpm_io.h"

TileWriter::TileWriter()
{
    twbase.refId    = 0;
}

void TileWriter::setEdgePoly(QTextStream & ts, const EdgePoly & epoly)
{
    for (auto & ep : epoly.get())
    {
        VertexPtr v1 = ep->v1;
        VertexPtr v2 = ep->v2;
        if (ep->getType() == EDGETYPE_LINE)
        {
            ts << "<Line>" << endl;
            VertexPtr v1 = ep->v1;
            VertexPtr v2 = ep->v2;
            setVertex(ts,v1,"Point");
            setVertex(ts,v2,"Point");
            ts << "</Line>" << endl;
        }
        else if (ep->getType() == EDGETYPE_CURVE)
        {
            QString str = QString("<Curve convex=\"%1\">").arg((ep->getCurveType() == CURVE_CONVEX) ? "t" : "f");
            ts << str << endl;
            QPointF p3 = ep->getArcCenter();
            setVertex(ts,v1,"Point");
            setVertex(ts,v2,"Point");
            setPoint(ts,p3,"Center");
            ts << "</Curve>" << endl;
        }
    }
}

void TileWriter::setTransform(QTextStream & ts, QTransform T)
{
    Xform xf;
    xf.setTransform(T);

    ts << "<tx>"    << xf.getTranslateX()    << "</tx>"    << endl;
    ts << "<ty>"    << xf.getTranslateY()    << "</ty>"    << endl;
    ts << "<scale>" << xf.getScale()         << "</scale>" << endl;
    ts << "<rot>"   << xf.getRotateRadians() << "</rot>"   << endl;
}

void TileWriter::setVertex(QTextStream & ts,VertexPtr v, QString name)
{
    QString qsid;
    if (twbase.hasReference(v))
    {
        qsid = twbase.getVertexReference(v);
        ts << "<" << name << qsid << "/>" << endl;
        return;
    }

    qsid = twbase.nextId();
    twbase.setVertexReference(twbase.getRef(),v);

    QPointF pt = v->pt;

    ts << "<" << name << qsid << ">";
    ts << pt.x() << "," << pt.y();
    ts << "</" << name << ">" << endl;
}

void TileWriter::setPoint(QTextStream & ts, QPointF pt, QString name)
{
    ts << "<" << name << ">";
    ts << pt.x() << "," << pt.y();
    ts << "</" << name << ">" << endl;
}
