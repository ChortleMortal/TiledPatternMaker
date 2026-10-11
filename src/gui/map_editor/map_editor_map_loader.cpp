#include "gui/map_editor/map_editor_map_loader.h"
#include "gui/top/view_controller.h"
#include "gui/viewers/map_editor_view.h"
#include "model/tilings/backgroundimage.h"
#include "model/tilings/tiling_reader.h"
#include "sys/sys.h"

MapEditorMapLoader::MapEditorMapLoader() : MosaicReader(Sys::systemViewController)
{
    maptype = MapType::None;
}

MapEditorMapLoader::~MapEditorMapLoader()
{
    //qDebug() << "MapEditorMapLoader: destructor";
}

MapPtr MapEditorMapLoader::loadMosaicMap(VersionedFile xfile)
{
    MapPtr map;
    
    Sys::dumpRefs();

    _xfile = xfile;
    qInfo().noquote() << "MapEditorMapLoader loading map from:" << _xfile.getPathedName();

    xml_document doc;
    xml_parse_result result = doc.load_file(_xfile.getPathedName().toStdString().c_str());

    if (result == false)
    {
        _failMessage = result.description();
        qWarning().noquote() << _failMessage;
        return map;
    }

    try
    {
        if (_debug) qDebug() << "MapEditorMapLoader - start parsing";

        xml_node node = doc.first_child();
        string str = node.name();
        if (_debug) qDebug().noquote() << str.c_str();
        if (str  != "vector")
        {
            return map;
        }

        xml_attribute attr = node.attribute("version");
        if (attr)
        {
            QString str = attr.value();
            _version = str.toUInt();
        }

        maptype = MapType::File;
        xml_node mtype = node.child("maptype");
        if (mtype)
        {
            QString val = mtype.child_value();
            if (val == "motif")
            {
                maptype = MapType::FileMotif;
            }
        }

        xml_node sz = node.child("size");
        if (sz)
        {
            _viewSize   = procViewSize(sz);
            _canvasSize = procCanvasSize(sz,_viewSize);

            Canvas * canvas = Sys::getActiveCanvas();
            canvas->setCanvasSize(_canvasSize);
            canvas->setViewSize(_viewSize);
        }

        xml_node ca = node.child("ModelSettings");
        if (ca)
        {
            Layer::ZLevel unused;
            QPointF unused2;
            procesToolkitGeoLayer(ca,_xf,unused,unused2);
        }

        xml_node xmlmap = node.child("map");
        if (xmlmap)
        {
            map = getMap(node);
        }
        
        Sys::dumpRefs();
    }
    catch (...)
    {
        qWarning() << "ERROR processing XML file"  << _xfile.getPathedName();
    }

    return map;
}

