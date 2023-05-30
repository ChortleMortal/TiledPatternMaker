#include <QImage>
#include <QFileDialog>
#include <QPainter>

#include "viewers/backgroundimageview.h"
#include "geometry/edge.h"
#include "geometry/point.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "makers/tiling_maker/tiling_mouseactions.h"
#include "settings/configuration.h"
#include "viewers/viewcontrol.h"

using std::make_shared;

BackgroundImageView * BackgroundImageView::mpThis = nullptr;

BackgroundImageView * BackgroundImageView::getInstance()
{
    if (!mpThis)
    {
        mpThis = new BackgroundImageView();
    }
    return mpThis;
}

void BackgroundImageView::releaseInstance()
{
    if (mpThis)
    {
        delete mpThis;
        mpThis = nullptr;
    }
}

BackgroundImageView::BackgroundImageView() : LayerController("Bkgd Image")
{
    view     = ViewControl::getInstance();
    config   = Configuration::getInstance();
    bkgdName = "Bkgd Image";

    frameData = view->frameSettings.getFrameData(VIEW_BKGD_IMG);

    setZValue(-20);

    _loaded     = false;
}

BackgroundImageView::~BackgroundImageView()
{
}

QString BackgroundImageView::getName()
{
    return bkgdName;
}

void BackgroundImageView::paint(QPainter *painter)
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
        drawPerspective(painter);
    }
}

bool BackgroundImageView::import(QString filename)
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

bool BackgroundImageView::load(QString imageName)
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

void BackgroundImageView::unload()
{
    _loaded         = false;
    bUseAdjusted    = false;
    resetPerspective();
    bkgdName = "Bkgd Image";

    Xform xf;
    setCanvasXform(xf);
    forceLayerRecalc(false);
}

void  BackgroundImageView::setUseAdjusted(bool use)
{
    bUseAdjusted = use;
    qDebug() << "useAdjusted" << bUseAdjusted;
}

void BackgroundImageView::showPixmap()
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
void BackgroundImageView::createBackgroundAdjustment(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft)
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

void BackgroundImageView::createAdjustedImage()
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

bool BackgroundImageView::saveAdjusted(QString newName)
{
    QString file = config->rootMediaDir + "bkgd_photos/" +  newName;
    qDebug() << "Saving adjusted:" << file;
    bool rv = adjustedImage.save(file);
    return rv;
}

void BackgroundImageView::correctPerspective(QPointF topLeft, QPointF topRight, QPointF botRight, QPointF botLeft)
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


const Xform  & BackgroundImageView::getCanvasXform()
{
    return xf_layer;
}

void BackgroundImageView::setCanvasXform(const Xform & xf)
{
    xf_layer = xf;
    forceLayerRecalc();
}

void BackgroundImageView::slot_setCenter(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    if (    view->getKbdMode(KBD_MODE_XFORM_VIEW)
        ||  view->getKbdMode(KBD_MODE_XFORM_BKGD)
        || (view->getKbdMode(KBD_MODE_XFORM_SELECTED) && isSelected()))
    {
        setCenterScreenUnits(spt);
    }
}

void BackgroundImageView::slot_mousePressed(QPointF spt, enum Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    if (!view->isActiveLayer(this)) return;

    startDragging(spt);
}

void BackgroundImageView::slot_mouseDragged(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    updateDragging(spt);
}

void BackgroundImageView::slot_mouseTranslate(QPointF pt)
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

void BackgroundImageView::slot_mouseMoved(QPointF spt)
{ Q_UNUSED(spt); }

void BackgroundImageView::slot_mouseReleased(QPointF spt)
{
    if (!view->isActiveLayer(this)) return;

    endDragging(spt);
}

void BackgroundImageView::slot_mouseDoublePressed(QPointF spt)
{ Q_UNUSED(spt); }


void BackgroundImageView::slot_wheel_scale(qreal delta)
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

void BackgroundImageView::slot_wheel_rotate(qreal delta)
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

void BackgroundImageView::slot_scale(int amount)
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

void BackgroundImageView::slot_rotate(int amount)
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

void BackgroundImageView::slot_moveX(int amount)
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

void BackgroundImageView::slot_moveY(int amount)
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


/////////
///
///  Perspective
///
/////////

Perspective::Perspective()
{
    skewMode = false;
}

void Perspective::resetPerspective()
{
    skewMode = false;
    sAccum.clear();
}

void Perspective::startDragging(QPointF spos)
{
    if (!skewMode)
        return;

    if (sAccum.size() == 0)
    {
        addPoint(spos);
    }
}

void Perspective::addPoint(QPointF spos)
{
    if (!skewMode)
        return;

    qDebug("Perspective::addPoint");

    VertexPtr vnew = make_shared<Vertex>(spos);

    int size = sAccum.size();
    if (size == 0)
    {
        sAccum.push_back(make_shared<Edge>(vnew));
        qDebug() << "point count = 1";
    }
    else if (size == 1)
    {
        EdgePtr last = sAccum.last();
        if (last->getType() == EDGETYPE_POINT)
        {
            last->setV2(vnew);
            qDebug() << "edge count =" << sAccum.size();
        }
        else
        {
            sAccum.push_back(make_shared<Edge>(last->v2,vnew));
            qDebug() << "edge count =" << sAccum.size();
        }
    }
    else if (size == 2)
    {
        EdgePtr last = sAccum.last();
        sAccum.push_back(make_shared<Edge>(last->v2,vnew));
        qDebug() << "edge count = " << sAccum.size();
        sAccum.push_back(make_shared<Edge>(vnew,sAccum.first()->v1));
        qDebug() << "completed with edge count" << sAccum.size();
        forceRedraw();
    }
    sLastDrag = QPointF();
}

void Perspective::updateDragging(QPointF spt)
{
    if (!skewMode)
        return;

    sLastDrag = spt;
    forceRedraw();
}

void Perspective::endDragging(QPointF spt )
{
    if (!skewMode)
        return;

    if (!Point::isNear(spt,sAccum.first()->v1->pt))
    {
        addPoint(spt);
    }
    sLastDrag = QPointF();
    forceRedraw();
}

void Perspective::drawPerspective(QPainter * painter)
{
    if (sAccum.size() > 0 && !sLastDrag.isNull())
    {
        // draws line while dragginhg
        QColor drag_color = QColor(206,179,102,230);
        painter->setPen(QPen(drag_color,3));
        painter->setBrush(QBrush(drag_color));
        painter->drawLine(sAccum.last()->v2->pt,sLastDrag);
        painter->drawEllipse(sLastDrag,10,10);
    }
}

void Perspective::forceRedraw()
{
    ViewControl::getInstance()->update();
}

void  Perspective::setSkewMode(bool enb)
{
    skewMode = enb;
    if (enb)
    {
        sAccum.clear();
        sLastDrag = QPointF();
    }
    forceRedraw();
}
