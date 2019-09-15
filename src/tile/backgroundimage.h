#ifndef BACKGROUNDIMAGE_H
#define BACKGROUNDIMAGE_H

#include <QtCore>
#include <QTransform>
#include <QGraphicsPixmapItem>

class Configuration;
class Canvas;

class BackgroundImage : public QGraphicsPixmapItem
{
public:
    BackgroundImage();
    ~BackgroundImage() override;

    virtual void  paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

    void reset();

    bool    loadAndCopy(QString filename);  // loads from new file
    bool    loadImageUsingName();           // loads from existing file

    void    bkgdImageChanged(bool showBkgd, bool perspectiveBkgd, bool transformBkgd);
    void    bkgdTransformChanged(bool transformBkgd);

    void    adjustBackground(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);
    void    adjustBackground();

    bool    saveAdjusted(QString newName);

    QTransform getTransform();
    void       setTransform(QTransform t);

    // public data
    QString    bkgdName;
    qreal      scale;
    qreal      rot;
    qreal      x;
    qreal      y;
    QTransform perspective;

protected:
    void       correctPerspective(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft);

private:
    Configuration * config;
    Canvas        * canvas;

    QImage          bkgdImage;
    QImage          adjustedImage;

    bool            _transformBkgd;
};

#endif // BACKGROUNDIMAGE_H
