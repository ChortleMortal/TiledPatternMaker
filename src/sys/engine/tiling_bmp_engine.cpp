#include <QDebug>
#include <QFile>
#include <QPainter>
#include "sys/engine/tiling_bmp_engine.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling_reader.h"
#include "model/mosaics/mosaic_reader.h"
#include "sys/sys/fileservices.h"
#include "model/settings/configuration.h"
#include "gui/top/view_controller.h"

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

bool TilingBMPEngine::saveBitmap(VersionedName name, QString pixmapPath)
{
    //qDebug() << "TilingBMPEngine::saveBitmap" << name << "BEGIN";

    TilingPtr tp;
    if (loadTiling(tp,name))
    {
        QSize sz = tp->getData().getCanvasSettings().getViewSize();
        QImage image(sz,QImage::Format_RGB32);
        image.fill(Qt::white);
        buildImage(tp,image);
        savePixmap(image,name,pixmapPath);
        //qDebug() << "TilingBMPEngine::saveBitmap" << name << "END";
        return true;
    }

    qWarning() << "TilingBMPEngine::saveBitmap" << name.get() << "FAIL";
    return false;
}

bool TilingBMPEngine::loadTiling(TilingPtr & tp, VersionedName name)
{
    qDebug().noquote() << "TilingBMPEngine::loadMosaic()" << name.get();

    VersionedFile file = FileServices::getFile(name,FILE_TILING);
    if (file.isEmpty())
    {
        qDebug() << "tiling file not found:" << name.get();
        return false;
    }

    QFile afile(file.getPathedName());
    if (!afile.exists())
    {
        qDebug() << "tiling file does not exist:" << name.get();
        return false;
    }

    qDebug().noquote() << "Loading:"  << file.getPathedName();

    // load
    TilingReader tl;
    tp = tl.readTilingXML(file);

    if (!tp)
    {
        qWarning().noquote() << "Load Error loading" << file.getPathedName();
        return false;
    }
    
    tp->setName(name);

    // size view to mosaic
    Canvas  & canvas = viewController->getCanvas();
    const CanvasSettings & cs  = tp->getData().getCanvasSettings();
    canvas.reInit();
    canvas.setModelAlignment(M_ALIGN_MOSAIC);
    canvas.initCanvasSize(cs.getCanvasSize());

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

void TilingBMPEngine::savePixmap(QImage &image, VersionedName name, QString pixmapPath)
{
    QPixmap pixmap;
    pixmap.convertFromImage(image);
    QString file   = pixmapPath + "/" + name.get() + ".bmp";
    qInfo() << "saving" << file;

    bool rv = pixmap.save(file);
    if (!rv)
        qDebug() << file << "save ERROR";
}
