#ifndef BACKGROUNDIMAGE_H
#define BACKGROUNDIMAGE_H

#include <QtCore>
#include <QTransform>
#include <QGraphicsPixmapItem>
#include "base/layer.h"

class  BackgroundImage : public Layer
{
public:
    BackgroundImage();
    ~BackgroundImage() override;

    void    paint(QPainter *painter) override;

    bool    loadAndCopy(QString filename);  // loads from new file
    bool    loadImageUsingName();           // loads from existing file

    void    bkgdImageChanged(bool showBkgd, bool perspectiveBkgd);

    void    adjustBackground(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);
    void    adjustBackground();

    bool    saveAdjusted(QString newName);

    bool    isLoaded() { return _loaded; }

    // public data
    QString    bkgdName;
    QTransform perspective;
    bool       bShowBkgd;
    bool       bAdjustPerspective;

protected:
    void       correctPerspective(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);

private:
    Workspace * workspace;

    QPixmap     pixmap;
    QImage      bkgdImage;
    QImage      adjustedImage;

    bool        _loaded;
};

#endif // BACKGROUNDIMAGE_H
