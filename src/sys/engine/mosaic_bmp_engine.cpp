#include <QDebug>
#include <QFile>
#include <QPainter>
#include "sys/engine/mosaic_bmp_engine.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_reader.h"
#include "sys/sys/fileservices.h"
#include "gui/top/view_controller.h"

/*
 * Thwe Engine runs in its own thread and so must not make any calls
 * to functions which use the GUI.  (signals and slots are safe)
 *
 */

MosaicBMPEngine::MosaicBMPEngine()
{
    // using this local ViewBontroller makes the engine thread-safe
    viewController = new ViewController;
    viewController->init(nullptr);      // disbles view
}

MosaicBMPEngine::~MosaicBMPEngine()
{
    delete viewController;
}

bool MosaicBMPEngine::saveBitmap(VersionedName name, QString pixmapPath)
{
    //qDebug() << "MosaicBMPEngine::saveBitmap" << name << "BEGIN";
    MosaicPtr mosaic;
    if (loadMosaic(mosaic,name))
    {
        QSize sz = mosaic->getCanvasSettings().getViewSize();
        QImage image(sz,QImage::Format_RGB32);
        QColor color = mosaic->getCanvasSettings().getBackgroundColor();
        image.fill(color);
        buildImage(mosaic,image);
        savePixmap(image,name.get(),pixmapPath);
        //qDebug() << "MosaicBMPEngine::saveBitmap" << name << "END";
        return true;
    }
    qWarning() << "MosaicBMPEngine::saveBitmap" << name.get() << "FAILED";
    return false;
}

bool MosaicBMPEngine::loadMosaic(MosaicPtr & mosaic, VersionedName name)
{
    qDebug().noquote() << "MosaicBMPEngine::loadMosaic()" << name.get();

    VersionedFile file = FileServices::getFile(name,FILE_MOSAIC);
    if (file.isEmpty())
    {
        qDebug() << "mosaic file not found:" << name.get();
        return false;
    }

    QFile afile(file.getPathedName());
    if (!afile.exists())
    {
        qDebug() << "mosaic file does not exist:" << file.getPathedName();
        return false;
    }

    qDebug().noquote() << "Loading:"  << file.getPathedName();

    // load
    MosaicReader reader;
    mosaic = reader.readXML(file);

    if (!mosaic)
    {
        qDebug()  << "Load ERROR" << reader.getFailMessage();
        return false;
    }

    mosaic->setName(name);

    // size view to mosaic
    // this uses a view conroller which is not attached to the view
    CanvasSettings csettings = mosaic->getCanvasSettings();

    Canvas  & canvas = viewController->getCanvas();
    canvas.reInit();
    canvas.setModelAlignment(M_ALIGN_MOSAIC);
    canvas.initCanvasSize(csettings.getCanvasSize());

    viewController->setBackgroundColor(VIEW_MOSAIC,csettings.getBackgroundColor());

    return true;
}

void MosaicBMPEngine::buildImage(MosaicPtr & mosaic, QImage & image)
{
    Q_ASSERT(mosaic);
    qDebug() << "Image size" << image.size();

    // build using the local ViewController
    mosaic->build(viewController);

    QPainter painter(&image);
    mosaic->enginePaint(&painter);
}

void MosaicBMPEngine::savePixmap(QImage &image, QString name, QString pixmapPath)
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
