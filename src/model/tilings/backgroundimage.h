#ifndef BACKGROUNDIMAGE_H
#define BACKGROUNDIMAGE_H

#include <QString>
#include <QImage>
#include <QPixmap>
#include "model/tilings/backgroundimage_perspective.h"
#include "gui/viewers/layer_controller.h"

class BackgroundImage : public LayerController, public BackgroundImagePerspective
{
public:
    BackgroundImage();

    void            paint(QPainter *painter) override;

    void            createBackgroundAdjustment(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);

    QTransform      getCanvasTransform() override;

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
    bool    useAdjusted() { return bUseAdjusted; }
    void    setUseAdjusted(bool use);
    bool    saveAdjusted(QString newName);
    void    correctPerspective(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);


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

    QImage  bkgdImage;
    QImage  adjustedImage;  // as adjusted
    QPixmap pixmap;
    QPixmap scaledPixmap; // is painted, created from bkgdImage or adjusted image

    QTransform perspectiveT;

    bool    loaded;
};


#endif // BACKGROUNDIMAGE_H
