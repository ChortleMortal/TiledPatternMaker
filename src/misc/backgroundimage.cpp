#include <QImage>
#include <QFileDialog>
#include <QPainter>

#include "misc/backgroundimage.h"
#include "geometry/edge.h"
#include "geometry/point.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "makers/tiling_maker/tiling_mouseactions.h"
#include "settings/configuration.h"
#include "viewers/viewcontrol.h"

using std::make_shared;

BkgdImgPtr BackgroundImage::spThis;

BkgdImgPtr BackgroundImage::getSharedInstance()
{
    if (!spThis)
    {
        spThis = make_shared<BackgroundImage>();
    }
    return spThis;
}

BackgroundImage::BackgroundImage() : LayerController("Bkgd Image")
{
    view     = ViewControl::getInstance();
    config   = Configuration::getInstance();
    bkgdName = "Bkgd Image";

    frameData = view->frameSettings.getFrameData(VIEW_BKGD_IMG);

    setZValue(-20);

    _loaded     = false;
    skewMode    = false;
}

QString BackgroundImage::getName()
{
    return bkgdName;
}

void BackgroundImage::paint(QPainter *painter)
{
    static constexpr QColor construction_color  = QColor(  0,128,  0,128);

    if (!_loaded)
    {
        return;
    }
    if (!config->showBackgroundImage)
    {
        return;
    }

    painter->save();

    QSizeF sz      = view->frameSettings.getZoomSize(config->getViewerType());
    QPointF view_center(qreal(sz.width())/2.0, qreal(sz.height())/2.0);
    QPointF delta  = view_center - pixmap.rect().center();

    QTransform t1 = QTransform::fromTranslate(delta.x(),delta.y());
    QTransform t2 = frameData->getDeltaTransform() * frameData->getTransform();
    QTransform t3 = getCanvasTransform();
    QTransform t4 =  t1 * t2 * t3;
    //qDebug().noquote() << "t4" << Transform::toInfoString(t4);
    painter->setTransform(t4);

    painter->drawPixmap(0,0,pixmap);

    painter->restore();

    drawCenter(painter);

    // draw accum
    if ( sAccum.size() > 0)
    {
        QPen pen(construction_color,3);
        QBrush brush(construction_color);
        painter->setPen(pen);
        painter->setBrush(brush);
        for (EdgePtr edge : sAccum)
        {
            if (edge->getType() == EDGETYPE_LINE)
            {
                QPointF p1 = edge->v1->pt;
                QPointF p2 = edge->v2->pt;
                painter->drawEllipse(p1,6,6);
                painter->drawEllipse(p2,6,6);
                painter->drawLine(p1, p2);
            }
            else if (edge->getType() == EDGETYPE_POINT)
            {
                QPointF p = edge->v1->pt;
                painter->drawEllipse(p,6,6);
            }
        }
    }

    if (mouse_interaction)
    {
        mouse_interaction->draw(painter);
    }
}

bool BackgroundImage::import(QString filename)
{
    qDebug() << "BackgroundImage::import()" << filename;
    QFileInfo info(filename);
    QString name = info.fileName();

    Configuration * config = Configuration::getInstance();
    QString newFilename = config->rootMediaDir + "bkgd_photos/" + name;

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

    QString filename = config->rootMediaDir + "bkgd_photos/" + bkgdName;
    qDebug() << "BackgroundImage::load()" << filename;

    _loaded = bkgdImage.load(filename);

    if (_loaded)
        qDebug() << "BackgroundImage: loaded OK - size =" << bkgdImage.size();
    else
        qWarning() << "BackgroundImage: LOAD ERROR";

    return _loaded;
}

void BackgroundImage::unload()
{
    _loaded         = false;
    bUseAdjusted    = false;
    skewMode        = false;
    mouse_interaction.reset();
    bkgdName = "Bkgd Image";

    Xform xf;
    setCanvasXform(xf);
    forceLayerRecalc(false);
}

void  BackgroundImage::setUseAdjusted(bool use)
{
    bUseAdjusted = use;
    qDebug() << "useAdjusted" << bUseAdjusted;
}

