#include <QImage>
#include <QFileDialog>
#include "tile/backgroundimage.h"
#include "base/configuration.h"
#include "base/workspace.h"
#include "geometry/transform.h"
#include "panels/dlg_name.h"

BackgroundImage::BackgroundImage() : Layer("Bkgd Image",LTYPE_BACKGROUND)
{
    workspace = Workspace::getInstance();

    setZValue(-20);

    _loaded            = false;
    bShowBkgd          = false;
    bAdjustPerspective = false;
}

BackgroundImage::~BackgroundImage()
{
}

void BackgroundImage::paint(QPainter *painter)
{
    if (!config->showBackgroundImage)
    {
        return;
    }

    painter->save();

    // center pixmap in the scene
    QSize sz = bkgdImage.size();  // or pixmap?
    qreal centerX   = (workspace->width() -  sz.width()) / 2;
    qreal centerY   = (workspace->height() - sz.height()) / 2;
    painter->translate(QPointF(centerX,centerY));

    QTransform t = getCanvasXform().toQTransform(getFrameTransform());
    painter->setTransform(t,true);
    qDebug() << "BackgroundImage::paint" << Transform::toInfoString(painter->transform());

    QRectF src(QPointF(0,0),sz);
    painter->drawPixmap(src,pixmap,src);

    painter->restore();
}

bool BackgroundImage::loadAndCopy(QString  filename)
{
    qDebug() << "BackgroundImage:: loadAndCopy:" << filename;
    QFileInfo info(filename);
    QString name = info.fileName();

    Configuration * config = Configuration::getInstance();
    QString bkgdDir = config->rootMediaDir + "bkgd_photos/";

    QString newFilename = bkgdDir + name;

    if (!QFile::exists(newFilename))
    {
        QFile::copy(filename,newFilename);
        qDebug() << "copy made:" << newFilename;
    }

    perspective = QTransform();  // reset
    bkgdName    = name;

    return loadImageUsingName();
}

bool BackgroundImage::loadImageUsingName()
{
    Configuration * config = Configuration::getInstance();
    QString filename = config->rootMediaDir + "bkgd_photos/" + bkgdName;
    qDebug() << "BackgroundImage:: loadImageUsingName:" << filename;
    bool rv = bkgdImage.load(filename);
    if (rv)
    {
        qDebug() << "BackgroundImage: loaded OK - size =" << bkgdImage.size();
        _loaded = true;
    }
    else
    {
        qWarning() << "BackgroundImage: LOAD ERROR";
        _loaded = false;
    }
    return rv;
}

void BackgroundImage::bkgdImageChanged(bool showBkgd, bool perspectiveBkgd)
{
    bShowBkgd          = showBkgd;
    bAdjustPerspective = perspectiveBkgd;

    if (bAdjustPerspective)
    {
        qDebug() << "using adjusted image";
        pixmap = QPixmap::fromImage(adjustedImage);
    }
    else
    {
        qDebug() << "using regular background image";
        pixmap = QPixmap::fromImage(bkgdImage);
    }
    workspace->update();
}


// this is perspective correction
// for images where camera was not normal to the plane of the tiling
void BackgroundImage::adjustBackground(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft)
{
    QSize sz      = pixmap.size();
    qreal offsetX = (workspace->width() -  sz.width()) / 2;
    qreal offsetY = (workspace->height() - sz.height()) / 2;
    QTransform t0 = QTransform::fromTranslate(offsetX,offsetY);

    QTransform bkgdXform = t0 * getCanvasXform().getTransform();
    QTransform t1        = bkgdXform.inverted();

    correctPerspective(
            t1.map(topLeft),
            t1.map(topRight),
            t1.map(botRight),
            t1.map(botLeft));

    qDebug().noquote() << "perspective:" << Transform::toString(perspective);
    qDebug().noquote() << "perspective:" << perspective;

    bAdjustPerspective = true;
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
    Configuration * config = Configuration::getInstance();
    QString file = config->rootMediaDir + "bkgd_photos/" +  newName;
    qDebug() << "Saving adjusted:" << file;
    bool rv = adjustedImage.save(file);
    return rv;
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
