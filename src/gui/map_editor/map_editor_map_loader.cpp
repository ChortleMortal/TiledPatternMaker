#include "gui/map_editor/map_editor_map_loader.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/map_editor_view.h"
#include "model/tilings/backgroundimage.h"
#include "model/tilings/tiling_reader.h"
#include "sys/sys.h"

MapEditorMapLoader::MapEditorMapLoader() : MosaicReader(Sys::viewController)
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

            SystemViewController * viewControl = Sys::viewController;
            auto & canvas = viewControl->getCanvas();
            canvas.setCanvasSize(_canvasSize);
        }

        xml_node ca = node.child("ModelSettings");
        if (ca)
        {
            int   unused;
            QPointF unused2;
            procesToolkitGeoLayer(ca,_xf,unused,unused2);
        }

        xml_node bi = node.child("BackgroundImage");
        if (bi)
        {
            _mosbip = getBackgroundImage(bi);
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

BkgdImagePtr MapEditorMapLoader::getBackgroundImage(xml_node & node)
{
    xml_attribute attr = node.attribute("name");
    QString name       = attr.value();

    BkgdImagePtr bip = std::make_shared<BackgroundImage>();
    bip->load(name);
    if (bip->isLoaded())
    {
        Xform xf;

        xml_node n = node.child("Scale");
        if (n)
        {
            QString str = n.child_value();
            xf.setScale(str.toDouble());
        }

        n = node.child("Rot");
        if (n)
        {
            QString str = n.child_value();
            xf.setRotateRadians(str.toDouble());
        }

        n = node.child("X");
        if (n)
        {
            QString str= n.child_value();
            xf.setTranslateX(str.toDouble());
        }

        n = node.child("Y");
        if (n)
        {
            QString str = n.child_value();
            xf.setTranslateY(str.toDouble());
        }

        n = node.child("Center");
        if (n)
        {
            QString str = n.child_value();
            QStringList qsl = str.split(",");
            qreal x = qsl[0].toDouble();
            qreal y = qsl[1].toDouble();
            QPointF bkgdCenter(x,y);
            if (!bkgdCenter.isNull())
            {
                qInfo() << "LOG2 correcting tiling background center" << bkgdCenter;

                auto tr = Sys::mapEditorView->getCanvasTransform();
                QPointF correction = tr.map(bkgdCenter);
                xf.applyTranslate(correction);
                //legacyCenterConverted = true;
            }
        }

        bip->setModelXform(xf,false,Sys::nextSigid());
        qDebug().noquote() << "map editor background image xform:" << xf.info();

        n= node.child("Perspective");
        bool usePerspective = false;
        if (n)
        {
            QString str = n.child_value();
            QTransform t = getQTransform(str);

            if (!t.isIdentity())
            {
                bip->setAdjustedTransform(t);
                bip->createAdjustedImage();
                usePerspective = true;
            }
        }
        bip->setUseAdjusted(usePerspective);
        bip->createPixmap();
    }
    return bip;
}
