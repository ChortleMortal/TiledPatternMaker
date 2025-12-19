#include <QDebug>
#include <QFile>
#include <QPainter>
#include "gui/top/system_view_controller.h"
#include "model/mosaics/border.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_reader.h"
#include "model/mosaics/mosaic_writer.h"
#include "sys/engine/mosaic_bmp_generator.h"
#include "sys/sys/fileservices.h"

/*
 * Thwe Engine runs in its own thread and so must not make any calls
 * to functions which use the GUI.  (signals and slots are safe)
 *
 */

MosaicBMPGenerator::MosaicBMPGenerator()
{
    // using this local ViewController makes the engine thread-safe
    bmpViewController = new SystemViewController;
    bmpViewController->disableAllViews();
}

MosaicBMPGenerator::~MosaicBMPGenerator()
{
    delete bmpViewController;
}

bool MosaicBMPGenerator::saveBitmap(VersionedName vname, QString pixmapPath)
{
    //qDebug() << "MosaicBMPEngine::saveBitmap" << name << "BEGIN";

    MosaicPtr mosaic = loadMosaic(vname);
    if (mosaic)
    {
        QSize sz = mosaic->getCanvasSettings().getViewSize();
        QImage image(sz,QImage::Format_RGB32);
        QColor color = mosaic->getCanvasSettings().getBackgroundColor();
        image.fill(color);
        buildImage(mosaic,image);
        savePixmap(image,vname.get(),pixmapPath);
        //qDebug() << "MosaicBMPEngine::saveBitmap" << name << "END";

#ifdef LEGACY_CONVERT_XML
        if (mosaic->legacyModelConverted())
        {
            // this forces an overwrite
            VersionedFile mosaicFile = FileServices::getFile(vname,FILE_MOSAIC);
            MosaicWriter writer;
            writer.writeXML(mosaicFile,mosaic);
        }
#endif
        return true;
    }
    else
    {
        qWarning() << "MosaicBMPEngine::saveBitmap" << vname.get() << "FAILED";
        return false;
    }
}

MosaicPtr MosaicBMPGenerator::loadMosaic(VersionedName vname)
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

void MosaicBMPGenerator::buildImage(MosaicPtr & mosaic, QImage & image)
{
    Q_ASSERT(mosaic);
    qDebug() << "Image size" << image.size();

    // build using the local ViewController
    Q_ASSERT(!bmpViewController->isAttached());
    mosaic->setViewController(bmpViewController);
    mosaic->build();

    QPainter painter(&image);
    mosaic->enginePaint(&painter);
}

void MosaicBMPGenerator::savePixmap(QImage &image, QString name, QString pixmapPath)
{
    Q_ASSERT(!name.contains(".xml"));

    QPixmap pixmap;
    pixmap.convertFromImage(image);
    QString file   = pixmapPath + "/" + name + ".bmp";
    qInfo() << "saving" << file;

    bool rv = pixmap.save(file);
    if (!rv)
        qDebug() << file << "save ERROR";
}
