#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include "model/tilings/backgroundimage.h"
#include "model/settings/configuration.h"

BackgroundImage::BackgroundImage()
{
    bkgdName = "Bkgd Image";
}

void BackgroundImage::unload()
{
    loaded         = false;
    bUseAdjusted   = false;
    bkgdName       = "Bkgd Image";
 }

bool BackgroundImage::importIfNeeded(QString filename)
{
    qDebug() << "BackgroundImage::import()" << filename;
    QFileInfo info(filename);
    QString name = info.fileName();

    QString newFilename = Sys::config->rootMediaDir + "bkgd_photos/" + name;

    if (QFile::exists(newFilename))
    {
        return true;
    }
    else if (QFile::copy(filename,newFilename))
    {
        qDebug() << "import created:" << newFilename;
        return true;
    }

    return false;
}

bool BackgroundImage::load(QString imageName)
{
    bkgdName    = imageName;

    QString filename = Sys::config->rootMediaDir + "bkgd_photos/" + bkgdName;
    qDebug() << "BackgroundImage::load()" << filename;

    loaded = bkgdImage.load(filename);

    if (loaded)
        qDebug() << "BackgroundImage: loaded OK - size =" << bkgdImage.size();
    else
        qWarning() << "BackgroundImage: LOAD ERROR";

    return loaded;
}

void BackgroundImage::createPixmap()
{
    qDebug() << "BackgroundImage::createPixmap";

    if (bUseAdjusted && !adjustedImage.isNull())
    {
        qDebug() << "using adjusted image";
        _pixmap = QPixmap::fromImage(adjustedImage);
    }
    else
    {
        qDebug() << "using regular background image";
        _pixmap = QPixmap::fromImage(bkgdImage);
    }
}

void BackgroundImage::createAdjustedImage()
{
    qDebug() << "BackgroundImage::createAdjustedImage";

    if (!bkgdImage.isNull() && !perspectiveT.isIdentity())
    {
        adjustedImage = bkgdImage.transformed(perspectiveT,Qt::SmoothTransformation);
    }
    else
    {
        adjustedImage = QImage();
        Q_ASSERT(adjustedImage.isNull());
    }
}

void BackgroundImage::setUseAdjusted(bool use)
{
    bUseAdjusted = use;
    qDebug() << "useAdjusted" << bUseAdjusted;
}

bool BackgroundImage::saveAdjusted(QString newName)
{
    QString file = Sys::config->rootMediaDir + "bkgd_photos/" +  newName;
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
    bool rv = QTransform::quadToQuad(fromPolygon, toPolygon, perspectiveT);
    if (!rv)
    {
        qDebug() << "Could not create the transformation matrix.";
    }
}


