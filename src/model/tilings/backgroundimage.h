#pragma once

#include <QString>
#include <QImage>
#include <QPixmap>
#include "model/tilings/backgroundimage_perspective.h"
#include "gui/viewers/layer_controller.h"
#include "sys/geometry/xform.h"

// what a tiling or mosaic XML holds about its background image - the image itself lives in Sys
struct BkgdRecord
{
    QString    name;                // empty = no background image
    Xform      xf;
    QTransform perspective;         // identity = not keystone adjusted
    bool       hasLegacyCenter = false;
    QPointF    legacyCenter;

    bool isEmpty() const { return name.isEmpty(); }
    void clear()         { *this = BkgdRecord(); }
};

class BackgroundImage : public LayerController
{
public:
    BackgroundImage();
    ~BackgroundImage();

    void    paint(QPainter *painter) override;

    QTransform getCanvasTransform() override;

    QPointF    keystoneScreenToModel(QPointF spt);
    QPointF    keystoneModelToScreen(QPointF mpt);

    bool    load(QString imageName);    // loads from existing imported file, clearing all previous image state
    bool    load(const BkgdRecord & rec);
    BkgdRecord toRecord();
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
    QPointF getCroppedOffset()                      { return croppedOffset; }

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
    QPointF    pixmapOffset;    // model-space center of the pixmap relative to the layer origin
    QPointF    croppedOffset;   // model-space center of the cropped image within the full image

    bool    loaded;
};
