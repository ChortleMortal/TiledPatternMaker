#include <QMessageBox>

#include "model/tilings/tiling_writer.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/transform.h"
#include "model/makers/tiling_maker.h"
#include "sys/sys/fileservices.h"
#include "sys/sys.h"
#include "model/mosaics/mosaic_writer.h"
#include "gui/top/controlpanel.h"
#include "model/settings/configuration.h"
#include "model/tilings/backgroundimage.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/tiledpatternmaker.h"

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
using Qt::endl;
#else
#define endl Qt::endl
#endif

using namespace pugi;
using std::string;

extern class TiledPatternMaker * theApp;

//const int currentTilingXMLVersion = 3;  // 26JUL20 excludes FillData
//const int currentTilingXMLVersion = 4;  // 13SEP20 restore FillData
//const int currentTilingXMLVersion = 5;  // 09NOV20 new background image positioning
//const int currentTilingXMLVersion = 6;  // 14NOV20 <Placement> becomes <Transform>
//const int currentTilingXMLVersion = 7;  // 10JUN23 <Feature> becomes <Tile>
//const int currentTilingXMLVersion = 8;  // 28FEB24 Tile Colors have colors in mosaic not in XML
// const int currentTilingXMLVersion = 9;  // 29MAR24 ConcaveArcs have different arc center
   const int currentTilingXMLVersion = 10; // 15APR24 Tilings have translate origins


bool TilingWriter::writeTilingXML(TilingPtr tiling)
{
    this->tiling = tiling;

    VersionedFile xfile = FileServices::getFile(tiling->getName(),FILE_TILING);
    if (!xfile.isEmpty())
    {
        // file already exists
        bool isOriginal  = xfile.getPathOnly().contains("original");
        bool isNewTiling = xfile.getPathOnly().contains("new_tilings");

        QMessageBox msgBox(Sys::controlPanel);
        msgBox.setText(QString("The tiling %1 already exists").arg(tiling->getName().get()));
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
            VersionedName vname = FileServices::getNextVersion(FILE_TILING,tiling->getName());
            tiling->setName(vname);
            if (isOriginal)
            {
                xfile.setFromFullPathname(Sys::originalTileDir + vname.get() + ".xml");
            }
            else if (isNewTiling)
            {
                xfile.setFromFullPathname(Sys::newTileDir + vname.get() + ".xml");
            }
            else
            {
                xfile.setFromFullPathname(Sys::testTileDir + vname.get() + ".xml");
            }
        }
        // save drops thru
        Q_UNUSED(save)
    }
    else
    {
        // new file
        if (Sys::config->saveTilingTest)
        {
            xfile.setFromFullPathname(Sys::testTileDir + tiling->getName().get() + ".xml");
        }
        else
        {
            xfile.setFromFullPathname(Sys::newTileDir + tiling->getName().get() + ".xml");
        }
    }

    QFile data(xfile.getPathedName());
    if (data.open(QFile::WriteOnly))
    {
        qDebug() << "Writing:"  << data.fileName();
        QTextStream str(&data);
        str.setRealNumberPrecision(16);
        writeTilingXML(str);
        data.close();

        bool rv = FileServices::reformatXML(xfile);
        if (rv)
        {
            QMessageBox box(Sys::controlPanel);
            box.setIcon(QMessageBox::Information);
            box.setText(QString("Saved: %1 - OK").arg(data.fileName()));
            box.exec();

            emit Sys::tilingMaker->sig_tilingWritten();
            return true;
        }
    }

    qWarning() << "Could not write tile file:"  << data.fileName();
    QMessageBox box(Sys::controlPanel);
    box.setIcon(QMessageBox::Critical);
    box.setText(QString("Error saving: %1 - FAILED").arg(data.fileName()));
    box.exec();
    return false;

}

