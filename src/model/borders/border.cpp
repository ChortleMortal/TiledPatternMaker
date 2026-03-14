#include <QPainter>

#include "gui/model_editors/border_edit/mouse_edit_border.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view.h"
#include "gui/viewers/crop_maker_view.h"
#include "gui/viewers/geo_graphics.h"
#include "model/borders/border.h"

/*
 * A border (defined in model units) is both a a layer, and a crop
 * The crop defines its dimensions and type
 * (Earlier versions used screen units, and need conversion to model units)
 * A border is style, but has no prototype map
 *
 * A border is locked either to the view size or to the mosaic content
 *     both cases uses the transform from first style.
 *     but when locked to view size, content is (or shoulde be) adjusted as the view size changes
 */

Border::Border(Mosaic * parent) : Style (PRIMARY,"Border")
{
    this->parent          = parent;
    borderType            = BORDER_NONE;
    styled                = false;
    _cropType             = CROP_UNDEFINED;
    _requiresConversion   = false;
    debugMouse            = false;
    setUseViewSize(false);

    setZLevel(BORDER_ZLEVEL);
    setClipable(false);

    connect(Sys::sysview, &SystemView::sig_viewSizeChanged, this, &Border::viewResized);
    connect(Sys::sysview, &SystemView::sig_viewMoved,       this, &Border::viewMoved);
}

Border::Border(Mosaic *parent, const Crop & crop) : Style(PRIMARY, "Border"), Crop(crop)
{
    this->parent          = parent;
    borderType            = BORDER_NONE;
    styled                = false;
    _cropType             = CROP_UNDEFINED;
    _requiresConversion   = false;
    debugMouse            = false;
    setUseViewSize(false);

    setZLevel(BORDER_ZLEVEL);
    setClipable(false);

    connect(Sys::sysview, &SystemView::sig_viewSizeChanged, this, &Border::viewResized);
    connect(Sys::sysview, &SystemView::sig_viewMoved,       this, &Border::viewMoved);
}

void Border::setModelXform(const Xform & xf, bool update, uint sigid)
{
    Layer::setModelXform(xf,update,sigid);

    if (getUseViewSize())
    {
        resetStyleRepresentation();
    }
}

void Border::paint(QPainter *painter)
{
    Q_ASSERT(getRequiresConversion() == false);

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QTransform tr = getLayerTransform();

    GeoGraphics gg(painter,tr);

    painter->save();
    draw(&gg);
    painter->restore();

    if (getMouseInteraction())
    {
        painter->save();
        getMouseInteraction()->draw(painter,mousePos);
        painter->restore();
    }
}

void Border::viewMoved()
{
    if (getUseViewSize())
    {
        resetStyleRepresentation();
    }
}

void Border::viewResized(QSize oldSize, QSize newSize)
{
    qDebug() << "Border::viewResized" << newSize;
    Q_UNUSED(oldSize);
    Q_UNUSED(newSize);
    viewMoved();
}

void Border::setUseViewSize(bool use)
{
    _useViewSize = use;
    if (use)
    {
        resetStyleRepresentation();
    }
}

void Border::setBorderSize(QSize viewSize)
{
    qDebug() << "Border::setBorderSize" << viewSize;

    switch(_cropType)
    {
    case CROP_RECTANGLE:
    {
        QRectF arect((QPointF()),QSizeF(viewSize));
        QRectF brect = screenToModel(arect);
        setRect(brect);
    }   break;

    case CROP_CIRCLE:
        break;

    case CROP_POLYGON:

    case CROP_UNDEFINED:
        break;
    }
}

void Border::convertCropToModelUnits()
{
    switch (_cropType)
    {
    case CROP_RECTANGLE:
    {
        QRectF rect = getRect();
        rect = screenToModel(rect);
        setRect(rect);
    }   break;

    case CROP_CIRCLE:
    {
        Circle c = getCircle();
        c = screenToModel(c);
        setCircle(c);
    }   break;

    case CROP_POLYGON:
    {
        QPolygonF poly = screenToModel(getAPolygon().get());
        setPolygon(poly);
    }   break;

    case CROP_UNDEFINED:
        break;
    }
}

void Border::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    Q_UNUSED(btn);

    if (!viewControl()->isEnabled(VIEW_MOSAIC))
        return;
    if (Sys::cropMakerView->getShowCrop(CM_MOSAIC))
        return;
    if (!Sys::controlPanel->isVisiblePage(PAGE_BORDER_MAKER))
        return;

    setMousePos(spt);

    auto c = dynamic_cast<Crop *>(this);
    auto mec = std::make_shared<MouseEditBorder>(this,mousePos,c);
    setMouseInteraction(mec);
    mec->updateDragging(mousePos);
}

void Border::slot_mouseDragged(QPointF spt)
{
    if (!viewControl()->isEnabled(VIEW_MOSAIC))
        return;

    if (Sys::cropMakerView->getShowCrop(CM_MOSAIC))
        return;

    setMousePos(spt);

    if (debugMouse) qDebug().noquote() << "drag" << mousePos << screenToModel(mousePos);
    BorderMouseActionPtr mec = getMouseInteraction();
    if (mec)
    {
        mec->updateDragging(mousePos);
    }

    forceRedraw();
}

void Border::slot_mouseMoved(QPointF spt)
{
    if (!viewControl()->isEnabled(VIEW_MOSAIC))
        return;
    if (Sys::cropMakerView->getShowCrop(CM_MOSAIC))
        return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "move" << mousePos;
}

void Border::slot_mouseReleased(QPointF spt)
{
    if (!viewControl()->isEnabled(VIEW_MOSAIC))
        return;
    if (Sys::cropMakerView->getShowCrop(CM_MOSAIC))
        return;

    setMousePos(spt);

    if (debugMouse) qDebug() << "release" << mousePos << screenToModel(mousePos);

    BorderMouseActionPtr mma = getMouseInteraction();
    if (mma)
    {
        mma->endDragging(mousePos);
        resetMouseInteraction();
        resetStyleRepresentation();
        forceRedraw();
    }
}

void Border::setMousePos(QPointF pt)
{
    Qt::KeyboardModifiers km = QApplication::keyboardModifiers();
    if (km & Qt::ControlModifier)
    {
        mousePos.setY(pt.y());
    }
    else if (km & Qt::ShiftModifier)
    {
        mousePos.setX(pt.x());
    }
    else
    {
        mousePos = pt;
    }
}

