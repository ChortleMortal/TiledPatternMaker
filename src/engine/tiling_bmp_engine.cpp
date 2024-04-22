#include <QDebug>
#include <QFile>
#include <QPainter>
#include "engine/tiling_bmp_engine.h"
#include "tile/tiling.h"
#include "tile/tiling_reader.h"
#include "mosaic/mosaic_reader.h"
#include "misc/fileservices.h"
#include "settings/configuration.h"
#include "viewers/view_controller.h"

/*
 * Thwe Engine runs in its own thread and so must not make any calls
 * to functions which use the GUI.  (signals and slots are safe)
 *
 */

TilingBMPEngine::TilingBMPEngine()
{
    config  = Sys::config;

    // using this local ViewBontroller makes the engine thread-safe
    viewController = new ViewController;
    viewController->init(nullptr);      // disbles view
    viewController->disablePrimeViews();
}

TilingBMPEngine::~TilingBMPEngine()
{
    delete viewController;
}

bool TilingBMPEngine::saveBitmap(QString name, QString pixmapPath)
{
    //qDebug() << "TilingBMPEngine::saveBitmap" << name << "BEGIN";

    TilingPtr tp;
    if (loadTiling(tp,name))
    {
        QSize sz = tp->getData().getSettings().getViewSize();
        QImage image(sz,QImage::Format_RGB32);
        image.fill(Qt::white);
        buildImage(tp,image);
        savePixmap(image,name,pixmapPath);
        //qDebug() << "TilingBMPEngine::saveBitmap" << name << "END";
        return true;
    }

    qWarning() << "TilingBMPEngine::saveBitmap" << name << "FAIL";
    return false;
}

bool TilingBMPEngine::loadTiling(TilingPtr & tp, QString name)
{
    qDebug().noquote() << "TilingBMPEngine::loadMosaic()" << name;

    QString file = FileServices::getTilingXMLFile(name);
    if (file.isEmpty())
    {
        qDebug() << "tiling file not found:" << name;
        return false;
    }

    QFile afile(file);
    if (!afile.exists())
    {
        qDebug() << "tiling file does not exist:" << name;
        return false;
    }

    qDebug().noquote() << "Loading:"  << file;

    // load
    TilingReader tl;
    tp = tl.readTilingXML(file);

    if (!tp)
    {
        qWarning().noquote() << "Load Error loading" << file;
        return false;
    }
    
    tp->setTitle(name);

    // size view to mosaic
    Canvas  & canvas = viewController->getCanvas();
    const CanvasSettings & model  = tp->getData().getSettings();
    canvas.reInit();
    canvas.setModelAlignment(M_ALIGN_MOSAIC);
    canvas.initCanvasSize(model.getCanvasSize());

    FillData filldata= tp->getCanvasSettings().getFillData();
    canvas.setFillData(filldata);     // set fill data in view (prototype reads this)

    return true;
}

void TilingBMPEngine::buildImage(TilingPtr &tp, QImage & image)
{
    Q_ASSERT(tp);
    qDebug() << "Image size" << image.size();

    tp->setViewController(viewController);

    QPainter painter(&image);
    tp->paint(&painter);
}

void TilingBMPEngine::savePixmap(QImage &image, QString name, QString pixmapPath)
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
