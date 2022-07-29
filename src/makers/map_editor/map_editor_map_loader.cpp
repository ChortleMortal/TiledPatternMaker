#include "makers/map_editor/map_editor_map_loader.h"
#include "viewers/viewcontrol.h"
#include "tile/tiling_loader.h"

MapEditorMapLoader::MapEditorMapLoader()
{
    maptype = MAPED_TYPE_UNKNOWN;
}

MapEditorMapLoader::~MapEditorMapLoader()
{
    //qDebug() << "MapEditorMapLoader: destructor";
}

MapPtr MapEditorMapLoader::loadMosaicMap(QString fileName)
{
    MapPtr map;

    view->dump(true);

    qInfo().noquote() << "MosaicLoader loading map from:" << fileName;
    _fileName = fileName;

    xml_document doc;
    xml_parse_result result = doc.load_file(fileName.toStdString().c_str());

    if (result == false)
    {
        _failMessage = result.description();
        qWarning().noquote() << _failMessage;
        return map;
    }

    try
    {
        if (_debug) qDebug() << "MosaicLoader - start parsing";

        nRefrCnt = 0;

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
            procSize(sz, _width, _height,_zwidth, _zheight);
            view->frameSettings.initialise(VIEW_MAP_EDITOR,QSize(_width,_height),QSize(_zwidth,_zheight));
            view->frameSettings.initialiseCommon(QSize(_width,_height),QSize(_zwidth,_zheight));
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
            TilingLoader::getBackgroundImage(bi);
        }

        xml_node xmlmap = node.child("map");
        if (xmlmap)
        {
            map = getMap(node);
        }

        view->dump(true);
    }
    catch (...)
    {
        qWarning() << "ERROR processing XML file"  << fileName;
    }

    return map;
}

