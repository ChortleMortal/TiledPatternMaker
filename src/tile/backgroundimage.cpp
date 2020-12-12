#include <QImage>
#include <QFileDialog>
#include "tile/backgroundimage.h"
#include "base/configuration.h"
#include "viewers/view.h"
#include "geometry/transform.h"
#include "panels/dlg_name.h"

BackgroundImage::BackgroundImage() : Layer("Bkgd Image",LTYPE_BACKGROUND)
{
    view   = View::getInstance();
    config = Configuration::getInstance();

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
    QRectF rect         = pixmap.rect();
    QPointF pix_center  = rect.center();

    QSize sz            = view->getDefinedFrameSize(config->viewerType);
    QPointF view_center(sz.width()/2, sz.height()/2);
    QPointF center      = view_center - pix_center;
    //qDebug() << rect << pix_center << center;

    painter->translate(center);

    xf_bkImg.setCenter(pix_center);

    QTransform tbi = xf_bkImg.toQTransform(QTransform());
    //qDebug() << "tbi" << Transform::toInfoString(tbi);
    painter->setTransform(tbi,true);

    QTransform t = getCanvasXform().toQTransform(getFrameTransform());
    painter->setTransform(t,true);

    //qDebug() << "BackgroundImage::paint" << Transform::toInfoString(painter->transform());
    painter->drawPixmap(rect,pixmap,rect);

    drawCenter(painter);

    painter->restore();
}

bool BackgroundImage::import(QString filename)
{
    qDebug() << "BackgroundImage::import()" << filename;
    QFileInfo info(filename);
    QString name = info.fileName();

    QString newFilename = config->rootMediaDir + "bkgd_photos/" + name;

    if (!QFile::exists(newFilename))
    {
        QFile::copy(filename,newFilename);
        qDebug() << "import created:" << newFilename;
    }

    return load(name);
}

bool BackgroundImage::load(QString imageName)
{
    bkgdName    = imageName;
    perspective = QTransform();  // reset

    QString filename = config->rootMediaDir + "bkgd_photos/" + bkgdName;
    qDebug() << "BackgroundImage::load()" << filename;

    _loaded = bkgdImage.load(filename);

    if (_loaded)
        qDebug() << "BackgroundImage: loaded OK - size =" << bkgdImage.size();
    else
        qWarning() << "BackgroundImage: LOAD ERROR";

    return _loaded;
}

void BackgroundImage::updateBkgdXform(const Xform & xf)
{
    // FIXME ??
    //xf_bkImg.update(xf);
    xf_bkImg = xf;
    forceLayerRecalc();
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
    view->update();
}


