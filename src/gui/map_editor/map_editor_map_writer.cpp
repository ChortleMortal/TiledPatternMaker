#include "gui/map_editor/map_editor_map_writer.h"
#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_db.h"
#include "sys/sys/fileservices.h"
#include "sys/qt/tpm_io.h"
#include "sys/sys.h"
#include "gui/viewers/backgroundimageview.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/map_editor_view.h"
#include "model/tilings/tiling_writer.h"

MapEditorMapWriter::MapEditorMapWriter(MapEditorView *view) : MosaicWriter()
{
    meView = view;
}

MapEditorMapWriter::~MapEditorMapWriter()
{
}

// this is used by MapEditor to save a map
bool MapEditorMapWriter::writeXML(VersionedFile xfile, MapPtr map, eMapEditorMapType mapType)
{
    qDebug() << "Writing XML:" << xfile.getPathedName();

    QFile xml(xfile.getPathedName());
    if (!xml.open(QFile::WriteOnly | QFile::Truncate))
    {
        _failMsg = QString("Could not open file to write: %1").arg(xfile.getPathedName());
        return false;
    }

    QTextStream ts(& xml);
    ts.setRealNumberPrecision(16);

    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;

    bool rv;
    try
    {
        refId = 0;
        qDebug() << "version=" << currentXMLVersion;
        QString qs = QString(" version=\"%1\"").arg(currentXMLVersion);
        ts << "<vector" << nextId() << qs << ">" << endl;

        if (MapEditorDb::isMotif(mapType))
        {
            ts << "<maptype>" << "motif" << "</maptype>" << endl;
        }

        // frame settings
        auto & canvas    = Sys::viewController->getCanvas();
        QSize size       = Sys::view->getSize();
        QSize zsize     = canvas.getSize();
        procSize(ts,size,zsize);

        // canvas settings
        const Xform & xf = meView->getModelXform();
        QString str = "ModelSettings";
        ts << "<" << str << ">" << endl;
        procesToolkitGeoLayer(ts,xf,0);
        ts << "</" << str << ">" << endl;

        // background
        TilingWriter::writeBackgroundImage(ts);

        // map
        rv = setMap(ts,map);
        if (!rv)
            return false;
        ts << "</vector>" << endl;

    }
    catch (...)
    {
        xml.close();
        _failMsg = QString("ERROR writing XML file: %1").arg(xfile.getPathedName());
        return false;
    }

    xml.close();

    if (!rv)
    {
        xml.close();
        _failMsg = QString("ERROR writing XML file: %1").arg(xfile.getPathedName());
        return false;
    }

    rv = FileServices::reformatXML(xfile);
    return rv;
}