void BackgroundImage::createPixmap()
{

    if (bUseAdjusted && !adjustedImage.isNull())
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
void BackgroundImage::createBackgroundAdjustment(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft)
{
    QSize sz      = pixmap.size();
    qreal offsetX = (view->width() -  sz.width()) / 2;
    qreal offsetY = (view->height() - sz.height()) / 2;
    QTransform t0 = QTransform::fromTranslate(offsetX,offsetY);

    QTransform bkgdXform = t0 * getCanvasTransform();
    QTransform t1        = bkgdXform.inverted();

    correctPerspective(
            t1.map(topLeft),
            t1.map(topRight),
            t1.map(botRight),
            t1.map(botLeft));

    qDebug().noquote() << "perspective:" << Transform::toString(perspective);
    qDebug().noquote() << "perspective:" << perspective;

    bUseAdjusted = true;     // since we have just created it, let's use it

    createAdjustedImage();
}

void BackgroundImage::createAdjustedImage()
{
    if (!bkgdImage.isNull() && !perspective.isIdentity())
    {
        adjustedImage = bkgdImage.transformed(perspective,Qt::SmoothTransformation);
    }
    else
    {
        adjustedImage = QImage();
        Q_ASSERT(adjustedImage.isNull());
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

////////////////////////////////////////////////////////
///
/// Layer slots
///
////////////////////////////////////////////////////////


const Xform  & BackgroundImage::getCanvasXform()
{
    return xf_layer;
}

void BackgroundImage::setCanvasXform(const Xform & xf)
{
    xf_layer = xf;
    forceLayerRecalc();
}

void BackgroundImage::slot_setCenter(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (    view->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  view->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        setCenterScreenUnits(spt);
    }
}

void BackgroundImage::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    if (!view->isActiveLayer(this)) return;

    if (skewMode)
    {
        if (mouse_interaction)
        {
            PerspectivePtr pp = std::dynamic_pointer_cast<Perspective>(mouse_interaction);
            if (pp)
            {
                pp->addPoint(spt);
            }
        }
        else
        {
            mouse_interaction = make_shared<Perspective>(spt);
        }
    }
}

void BackgroundImage::slot_mouseDragged(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (mouse_interaction)
        mouse_interaction->updateDragging(spt);
}

void BackgroundImage::slot_mouseTranslate(QPointF pt)
{
    if (!view->isActiveLayer(this)) return;

    if (    view->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  view->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateX(xf.getTranslateX() + pt.x());
        xf.setTranslateY(xf.getTranslateY() + pt.y());
        setCanvasXform(xf);
    }
}

void BackgroundImage::slot_mouseMoved(QPointF spt)
{ Q_UNUSED(spt); }

void BackgroundImage::slot_mouseReleased(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (mouse_interaction)
    {
        mouse_interaction->endDragging(spt);
        mouse_interaction.reset();
    }
}

void BackgroundImage::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }


void BackgroundImage::slot_wheel_scale(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (    view->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  view->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1.0 + delta));
        setCanvasXform(xf);
    }
}

void BackgroundImage::slot_wheel_rotate(qreal delta)
{
    if (!view->isActiveLayer(this)) return;

    if (    view->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  view->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setRotateDegrees(xf.getRotateDegrees() + delta);
        setCanvasXform(xf);
    }
}

void BackgroundImage::slot_scale(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (    view->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  view->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setScale(xf.getScale() * (1 + static_cast<qreal>(amount)/100.0));
        setCanvasXform(xf);
    }
}

void BackgroundImage::slot_rotate(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (    view->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  view->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setRotateRadians(xf.getRotateRadians() + qDegreesToRadians(static_cast<qreal>(amount)));
        setCanvasXform(xf);
    }
}

void BackgroundImage::slot_moveX(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (    view->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  view->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateX(xf.getTranslateX() + amount);
        setCanvasXform(xf);
    }
}

void BackgroundImage::slot_moveY(int amount)
{
    if (!view->isActiveLayer(this)) return;

    if (    view->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  view->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        Xform xf = getCanvasXform();
        xf.setTranslateY(xf.getTranslateY() + amount);
        setCanvasXform(xf);
    }
}

void  BackgroundImage::setSkewMode(bool enb)
{
    skewMode = enb;
    if (!enb)
    {
        mouse_interaction.reset();
        sAccum.clear();
    }
}

/////////
///
///  Perspective
///
/////////

Perspective::Perspective(QPointF spt)
{
    bip = BackgroundImage::getSharedInstance();
    EdgePoly & saccum = bip->sAccum;
    qDebug() << "click size=" << saccum.size();
    if (saccum.size() == 0)
    {
        addPoint(spt);
    }
}

void Perspective::addPoint(QPointF spos)
{
    qDebug("Perspective::addPoint");

    VertexPtr vnew = make_shared<Vertex>(spos);

    EdgePoly & accum = bip->sAccum;
    int size = accum.size();

    if (size == 0)
    {
        accum.push_back(make_shared<Edge>(vnew));
        qDebug() << "point count = 1";
    }
    else if (size == 1)
    {
        EdgePtr last = accum.last();
        if (last->getType() == EDGETYPE_POINT)
        {
            last->setV2(vnew);
            qDebug() << "edge count =" << accum.size();
        }
        else
        {
            accum.push_back(make_shared<Edge>(last->v2,vnew));
            qDebug() << "edge count =" << accum.size();
        }
    }
    else if (size == 2)
    {
        EdgePtr last = accum.last();
        accum.push_back(make_shared<Edge>(last->v2,vnew));
        qDebug() << "edge count = " << accum.size();
        accum.push_back(make_shared<Edge>(vnew,accum.first()->v1));
        qDebug() << "completed with edge count =" << accum.size();
        bip->forceRedraw();
    }
}

void Perspective::updateDragging(QPointF spt)
{
    sLastDrag = spt;
    bip->forceRedraw();
}

void Perspective::endDragging(QPointF spt )
{
    EdgePoly & saccum = bip->sAccum;
    if (!Point::isNear(spt,saccum.first()->v1->pt))
    {
        addPoint(spt);
    }
    bip->forceRedraw();
}

void Perspective::draw(QPainter * painter)
{

    EdgePoly & saccum = bip->sAccum;
    if (saccum.size() > 0)
    {
        if (!sLastDrag.isNull())
        {
            QColor drag_color = QColor(206,179,102,230);
            painter->setPen(QPen(drag_color,3));
            painter->setBrush(QBrush(drag_color));
            painter->drawLine(saccum.last()->v2->pt,sLastDrag);
            painter->drawEllipse(sLastDrag,10,10);
        }
    }
}

