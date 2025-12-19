#include <QMessageBox>

#include "gui/top/controlpanel.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/mosaic_writer.h"
#include "model/tilings/backgroundimage.h"
#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling_writer.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/transform.h"
#include "sys/sys.h"
#include "sys/sys/fileservices.h"
#include "sys/tiledpatternmaker.h"

using Qt::endl;
using namespace pugi;
using std::string;

extern class TiledPatternMaker * theApp;

//const int currentTilingXMLVersion = 3;  // 26JUL20 excludes FillData
//const int currentTilingXMLVersion = 4;  // 13SEP20 restore FillData
//const int currentTilingXMLVersion = 5;  // 09NOV20 new background image positioning
//const int currentTilingXMLVersion = 6;  // 14NOV20 <Placement> becomes <Transform>
//const int currentTilingXMLVersion = 7;  // 10JUN23 <Feature> becomes <Tile>
//const int currentTilingXMLVersion = 8;  // 28FEB24 Tile Colors have colors in mosaic not in XML
//const int currentTilingXMLVersion = 9;  // 29MAR24 ConcaveArcs have different arc center
//const int currentTilingXMLVersion = 10; // 15APR24 Tilings have translate origins
//const int currentTilingXMLVersion = 11; // 19OCT25 Restore saving of tile colors for legacy usage
//const int currentTilingXMLVersion = 12; // 19OCT25 No model centers and conversion of legacy XML
  const int currentTilingXMLVersion = 13; // 30OCT25 alignment of backgrounds and coversion of legacy XMKL


// called by TilingManager::saveTiling
bool TilingWriter::writeTilingXML(VersionedFile vfile, TilingPtr tiling)
{
    this->tiling = tiling;

    QFile data(vfile.getPathedName());
    if (data.open(QFile::WriteOnly))
    {
        qDebug() << "Writing:"  << data.fileName();
        QTextStream str(&data);
        str.setRealNumberPrecision(16);
        writeXML(str);
        data.close();

        bool rv = FileServices::reformatXML(vfile);
        if (rv)
        {
            return true;
        }
    }

    qWarning() << "Could not write tile file:"  << data.fileName();
    return false;
}

void TilingWriter::writeXML(QTextStream & out)
{
    refId    = 0;
    vertex_ids.clear();

    out << "<?xml version=\"1.0\"?>" << endl;
    QString qs = QString("<Tiling version=\"%1\">").arg(currentTilingXMLVersion);
    out << qs << endl;

    // write file name into the tiling XML
    out << "<Name>" << tiling->getVName().get() << "</Name>" << endl;

    // fill paratmeters not part of original taprats
    int minX,minY,maxX,maxY;
    bool singleton;
    tiling->hdr().getCanvasSettings().getFillData().get(singleton,minX,maxX,minY,maxY);
    if (!singleton)
    {
        out << "<Fill singleton = \"f\">" << minX << "," << maxX << "," << minY << "," << maxY << "</Fill>" << endl;
    }
    else
    {
        out << "<Fill singleton = \"t\">0,0,0,0</Fill>";
    }
    QPointF t1 = tiling->hdr().getTrans1();
    QPointF t2 = tiling->hdr().getTrans2();
    QPointF origin = tiling->hdr().getTranslateOrigin();
    out << "<T0>" <<  origin.x() << "," << origin.y() << "</T0>" << endl;
    out << "<T1>" <<  t1.x() << "," << t1.y() << "</T1>" << endl;
    out << "<T2>" <<  t2.x() << "," << t2.y() << "</T2>" << endl;

    writeViewSettings(out);

    // Regroup tiles by their translation so that we write each tile only once.
    TilingUnit tunit(tiling->unit());
    tunit.removeExcludeds();
    const QVector<UnitPlacedTiles> &  group = tunit.getUnitPlacedTiles();
    //structure is tile then placements. so tile is not duplicated. I dont know if this adds any value
    int index = 0;
    for (auto & apair : group)
    {
        TilePtr tile              = apair.first;
        PlacedTiles placemnets = apair.second;
        PlacedTilePtr placedTile  = placemnets.first();

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
            const EdgeSet & eset = tile->getBase();       // bugfx 20AUG23
            setEdgeSet(out,eset);
        }

        // legacy storage of background colors
        ColorGroup & cg = tiling->legacyTileColors();
        if (cg.size() > index)
        {
            ColorSet * tileColors  = & cg[index++];
            int sz = tileColors->size();
            if (sz)
            {
                QString s = "<BkgdColors>";
                for (int i = 0; i < (sz-1); i++)
                {
                    QColor color = tileColors->getQColor(i);
                    s += color.name();
                    s += ",";
                }
                QColor color = tileColors->getQColor(sz-1);
                s += color.name();
                s += "</BkgdColors>";
                out << s << endl;
            }
        }

        for(auto it= placemnets .begin(); it != placemnets .end(); it++ )
        {
            PlacedTilePtr & pf = *it;
            QTransform t = pf->getPlacement();
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

    auto bip = tiling->getBkgdImage();
    if (bip)
    {
        writeBackgroundImage(out,bip);
    }

    out << "</Tiling>" << endl;
}

void TilingWriter::writeBackgroundImage(QTextStream & out, BkgdImagePtr bip)
{
    QString astring = QString("<BackgroundImage name=\"%1\">").arg(bip->getTitle());
    out << astring << endl;

    const Xform & xform = bip->getModelXform();
    out << "<Scale>" << xform.getScale()           << "</Scale>" << endl;
    out << "<Rot>"   << xform.getRotateRadians()   << "</Rot>"  << endl;
    out << "<X>"     << xform.getTranslateX()      << "</X>" << endl;
    out << "<Y>"     << xform.getTranslateY()      << "</Y>" << endl;

    if (bip->useAdjusted())
    {
        out << "<Perspective>";
        out << Transform::writeInfo(bip->getAdjustedTransform());
        out << "</Perspective>" << endl;
    }

    out << "</BackgroundImage>" << endl;
}

void TilingWriter::writeViewSettings(QTextStream & out)
{
    out << "<ViewSettings>" <<  endl;

    QSize size = tiling->hdr().getCanvasSettings().getViewSize();
    out << "<width>"  << size.width()  << "</width>" << endl;
    out << "<height>" << size.height() << "</height>" << endl;
    
    QSize zsize = tiling->hdr().getCanvasSettings().getCanvasSize();
    out << "<zwidth>"  << zsize.width()  << "</zwidth>" << endl;
    out << "<zheight>" << zsize.height() << "</zheight>" << endl;
    
    procesToolkitGeoLayer(out,tiling->getModelXform(),0);

    out << "</ViewSettings>" <<  endl;
}

void TilingWriter::procesToolkitGeoLayer(QTextStream & ts, const Xform & xf, int zlevel)
{
    ts << "<left__delta>"  << xf.getTranslateX()      << "</left__delta>"  << endl;
    ts << "<top__delta>"   << xf.getTranslateY()      << "</top__delta>"   << endl;
    ts << "<width__delta>" << xf.getScale()           << "</width__delta>" << endl;
    ts << "<theta__delta>" << xf.getRotateRadians()   << "</theta__delta>" << endl;
    ts << "<Z>"            << zlevel                  << "</Z>"        << endl;
}

void TilingWriter::setEdgeSet(QTextStream & ts, const EdgeSet & eset)
{
    for (auto it = eset.begin(); it != eset.end(); it++)
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

