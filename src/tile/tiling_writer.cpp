#include <QMessageBox>

#include "tile/tiling_writer.h"
#include "geometry/edge.h"
#include "geometry/transform.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/fileservices.h"
#include "misc/tpm_io.h"
#include "mosaic/mosaic_writer.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "tile/placed_tile.h"
#include "tile/tile.h"
#include "tile/tiling.h"
#include "tiledpatternmaker.h"
#include "viewers/backgroundimageview.h"

using namespace pugi;
using std::string;

extern class TiledPatternMaker * theApp;

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
        bool isOriginal  = filename.contains("original");
        bool isNewTiling = filename.contains("new_tilings");
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
            name = FileServices::getNextVersion(FILE_TILING,name);
            tiling->setName(name);
            if (isOriginal)
            {
                filename = config->originalTileDir + name + ".xml";
            }
            else if (isNewTiling)
            {
                filename = config->newTileDir + name + ".xml";
            }
            else
            {
                filename = config->testTileDir + name + ".xml";
            }
        }
        // save drops thru
        Q_UNUSED(save)
    }
    else
    {
        // new file
        if (config->saveTilingTest)
            filename = config->testTileDir + name + ".xml";
        else
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

            auto tilingMaker = TilingMaker::getInstance();
            emit tilingMaker->sig_tilingWritten(name);
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

    // Regroup tiles by their translation so that we write each tile only once.
    TileGroup fgroup = tiling->regroupTiles();

    out << "<?xml version=\"1.0\"?>" << endl;
    QString qs = QString("<Tiling version=\"%1\">").arg(currentTilingXMLVersion);
    out << qs << endl;

    out << "<Name>" << tiling->getName() << "</Name>" << endl;

    // fill paratmeters not part of original taprats
    int minX,minY,maxX,maxY;
    bool singleton;
    tiling->getData().getFillData().get(singleton,minX,maxX,minY,maxY);
    if (!singleton)
    {
        out << "<Fill singleton = \"f\">" << minX << "," << maxX << "," << minY << "," << maxY << "</Fill>" << endl;
    }
    else
    {
        out << "<Fill singleton = \"t\">0,0,0,0</Fill>";
    }
    QPointF t1 = tiling->getData().getTrans1();
    QPointF t2 = tiling->getData().getTrans2();
    out << "<T1>" <<  t1.x() << "," << t1.y() << "</T1>" << endl;
    out << "<T2>" <<  t2.x() << "," << t2.y() << "</T2>" << endl;

    writeViewSettings(out);

    //structure is tile then placements. so tile is not duplicated. I dont know if this adds any value
    for (auto& apair : fgroup)
    {
        TilePtr tile                    = apair.first;
        PlacedTiles & placedTiles  = apair.second;
        PlacedTilePtr placedTile   = placedTiles .first();

        if (placedTile->isGirihShape())    // TODO verify this code works
        {
            // saved girih shapesd have a translation
            out << "<Feature type=\"girih\" name=\"" << placedTile->getGirihShapeName() << "\">" << endl;

        }
        else if (tile->isRegular())
        {
            // regular features have sides and rotation
            out << "<Feature type=\"regular\" sides=\"" << tile->numPoints() << "\" rotation=\"" << tile->getRotation() << "\" scale=\"" << tile->getScale() << "\">" << endl;
        }
        else
        {
            // edge polys have rotation, numSides can be calculated
            out << "<Feature type=\"edgepoly\" rotation=\"" << tile->getRotation() << "\" scale=\"" << tile->getScale() << "\">" << endl;
            EdgePoly epoly = tile->getEdgePoly();
            setEdgePoly(out,epoly);
        }

        // background colors
        ColorSet * tileColors  = tile->getTileColors();
        int sz = tileColors->size();
        if (sz)
        {
            QString s = "<BkgdColors>";
            for (int i = 0; i < (sz-1); i++)
            {
                QColor color = tileColors->getColor(i).color;
                s += color.name();
                s += ",";
            }
            QColor color = tileColors->getColor(sz-1).color;
            s += color.name();
            s += "</BkgdColors>";
            out << s << endl;
        }

        for(auto it= placedTiles .begin(); it != placedTiles .end(); it++ )
        {
            PlacedTilePtr & pf = *it;
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

    writeBackgroundImage(out);

    out << "</Tiling>" << endl;
}

void TilingWriter::writeBackgroundImage(QTextStream & out)
{
    auto bkgd = BackgroundImageView::getInstance();
    if (bkgd->isLoaded())
    {
        QString astring = QString("<BackgroundImage name=\"%1\">").arg(bkgd->getName());
        out << astring << endl;

        const Xform & xform = bkgd->getCanvasXform();
        out << "<Scale>" << xform.getScale()           << "</Scale>" << endl;
        out << "<Rot>"   << xform.getRotateRadians()   << "</Rot>"  << endl;
        out << "<X>"     << xform.getTranslateX()      << "</X>" << endl;
        out << "<Y>"     << xform.getTranslateY()      << "</Y>" << endl;
        QPointF center = xform.getModelCenter();
        out << "<Center>" << center.x() << "," << center.y() << "</Center>"<< endl;

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

    QSize size = tiling->getData().getSettings().getSize();
    out << "<width>"  << size.width()  << "</width>" << endl;
    out << "<height>" << size.height() << "</height>" << endl;

    QSize zsize = tiling->getData().getSettings().getZSize();
    out << "<zwidth>"  << zsize.width()  << "</zwidth>" << endl;
    out << "<zheight>" << zsize.height() << "</zheight>" << endl;

    MosaicWriter::procesToolkitGeoLayer(out,tiling->getCanvasXform(),0);

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

