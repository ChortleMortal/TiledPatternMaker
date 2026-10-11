#pragma once

#include <QFrame>

typedef std::shared_ptr<class TileMotif>    TileMotifPtr;
typedef std::weak_ptr<class TileMotif>      WeakTileMotifPtr;
typedef std::shared_ptr<class Map>          MapPtr;

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

class TileMotifButton : public QFrame
{
    Q_OBJECT

public:
    TileMotifButton(int index, TileMotifPtr tileMotif = nullptr, QTransform t = QTransform());

    void         setTileMotif(TileMotifPtr tm);
    TileMotifPtr getTileMotif();

    void    paintEvent(QPaintEvent *event) override;

    QSizeF  getMinimumSize();
    int     getIndex() { return index; }

    static QTransform resetViewport(TileMotifPtr tileMotif, QRect rect);

    void    tally();
    void    setTallyString(QString str)     { tallyStr = str; }

protected:
    static QTransform lookAt(QRectF rect, QRect frameRect);
    static QTransform centerInside(QRectF first, QRectF other);

private:
    WeakTileMotifPtr tileMotif;

    int              index;

    QTransform       btnTransform;
    QTransform       placement;

    static QColor    tileIinterior;
    static QColor    tileBorder;
    QString          tallyStr;

};
