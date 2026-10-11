#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QPainter>

#include "gui/top/view_controller.h"
#include "model/tilings/backgroundimage.h"
#include "sys/geometry/transform.h"
#include "sys/sys.h"

BackgroundImage::BackgroundImage() : LayerController(Type::BkgdImage,Model::Primary, ZLevel::Bkgd_minus20, "Bkgd Image View")
{
    unload();
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
    pixmapTL = QPointF(-sz.width()/2.0, -sz.height()/2.0) + pixmapOffset * modelScale;

    // Build layer transform but strip out scale (since pixmap is already scaled)
    worldT = getLayerTransform();
    worldT.scale(1.0/modelScale, 1.0/modelScale); // normalize out scale

#if 0   // removed 10MAY26
    // fill the background
    QColor color = Sys::systemViewController->getBkgdColor(VIEW_BKGD_IMG);
    painter->fillRect(Sys::systemViewController->getViewRect(), color);
#endif
    // paint
    painter->save();

    painter->setWorldTransform(worldT, false);

    painter->drawPixmap(pixmapTL, scaledPixmap);

    painter->restore();

    if (cropper.isActive())
    {
        cropper.draw(painter);
    }
    else if (adjuster.isActive() || adjuster.hasPoints())
    {
        // keeps the keystone rectangle visible after Complete, until it's explicitly cleared (e.g. on save)
        adjuster.draw(painter);
    }

    drawLayerModelCenter(painter);
}

void BackgroundImage::unload()
{
    loaded         = false;
    bUseAdjusted   = false;
    bUseCropped    = false;
    bkgdName       = "Bkgd Image";

    bkgdImage      = QImage();
    adjustedImage  = QImage();
    croppedImage   = QImage();
    pixmap         = QPixmap();
    pixmapOffset   = QPointF();
    croppedOffset  = QPointF();
    perspectiveT   = QTransform();
    crop           = Crop();
    adjuster.clear();
}

bool BackgroundImage::importIfNeeded(QString filename)
{
    qDebug() << "BackgroundImage::import()" << filename;
    QFileInfo info(filename);
    QString name = info.fileName();

    QString newFilename = Sys::bkgdPhotosDir + name;

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
    unload();
    bkgdName    = imageName;

    QString filename = Sys::bkgdPhotosDir + bkgdName;
    qDebug() << "BackgroundImage::load()" << filename;

    QImageReader reader(filename);
    reader.setAutoTransform(true);     // applies EXIF orientation (e.g. camera-rotated JPEGs)

    bkgdImage = reader.read();
    loaded    = !bkgdImage.isNull();

    if (loaded)
        qDebug() << "BackgroundImage: loaded OK - size =" << bkgdImage.size();
    else
        qWarning() << "BackgroundImage: LOAD ERROR";

    return loaded;
}

bool BackgroundImage::load(const BkgdRecord & rec)
{
    if (rec.isEmpty())
    {
        unload();
        return false;
    }

    if (!load(rec.name))
    {
        unload();
        return false;
    }

    setModelXform(rec.xf,false,Sys::nextSigid());

    if (rec.hasLegacyCenter)
        setLegacyCenter(rec.legacyCenter);

    if (!rec.perspective.isIdentity())
    {
        perspectiveT = rec.perspective;
        createAdjustedImage();
        bUseAdjusted = true;
    }
    createPixmap();
    return true;
}

BkgdRecord BackgroundImage::toRecord()
{
    BkgdRecord rec;
    if (loaded)
    {
        rec.name = bkgdName;
        rec.xf   = getModelXform();
        if (bUseAdjusted)
            rec.perspective = perspectiveT;
    }
    return rec;
}

void BackgroundImage::createPixmap()
{
    qDebug() << "BackgroundImage::createPixmap";

    if (bUseAdjusted && !adjustedImage.isNull())
    {
        qDebug() << "using adjusted image";
        pixmap = QPixmap::fromImage(adjustedImage);
        pixmapOffset = QPointF();
    }
    else if (bUseCropped && !croppedImage.isNull())
    {
        qDebug() << "using cropped image";
        pixmap = QPixmap::fromImage(croppedImage);
        pixmapOffset = croppedOffset;     // stays where it was cut from
    }
    else
    {
        qDebug() << "using regular background image";
        pixmap = QPixmap::fromImage(bkgdImage);
        pixmapOffset = QPointF();
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
        // crop rect is in model space, where the full image is centered on the origin
        QPointF imageCenter(bkgdImage.width()/2.0, bkgdImage.height()/2.0);
        QRect rect = crop.getRect().translated(imageCenter).toRect();

        croppedImage  = bkgdImage.copy(rect);
        croppedOffset = QRectF(rect).center() - imageCenter;
    }
    else
    {
        croppedImage  = QImage();
        croppedOffset = QPointF();
        Q_ASSERT(croppedImage.isNull());
    }
}

bool BackgroundImage::saveAdjusted(QString newName)
{
    QString file = Sys::bkgdPhotosDir +  newName;
    qDebug() << "Saving adjusted:" << file;
    bool rv = adjustedImage.save(file);
    return rv;
}


bool BackgroundImage::saveCropped(QString newName)
{
    QString file = Sys::bkgdPhotosDir +  newName;
    qDebug() << "Saving cropped:" << file;
    bool rv = croppedImage.save(file);
    return rv;
}

// this is perspective correction
// for images where camera was not normal to the plane of the tiling
// topLeft/topRight/botRight/botLeft are already in keystone model space (see keystoneScreenToModel)
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

    if(leftLine.length() > rightLine.length())
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

// keystone points are raw pixel coordinates in the currently-displayed pixmap. paint() draws
// that pixmap centered at the layer's model origin (pixmapTL = -pixmap.size()/2, before the
// layer transform), so pixel-space <-> model-space is just that centering offset, and
// model-space <-> screen is exactly Layer::modelToScreen/screenToModel - already correct and
// already used everywhere else, so reused here rather than re-deriving the canvas+model
// composition by hand.
QPointF BackgroundImage::keystoneScreenToModel(QPointF spt)
{
    QPointF center(pixmap.width()/2.0, pixmap.height()/2.0);
    return screenToModel(spt) + center;
}

QPointF BackgroundImage::keystoneModelToScreen(QPointF mpt)
{
    if (bUseAdjusted && !adjustedImage.isNull())
    {
        // the displayed pixmap is now the warped result - carry the corner from its
        // original photo-space position to where the correction actually put it.
        // trueMatrix is the same adjustment QImage::transformed() applies internally
        // (it shifts the result so it has no negative coordinates), so this has to be
        // used rather than perspectiveT alone to land on the right pixel.
        QTransform trueT = QPixmap::trueMatrix(perspectiveT, bkgdImage.width(), bkgdImage.height());
        mpt = trueT.map(mpt);
    }

    QPointF center(pixmap.width()/2.0, pixmap.height()/2.0);
    return modelToScreen(mpt - center);
}

QTransform BackgroundImage::getCanvasTransform()
{
    // background images are not warped by canvas distortion, unlike the design content they surround
    QTransform t  = viewControl()->getCanvas()->getUndistortedTransform();
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

    if (!viewControl()->isEnabled(Layer::Type::BkgdImage)) return;

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
    if (!viewControl()->isEnabled(Layer::Type::BkgdImage)) return;

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
    if (!viewControl()->isEnabled(Layer::Type::BkgdImage)) return;

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