void TilingWriter::writeTilingXML(QTextStream & out)
{
    refId    = 0;
    vertex_ids.clear();

    out << "<?xml version=\"1.0\"?>" << endl;
    QString qs = QString("<Tiling version=\"%1\">").arg(currentTilingXMLVersion);
    out << qs << endl;

    // write file name into the tiling XML
    out << "<Name>" << tiling->getName().get() << "</Name>" << endl;

    // fill paratmeters not part of original taprats
    int minX,minY,maxX,maxY;
    bool singleton;
    tiling->getCanvasSettings().getFillData().get(singleton,minX,maxX,minY,maxY);
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
    QPointF origin = tiling->getTranslateOrigin();
    out << "<T0>" <<  origin.x() << "," << origin.y() << "</T0>" << endl;
    out << "<T1>" <<  t1.x() << "," << t1.y() << "</T1>" << endl;
    out << "<T2>" <<  t2.x() << "," << t2.y() << "</T2>" << endl;

    writeViewSettings(out);

    // Regroup tiles by their translation so that we write each tile only once.
    TilingUnit & group = tiling->getTilingUnit();

    //structure is tile then placements. so tile is not duplicated. I dont know if this adds any value
    for (auto & apair : group)
    {
        TilePtr tile                = apair.first;
        TilingPlacements placemnets = apair.second;
        PlacedTilePtr placedTile    = placemnets.first();

        if (placedTile->isGirihShape())    // TODO verify this code works
        {
            // saved girih shapesd have a translation
            out << "<Tile type=\"girih\" name=\"" << placedTile->getGirihShapeName().get() << "\">" << endl;
        }
        else if (tile->isRegular())
        {
            // regular Tiles have sides and rotation
            out << "<Tile type=\"regular\" sides=\"" << tile->numPoints() << "\" rotation=\"" << tile->getRotation() << "\" scale=\"" << tile->getScale() << "\">" << endl;
        }
        else
        {
            // edge polys have rotation, numSides can be calculated
            out << "<Tile type=\"edgepoly\" rotation=\"" << tile->getRotation() << "\" scale=\"" << tile->getScale() << "\">" << endl;
            const EdgePoly epoly = tile->getBase();       // bugfx 20AUG23
            setEdgePoly(out,epoly);
        }

        for(auto it= placemnets .begin(); it != placemnets .end(); it++ )
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

        out << "</Tile>" << endl;
    }

    out << "<Desc>" << tiling->getDescription() << "</Desc>" << endl;
    out << "<Auth>" << tiling->getAuthor() << "</Auth>" << endl;

    writeBackgroundImage(out);

    out << "</Tiling>" << endl;
}

void TilingWriter::writeBackgroundImage(QTextStream & out)
{
    auto bkgd  = Sys::backgroundImageView->getImage();
    if (bkgd && bkgd->isLoaded())
    {
        QString astring = QString("<BackgroundImage name=\"%1\">").arg(bkgd->getTitle());
        out << astring << endl;
        
        const Xform & xform = Sys::backgroundImageView->getModelXform();
        out << "<Scale>" << xform.getScale()           << "</Scale>" << endl;
        out << "<Rot>"   << xform.getRotateRadians()   << "</Rot>"  << endl;
        out << "<X>"     << xform.getTranslateX()      << "</X>" << endl;
        out << "<Y>"     << xform.getTranslateY()      << "</Y>" << endl;
        QPointF center = xform.getModelCenter();
        out << "<Center>" << center.x() << "," << center.y() << "</Center>"<< endl;

        if (bkgd->useAdjusted())
        {
            out << "<Perspective>";
            out << Transform::writeInfo(bkgd->getAdjustedTransform());
            out << "</Perspective>" << endl;
        }

        out << "</BackgroundImage>" << endl;
    }
}

void TilingWriter::writeViewSettings(QTextStream & out)
{
    out << "<ViewSettings>" <<  endl;

    QSize size = tiling->getData().getCanvasSettings().getViewSize();
    out << "<width>"  << size.width()  << "</width>" << endl;
    out << "<height>" << size.height() << "</height>" << endl;
    
    QSize zsize = tiling->getData().getCanvasSettings().getCanvasSize();
    out << "<zwidth>"  << zsize.width()  << "</zwidth>" << endl;
    out << "<zheight>" << zsize.height() << "</zheight>" << endl;
    
    MosaicWriter::procesToolkitGeoLayer(out,tiling->getModelXform(),0);

    out << "</ViewSettings>" <<  endl;
}

void TilingWriter::setEdgePoly(QTextStream & ts, const EdgePoly & epoly)
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

