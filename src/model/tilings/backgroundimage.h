#ifndef BACKGROUNDIMAGE_H
#define BACKGROUNDIMAGE_H

#include <QString>
#include <QImage>
#include <QPixmap>
#include "model/tilings/backgroundimage_perspective.h"
#include "gui/viewers/layer_controller.h"

class BackgroundImage : public LayerController
{
public:
    BackgroundImage();
    ~BackgroundImage();

    void    paint(QPainter *painter) override;

    void    createBackgroundAdjustment(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);

    QTransform getCanvasTransform() override;

    bool    load(QString imageName);    // loads from existing imported file
    bool    importIfNeeded(QString filename);   // import into media/bkgds
    bool    isLoaded()                              { return loaded; }
    void    unload();
    QString getTitle()                              { return bkgdName; };

    void    createPixmap();
    QPixmap & getPixmap()                           { return pixmap; }

    void    setAdjustedTransform(QTransform & adj)  { perspectiveT = adj; }
    QTransform  getAdjustedTransform()              { return perspectiveT; }

    void    createAdjustedImage();
    bool    saveAdjusted(QString newName);
    void    setUseAdjusted(bool use)                { bUseAdjusted = use;};
    bool    useAdjusted()                           { return bUseAdjusted; }

    void    createCroppedImage();
    bool    saveCropped(QString newName);
    void    setUseCropped(bool use)                 { bUseCropped = use;}
    bool    useCropped()                            { return bUseCropped; }

    void    correctPerspective(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);

    void    startCropper();
    void    endCopper();

    BackgroundImagePerspective adjuster;
    BackgroundImageCropper     cropper;
    Crop                       crop;

public slots:
    void slot_mousePressed(QPointF spt, Qt::MouseButton btn) override;
    void slot_mouseDragged(QPointF spt)       override;
    void slot_mouseMoved(QPointF spt)         override;
    void slot_mouseReleased(QPointF spt)      override;
    void slot_mouseDoublePressed(QPointF spt) override;

    void iamaLayerController() override {}

protected:
    void    drawSkew(QPainter * painter);

private:
    QString bkgdName;
    bool    bUseAdjusted;
    bool    bUseCropped;

    QImage  bkgdImage;
    QImage  adjustedImage;
    QImage  croppedImage;

    QPixmap pixmap;
    QPixmap scaledPixmap; // is painted, created from bkgdImage or adjusted image

    QTransform perspectiveT;    // used by keystone adjust
    QTransform worldT;
    QPointF    pixmapTL;

    bool    loaded;
};


#endif // BACKGROUNDIMAGE_H
