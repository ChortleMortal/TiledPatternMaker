#ifndef MOSAIC_BMP_ENGINE_H
#define MOSAIC_BMP_ENGINE_H

#include "sys/sys/versioning.h"
#include <QImage>
#include <QList>

typedef std::shared_ptr<class Tiling>        TilingPtr;
typedef std::shared_ptr<class Mosaic>        MosaicPtr;

class MosaicBMPEngine
{
public:
    MosaicBMPEngine();
    ~MosaicBMPEngine();
    bool    saveBitmap(VersionedName name, QString pixmapPath);

protected:
    void    savePixmap(QImage & image, QString name, QString pixmapPath);
    bool    loadMosaic(MosaicPtr &mosaic, VersionedName name);
    void    buildImage(MosaicPtr &mosaic, QImage &image);

private:
    class ViewController * viewController;
};

#endif
