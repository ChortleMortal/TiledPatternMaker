#include "gui/map_editor/map_editor_map_loader.h"
#include "sys/sys.h"
#include "gui/top/view_controller.h"
#include "model/tilings/tiling_reader.h"

MapEditorMapLoader::MapEditorMapLoader()
{
    maptype = MAPED_TYPE_UNKNOWN;
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

        maptype = MAPED_LOADED_FROM_FILE;
        xml_node mtype = node.child("maptype");
        if (mtype)
        {
            QString val = mtype.child_value();
            if (val == "motif")
            {
                maptype = MAPED_LOADED_FROM_FILE_MOTIF;
            }
        }

        xml_node sz = node.child("size");
        if (sz)
        {
            _viewSize   = procViewSize(sz);
            _canvasSize = procCanvasSize(sz,_viewSize);

            ViewController * viewControl = Sys::viewController;
            auto & canvas = viewControl->getCanvas();
            canvas.initCanvasSize(_canvasSize);
        }

        xml_node ca = node.child("ModelSettings");
        if (ca)
        {
            int   unused;
            procesToolkitGeoLayer(ca,_xf,unused);
        }

        xml_node bi = node.child("BackgroundImage");
        if (bi)
        {
            _bip = TilingReader::getBackgroundImage(bi);
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

