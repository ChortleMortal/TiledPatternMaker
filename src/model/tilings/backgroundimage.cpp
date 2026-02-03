#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QPainter>

#include "gui/top/system_view_controller.h"
#include "model/settings/configuration.h"
#include "model/tilings/backgroundimage.h"
#include "sys/geometry/transform.h"

BackgroundImage::BackgroundImage() : LayerController(VIEW_BKGD_IMG,PRIMARY,"Bkgd Image View")
{
    unload();
    setZLevel(BKGD_IMG_ZLEVEL);
}

BackgroundImage::~BackgroundImage()
{
}

void BackgroundImage::paint(QPainter *painter)
{
    if (!loaded)
        return;

    //qDebug() << "BackgroundImage::paint";

    // Pre-scale only
    qreal modelScale = xf_model.getScale();
    QTransform scaleOnly;
    scaleOnly.scale(modelScale, modelScale);
    QPixmap scaledPixmap = pixmap.transformed(scaleOnly);

    // Draw pixmap always centered
    QSizeF sz = scaledPixmap.size();
    pixmapTL = QPointF(-sz.width()/2.0, -sz.height()/2.0);

    // Build layer transform but strip out scale (since pixmap is already scaled)
    worldT = getLayerTransform();
    worldT.scale(1.0/modelScale, 1.0/modelScale); // normalize out scale

    // fill the background
    QColor color = Sys::viewController->getBackgroundColor(VIEW_BKGD_IMG);
    painter->fillRect(Sys::viewController->viewRect(), color);

    // paint
    painter->save();

    painter->setWorldTransform(worldT, false);

    painter->drawPixmap(pixmapTL, scaledPixmap);

    painter->restore();

    if (adjuster.isActive())
    {
        adjuster.draw(painter);
    }
    else if (cropper.isActive())
    {
        cropper.draw(painter);
    }

    drawLayerModelCenter(painter);
}

void BackgroundImage::unload()
{
    loaded         = false;
    bUseAdjusted   = false;
    bUseCropped    = false;
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
        pixmap = QPixmap::fromImage(adjustedImage);
    }
    else if (bUseCropped && !croppedImage.isNull())
    {
        qDebug() << "using cropped image";
        pixmap = QPixmap::fromImage(croppedImage);
    }
    else
    {
        qDebug() << "using regular background image";
        pixmap = QPixmap::fromImage(bkgdImage);
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

void BackgroundImage::createCroppedImage()
{
    qDebug() << "BackgroundImage::createCroppedImage";

    if (!bkgdImage.isNull())
    {
        // this is tricky code - change at your peril
        qreal modelScale = xf_model.getScale();
        QRectF rect  = crop.getRect();
        QPointF TL   = rect.topLeft();
        QPointF PTL2 = pixmapTL / modelScale;
        TL -= PTL2;
        rect.moveTopLeft(TL);

        croppedImage = bkgdImage.copy(rect.toRect());
    }
    else
    {
        croppedImage = QImage();
        Q_ASSERT(croppedImage.isNull());
    }
}

bool BackgroundImage::saveAdjusted(QString newName)
{
    QString file = Sys::config->rootMediaDir + "bkgd_photos/" +  newName;
    qDebug() << "Saving adjusted:" << file;
    bool rv = adjustedImage.save(file);
    return rv;
}


bool BackgroundImage::saveCropped(QString newName)
{
    QString file = Sys::config->rootMediaDir + "bkgd_photos/" +  newName;
    qDebug() << "Saving cropped:" << file;
    bool rv = croppedImage.save(file);
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


// this is perspective correction
// for images where camera was not normal to the plane of the tiling
void BackgroundImage::createBackgroundAdjustment(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft)
{
    QSize sz      = pixmap.size();
    qreal offsetX = qreal(Sys::viewController->viewWidth() -  sz.width()) / 2.0;
    qreal offsetY = qreal(Sys::viewController->viewHeight() - sz.height()) / 2.0;
    QTransform t0 = QTransform::fromTranslate(offsetX,offsetY);

    QTransform bkgdXform = t0 * getModelTransform();
    QTransform t1        = bkgdXform.inverted();

    correctPerspective(
        t1.map(topLeft),
        t1.map(topRight),
        t1.map(botRight),
        t1.map(botLeft));
}

QTransform BackgroundImage::getCanvasTransform()
{
    QTransform t  = Layer::getCanvasTransform();
    QPointF trans = Transform::trans(t);
    QTransform t1;
    t1.translate(trans.x(), trans.y());
    return t1;
}

////////////////////////////////////////////////////////
///
/// Layer slots
///
////////////////////////////////////////////////////////

void BackgroundImage::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    if (!viewControl()->isEnabled(VIEW_BKGD_IMG)) return;

    if (adjuster.isActive())
    {
        if (adjuster.startDragging(spt))
            emit sig_updateView();
    }
    else if (cropper.isActive())
    {
        if (cropper.startDragging(spt))
            emit sig_updateView();
    }
}

void BackgroundImage::slot_mouseDragged(QPointF spt)
{
    if (!viewControl()->isEnabled(VIEW_BKGD_IMG)) return;

    if (adjuster.isActive())
    {
        if (adjuster.updateDragging(spt))
            emit sig_updateView();
    }
    else if (cropper.isActive())
    {
        if (cropper.updateDragging(spt))
            emit sig_updateView();
    }
}

void BackgroundImage::slot_mouseMoved(QPointF spt)
{ Q_UNUSED(spt); }

void BackgroundImage::slot_mouseReleased(QPointF spt)
{
    if (!viewControl()->isEnabled(VIEW_BKGD_IMG)) return;

    if (adjuster.isActive())
    {
        if (adjuster.endDragging(spt))
            emit sig_updateView();
    }
    else if (cropper.isActive())
    {
        if (cropper.endDragging(spt))
            emit sig_updateView();
    }
}

void BackgroundImage::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }
