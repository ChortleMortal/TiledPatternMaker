////////////////////////////////////////////////////////////////////////////
//
// placedTile.java
//
// A placedTile is a Tile together with a transform matrix.
// It allows us to share an underlying feature while putting several
// copies together into a tiling.  A tiling is represented as a
// collection of placedTiles (that may share Tile) that together
// make up a translational unit.

#include <QFile>
#include <QDebug>

//#include "geometry/transform.h"
#include "settings/configuration.h"
#include "tile/placed_tile.h"
#include "tile/tile.h"
#include "tile/tile_writer.h"
#include "tile/tile_reader.h"
#include "tile/tiling.h"
#include "misc/fileservices.h"
#include "misc/tpm_io.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/tiling_maker/tiling_monitor.h"

using std::make_shared;

PlacedTile::PlacedTile()
{
    clearViewState();
    _show = true;

    connect(this, &PlacedTile::sig_tileChanged, Sys::tilingMaker->getTilingMonitor(), &TilingMonitor::slot_tileChanged);
}

PlacedTile::PlacedTile(TilePtr tile, QTransform T)
{
    this->tile = tile;
    this->T       = T;
    clearViewState();
    _show = true;
    //qDebug() << "setTransform1=" << Transform::toInfoString(T);

    connect(this, &PlacedTile::sig_tileChanged, Sys::tilingMaker->getTilingMonitor(), &TilingMonitor::slot_tileChanged);
}

PlacedTilePtr PlacedTile::copy()
{
    PlacedTilePtr pfp    = make_shared<PlacedTile>();
    pfp->girihShapeName  = girihShapeName;
    pfp->T               = T;
    pfp->tile            = tile->copy();

    return pfp;
}

void PlacedTile::setTransform(QTransform newT)
{
    //qDebug() << "setTransform3 before =" << Transform::toInfoString(T);
    T = newT;
    //qDebug() << "setTransform3 after =" << Transform::toInfoString(T);
    emit sig_tileChanged();
}

void PlacedTile::setTile(TilePtr tile)
{
    this->tile = tile;
    emit sig_tileChanged();
}

void PlacedTile::setShow(bool show)
{
    _show = show;
    emit sig_tileChanged();
}

bool PlacedTile::loadFromGirihShape(QString name)
{
    QString root     = Sys::config->rootMediaDir;
    QString filename = root + "girih_shapes" + "/" + name + ".xml";

    xml_document doc;
    xml_parse_result result = doc.load_file(filename.toStdString().c_str());
    if (result == false)
    {
        qWarning() << "Badly constructed Girih Shape XML file" << filename;
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

    emit sig_tileChanged();

    return true;
}

TilePtr PlacedTile::getTile()
{
    return tile;
}

QTransform PlacedTile::getTransform()
{
    return T;
}

EdgePoly PlacedTile::getPlacedEdgePoly()
{
    const EdgePoly & ep = tile->getEdgePoly();
    EdgePoly ep2        = ep.map(T);
    return ep2;
}

bool PlacedTile::saveAsGirihShape(QString name)
{
    QString root     = Sys::config->rootMediaDir;
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

void PlacedTile::saveGirihShape(QTextStream & ts, QString name)
{
    TileWriter fw;

    ts << "<?xml version=\"1.0\"?>" << endl;

    if (tile->isRegular())
    {
        QString str = QString("<Poly name=\"%1\" type=\"regular\" sides=\"%2\">").arg(name).arg(tile->numPoints());
        ts << str << endl;
    }
    else
    {
        QString str = QString("<Poly name=\"%1\">").arg(name);
        ts << str << endl;
        fw.setEdgePoly(ts,tile->getEdgePoly());
    }
    fw.setTransform(ts,T);
    ts << "</Poly>" << endl;
}

void PlacedTile::loadGirihShape(xml_node & poly_node)
{
    TileReader tr;
    EdgePoly ep = tr.getEdgePoly(poly_node);
    tile        = make_shared<Tile>(ep,0);
    T           = tr.getTransform(poly_node);
}

void PlacedTile::loadGirihShape(int sides, pugi::xml_node & poly_node)
{
    TileReader tr;
    tile     = make_shared<Tile>(sides,0);
    T        = tr.getTransform(poly_node);
}

void PlacedTile::loadGirihShapeOld(xml_node & poly_node)
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
    tile = make_shared<Tile>(epoly,0);
}

const EdgePoly & PlacedTile::getTileEdgePoly()
{
    return tile->getEdgePoly();
}

EdgePoly & PlacedTile::getTileEdgePolyRW()
{
    return tile->getEdgePolyRW();
}

QPolygonF  PlacedTile::getTilePoints()
{
    return tile->getPoints();
}

QPolygonF  PlacedTile::getPlacedPoints()
{
    return T.map(tile->getPoints());
}
