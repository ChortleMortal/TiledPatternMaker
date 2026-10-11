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

#include "model/tilings/placed_tile.h"
#include "model/tilings/tile.h"
#include "model/tilings/tile_writer.h"
#include "model/tilings/tile_reader.h"
#include "model/tilings/tiling.h"
#include "sys/sys/fileservices.h"
#include "sys/qt/tpm_io.h"
#include "sys/geometry/transform.h"
#include "model/makers/tiling_maker.h"

using std::make_shared;

PlacedTile::PlacedTile() {}

PlacedTile::PlacedTile(TilePtr tile, QTransform T)
{
    this->tile  = tile;
    this->placement = T;
}

bool PlacedTile::operator == (const PlacedTile & other)
{
    if (placement != other.placement)
        return false;

    Tile & thisTile = *tile.get();
    Tile & otherTile = *other.tile.get();
    if (thisTile != otherTile)
        return false;

    return true;
}

PlacedTilePtr PlacedTile::copy()
{
    PlacedTilePtr pfp    = make_shared<PlacedTile>();
    pfp->girihShapeName  = girihShapeName;
    pfp->placement       = placement;
    pfp->tile            = tile->copy();
    pfp->state           = state;
    return pfp;
}

void PlacedTile::setPlacement(QTransform newT)
{
    //qDebug() << "setTransform3 before =" << Transform::toInfoString(T);
    placement = newT;
    //qDebug() << "setTransform3 after =" << Transform::toInfoString(T);
    TilingPtr tiling = Sys::tilingMaker->getSelectedTiling();
    tiling->setTilingViewChanged();
}

void PlacedTile::setTile(TilePtr tile)
{
    this->tile = tile;
    TilingPtr tiling = Sys::tilingMaker->getSelectedTiling();
    tiling->setTilingViewChanged();
}

void PlacedTile::setShow(bool show)
{
    state.show = show;
    TilingPtr tiling = Sys::tilingMaker->getSelectedTiling();
    tiling->setTilingViewChanged();
}

bool PlacedTile::loadFromGirihShape(VersionedName vname)
{
    QString filename = Sys::girihDir + vname.get() + ".xml";

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
    girihShapeName = vname;

    TilingPtr tiling = Sys::tilingMaker->getSelectedTiling();
    if (tiling)
    {
        tiling->setTilingViewChanged();
    }
    return true;
}

TilePtr PlacedTile::getTile()
{
    return tile;
}

QTransform PlacedTile::getPlacement()
{
    return placement;
}

QTransform PlacedTile::getWorldTransform(const QTransform & distortion)
{
    // distortion first, in the tile's own local frame, placement (position) last -
    // this is what keeps the tile in situ (placing-then-distorting would scale every
    // tile's position away from the distortion's fixed origin, sliding the whole
    // tiling instead of warping tiles where they sit)
    return distortion * placement;
}

QPolygonF  PlacedTile::getWorldPoints(const QTransform & distortion)
{
    return getWorldTransform(distortion).map(tile->getPoints());
}

EdgePoly PlacedTile::getWorldEdgePoly(const QTransform & distortion)
{
    const EdgePoly & ep = tile->getEdgePoly();
    return ep.map(getWorldTransform(distortion));
}

bool PlacedTile::saveAsGirihShape(VersionedName vname)
{
    QString filename = Sys::girihDir + vname.get() + ".xml";

    VersionedFile xfile;
    xfile.setFromFullPathname(filename);

    QFile data(filename);
    if (data.open(QFile::WriteOnly))
    {
        qDebug() << "Writing:"  << data.fileName();
        QTextStream str(&data);
        str.setRealNumberPrecision(16);
        saveGirihShape(str,vname);
        data.close();

        if (FileServices::reformatXML(xfile))
        {
            girihShapeName = vname;
            return true;
        }
    }
    qWarning() << "Could not write tile file:"  << filename;

    return false;
}

void PlacedTile::saveGirihShape(QTextStream & ts, VersionedName vname)
{
    TileWriter tileWriter;

    ts << "<?xml version=\"1.0\"?>" << endl;

    if (tile->isRegular())
    {
        QString str = QString("<Poly name=\"%1\" type=\"regular\" sides=\"%2\">").arg(vname.get()).arg(tile->numPoints());
        ts << str << endl;
    }
    else
    {
        QString str = QString("<Poly name=\"%1\">").arg(vname.get());
        ts << str << endl;
        tileWriter.setEdgePoly(ts,tile->getEdgePoly());
    }
    tileWriter.setTransform(ts,placement);
    ts << "</Poly>" << endl;
}

void PlacedTile::loadGirihShape(xml_node & poly_node)
{
    ReaderBase mrbase;
    TileReader tr(&mrbase);
    EdgeSet eset = tr.getEdgeSet(poly_node);
    tile         = make_shared<Tile>(eset);
    placement    = tr.getTransform(poly_node);
}

void PlacedTile::loadGirihShape(int sides, pugi::xml_node & poly_node)
{
    ReaderBase mrbase;
    TileReader tr(&mrbase);
    tile        = make_shared<Tile>(sides,0);
    placement   = tr.getTransform(poly_node);
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
    tile = make_shared<Tile>(epoly);
}

QString PlacedTile::info()
{
    return tile->summary() + state.info();
}

void PlacedTile::dump()
{
    qDebug().noquote() << tile->summary()  << Transform::info(placement);
    getTile()->dumpPts();
}

QString  PlacedTileState::info()
{
    QString astring;
    QTextStream ts(&astring);
    if (show)
        ts << "show ";
    if (overlapping)
        ts << "overlapping ";
    if (touching)
        ts << "touching ";
    switch(excluded.get())
    {
    case Tristate::eTristate::True:
        ts << "state=excluded ";
        break;
    case Tristate::eTristate::False:
        ts << "state=included ";
        break;
    case Tristate::eTristate::Undefined:
        ts << "state=undefined ";
        break;
    }
    if (unique)
        ts << "unique ";
    return astring;
}

QString PlacedTileSet::info()
{
    int undefined = 0;
    int included  = 0;
    int excluded  = 0;

    for (PlacedTilePtr ptp : *this)
    {
        switch(ptp->getState().excluded.get())
        {
        case Tristate::eTristate::True:
            excluded++;
            break;
        case Tristate::eTristate::False:
            included++;
            break;
        case Tristate::eTristate::Undefined:
            undefined++;
            break;
        }
    }
    QString s = QString("included=%1 excluded=%2 undefined=%3").arg(included).arg(excluded).arg(undefined);
    return s;
}
