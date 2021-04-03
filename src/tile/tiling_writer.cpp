#include "tile/tiling_writer.h"
#include "tile/backgroundimage.h"
#include "geometry/transform.h"
#include "base/configuration.h"
#include "base/fileservices.h"
#include "base/tiledpatternmaker.h"
#include "panels/panel.h"
#include "base/mosaic_writer.h"

using namespace pugi;
using std::string;


//const int currentTilingXMLVersion = 3;  // 26JUL20 excludes FillData
//const int currentTilingXMLVersion = 4;  // 13SEP20 restore FillData
//const int currentTilingXMLVersion = 5;  // 09NOV20 new background image positioning
  const int currentTilingXMLVersion = 6;  // 14NOV20 <Placement> becomes <Transform>

bool TilingWriter::writeTilingXML()
{
    Configuration * config = Configuration::getInstance();

    // the name is in the tiling
    QString name = tiling->getName();
    QString filename = FileServices::getTilingFile(name);

    if (!filename.isEmpty())
    {
        // file already exists
        bool isOriginal = filename.contains("original");
        QMessageBox msgBox(ControlPanel::getInstance());
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
            tiling->setName(name);
            if (isOriginal)
            {
                filename = config->originalTileDir + name + ".xml";
            }
            else
            {
                filename = config->newTileDir + name + ".xml";
           }
        }
        // save drops thru
        Q_UNUSED(save)
    }
    else
    {
        // new file
        filename = config->newTileDir + name + ".xml";
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
            QMessageBox box(ControlPanel::getInstance());
            box.setIcon(QMessageBox::Information);
            box.setText(QString("Saved: %1 - OK").arg(data.fileName()));
            box.exec();

            emit theApp->sig_tilingLoaded(name);
            return true;
        }
    }

    qWarning() << "Could not write tile file:"  << data.fileName();
    QMessageBox box(ControlPanel::getInstance());
    box.setIcon(QMessageBox::Critical);
    box.setText(QString("Error saving: %1 - FAILED").arg(data.fileName()));
    box.exec();
    return false;

}

void TilingWriter::writeTilingXML(QTextStream & out)
{
    refId    = 0;
    vertex_ids.clear();

    // Regroup features by their translation so that we write each feature only once.
    FeatureGroup fgroup = tiling->regroupFeatures();

    out << "<?xml version=\"1.0\"?>" << endl;
    QString qs = QString("<Tiling version=\"%1\">").arg(currentTilingXMLVersion);
    out << qs << endl;
    out << "<Name>" << tiling->getName() << "</Name>" << endl;

    // fill paratmeters not part of original taprats
    int minX,minY,maxX,maxY;
    tiling->getFillData().get(minX,maxX,minY,maxY);
    out << "<Fill>" << minX << "," << maxX << ","
                    << minY << "," << maxY << "</Fill>" << endl;
    QPointF t1 = tiling->getTrans1();
    QPointF t2 = tiling->getTrans2();
    out << "<T1>" <<  t1.x() << "," << t1.y() << "</T1>" << endl;
    out << "<T2>" <<  t2.x() << "," << t2.y() << "</T2>" << endl;

    writeViewSettings(out);

    //structure is feature then placements. so feature is not duplicated. I dont know if this adds any value
    for (auto& apair : fgroup)
    {
        FeaturePtr feature                = apair.first;
        QVector<PlacedFeaturePtr> & qvpfp = apair.second;
        PlacedFeaturePtr placedFeature    = qvpfp.first();

        if (placedFeature->isGirihShape())    // TODO verify this code works
        {
            // saved girih shapesd have a translation
            out << "<Feature type=\"girih\" name=\"" << placedFeature->getGirihShapeName() << "\">" << endl;

        }
        else if (feature->isRegular())
        {
            // regular features have sides and rotation
            out << "<Feature type=\"regular\" sides=\"" << feature->numPoints() << "\" rotation=\"" << feature->getRotation() << "\" scale=\"" << feature->getScale() << "\">" << endl;
        }
        else
        {
            // edge polys have rotation, numSides can be calculated
            out << "<Feature type=\"edgepoly\" rotation=\"" << feature->getRotation() << "\" scale=\"" << feature->getScale() << "\">" << endl;
            EdgePoly epoly = feature->getEdgePoly();
            setEdgePoly(out,epoly);
        }

        // background colors
        ColorSet * bkgdColors  = feature->getBkgdColors();
        int sz = bkgdColors->size();
        if (sz)
        {
            QString s = "<BkgdColors>";
            for (int i = 0; i < (sz-1); i++)
            {
                QColor color = bkgdColors->getColor(i).color;
                s += color.name();
                s += ",";
            }
            QColor color = bkgdColors->getColor(sz-1).color;
            s += color.name();
            s += "</BkgdColors>";
            out << s << endl;
        }

        for(auto it= qvpfp.begin(); it != qvpfp.end(); it++ )
        {
            PlacedFeaturePtr & pf = *it;
            QTransform t = pf->getTransform();
            out << "<Placement>";
            out << "<scale>" << Transform::scalex(t) << "</scale>";
            out << "<rot>"   << qRadiansToDegrees(Transform::rotation(t)) << "</rot>";
            out << "<tranX>" << Transform::transx(t) << "</tranX>";
            out << "<tranY>" << Transform::transy(t) << "</tranY>";
            out << "</Placement>" << endl;
        }

        out << "</Feature>" << endl;
    }

    out << "<Desc>" << tiling->getDescription() << "</Desc>" << endl;
    out << "<Auth>" << tiling->getAuthor() << "</Auth>" << endl;

    BkgdImgPtr bkgd = tiling->getBackground();
    writeBackgroundImage(out,bkgd);

    out << "</Tiling>" << endl;
}

