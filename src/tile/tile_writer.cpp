#include <QTransform>
#include "tile_writer.h"
#include "geometry/xform.h"
#include "geometry/edge.h"
#include "misc/tpm_io.h"

TileWriter::TileWriter()
{
    refId    = 0;
}

void TileWriter::setEdgePoly(QTextStream & ts, const EdgePoly & epoly)
{
    for (auto it = epoly.begin(); it != epoly.end(); it++)
    {
        EdgePtr ep = *it;
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
            QString str = QString("<Curve convex=\"%1\">").arg(ep->isConvex() ? "t" : "f");
            ts << str << endl;
            QPointF p3 = ep->getArcCenter();
            setVertex(ts,v1,"Point");
            setVertex(ts,v2,"Point");
            setPoint(ts,p3,"Center");
            ts << "</Curve>" << endl;
        }
        else if (ep->getType() == EDGETYPE_CHORD)
        {
            QString str = QString("<Chord convex=\"%1\">").arg(ep->isConvex() ? "t" : "f");
            ts << str << endl;
            QPointF p3 = ep->getArcCenter();
            setVertex(ts,v1,"Point");
            setVertex(ts,v2,"Point");
            setPoint(ts,p3,"Center");
            ts << "</Chord>" << endl;
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
    if (hasReference(v))
    {
        qsid = getVertexReference(v);
        ts << "<" << name << qsid << "/>" << endl;
        return;
    }

    qsid = nextId();
    setVertexReference(getRef(),v);

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