// this is perspective correction
// for images where camera was not normal to the plane of the tiling
void BackgroundImage::adjustBackground(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft)
{
    QSize sz      = pixmap.size();
    qreal offsetX = (view->width() -  sz.width()) / 2;
    qreal offsetY = (view->height() - sz.height()) / 2;
    QTransform t0 = QTransform::fromTranslate(offsetX,offsetY);

    QTransform bkgdXform = t0 * xf_bkImg.getTransform();
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

void BackgroundImage::slot_mouseTranslate(QPointF pt)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_XFORM_VIEW:
        Layer::slot_mouseTranslate(pt);
        break;
    case KBD_MODE_XFORM_BKGD:
        xf_bkImg.setTranslateX(xf_bkImg.getTranslateX() + pt.x());
        xf_bkImg.setTranslateY(xf_bkImg.getTranslateY() + pt.y());
        forceRedraw();
        break;
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_UNIQUE_FEATURE:
    case KBD_MODE_XFORM_PLACED_FEATURE:
        break;
    default:
        qWarning() << "BackgroundImage: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

void BackgroundImage::slot_moveX(int amount)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_XFORM_VIEW:
        Layer::slot_moveX(amount);
        break;
    case KBD_MODE_XFORM_BKGD:
        xf_bkImg.setTranslateX(xf_bkImg.getTranslateX() + amount);
        forceRedraw();
        break;
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_UNIQUE_FEATURE:
    case KBD_MODE_XFORM_PLACED_FEATURE:
        break;
    default:
        qWarning() << "BackgroundImage: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

void BackgroundImage::slot_moveY(int amount)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_XFORM_VIEW:
        Layer::slot_moveY(amount);
        break;
    case KBD_MODE_XFORM_BKGD:
        xf_bkImg.setTranslateY(xf_bkImg.getTranslateY() + amount);
        forceRedraw();
        break;
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_UNIQUE_FEATURE:
    case KBD_MODE_XFORM_PLACED_FEATURE:
        break;
    default:
        qWarning() << "BackgroundImage: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

void BackgroundImage::slot_rotate(int amount)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_XFORM_VIEW:
        Layer::slot_rotate(amount);
        break;
    case KBD_MODE_XFORM_BKGD:
        xf_bkImg.setRotateRadians(xf_bkImg.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        forceRedraw();
        break;
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_UNIQUE_FEATURE:
    case KBD_MODE_XFORM_PLACED_FEATURE:
        break;
    default:
        qWarning() << "BackgroundImage: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

void BackgroundImage::slot_wheel_rotate(qreal delta)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_XFORM_VIEW:
        Layer::slot_wheel_rotate(delta);
        break;
    case KBD_MODE_XFORM_BKGD:
        xf_bkImg.setRotateDegrees(xf_bkImg.getRotateDegrees() + delta);
        forceRedraw();
        break;
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_UNIQUE_FEATURE:
    case KBD_MODE_XFORM_PLACED_FEATURE:
        break;
    default:
        qWarning() << "BackgroundImage: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

void BackgroundImage::slot_scale(int amount)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_XFORM_VIEW:
        Layer::slot_scale(amount);
        break;
    case KBD_MODE_XFORM_BKGD:
        xf_bkImg.setScale(xf_bkImg.getScale() + static_cast<qreal>(amount)/100.0);
        forceRedraw();
        break;
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_UNIQUE_FEATURE:
    case KBD_MODE_XFORM_PLACED_FEATURE:
        break;
    default:
        qWarning() << "BackgroundImage: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

void BackgroundImage::slot_wheel_scale(qreal delta)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_XFORM_VIEW:
        Layer::slot_wheel_scale(delta);
        break;
    case KBD_MODE_XFORM_BKGD:
        xf_bkImg.setScale(xf_bkImg.getScale() + delta);
        forceRedraw();
        break;
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_UNIQUE_FEATURE:
    case KBD_MODE_XFORM_PLACED_FEATURE:
        break;
    default:
        qWarning() << "BackgroundImage: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

void BackgroundImage::slot_setCenterScreen(QPointF spt)
{
    switch (config->kbdMode)
    {
    case KBD_MODE_XFORM_VIEW:
        Layer::slot_setCenterScreen(spt);
        break;
    case KBD_MODE_XFORM_BKGD:
        break;
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_UNIQUE_FEATURE:
    case KBD_MODE_XFORM_PLACED_FEATURE:
        break;
    default:
        qWarning() << "BackgroundImage: Unexpected keyboard mode" << view->getKbdModeStr();
        break;
    }
}

void  BackgroundImage::drawCenter(QPainter * painter)
{
    if (config->showCenterDebug || config->showCenterMouse)
    {
        QPointF pt = xf_bkImg.getCenter();
        //qDebug() << "BackgroundImage::drawCenter:" << pt;
        qreal len = 13;
        QColor color(Qt::darkYellow);
        painter->setPen(QPen(color));
        color.setAlpha(128);
        painter->setBrush(QBrush(color));
        painter->drawEllipse(pt,len,len);
        painter->setPen(QPen(Qt::blue));
        painter->drawLine(QPointF(pt.x()-len,pt.y()),QPointF(pt.x()+len,pt.y()));
        painter->drawLine(QPointF(pt.x(),pt.y()-len),QPointF(pt.x(),pt.y()+len));
    }
}
