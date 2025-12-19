#ifndef TILING_BMP_GENERATOR_H
#define TILING_BMP_GENERATOR_H

#include <QImage>
#include <QList>
#include "sys/sys/versioning.h"

typedef std::shared_ptr<class Tiling>        TilingPtr;
typedef std::shared_ptr<class Mosaic>        MosaicPtr;

class TilingBMPGenerator
{
public:
    TilingBMPGenerator();
    ~TilingBMPGenerator();

    bool        saveBitmap(VersionedName name, QString pixmapPath);

protected:
    TilingPtr   loadTiling(VersionedName name);
    void        savePixmap(QImage & image, VersionedName name, QString pixmapPath);
    void        buildImage(TilingPtr &mp, QImage &image);

private:
    class SystemViewController * bmpViewController;
};

#endif
