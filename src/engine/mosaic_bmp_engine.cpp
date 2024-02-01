#include <QDebug>
#include <QFile>
#include <QPainter>
#include "engine/mosaic_bmp_engine.h"
#include "mosaic/mosaic.h"
#include "mosaic/mosaic_reader.h"
#include "misc/fileservices.h"
#include "viewers/view_controller.h"

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

bool MosaicBMPEngine::saveBitmap(QString name, QString pixmapPath)
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
        savePixmap(image,name,pixmapPath);
        //qDebug() << "MosaicBMPEngine::saveBitmap" << name << "END";
        return true;
    }
    qWarning() << "MosaicBMPEngine::saveBitmap" << name << "FAIL";
    return false;
}

bool MosaicBMPEngine::loadMosaic(MosaicPtr & mosaic, QString name)
{
    qDebug().noquote() << "MosaicBMPEngine::loadMosaic()" << name;

    QString file = FileServices::getMosaicXMLFile(name);
    if (file.isEmpty())
    {
        qDebug() << "mosaic file not found:" << name;
        return false;
    }

    QFile afile(file);
    if (!afile.exists())
    {
        qDebug() << "mosaic file does not exist:" << name;
        return false;
    }

    qDebug().noquote() << "Loading:"  << file;

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
    Canvas  & canvas = viewController->getCanvas();
    canvas.reInit();
    canvas.setModelAlignment(M_ALIGN_MOSAIC);
    canvas.initCanvasSize(mosaic->getCanvasSettings().getCanvasSize());
    auto fd = mosaic->getCanvasSettings().getFillData();
    canvas.setFillData(fd);       // set fill data in view (prototype reads this)

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