void TilingWriter::writeBackgroundImage(QTextStream & out, BkgdImgPtr bkgd)
{
    if (bkgd && bkgd->isLoaded())
    {
        QString astring = QString("<BackgroundImage name=\"%1\">").arg(bkgd->getName());
        out << astring << endl;

        Xform xform = bkgd->getBkgdXform();
        out << "<Scale>" << xform.getScale()           << "</Scale>" << endl;
        out << "<Rot>"   << xform.getRotateRadians()   << "</Rot>"  << endl;
        out << "<X>"     << xform.getTranslateX()      << "</X>" << endl;
        out << "<Y>"     << xform.getTranslateY()      << "</Y>" << endl;

        if (!bkgd->perspective.isIdentity())
        {
            out << "<Perspective>";
            out << Transform::toString(bkgd->perspective);
            out << "</Perspective>" << endl;
        }

        out << "</BackgroundImage>" << endl;
    }
}

void TilingWriter::writeViewSettings(QTextStream & out)
{
    out << "<ViewSettings>" <<  endl;

    QSize size = tiling->getSize();
    out << "<width>"  << size.width()  << "</width>" << endl;
    out << "<height>" << size.height() << "</height>" << endl;

    MosaicWriter::procesToolkitGeoLayer(out,tiling->getCanvasXform());

    out << "</ViewSettings>" <<  endl;
}

void TilingWriter::setEdgePoly(QTextStream & ts, EdgePoly & epoly)
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
    }
}

void TilingWriter::setVertex(QTextStream & ts,VertexPtr v, QString name)
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

void TilingWriter::setPoint(QTextStream & ts, QPointF pt, QString name)
{
    ts << "<" << name << ">";
    ts << pt.x() << "," << pt.y();
    ts << "</" << name << ">" << endl;
}

QString TilingWriter::getVertexReference(VertexPtr ptr)
{
    int id =  vertex_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

QString  TilingWriter::id(int id)
{
    //qDebug() << "id=" << id;
    QString qs = QString(" id=\"%1\"").arg(id);
    return qs;
}

QString  TilingWriter::nextId()
{
    return id(++refId);
}

void TilingWriter::setVertexReference(int id, VertexPtr ptr)
{
    vertex_ids[ptr] = id;
}

bool TilingWriter::hasReference(VertexPtr v)
{
    return vertex_ids.contains(v);
}

