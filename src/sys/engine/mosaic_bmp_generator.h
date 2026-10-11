#pragma once

#include "sys/sys/versioning.h"
#include <QImage>
#include <QList>

typedef std::shared_ptr<class Tiling>        TilingPtr;
typedef std::shared_ptr<class Mosaic>        MosaicPtr;

class ViewController;

class MosaicBMPGenerator
{
public:
    MosaicBMPGenerator();
    ~MosaicBMPGenerator();

    bool        saveBitmap(VersionedName vname, QString pixmapPath);

protected:
    MosaicPtr   loadMosaic(VersionedName vname);
    void        savePixmap(QImage & image, QString name, QString pixmapPath);
    void        buildImage(MosaicPtr &mosaic, QImage &image);

private:
    class ViewController * bmpViewController;
};
