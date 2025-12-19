#include <QDebug>
#include <QFile>
#include <QPainter>

#include "sys/engine/tiling_bmp_generator.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling_manager.h"
#include "model/tilings/tiling_reader.h"
#include "model/mosaics/mosaic_reader.h"
#include "model/tilings/tiling_writer.h"
#include "sys/sys/fileservices.h"
#include "gui/top/system_view_controller.h"

/*
 * Thwe Engine runs in its own thread and so must not make any calls
 * to functions which use the GUI.  (signals and slots are safe)
 *
 */

TilingBMPGenerator::TilingBMPGenerator()
{
    // using this local ViewBontroller makes the engine thread-safe
    bmpViewController = new SystemViewController;
    bmpViewController->disableAllViews();
}

TilingBMPGenerator::~TilingBMPGenerator()
{
    delete bmpViewController;
}

bool TilingBMPGenerator::saveBitmap(VersionedName vname, QString pixmapPath)
{
    //qDebug() << "TilingBMPEngine::saveBitmap" << name << "BEGIN";

    TilingPtr tp = loadTiling(vname);
    if (tp)
    {
        QSize sz = tp->hdr().getCanvasSettings().getViewSize();
        QImage image(sz,QImage::Format_RGB32);
        image.fill(Qt::white);
        buildImage(tp,image);
        savePixmap(image,vname,pixmapPath);
        //qDebug() << "TilingBMPEngine::saveBitmap" << name << "END";

#ifdef LEGACY_CONVERT_XML
        if (tp->legacyModelConverted())
        {
            VersionedFile vfile = FileServices::getFile(vname,FILE_TILING);
            TilingWriter tw;
            tw.writeTilingXML(vfile,tp);
        }
#endif
        return true;
    }
    else
    {
        qWarning() << "TilingBMPEngine::saveBitmap" << vname.get() << "FAIL";
        return false;
    }
}

TilingPtr TilingBMPGenerator::loadTiling(VersionedName name)
{
    qDebug().noquote() << "TilingBMPEngine::loadMosaic()" << name.get();

    TilingPtr tiling;

    VersionedFile file = FileServices::getFile(name,FILE_TILING);
    if (file.isEmpty())
    {
        qDebug() << "tiling file not found:" << name.get();
        return tiling;
    }

    QFile afile(file.getPathedName());
    if (!afile.exists())
    {
        qDebug() << "tiling file does not exist:" << name.get();
        return tiling;
    }

    qDebug().noquote() << "Loading:"  << file.getPathedName();

    // load
    TilingReader reader(bmpViewController);
    ReaderBase mrbase;
    tiling = reader.readTilingXML(file,&mrbase);

    if (!tiling)
    {
        qWarning().noquote() << "Load Error loading" << file.getPathedName();
        return tiling;
    }
    
    tiling->setVName(name);

    // size view to mosaic
    Canvas  & canvas = bmpViewController->getCanvas();
    const CanvasSettings & cs  = tiling->hdr().getCanvasSettings();
    canvas.setCanvasSize(cs.getCanvasSize());

    bmpViewController->setSelectedPrimaryLayer(tiling);

    return tiling;
}

void TilingBMPGenerator::buildImage(TilingPtr &tp, QImage & image)
{
    Q_ASSERT(tp);
    qDebug() << "Image size" << image.size();

    tp->setViewController(bmpViewController);

    QPainter painter(&image);
    tp->paint(&painter);
}

void TilingBMPGenerator::savePixmap(QImage &image, VersionedName name, QString pixmapPath)
{
    QPixmap pixmap;
    pixmap.convertFromImage(image);
    QString file   = pixmapPath + "/" + name.get() + ".bmp";
    qInfo() << "saving" << file;

    bool rv = pixmap.save(file);
    if (!rv)
        qDebug() << file << "save ERROR";
}
