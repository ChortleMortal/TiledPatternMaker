#include <QImage>
#include <QFileDialog>
#include "tile/backgroundimage.h"
#include "base/configuration.h"
#include "base/canvas.h"
#include "geometry/Transform.h"
#include "panels/dlg_name.h"

BackgroundImage::BackgroundImage()
{
    config = Configuration::getInstance();
    canvas = Canvas::getInstance();

    reset();

    setZValue(-20);
}

BackgroundImage::~BackgroundImage()
{
    qDebug() << "BackgroundImage destructor";
}

void BackgroundImage::reset()
{
    bkgdName.clear();
    scale = 1.0;
    rot   = 0.0;
    x     = 0.0;
    y     = 0.0;
}

void BackgroundImage::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //qDebug() << "BackgroundImage::paint";

    // center pixmap in the scene
    QSize sz      = pixmap().size();
    qreal offsetX = (canvas->scene->width() -  sz.width()) / 2;
    qreal offsetY = (canvas->scene->height() - sz.height()) / 2;
    painter->translate(QPointF(offsetX,offsetY));

    if (_transformBkgd)
    {
        // apply specified transforms
        painter->rotate(rot);
        painter->scale(scale,scale);
        painter->translate(QPointF(x,y));
    }

    QGraphicsPixmapItem::paint(painter,option, widget);
    painter->resetTransform();
}

bool BackgroundImage::loadAndCopy(QString  filename)
{
    qDebug() << " loadAndCopy:" << filename;
    QFileInfo info(filename);
    QString name = info.fileName();

    QString bkgdDir = config->rootMediaDir + "bkgd_photos/";

    QString newFilename = bkgdDir + name;

    if (!QFile::exists(newFilename))
    {
        QFile::copy(filename,newFilename);
        qDebug() << "copy made:" << newFilename;
    }

    QTransform t;
    setTransform(t);
    perspective = t;

    bkgdName    = name;
    return loadImageUsingName();
}

bool BackgroundImage::loadImageUsingName()
{
    QString filename = config->rootMediaDir + "bkgd_photos/" + bkgdName;
    qDebug() << "loadImageUsingName:" << filename;
    bool rv = bkgdImage.load(filename);
    if (rv)
        qInfo() << "Loaded OK - size =" << bkgdImage.size();
    else
        qWarning() << "LOAD ERROR";
    return rv;
}

void BackgroundImage::bkgdImageChanged(bool showBkgd, bool perspectiveBkgd, bool transformBkgd)
{
    _transformBkgd = transformBkgd;

    canvas->scene->removeItem(this);

    if (!showBkgd)
    {
        return;
    }

    if (perspectiveBkgd)
    {
        qDebug() << "using adjusted image";
        setPixmap(QPixmap::fromImage(adjustedImage));
    }
    else
    {
        qDebug() << "using regular background image";
        setPixmap(QPixmap::fromImage(bkgdImage));
    }

    canvas->scene->addItem(this);
}

void BackgroundImage::bkgdTransformChanged(bool transformBkgd)
{
    _transformBkgd = transformBkgd;
    canvas->update();
}

// this is perspective correction
// for images where camera was not normal to the plane of the tiling
void BackgroundImage::adjustBackground(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft)
{
    QSize sz      = pixmap().size();
    qreal offsetX = (canvas->scene->width() -  sz.width()) / 2;
    qreal offsetY = (canvas->scene->height() - sz.height()) / 2;
    QTransform t0 = QTransform::fromTranslate(offsetX,offsetY);

    QTransform r;
    r.rotate(rot);

    QTransform s = QTransform::fromScale(scale,scale);
    s.scale(scale,scale);
    QTransform t = QTransform::fromTranslate(x,y);

    QTransform bkgdXform = t0 * s * r * t;
    QTransform t1 = bkgdXform.inverted();

    correctPerspective(
            t1.map(topLeft),
            t1.map(topRight),
            t1.map(botRight),
            t1.map(botLeft));

    qDebug().noquote() << "perspective:" << Transform::toString(perspective);
    qDebug().noquote() << "perspective:" << perspective;

    adjustBackground();
}

void BackgroundImage::adjustBackground()
{
    if (!bkgdImage.isNull())
    {
        adjustedImage = bkgdImage.transformed(perspective,Qt::SmoothTransformation);
    }
}

bool BackgroundImage::saveAdjusted(QString newName)
{
    QString file = config->rootMediaDir + "bkgd_photos/" +  newName;
    qDebug() << "Saving adjusted:" << file;
    bool rv = adjustedImage.save(file);
    return rv;
}

QTransform BackgroundImage::getTransform()
{
    QTransform r;
    r.rotate(rot);
    QTransform s;
    s.scale(scale,scale);
    QTransform t;
    t.translate(x,y);

    QTransform t0 = s * r * t;

    return t0;
}

void BackgroundImage::setTransform(QTransform t)
{
    scale = Transform::scalex(t);
    rot   = Transform::rotation(t);
    x     = Transform::transx(t);
    y     = Transform::transy(t);
}

void BackgroundImage::correctPerspective(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft)
{
    qreal width = 0;
    qreal height = 0;

    // Get the lines limiting the new image area
    QLineF topLine(topLeft,topRight);
    QLineF bottomLine(botLeft, botRight);
    QLineF leftLine(topLeft, botLeft);
    QLineF rightLine(topRight, botRight);

    // Select the longest lines
    if(topLine.length() > bottomLine.length())
    {
        width = topLine.length();
    }
    else
    {
        width = bottomLine.length();
    }

    if(topLine.length() > bottomLine.length())
    {
        height = leftLine.length();
    }
    else
    {
        height = rightLine.length();
    }

    // Create the QPolygonF containing the corner points
    // in user specified quadrilateral arrangement
    QPolygonF fromPolygon;
    fromPolygon << topLeft;
    fromPolygon << topRight;
    fromPolygon << botRight;
    fromPolygon << botLeft;

    // target polygon
    QPolygonF toPolygon;
    toPolygon << QPointF(0, 0);
    toPolygon << QPointF(width, 0);
    toPolygon << QPointF(width, height);
    toPolygon << QPointF(0, height);

    // create the matrix
    bool rv = QTransform::quadToQuad(fromPolygon, toPolygon, perspective);
    if (!rv)
    {
        qDebug() << "Could not create the transformation matrix.";
    }
}

