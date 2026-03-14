#include <QDebug>
#include <QFile>
#include <QPainter>
#include "gui/top/system_view_controller.h"
#include "model/borders/border.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_reader.h"
#include "model/mosaics/mosaic_writer.h"
#include "sys/engine/mosaic_xml_regenerator.h"
#include "sys/sys/fileservices.h"
#include "model/makers/mosaic_maker.h"
/*
 * Thwe Engine runs in its own thread and so must not make any calls
 * to functions which use the GUI.  (signals and slots are safe)
 *
 */

MosaicXMLRegenerator::MosaicXMLRegenerator()
{
    // using this local ViewController makes the engine thread-safe
    bmpViewController = new SystemViewController;
    bmpViewController->disableAllViews();
}

MosaicXMLRegenerator::~MosaicXMLRegenerator()
{
    delete bmpViewController;
}

bool MosaicXMLRegenerator::regemerateXML(VersionedName vname)
{
    //qDebug() << "MosaicBMPEngine::saveBitmap" << name << "BEGIN";

    MosaicPtr mosaic = loadMosaic(vname);
    if (mosaic)
    {
        mosaic->dumpStyles();
        mosaic->build();
        saveMosaic(mosaic,true);
    }
    else
    {
        qWarning() << "MosaicBMPEngine::saveBitmap" << vname.get() << "FAILED";
        return false;
    }
    return true;
}

MosaicPtr MosaicXMLRegenerator::loadMosaic(VersionedName vname)
{
    qDebug().noquote() << "MosaicBMPEngine::loadMosaic()" << vname.get();

    MosaicPtr mosaic;

    VersionedFile file = FileServices::getFile(vname,FILE_MOSAIC);
    if (file.isEmpty())
    {
        qDebug() << "mosaic file not found:" << vname.get();
        return mosaic;
    }

    QFile afile(file.getPathedName());
    if (!afile.exists())
    {
        qDebug() << "mosaic file does not exist:" << file.getPathedName();
        return mosaic;
    }

    qDebug().noquote() << "Loading:"  << file.getPathedName();

    // load
    MosaicReader reader(bmpViewController);
    mosaic = reader.readXML(file);

    if (!mosaic)
    {
        qDebug()  << "Load ERROR" << reader.getFailMessage();
        return mosaic;
    }

    mosaic->setName(vname);

    Q_ASSERT(!bmpViewController->isAttached());
    mosaic->setViewController(bmpViewController);

    // size view to mosaic
    // this uses a view conroller which is not attached to the view
    CanvasSettings csettings = mosaic->getCanvasSettings();

    Canvas  & canvas = bmpViewController->getCanvas();
    canvas.setCanvasSize(csettings.getCanvasSize());

    auto border = mosaic->getBorder();
    if (border)
    {
        // Note: version 17 uses model units
        // Note: version 23 includes model transform (toolkit.GeoLayer)

        if (mosaic->getLoadedXMLVersion() < 23)
        {
            auto style  = mosaic->getFirstRegularStyle();
            auto xf     = style->getModelXform();
            border->setModelXform(xf,true,Sys::nextSigid());
        }

        if (border->getRequiresConversion())
        {
            Q_ASSERT(mosaic->getLoadedXMLVersion() < 17);
            border->legacy_convertToModelUnits();
            border->setRequiresConversion(false);
            border->resetStyleRepresentation();
            border->createStyleRepresentation();
        }
    }

    bmpViewController->setBackgroundColor(VIEW_MOSAIC,csettings.getBackgroundColor());

    bmpViewController->setSelectedPrimaryLayer(mosaic->getFirstRegularStyle());

    return mosaic;
}

bool MosaicXMLRegenerator::saveMosaic(MosaicPtr mosaic, bool)
{
    qDebug() << "TiledPatternMaker::saveMosaic"  << mosaic->getName().get();

#if 0
    // match size of mosaic view
    auto & canvas    = Sys::viewController->getCanvas();
    QSize size       = canvas.getViewSize();
    QSize zsize      = canvas.getCanvasSize();

    CanvasSettings cs = _mosaic->getCanvasSettings();
    cs.setViewSize(size);
    cs.setCanvasSize(zsize);
    _mosaic->setCanvasSettings(cs);

#endif
    VersionedName vname = mosaic->getName();

    VersionedFile mosaicFile = FileServices::getFile(vname,FILE_MOSAIC);

    MosaicWriter writer;
    bool rv = writer.writeXML(mosaicFile,mosaic);
    if (!rv)
    {
        QString astring = QString("Save File (%1) FAILED").arg(mosaicFile.getPathedName());
        qWarning().noquote() << astring;
    }
    return rv;
}
