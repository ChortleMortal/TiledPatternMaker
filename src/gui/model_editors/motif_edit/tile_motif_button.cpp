#include <QDebug>

#include "gui/model_editors/motif_edit/tile_motif_button.h"
#include "gui/viewers/geo_graphics.h"
#include "gui/viewers/viewer_services.h"
#include "model/makers/motif_maker.h"
#include "model/motifs/motif.h"
#include "model/prototypes/tile_motif.h"
#include "model/tilings/tile.h"         // required
#include "sys/geometry/map.h"           // required
#include "sys/sys/debugflags.h"

////////////////////////////////////////////////////////////////////////////
//
// FeatureButton.java
//
// A tile motif button is an (optionally clickable) GeoView that displays
// a TileMotif.
// It draws the underlying tile with the figure overlaid.
// It includes some optimization for drawing radially symmetric figures
// without building the complete map.
//
// These buttons are meant to function like radio buttons -- they live
// in groups, and one is active at any given time.  If a button is active,
// it gets drawn with a red border.
//
// This class is also used to show the large TileMotif being edited
// in the main editing window.  Nice!

QColor TileMotifButton::tileBorder    = QColor( 140, 140, 160 );
QColor TileMotifButton::tileIinterior = QColor( 240, 240, 255 );

TileMotifButton::TileMotifButton(int index, TileMotifPtr tileMotif, QTransform t)
{
    this->tileMotif     = tileMotif;
    this->index         = index;
    placement           = t;

    setStyleSheet("background-color: white;");
    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(1);
}

QSizeF TileMotifButton::getMinimumSize()
{
    return QSizeF(40, 40);
}

TileMotifPtr TileMotifButton::getTileMotif()
{
    return tileMotif.lock();
}

void TileMotifButton::setTileMotif(TileMotifPtr tm)
{
    if (tm == tileMotif.lock())
        return;

    tileMotif = tm;
    if (tm)
    {
        qDebug().noquote() << "TileMotifButton::setTileMotif() index:" << index << "tileMotif:" << tm.get();
        //del->describe();
    }
    else
    {
        qDebug().noquote() << "TileMotifButton::setTileMotif() index:" << index << "tileMotif: 0";
    }

    update();
}

QTransform TileMotifButton::resetViewport(TileMotifPtr tileMotif, QRect rect)
{
    // Reset the viewport to frame the tile motif.
    if (!tileMotif)
        return QTransform();
    return lookAt(tileMotif->boundingRect(),rect);
}

QTransform TileMotifButton::lookAt(QRectF rect, QRect frameRect)
{
    // Leave some breathing room around the rectangle to look at.
    // Having the region of interest bleed right to edge of the
    // window doesn't look too good.

    //qDebug() << "lookAt:" << index << rect;

    QSizeF d         = rect.size();
    d *= 1.20;
    QPointF center   = rect.center();
    QRectF paintRect = QRectF(center.x()-(d.width()/2), center.y()-(d.height()/2), d.width(), d.height());

    QTransform t;
    if ( d.width() > 0 && d.height() > 0 )
    {
        t = centerInside(paintRect,frameRect);
    }
    return t;
}

QTransform TileMotifButton::centerInside(QRectF first, QRectF other)
{
/*
 * A useful routine when doing things like printing to
 * postscript.  Given two rectangles, find a good transform
 * for centering the first one inside the second one.
 */
    double xscale = other.width() / first.width();
    double yscale = other.height() / first.height();

    double scale = std::min(xscale, yscale);

    QTransform Ts = QTransform().scale(scale, scale);

    QPointF my_center   = first.center();
    QPointF your_center = other.center();

    QTransform your = QTransform::fromTranslate(your_center.x(),your_center.y());
    QTransform my   = QTransform::fromTranslate(-my_center.x(),-my_center.y());
    QTransform res  = my   * Ts * your;
    //qDebug() << "res" << Transform::toInfoString(res);
    return res;
}

void TileMotifButton::tally()
{
    auto timo = tileMotif.lock();
    MotifViewData & mvd = Sys::motifMaker->motifData;
    if (mvd.isVisible(timo) && mvd.isDelegated(timo))
        setStyleSheet("background-color: rgb(255,220,220); border: 2px solid red;");
    else if (mvd.isVisible(timo))
        setStyleSheet("background-color: rgb(255,220,220);");
    else
        setStyleSheet("background-color: white;");
}

void TileMotifButton::paintEvent(QPaintEvent * event)
{
    auto timo = tileMotif.lock();
    if (!timo)
    {
        QFrame::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

#ifdef INVERT_VIEW
    // invert
    painter.translate(0,height());
    painter.scale(1.0, -1.0);
#endif

    btnTransform = resetViewport(tileMotif.lock(),frameRect());
    if (index < 0)
    {
        btnTransform *= placement;
    }

    GeoGraphics gg(&painter, btnTransform);

    ViewerServices::drawTile(&gg,timo->getTile(),QBrush(tileIinterior),QPen(tileBorder,3));

    ViewerServices::drawMotif(&gg,timo->getMotif(),QPen(Qt::black,3));

    if (Sys::flags->flagged(MOTIF_FLAG_3))
    {
        MotifViewData & mvd = Sys::motifMaker->motifData;
        QString str;
        if (mvd.isDelegated(timo))
            str += "Dlg ";
        if (mvd.isVisible(timo))
            str += "Vis";
        else
            str += "Hid";

        painter.drawText(5,15,str);
        painter.drawText(5,35,tallyStr);
    }

    QFrame::paintEvent(event);
}
