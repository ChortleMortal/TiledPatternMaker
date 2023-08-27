﻿#pragma once
#ifndef MOTIF_BUTTON
#define MOTIF_BUTTON

#include <QFrame>

typedef std::shared_ptr<class DesignElement>    DesignElementPtr;
typedef std::weak_ptr<class DesignElement>      WeakDesignElementPtr;
typedef std::shared_ptr<class Map>              MapPtr;

////////////////////////////////////////////////////////////////////////////
//
// FeatureButton.java
//
// A tile button is an (optionally clickable) GeoView that displays
// a DesignElement (perhaps DesignElementButton would have been a better
// name).  It draws the underlying tile with the figure overlaid.
// It includes some optimization for drawing radially symmetric figures
// without building the complete map.
//
// These buttons are meant to function like radio buttons -- they live
// in groups, and one is active at any given time.  If a button is active,
// it gets drawn with a red border.
//
// This class is also used to show the large DesignElement being edited
// in the main editing window.  Nice!

class DesignElementButton : public QFrame
{
    Q_OBJECT

public:
    DesignElementButton(int index);
    DesignElementButton(DesignElementPtr designElement, int index);

    void    setDesignElement(DesignElementPtr del);
    DesignElementPtr getDesignElement();

    void    paintEvent(QPaintEvent *event) override;

    QSizeF  getMinimumSize();
    int     getIndex() { return index; }

    void    setSize(QSize d );
    void    setSize( int w, int h );
    void    setViewTransform();

    static QTransform resetViewport(int index, DesignElementPtr designElement, QRect frameRect);
    static QTransform resetViewport(int index, MapPtr map, QRect frameRect);

    void    tally();
    void    setTallyString(QString str) { tallyStr = str; }

    void    setSelection(bool selection) { selected = selection; }
    bool    isSelected()  { return selected; }

    void    setDelegation(bool delegation) { delegated = delegation; }
    bool    isDelegated() { return delegated; }

protected:
    void   construct(DesignElementPtr del, int index);
    static QTransform lookAt(int index, QRectF rect, QRect frameRect);
    static QTransform centerInside(int index, QRectF first, QRectF other);

private:
    WeakDesignElementPtr designElement;

    int              index;
    bool             selected;      // in multi more than one can be selected
    bool             delegated;     // only one has the current delgation to match the editor
    QTransform       transform;
    static QColor    tileIinterior;
    static QColor    tileBorder;
    QString          tallyStr;

};

#endif
