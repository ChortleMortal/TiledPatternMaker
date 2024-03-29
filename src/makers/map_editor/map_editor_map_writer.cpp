﻿#include "makers/map_editor/map_editor_map_writer.h"
#include "makers/map_editor/map_editor.h"
#include "makers/map_editor/map_editor_db.h"
#include "misc/fileservices.h"
#include "misc/tpm_io.h"
#include "misc/sys.h"
#include "viewers/backgroundimageview.h"
#include "viewers/view_controller.h"
#include "viewers/map_editor_view.h"
#include "tile/tiling_writer.h"

MapEditorMapWriter::MapEditorMapWriter(MapEditorView *view) : MosaicWriter()
{
    meView = view;
}

MapEditorMapWriter::~MapEditorMapWriter()
{
}

// this is used by MapEditor to save a map
bool MapEditorMapWriter::writeXML(QString fileName, MapPtr map, eMapEditorMapType mapType)
{
    qDebug() << "Writing XML:" << fileName;
    _fileName = fileName;

    QFile xml(fileName);
    if (!xml.open(QFile::WriteOnly | QFile::Truncate))
    {
        _failMsg = QString("Could not open file to write: %1").arg(fileName);
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
        QSize size       = Sys::view->getCurrentSize();
        QSizeF zsize     = canvas.getSize();
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
        _failMsg = QString("ERROR writing XML file: %1").arg(fileName);
        return false;
    }

    xml.close();

    if (!rv)
    {
        xml.close();
        _failMsg = QString("ERROR writing XML file: %1").arg(fileName);
        return false;
    }

    rv = FileServices::reformatXML(fileName);
    return rv;
}

