#ifndef BACKGROUNDIMAGE_H
#define BACKGROUNDIMAGE_H

#include <QString>
#include <QImage>
#include <QPixmap>
#include "viewers//backgroundimageview.h"

class BackgroundImage
{
public:
    BackgroundImage();

    bool    load(QString imageName);    // loads from existing imported file
    bool    importIfNeeded(QString filename);   // import into media/bkgds

    bool    isLoaded()                              { return loaded; }
    void    unload();
    QString getTitle()                              { return bkgdName; };

    void    createPixmap();

    QPixmap & getPixmap()                           { return _pixmap; }

    void    setAdjustedTransform(QTransform & adj)  { perspectiveT = adj; }
    QTransform  getAdjustedTransform()              { return perspectiveT; }

    void    setImageXform(const Xform & xf)         { image_xf = xf;}
    Xform   & getImageXform()                       { return image_xf; }

    void    createAdjustedImage();

    bool    useAdjusted() { return bUseAdjusted; }
    void    setUseAdjusted(bool use);
    bool    saveAdjusted(QString newName);
    void    correctPerspective(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);

protected:

private:
    QString bkgdName;
    bool    bUseAdjusted;

    Xform   image_xf;

    QImage  bkgdImage;      // as loaded
    QImage  adjustedImage;  // as adjusted
    QPixmap _pixmap;

    QTransform perspectiveT;

    bool    loaded;
};


#endif // BACKGROUNDIMAGE_H
