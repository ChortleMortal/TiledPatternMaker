#ifndef TILING_BMP_ENGINE_H
#define TILING_BMP_ENGINE_H

#include <QImage>
#include <QList>

typedef std::shared_ptr<class Tiling>        TilingPtr;
typedef std::shared_ptr<class Mosaic>        MosaicPtr;

class TilingBMPEngine
{
public:
    TilingBMPEngine();
    ~TilingBMPEngine();
    bool    saveBitmap(QString name, QString pixmapPath);

protected:
    void    savePixmap(QImage & image, QString name, QString pixmapPath);
    bool    loadTiling(TilingPtr &tp, QString name);
    void    buildImage(TilingPtr &mp, QImage &image);

private:
    class Configuration  * config;
    class ViewController * viewController;
};

#endif
