#ifndef TILING_BMP_ENGINE_H
#define TILING_BMP_ENGINE_H

#include <QImage>
#include <QList>
#include "sys/sys/versioning.h"

typedef std::shared_ptr<class Tiling>        TilingPtr;
typedef std::shared_ptr<class Mosaic>        MosaicPtr;

class TilingBMPEngine
{
public:
    TilingBMPEngine();
    ~TilingBMPEngine();
    bool    saveBitmap(VersionedName name, QString pixmapPath);

protected:
    void    savePixmap(QImage & image, VersionedName name, QString pixmapPath);
    bool    loadTiling(TilingPtr &tp, VersionedName name);
    void    buildImage(TilingPtr &mp, QImage &image);

private:
    class Configuration  * config;
    class ViewController * viewController;
};

#endif
